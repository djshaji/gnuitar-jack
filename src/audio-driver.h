/* Common definitions for audio drivers. (c) 2006 Antti S. Lankila
 * Licensed under GPL.
 * $Id$
 *
 * $Log$
 * Revision 1.2  2006/08/06 20:45:52  alankila
 * - use stdint type rather than glib type
 *
 * Revision 1.1  2006/08/06 20:14:54  alankila
 * - split pump.h into several domain-specific headers to reduce file
 *   interdependencies (everyone included pump.h). New files are:
 *   - effect.h for effect definitions
 *   - audio-driver.h for work relating to audio drivers
 *   - audio-midi.h for MIDI interaction.
 *
 */

#ifndef _AUDIO_DRIVER_H_
#define _AUDIO_DRIVER_H_ 1

#include <stdint.h>

#define MAX_SAMPLE (32767 << 8)

#ifdef FLOAT_DSP
typedef float           DSP_SAMPLE;
#else
typedef int_least32_t   DSP_SAMPLE;
#endif

typedef struct {
    DSP_SAMPLE     *data;
    DSP_SAMPLE     *data_swap;
    unsigned int    len;
    unsigned int    channels;
} data_block_t;

struct audio_driver_channels {
    unsigned int in, out;
};

typedef struct {
    char    *str;
    int     enabled;
    struct audio_driver_channels *channels;
    
    int     (*init)(void);
    void    (*finish)(void);
} audio_driver_t;

extern audio_driver_t *audio_driver;

typedef int16_t  SAMPLE16;
typedef int32_t  SAMPLE32;

#ifdef _WIN32
    #define MAX_BUFFERS	1024	/* number of input/output sound buffers */
#endif

#define MIN_BUFFER_SIZE 128
#define MAX_BUFFER_SIZE 65536
#define MAX_CHANNELS 4
#define MAX_SAMPLE_RATE 48000

extern char alsadevice_str[64];
extern unsigned short n_input_channels;
extern unsigned short n_output_channels;
extern unsigned int sample_rate;
extern unsigned int buffer_size;

#ifndef _WIN32
    extern unsigned int fragments;
#else
    extern unsigned int nbuffers;
    extern unsigned int overrun_threshold;
#endif

#ifndef _WIN32
extern DSP_SAMPLE       procbuf[MAX_BUFFER_SIZE * MAX_CHANNELS];
extern DSP_SAMPLE       procbuf2[MAX_BUFFER_SIZE * MAX_CHANNELS];
#else
/* sadly, Windows and Linux have a different idea what the size of the buffer is.
 * Linux world talks about size in frames because that is most convenient for ALSA
 * and JACK (but less so for OSS). */
extern DSP_SAMPLE       procbuf[MAX_BUFFER_SIZE / sizeof(SAMPLE16)];
extern DSP_SAMPLE       procbuf2[MAX_BUFFER_SIZE / sizeof(SAMPLE16)];
#endif

#endif