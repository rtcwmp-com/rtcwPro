/*
===========================================================================

Return to Castle Wolfenstein multiplayer GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein multiplayer GPL Source Code (RTCW MP Source Code).  

RTCW MP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW MP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW MP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW MP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW MP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

//This file is from CNQ3 with minor modifications to use libunwind directly as a static linked library

#define _GNU_SOURCE
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>

// empty struct size being 0 or 1 byte
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextern-c-compat"
#define UNW_LOCAL_ONLY
#include "libunwind.h"
#pragma clang diagnostic pop

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../qcommon/crash.h"
#include "linux_local.h"


/*
A sub-process created by fork from a signal handler will most certainly
run in the context of the signal handler and should therefore also be
limited to calling async-signal-safe functions only.

So the idea of launching a new process from a handler for doing the heavy
lifting (resolving symbols etc) is completely dead.
*/

typedef struct siginfo_s {
	int sig;
	char name[16];
	char desc[128];
} signalinfo_t;

#define num_crash_signals 5
signalinfo_t crash_list[num_crash_signals] =
{
	{SIGILL, "SIGILL", "illegal instruction"},
	{SIGIOT, "SIGIOT",	"IOT trap (a synonym for SIGABRT)"},
	{SIGBUS, "SIGBUS",	"bus error (bad memory access)" },
	{SIGFPE, "SIGFPE",	"fatal arithmetic error" },
	{SIGSEGV, "SIGSEGV",	"invalid memory reference"}
};

#define num_term_signals 5
signalinfo_t term_list[num_term_signals] =
{
	{SIGHUP, "SIGHUP", "hangup detected on controlling terminal or death of controlling process"},
	{SIGQUIT, "SIGQUIT",	"quit from keyboard"},
	{SIGTRAP, "SIGTRAP",	"trace/breakpoint trap" },
	{SIGTERM, "SIGTERM",	"termination signal" },
	{SIGINT, "SIGINT",	"interrupt"}
};



static char sig_backTraceFilePath[MAX_OSPATH];
static char sig_jsonFilePath[MAX_OSPATH];


static void Sig_Unwind_GetContext();
static void Sig_Unwind_Print(FILE* file);
static void Sig_Unwind_PrintASS(int fd); // async-signal-safe


// both of these can call Com_Error, which is not safe
#define Q_strncpyz	Sig_strncpyz
#define Q_strcat	Sig_strcat

// make sure we don't use these
#define strcpy		do_not_use_strcpy
#define strcat		do_not_use_strcat


// async-signal-safe
static void Sig_strncpyz(char* dest, const char* src, int size)
{
	if (!dest || !src || size < 1)
		return;

	strncpy(dest, src, size - 1);
	dest[size - 1] = '\0';
}


// async-signal-safe
static void Sig_strcat(char* dest, int size, const char* src)
{
	if (!dest || !src || size < 1)
		return;

	const int destLength = strlen(dest);
	if (destLength >= size)
		return;

	Sig_strncpyz(dest + destLength, src, size - destLength);
}


static const char* Sig_GetDescription(int sig)
{
	for (int i = 0; i < num_crash_signals; i++) {
		if (crash_list[i].sig == sig) {
			return crash_list[i].desc;
		}
	}
	for (int i = 0; i < num_term_signals; i++) {
		if (term_list[i].sig == sig) {
			return term_list[i].desc;
		}
	}
	return "unhandled signal";
}


static const char* Sig_GetName(int sig)
{
	for (int i = 0; i < num_crash_signals; i++) {
		if (crash_list[i].sig == sig) {
			return crash_list[i].name;
		}
	}
	for (int i = 0; i < num_term_signals; i++) {
		if (term_list[i].sig == sig) {
			return term_list[i].name;
		}
	} 
	return "unhandled signal";
}


static void Sig_UpdateFilePaths()
{
	// gmtime and va (sprintf) are not async-signal-safe

	const time_t epochTime = time(NULL);
	struct tm* const utcTime = gmtime(&epochTime);
	const int y = 1900 + utcTime->tm_year;
	const int m = utcTime->tm_mon + 1;
	const int d = utcTime->tm_mday;
	const int h = utcTime->tm_hour;
	const int mi = utcTime->tm_min;
	const int s = utcTime->tm_sec;

	const char* const bn = va(
		"%s-crash-%d.%02d.%02d-%02d.%02d.%02d",
		q_argv[0], y, m, d, h, mi, s);
	char baseName[MAX_OSPATH];
	Q_strncpyz(baseName, bn, sizeof(baseName));
	Q_strncpyz(sig_backTraceFilePath, va("%s.bt", baseName), sizeof(sig_backTraceFilePath));
	Q_strncpyz(sig_jsonFilePath, va("%s.json", baseName), sizeof(sig_jsonFilePath));
}


static const char* Sig_BackTraceFilePath()
{
	if (sig_backTraceFilePath[0] == '\0')
		return "rtcwpro-crash.bt";

	return sig_backTraceFilePath;
}


static const char* Sig_JSONFilePath()
{
	if (sig_jsonFilePath[0] == '\0')
		return "rtcwpro-crash.json";

	return sig_jsonFilePath;
}


static void Sig_WriteJSON(int sig)
{
	FILE* const file = fopen(Sig_JSONFilePath(), "w");
	if (file == NULL)
		return;

	JSONW_BeginFile(file);
	JSONW_IntegerValue("signal", sig);
	JSONW_StringValue("signal_name", Sig_GetName(sig));
	JSONW_StringValue("signal_description", Sig_GetDescription(sig));
	JSONW_EndFile();
	fclose(file);
}


static void Sig_Backtrace_Print(FILE* file)
{
	void* addresses[64];
	const int addresscount = backtrace(addresses, sizeof(addresses));
	if (addresscount > 0)
	{
		fflush(file);
		backtrace_symbols_fd(addresses, addresscount, fileno(file));
	}
	else
	{
		fprintf(file, "The call to backtrace failed\r\n");
	}
}


static void libbt_ErrorCallback(void* data, const char* msg, int errnum)
{
	fprintf((FILE*)data, "libbacktrace error: %s (%d)\r\n", msg, errnum);
}


static void Sig_Print(int fd, const char* msg)
{
	write(fd, msg, strlen(msg));
}


static void Sig_PrintLine(int fd, const char* msg)
{
	write(fd, msg, strlen(msg));
	write(fd, "\r\n", 2);
}


static void Sig_PrintSignalCaught(int sig)
{
	// fprintf is not async-signal-safe, but write is
	Sig_Print(STDERR_FILENO, "\nSignal caught: ");
	Sig_Print(STDERR_FILENO, Sig_GetName(sig));
	Sig_Print(STDERR_FILENO, " (");
	Sig_Print(STDERR_FILENO, Sig_GetDescription(sig));
	Sig_Print(STDERR_FILENO, ")\r\n");
}


static void Sig_HandleExit()
{
	Lin_ConsoleInputShutdown();
}


static volatile sig_atomic_t sig_termRequested = 0;


static void Sig_HandleTermSignal(int sig)
{
	Sig_PrintSignalCaught(sig);
	sig_termRequested = 1;
}


static void Sig_PrintAttempt(const char* what)
{
	Sig_Print(STDERR_FILENO, "Attempting to ");
	Sig_Print(STDERR_FILENO, what);
	Sig_Print(STDERR_FILENO, "...");
}


static void Sig_PrintDone()
{
	Sig_Print(STDERR_FILENO, " done\r\n");
}


static void Sig_HandleCrash(int sig)
{
	//
	// Phase 1: async-signal-safe code
	//

	Sig_Unwind_GetContext();

	const int fd = open(Sig_BackTraceFilePath(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
	if (fd != -1)
	{
		Sig_PrintAttempt("write safe libunwind stack trace");
		Sig_PrintLine(fd, "safe libunwind stack trace:");
		Sig_Unwind_PrintASS(fd);
		Sig_PrintDone();


		// user R+W - group R - others R
		chmod(Sig_BackTraceFilePath(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	}

	//
	// Phase 2: buckle up
	//

	Sig_PrintAttempt("restore tty settings");
	Lin_ConsoleInputShutdown();
	Sig_PrintDone();

	Sig_PrintAttempt("write JSON report");
	Sig_WriteJSON(sig);
	Sig_PrintDone();

	if (fd != -1)
	{
		FILE* const file = fdopen(fd, "a");

		Sig_PrintAttempt("write backtrace stack trace");
		fprintf(file, "\r\n\r\nbacktrace_symbols_fd stack trace:\r\n");
		Sig_Backtrace_Print(file);
		fflush(file);
		Sig_PrintDone();

		Sig_PrintAttempt("write detailed libunwind stack trace");
		fprintf(file, "\r\n\r\ndetailed libunwind stack trace:\r\n");
		Sig_Unwind_Print(file);
		fflush(file);
		Sig_PrintDone();
	}
}


static void Sig_HandleCrashSignal(int sig)
{
	static volatile sig_atomic_t counter = 0;

	Sig_PrintSignalCaught(sig);

	++counter;

	if (counter == 1)
		Sig_HandleCrash(sig);
	else if (counter >= 2)
		Sig_PrintLine(STDERR_FILENO, "Double fault! Shutting down immediately.");

	_exit(1);
}


static void Sig_RegisterSignals(signalinfo_t* signals, int count, void (*handler)(int), int flags)
{
	sigset_t mask;
	sigemptyset(&mask);

	struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_handler = handler;
	action.sa_mask = mask;
	action.sa_flags = flags;
	for (int i = 0; i < count; ++i)
		sigaction(signals[i].sig, &action, NULL);
}


void SIG_InitChild()
{
	// This is unfortunately needed because some code might
	// call exit and bypass all the clean-up work without
	// there ever being a real crash.
	// This happens for instance with the "fatal IO error"
	// of the X server.
	atexit(Sig_HandleExit);

	// The crash handler gets SA_NODEFER so that we can handle faults it generates.
	Sig_RegisterSignals(crash_list, num_crash_signals, Sig_HandleCrashSignal, SA_NODEFER);
	Sig_RegisterSignals(term_list, num_term_signals, Sig_HandleTermSignal, 0);

	// Must do this now because it's not safe in a signal handler.
	Sig_UpdateFilePaths();
}


void SIG_InitParent()
{
	// Signals are sent to both the parent and the child, which can be an issue.
	// Our parent process has to ignore all signals handled by the child and
	// let the child do whatever needs to be done.

	for (int i = 0; i < num_crash_signals; ++i)
		signal(crash_list[i].sig, SIG_IGN);

	for (int i = 0; i < num_term_signals; ++i)
		signal(term_list[i].sig, SIG_IGN);
}


void SIG_Frame()
{
	// Must do this now because it's not safe in a signal handler.
	Sig_UpdateFilePaths();

	if (sig_termRequested != 0)
		Com_Quit(0);
}



static unw_context_t context;


static void Sig_Unwind_GetContext()
{
	if (unw_getcontext(&context) != 0)
	{
		Sig_PrintLine(STDERR_FILENO, "The call to libunwind's getcontext failed");
	}
}




static int Sig_Unwind_GetFileAndLine(unw_word_t addr, char* file, size_t flen, int* line)
{
	static char buf[1024];

	Dl_info info;
	dladdr((unsigned long)addr, &info);

	sprintf(buf, "addr2line -C -e %s -f -i %lx", q_argv[0], (unsigned long)addr - (unsigned long)info.dli_fbase);
	FILE* f = popen(buf, "r");
	if (f == NULL)
		return 0;

	fgets(buf, sizeof(buf), f); // function name
	fgets(buf, sizeof(buf), f); // file and line
	pclose(f);

	if (buf[0] == '?')
		return 0;

	// file name before ':'
	char* p = buf;
	while (*p != ':')
		++p;
	*p = '\0';

	// skip the folder names
	char* name = strrchr(buf, '/');
	name = name ? (name + 1) : buf;

	// line number
	++p;
	Q_strncpyz(file, name, flen);
	sscanf(p, "%d", line);

	return 1;
}


void Sig_Unwind_Print(FILE* fp)
{
	static char name[1024];
	static char file[1024];

	unw_cursor_t cursor;
	if (unw_init_local(&cursor, &context) != 0)
	{
		fprintf(fp, "The call to libunwind's init_local failed\r\n");
		return;
	}

	while (unw_step(&cursor) > 0)
	{
		const char* func = name;
		unw_word_t ip, sp, offp;
		if (unw_get_proc_name(&cursor, name, sizeof(name), &offp))
		{
			Q_strncpyz(name, "???", sizeof(name));
		}

		int line = 0;
		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		if (Sig_Unwind_GetFileAndLine((long)ip, file, sizeof(file), &line))
			fprintf(fp, "%s at %s:%d\n", name, file, line);
		else
			fprintf(fp, "%s\n", name);
	}
}


// async-signal-safe
void Sig_Unwind_PrintASS(int fd)
{
	static char name[1024];

	unw_cursor_t cursor;
	if (unw_init_local(&cursor, &context) != 0)
		return;

	while (unw_step(&cursor) > 0)
	{
		unw_word_t offp;
		if (unw_get_proc_name(&cursor, name, sizeof(name), &offp))
			Q_strncpyz(name, "???", sizeof(name));

		Sig_PrintLine(fd, name);
	}
}
