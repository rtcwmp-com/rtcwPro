/*
===========================================================================
Parts taken from CNQ3:
Copyright (C) 2017-2019 Gian 'myT' Schellenbaum

This file is part of RtcwPro.

RtcwPro is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

RtcwPro is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RtcwPro. If not, see <https://www.gnu.org/licenses/>.
===========================================================================
*/


#include "../qcommon/q_shared.h"
#include "../client/snd_local.h"

#include <SDL2/SDL.h>

static const int bits = 16;
static const int channels = 2;
static const int samples = 512;
static const SDL_AudioFormat format = AUDIO_S16SYS;


void Snd_Memset( void* dest, const int val, const size_t count ) {
	Com_Memset( dest,val,count );
	return;
}

typedef struct audio_s {
	qbool valid;
	int q3SamplePos;
	int q3Bytes;
	SDL_AudioDeviceID device;
} audio_t;

static audio_t audio;

static void FillAudioBufferCallback( void* userData, Uint8* sdlBuffer, int sdlBytesToWrite )
{
	if (sdlBuffer == NULL || sdlBytesToWrite == 0)
		return;

	if (!audio.valid) {
		memset(sdlBuffer, 0, sdlBytesToWrite);
		return;
	}

	// fix up sample offset if needed
	const int bytesPerSample = dma.samplebits / 8;
	int q3BytePos = audio.q3SamplePos * bytesPerSample;
	if (q3BytePos >= audio.q3Bytes) {
		q3BytePos = 0;
		audio.q3SamplePos = 0;
	}

	// compute the sizes for the memcpy call(s)
	int q3BytesToEnd = audio.q3Bytes - q3BytePos;
	int bytes1 = sdlBytesToWrite;
	int bytes2 = 0;
	if (bytes1 > q3BytesToEnd) {
		bytes1 = q3BytesToEnd;
		bytes2 = sdlBytesToWrite - q3BytesToEnd;
	}

	// copy the new mixed data to the device
	memcpy(sdlBuffer, dma.buffer + q3BytePos, bytes1);
	if (bytes2 > 0) {
		memcpy(sdlBuffer + bytes1, dma.buffer, bytes2);
		audio.q3SamplePos = bytes2 / bytesPerSample;
	} else {
		audio.q3SamplePos += bytes1 / bytesPerSample;
	}

	// fix up sample offset if needed
	//if (audio.q3SamplePos * bytesPerSample >= audio.q3Bytes)
	if (audio.q3SamplePos >= audio.q3Bytes)
		audio.q3SamplePos = 0;
}

static struct
{
	Uint16	enumFormat;
	char		*stringFormat;
} formatToStringTable[ ] =
{
	{ AUDIO_U8,     "AUDIO_U8" },
	{ AUDIO_S8,     "AUDIO_S8" },
	{ AUDIO_U16LSB, "AUDIO_U16LSB" },
	{ AUDIO_S16LSB, "AUDIO_S16LSB" },
	{ AUDIO_U16MSB, "AUDIO_U16MSB" },
	{ AUDIO_S16MSB, "AUDIO_S16MSB" },
	{ AUDIO_F32LSB, "AUDIO_F32LSB" },
	{ AUDIO_F32MSB, "AUDIO_F32MSB" }
};

static int formatToStringTableSize = ARRAY_LEN( formatToStringTable );

/*
===============
SNDDMA_PrintAudiospec
===============
*/
static void SNDDMA_PrintAudiospec(const char *str, const SDL_AudioSpec *spec)
{
	int		i;
	char	*fmt = NULL;

	Com_Printf("%s:\n", str);

	for( i = 0; i < formatToStringTableSize; i++ ) {
		if( spec->format == formatToStringTable[ i ].enumFormat ) {
			fmt = formatToStringTable[ i ].stringFormat;
		}
	}

	if( fmt ) {
		Com_Printf( "  Format:   %s\n", fmt );
	} else {
		Com_Printf( "  Format:   " S_COLOR_RED "UNKNOWN\n");
	}

	Com_Printf( "  Freq:     %d\n", (int) spec->freq );
	Com_Printf( "  Samples:  %d\n", (int) spec->samples );
	Com_Printf( "  Channels: %d\n", (int) spec->channels );
}

qboolean SNDDMA_Init( void ) {

	if (audio.valid)
		return qtrue;

	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		Com_Printf("SDL_Init failed: %s\n", SDL_GetError());
		return qfalse;
	}

		// open the default audio device
	SDL_AudioSpec desired;
	memset(&desired, 0, sizeof(desired));
	desired.freq = s_khz->integer == 44 ? 44100 : 22050;
	desired.format = format;
	desired.samples = samples;
	desired.channels = channels;
	desired.callback = &FillAudioBufferCallback;
	SDL_AudioSpec obtained;
	memset(&obtained, 0, sizeof(obtained));
	audio.device = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
	if (audio.device == 0) {
		Com_Printf("SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
		SNDDMA_Shutdown();
		return qfalse;
	}

	SNDDMA_PrintAudiospec("SDL_AudioSpec", &obtained);

	// save all the data we need to
	int q3Samples = (obtained.samples * obtained.channels) * 10;
	// samples must be divisible by number of channels
	q3Samples -= q3Samples % obtained.channels;

	audio.q3SamplePos = 0;
	dma.samplebits = obtained.format & 0xFF;
	dma.channels = obtained.channels;
	dma.samples = q3Samples;
	//fullsamples ? 
	dma.submission_chunk = 1;
	dma.speed = obtained.freq;
	audio.q3Bytes = dma.samples * (dma.samplebits / 8);
	dma.buffer = (byte*)calloc(1, audio.q3Bytes);
	audio.valid = qtrue;

	// opened devices are always paused by default
	SDL_PauseAudioDevice(audio.device, 0);
	return qtrue;
}


int SNDDMA_GetDMAPos( void ) {
	if (!audio.valid)
		return 0;

	return audio.q3SamplePos;
}

void SNDDMA_Shutdown( void ) {
	if (audio.device != 0) {
		SDL_PauseAudioDevice(audio.device, 1);
		SDL_CloseAudioDevice(audio.device);
		audio.device = 0;
	}

	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	free(dma.buffer);
	dma.buffer = NULL;
	audio.q3SamplePos = 0;
	audio.q3Bytes = 0;
	audio.valid = qfalse;
}

/*
==============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
===============
*/
void SNDDMA_Submit( void ) {
	if (!audio.valid)
		return;

	// let SDL call our registered callback function again
	SDL_UnlockAudioDevice(audio.device);
}

void SNDDMA_BeginPainting( void ) {
	if (!audio.valid)
		return;

	// prevent SDL from calling our registered callback function
	SDL_LockAudioDevice(audio.device);
}
