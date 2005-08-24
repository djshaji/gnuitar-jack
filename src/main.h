/*
 * GNUitar
 * Utility functions
 * Copyright (C) 2000,2001,2003 Max Rudensky         <fonin@ziet.zhitomir.ua>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

#ifndef _MAIN_H_
#define _MAIN_H_ 1

#ifndef _WIN32
/* compile-time decision is easier to make working first */
#    ifdef HAVE_ALSA
#        include "audio-alsa.h"
#        define AUDIO_THREAD alsa_audio_thread
#        define AUDIO_INIT   alsa_init_sound
#        define AUDIO_FINISH alsa_finish_sound
#    endif
#    ifdef HAVE_OSS
#        include "audio-oss.h"
#        define AUDIO_THREAD oss_audio_thread
#        define AUDIO_INIT   oss_init_sound
#        define AUDIO_FINISH oss_finish_sound
#    endif
#else
#    include "audio-windows.h"
#endif

enum GnuitarErr {
    ERR_NOERROR = 0,
    ERR_THREAD, 		/* cannot create audio thread */
    ERR_DEMOEXPIRED,		/* demo version is expired */
    ERR_WAVEINOPEN,		/* cannot open wave in device */
    ERR_WAVEOUTOPEN,		/* cannot open wave out device */
    ERR_WAVEFRAGMENT,   	/* cannot set fragments */
    ERR_WAVEDUPLEX,		/* cannot open device in duplex mode */
    ERR_WAVENOTDUPLEX,  	/* device is not full-duplex capable */
    ERR_WAVEGETCAPS,		/* cannot get device capabilities */
    ERR_WAVESETBIT,		/* cannot set 8/16bits */
    ERR_WAVESETCHANNELS,	/* cannot set mono/stereo mode */
    ERR_WAVESETRATE,		/* cannot set sampling rate */
    ERR_WAVEOUTHDR,		/* error preparing write header */
    ERR_WAVEINHDR,		/* error preparing record header */
    ERR_WAVEINQUEUE,		/* error queuing record header */
    ERR_WAVEINRECORD,           /* recording error */
    ERR_DSOUNDOPEN,		
    ERR_DSOUNDBUFFER,	
    ERR_DSOUNDPLAYBACK,	
    ERR_DSCOOPLEVEL
};
    
extern volatile int state;
#ifndef _WIN32
extern pthread_mutex_t snd_open;

extern SAMPLE       rdbuf[MAX_BUFFER_SIZE / sizeof(SAMPLE)];
extern DSP_SAMPLE   procbuf[MAX_BUFFER_SIZE / sizeof(SAMPLE)];
#else

extern char            wrbuf[MIN_BUFFER_SIZE * MAX_BUFFERS];
extern char            rdbuf[MIN_BUFFER_SIZE * MAX_BUFFERS];
extern DSP_SAMPLE      procbuf[MAX_BUFFER_SIZE];
#endif

/*
 * Program states:
 */
#define STATE_PROCESS		0	/* 0 - recording/playback is on */
#define STATE_PAUSE		1	/* 1 - playback is paused */
#define STATE_EXIT		2	/* 2 - exit thread */
#define STATE_START		3	/* 3 - the thread is just started
					 * (first loop), playback is on */
#define STATE_START_PAUSE	4	/* 4 - application is starting,
					 * playback is paused */
#endif
