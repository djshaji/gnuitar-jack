/*
 * $Id$
 *
 * $Log$
 * Revision 1.1  2001/01/11 13:21:53  fonin
 * Initial revision
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <sched.h>
#include <pthread.h>
#include <linux/soundcard.h>

#include "pump.h"
#include "tracker.h"
#include "gui.h"

pthread_t       audio_thread;


static int      stop = 0;
int             fd;

char            version[32] = "GNUitar $Name$";

void           *
audio_thread_start(void *V)
{
    int             count,
                    i;

    short           rdbuf[BUFFER_SIZE / 2];
    int             procbuf[BUFFER_SIZE / 2];

    while (!stop) {
	count = read(fd, rdbuf, BUFFER_SIZE);
	if (count != BUFFER_SIZE) {
	    fprintf(stderr, "Cannot read samples!\n");
	    close(fd);
	    exit(0);
	}

	for (i = 0; i < count / 2; i++)
	    procbuf[i] = rdbuf[i];

	pump_sample(procbuf, count / 2);

	for (i = 0; i < count / 2; i++) {
	    int             W = procbuf[i];
	    if (W < -32767)
		W = -32767;
	    if (W > 32767)
		W = 32767;
	    rdbuf[i] = W;
	}

	count = write(fd, rdbuf, BUFFER_SIZE);
	if (count != BUFFER_SIZE) {
	    fprintf(stderr, "Cannot write samples!\n");
	    close(fd);
	    exit(0);
	}
    }
}

void
signal_ctrlc(int sig)
{
    stop = 1;
}

int
main(int argc, char **argv)
{
    int             max_priority,
                    i;
    struct sched_param p;


    if ((fd = open("/dev/dsp", O_RDWR)) == -1) {
	fprintf(stderr, "Cannot open audio device!\n");
	return -1;
    }

    i = 0x7fff0008;
    if (ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &i) < 0) {
	fprintf(stderr, "Cannot setup fragments!\n");
	close(fd);
	return -1;
    }

    if (ioctl(fd, SNDCTL_DSP_SETDUPLEX, 0) == -1) {
	fprintf(stderr, "Cannot setup fullduplex audio!\n");
	close(fd);
	return -1;
    }

    if (ioctl(fd, SNDCTL_DSP_GETCAPS, &i) == -1) {
	fprintf(stderr, "Cannot get soundcard capabilities!\n");
	close(fd);
	return -1;
    }

    if (!(i & DSP_CAP_DUPLEX)) {
	fprintf(stderr,
		"Sorry but your soundcard isn't full duplex capable!\n");
	close(fd);
	return -1;
    }

    i = AFMT_S16_LE;
    if (ioctl(fd, SNDCTL_DSP_SETFMT, &i) == -1) {
	fprintf(stderr, "Cannot setup 16 bit audio!\n");
	close(fd);
	return -1;
    }

    i = 0;
    if (ioctl(fd, SNDCTL_DSP_STEREO, &i) == -1) {
	fprintf(stderr, "Cannot setup mono audio!\n");
	close(fd);
	return -1;
    }

    i = SAMPLE_RATE;
    if (ioctl(fd, SNDCTL_DSP_SPEED, &i) == -1) {
	fprintf(stderr, "Cannot setup sampling frequency %dHz!\n", i);
	close(fd);
	return -1;
    }

    gtk_init(&argc, &argv);
    init_gui();


    pump_start(argc, argv);

    max_priority = sched_get_priority_max(SCHED_FIFO);
    p.sched_priority = max_priority;

    if (sched_setscheduler(0, SCHED_FIFO, &p)) {
	printf
	    ("Failed to set scheduler priority. (Are you running as root?)\n");
	printf("Continuing with default priority\n");
    }

    if (pthread_create(&audio_thread, NULL, audio_thread_start, NULL)) {
	fprintf(stderr, "\nAudio thread creation failed!");
	exit(1);
    }

    gtk_main();

    pump_stop();

    tracker_done();

    close(fd);

    return 0;
}
