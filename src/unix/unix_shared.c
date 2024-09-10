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

#include <dlfcn.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <pwd.h>
#if defined(__linux__)
#include <sys/sysinfo.h>
#elif defined(__FreeBSD__)
#include <sys/user.h>
#include <sys/sysctl.h>
#endif
#include <inttypes.h>
#ifdef DEDICATED
#include <sys/wait.h>
#endif

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

#include "linux_local.h"

//=============================================================================

// Used to determine CD Path
static char cdPath[MAX_OSPATH];

// Used to determine local installation path
static char installPath[MAX_OSPATH];

// Used to determine where to store user-specific files
static char homePath[MAX_OSPATH];

static void LIN_MicroSleep(int us)
{
	struct timespec req, rem;
	req.tv_sec = us / 1000000;
	req.tv_nsec = (us % 1000000) * 1000;
	while (clock_nanosleep(CLOCK_REALTIME, 0, &req, &rem) == EINTR) {
		req = rem;
	}
}


void Sys_Sleep(int ms)
{
	LIN_MicroSleep(ms * 1000);
}


/*
================
Sys_Milliseconds
================
*/
/* base time in seconds, that's our origin
   timeval:tv_sec is an int:
   assuming this wraps every 0x7fffffff - ~68 years since the Epoch (1970) - we're safe till 2038
   using unsigned long data type to work right with Sys_XTimeToSysTime */
unsigned long sys_timeBase = 0;
/* current time in ms, using sys_timeBase as origin
   NOTE: sys_timeBase*1000 + curtime -> ms since the Epoch
	 0x7fffffff ms - ~24 days
   although timeval:tv_usec is an int, I'm not sure wether it is actually used as an unsigned int
	 (which would affect the wrap period) */
int curtime;
int Sys_Milliseconds( void ) {
	struct timeval tp;

	gettimeofday( &tp, NULL );

	if ( !sys_timeBase ) {
		sys_timeBase = tp.tv_sec;
		return tp.tv_usec / 1000;
	}

	curtime = ( tp.tv_sec - sys_timeBase ) * 1000 + tp.tv_usec / 1000;

	return curtime;
}


qbool Sys_HardReboot()
{
#ifdef DEDICATED
	return qtrue;
#else
	return qfalse;
#endif
}


#ifdef DEDICATED


static int Lin_RunProcess(char** argv)
{
	const pid_t pid = fork();
	if (pid == 0) {
		if (execve(argv[0], argv, NULL) == -1) {
			fprintf(stderr, "failed to launch child process: %s\n", strerror(errno));
			_exit(1); // quit without calling atexit handlers
			return 0;
		}
	}

	int status;
	while (waitpid(pid, &status, WNOHANG) == 0)
		sleep(1); // in seconds

	return WEXITSTATUS(status);
}


void Lin_HardRebootHandler(int argc, char** argv)
{
	for (int i = 0; i < argc; ++i) {
		if (!Q_stricmp(argv[i], "nohardreboot")) {
			return;
		}
	}

	static char* args[256];
	if (argc + 2 >= sizeof(args) / sizeof(args[0])) {
		fprintf(stderr, "too many arguments: %d\n", argc);
		_exit(1); // quit without calling atexit handlers
		return;
}

	for (int i = 0; i < argc; ++i)
		args[i] = argv[i];
	args[argc + 0] = (char*)"nohardreboot";
	args[argc + 1] = NULL;

	SIG_InitParent();

	for (;;) {
		if (Lin_RunProcess(args) == 0)
			_exit(0); // quit without calling atexit handlers
	}
}


#endif


static qbool lin_hasParent = qfalse;
static pid_t lix_parentPid;


static const char* Lin_GetExeName(const char* path)
{
	const char* lastSlash = strrchr(path, '/');
	if (lastSlash == NULL)
		return path;

	return lastSlash + 1;
}


void Lin_TrackParentProcess()
{
#if defined(__linux__)

	static char cmdLine[1024];

	char fileName[128];
	Com_sprintf(fileName, sizeof(fileName), "/proc/%d/cmdline", (int)getppid());

	const int fd = open(fileName, O_RDONLY);
	if (fd == -1)
		return;

	const qbool hasCmdLine = read(fd, cmdLine, sizeof(cmdLine)) > 0;
	close(fd);

	if (!hasCmdLine)
		return;

	cmdLine[sizeof(cmdLine) - 1] = '\0';
	lin_hasParent = strcmp(Lin_GetExeName(cmdLine), Lin_GetExeName(q_argv[0])) == 0;

#elif defined(__FreeBSD__)

	static char cmdLine[1024];

	int mib[4];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_ARGS;
	mib[3] = getppid();
	size_t length = sizeof(cmdLine);
	if (sysctl(mib, 4, cmdLine, &length, NULL, 0) != 0)
		return;

	cmdLine[sizeof(cmdLine) - 1] = '\0';
	lin_hasParent = strcmp(Lin_GetExeName(cmdLine), Lin_GetExeName(q_argv[0])) == 0;

#endif
}


qbool Sys_HasRtcwProParent()
{
	return lin_hasParent;
}


static int Sys_GetProcessUptime(pid_t pid)
{
#if defined(__linux__)

	// length must be in sync with the fscanf call!
	static char word[256];

	// The process start time is the 22nd column and
	// encoded as jiffies after system boot.
	const int jiffiesPerSec = sysconf(_SC_CLK_TCK);
	if (jiffiesPerSec <= 0)
		return -1;

	char fileName[128];
	Com_sprintf(fileName, sizeof(fileName), "/proc/%" PRIu64 "/stat", (uint64_t)pid);
	FILE* const file = fopen(fileName, "r");
	if (file == NULL)
		return -1;

	for (int i = 0; i < 21; ++i) {
		if (fscanf(file, "%255s", word) != 1) {
			fclose(file);
			return -1;
		}
	}

	int64_t jiffies;
	const qbool success = fscanf(file, "%" PRId64, &jiffies) == 1;
	fclose(file);

	if (!success)
		return -1;

	const int64_t secondsSinceBoot = jiffies / (int64_t)jiffiesPerSec;
	struct sysinfo info;
	sysinfo(&info);
	const int64_t uptime = (int64_t)info.uptime - secondsSinceBoot;

	return (int)uptime;

#elif defined(__FreeBSD__)

	int mib[4];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PID;
	mib[3] = pid;
	struct kinfo_proc kp;
	size_t len = sizeof(kp);
	if (sysctl(mib, 4, &kp, &len, NULL, 0) != 0) {
		return -1;
	}

	struct timeval now;
	gettimeofday(&now, NULL);

	return (int)(now.tv_sec - kp.ki_start.tv_sec);

#else

	return -1;

#endif
}

int Sys_GetUptimeSeconds(qbool parent)
{
	return Sys_GetProcessUptime(parent ? getppid() : getpid());
}

void Sys_LoadHistory()
{
#ifdef DEDICATED
	History_LoadFromFile(tty_GetHistory());
#else
	//History_LoadFromFile(&g_history);
#endif
}


void Sys_SaveHistory()
{
#ifdef DEDICATED
	History_SaveToFile(tty_GetHistory());
#else
	//History_SaveToFile(&g_history);
#endif
}



//#if 0 // bk001215 - see snapvector.nasm for replacement
#if ( defined __APPLE__ ) // rcg010206 - using this for PPC builds...
long fastftol( float f ) { // bk001213 - from win32/win_shared.c
	//static int tmp;
	//	__asm fld f
	//__asm fistp tmp
	//__asm mov eax, tmp
	return (long)f;
}

void Sys_SnapVector( float *v ) { // bk001213 - see win32/win_shared.c
	// bk001213 - old linux
	v[0] = rint( v[0] );
	v[1] = rint( v[1] );
	v[2] = rint( v[2] );
}
#endif


void    Sys_Mkdir( const char *path ) {
	mkdir( path, 0777 );
}

char *strlwr( char *s ) {
	if ( s == NULL ) { // bk001204 - paranoia
		assert( 0 );
		return s;
	}
	while ( *s ) {
		*s = tolower( *s );
		s++;
	}
	return s; // bk001204 - duh
}

//============================================

#define MAX_FOUND_FILES 0x1000

// bk001129 - new in 1.26
void Sys_ListFilteredFiles( const char *basedir, char *subdirs, char *filter, char **list, int *numfiles ) {
	char search[MAX_OSPATH], newsubdirs[MAX_OSPATH];
	char filename[MAX_OSPATH];
	DIR         *fdir;
	struct dirent *d;
	struct stat st;

	if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
		return;
	}

	if ( strlen( subdirs ) ) {
		Com_sprintf( search, sizeof( search ), "%s/%s", basedir, subdirs );
	} else {
		Com_sprintf( search, sizeof( search ), "%s", basedir );
	}

	if ( ( fdir = opendir( search ) ) == NULL ) {
		return;
	}

	while ( ( d = readdir( fdir ) ) != NULL ) {
		Com_sprintf( filename, sizeof( filename ), "%s/%s", search, d->d_name );
		if ( stat( filename, &st ) == -1 ) {
			continue;
		}

		if ( st.st_mode & S_IFDIR ) {
			if ( Q_stricmp( d->d_name, "." ) && Q_stricmp( d->d_name, ".." ) ) {
				if ( strlen( subdirs ) ) {
					Com_sprintf( newsubdirs, sizeof( newsubdirs ), "%s/%s", subdirs, d->d_name );
				} else {
					Com_sprintf( newsubdirs, sizeof( newsubdirs ), "%s", d->d_name );
				}
				Sys_ListFilteredFiles( basedir, newsubdirs, filter, list, numfiles );
			}
		}
		if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}
		Com_sprintf( filename, sizeof( filename ), "%s/%s", subdirs, d->d_name );
		if ( !Com_FilterPath( filter, filename, qfalse ) ) {
			continue;
		}
		list[ *numfiles ] = CopyString( filename );
		( *numfiles )++;
	}

	closedir( fdir );
}

// bk001129 - in 1.17 this used to be
// char **Sys_ListFiles( const char *directory, const char *extension, int *numfiles, qboolean wantsubs )
char **Sys_ListFiles( const char *directory, const char *extension, char *filter, int *numfiles, qboolean wantsubs ) {
	struct dirent *d;
	// char *p; // bk001204 - unused
	DIR     *fdir;
	qboolean dironly = wantsubs;
	char search[MAX_OSPATH];
	int nfiles;
	char        **listCopy;
	char        *list[MAX_FOUND_FILES];
	//int			flag; // bk001204 - unused
	int i;
	struct stat st;

	int extLen;

	if ( filter ) {

		nfiles = 0;
		Sys_ListFilteredFiles( directory, "", filter, list, &nfiles );

		list[ nfiles ] = 0;
		*numfiles = nfiles;

		if ( !nfiles ) {
			return NULL;
		}

		listCopy = Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) );
		for ( i = 0 ; i < nfiles ; i++ ) {
			listCopy[i] = list[i];
		}
		listCopy[i] = NULL;

		return listCopy;
	}

	if ( !extension ) {
		extension = "";
	}

	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		dironly = qtrue;
	}

	extLen = strlen( extension );

	// search
	nfiles = 0;

	if ( ( fdir = opendir( directory ) ) == NULL ) {
		*numfiles = 0;
		return NULL;
	}

	while ( ( d = readdir( fdir ) ) != NULL ) {
		Com_sprintf( search, sizeof( search ), "%s/%s", directory, d->d_name );
		if ( stat( search, &st ) == -1 ) {
			continue;
		}
		if ( ( dironly && !( st.st_mode & S_IFDIR ) ) ||
			 ( !dironly && ( st.st_mode & S_IFDIR ) ) ) {
			continue;
		}

		if ( *extension ) {
			if ( strlen( d->d_name ) < strlen( extension ) ||
				 Q_stricmp(
					 d->d_name + strlen( d->d_name ) - strlen( extension ),
					 extension ) ) {
				continue; // didn't match
			}
		}

		if ( nfiles == MAX_FOUND_FILES - 1 ) {
			break;
		}
		list[ nfiles ] = CopyString( d->d_name );
		nfiles++;
	}

	list[ nfiles ] = 0;

	closedir( fdir );

	// return a copy of the list
	*numfiles = nfiles;

	if ( !nfiles ) {
		return NULL;
	}

	listCopy = Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) );
	for ( i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	return listCopy;
}

void    Sys_FreeFileList( char **list ) {
	int i;

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}

char *Sys_Cwd( void ) {
	static char cwd[MAX_OSPATH];

	getcwd( cwd, sizeof( cwd ) - 1 );
	cwd[MAX_OSPATH - 1] = 0;

	return cwd;
}

void Sys_SetDefaultCDPath( const char *path ) {
	Q_strncpyz( cdPath, path, sizeof( cdPath ) );
}

char *Sys_DefaultCDPath( void ) {
	return cdPath;
}

/*
==============
Sys_DefaultBasePath
==============
*/
char *Sys_DefaultBasePath( void ) {
	if ( installPath[0] ) {
		return installPath;
	} else {
		return Sys_Cwd();
	}
}

void Sys_SetDefaultInstallPath( const char *path ) {
	Q_strncpyz( installPath, path, sizeof( installPath ) );
}

char *Sys_DefaultInstallPath( void ) {
	if ( *installPath ) {
		return installPath;
	} else {
		return Sys_Cwd();
	}
}

void Sys_SetDefaultHomePath( const char *path ) {
	Q_strncpyz( homePath, path, sizeof( homePath ) );
}

char *Sys_DefaultHomePath( void ) {
	char *p;

	if ( *homePath ) {
		return homePath;
	}

	if ( ( p = getenv( "HOME" ) ) != NULL ) {
		Q_strncpyz( homePath, p, sizeof( homePath ) );
#ifdef MACOS_X
		Q_strcat( homePath, sizeof( homePath ), "/Library/Application Support/WolfensteinMP" );
#else
		Q_strcat( homePath, sizeof( homePath ), "/.wolf" );
#endif
		if ( mkdir( homePath, 0777 ) ) {
			if ( errno != EEXIST ) {
				Sys_Error( "Unable to create directory \"%s\", error is %s(%d)\n", homePath, strerror( errno ), errno );
			}
		}
		return homePath;
	}
	return ""; // assume current dir
}

//============================================

int Sys_GetProcessorId( void ) {
	// TODO TTimo add better CPU identification?
	// see Sys_GetHighQualityCPU
	return CPUID_GENERIC;
}

int Sys_GetHighQualityCPU() {
	// TODO TTimo see win_shared.c IsP3 || IsAthlon
	return 0;
}

void Sys_ShowConsole( int visLevel, qboolean quitOnClose ) {
}

char *Sys_GetCurrentUser( void ) {
	struct passwd *p;

	if ( ( p = getpwuid( getuid() ) ) == NULL ) {
		return "player";
	}
	return p->pw_name;
}

void *Sys_InitializeCriticalSection() {
	return (void *)-1;
}

void Sys_EnterCriticalSection( void *ptr ) {
}

void Sys_LeaveCriticalSection( void *ptr ) {
}
