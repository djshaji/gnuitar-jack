/*
 * GNUitar
 * Pump module - processeing sound
 * Copyright (C) 2000,2001 Max Rudensky		<fonin@ziet.zhitomir.ua>
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
 *
 * $Log$
 * Revision 1.5  2001/06/02 14:05:59  fonin
 * Added GNU disclaimer.
 *
 * Revision 1.4  2001/03/25 17:42:32  fonin
 * open() can overwrite existing files from now, because program switches back to real user priorities after start.
 *
 * Revision 1.3  2001/03/25 12:10:06  fonin
 * Text messages begin from newline rather than end with it.
 *
 * Revision 1.2  2001/01/13 10:02:35  fonin
 * Fix: setuid root program shouldnt overwrite existing files.
 *
 * Revision 1.1.1.1  2001/01/11 13:22:01  fonin
 * Version 0.1.0 Release 1 beta
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pump.h"
#include "gui.h"

#include "autowah.h"
#include "phasor.h"
#include "chorus.h"
#include "delay.h"
#include "echo.h"
#include "tremolo.h"
#include "vibrato.h"
#include "distort.h"
#include "sustain.h"
#include "reverb.h"
#include "tracker.h"

struct effect  *effects[MAX_EFFECTS];
int             n = 0;
unsigned short  audio_lock = 0;	/*
				 * when nonzero pause pumping 
				 */
extern char     version[];
unsigned short  write_track = 0;	/*
					 * when nonzero we should write
					 * sample to disk 
					 */
extern void     initSinLookUp(void);	/*
					 * from chorus.c 
					 */

int
pump_sample(int *s, int size)
{
    struct data_block db;
    int             i;

    if (audio_lock)
	return 0;

    db.data = s;
    db.len = size;

    /*
     * Pumping
     */
    for (i = 0; i < n; i++) {
	effects[i]->proc_filter(effects[i], &db);
    }

    /*
     * Writing track
     */
    if (write_track) {
	track_write(s, size);
    }

    return 0;
}

struct effect_creator effect_list[] = {
    {"autowah", autowah_create},
    {"distort", distort_create},
    {"delay", delay_create},
    {"reverb", reverb_create},
    {"vibrato", vibrato_create},
    {"chorus", chorus_create},
    {"echo", echo_create},
    {"phasor", phasor_create},
    {"tremolo", tremolo_create},
    {"sustain", sustain_create},
    {NULL, NULL}
};

void
pump_start(int argc, char **argv)
{
    int             i,
                    j;

    void            (*create_f[10]) (struct effect *);
    /*
     * =    { monitor_create, echo_create, chorus_create, NULL };
     */

    initSinLookUp();

    audio_lock = 1;
    j = 0;

    if (argc == 1) {
	int             k = 0;
	printf("\nPossible effects:");
	while (effect_list[k].str) {
	    printf("\n  %s", effect_list[k].str);
	    k++;
	}
    }
    for (i = 1; i < argc; i++) {
	int             k = 0;
	while (effect_list[k].str && strcmp(argv[i], effect_list[k].str)) {
	    k++;
	}
	if (effect_list[k].str) {
	    create_f[j++] = effect_list[k].create_f;
	    printf("\nadding %s", effect_list[k].str);
	    gtk_clist_append(GTK_CLIST(processor), &effect_list[k].str);
	} else {
	    printf("\n%s is not a known effect", argv[i]);
	}
    }
    create_f[j++] = NULL;

    /*
     * Cleaning effects[]
     */
    for (j = 0; j < MAX_EFFECTS; j++) {
	effects[j] = NULL;
    }

    while (n < MAX_EFFECTS && create_f[n]) {
	effects[n] = (struct effect *) calloc(1, sizeof(struct effect));
	create_f[n] (effects[n]);
	effects[n]->proc_init(effects[n]);
	n++;
    }
    audio_lock = 0;
}

void
pump_stop(void)
{
    int             i;

    audio_lock = 1;
    for (i = 0; i < n; i++) {
	effects[i]->proc_done(effects[i]);
    }
    n = 0;
}

void
passthru(struct effect *p, struct data_block *db)
{
}

void
save_pump(char *fname)
{
    int             i;
    int             fd = 0;

    fprintf(stderr, "\nWriting profile (%s)...", fname);
    if ((fd = open(fname, O_WRONLY | O_CREAT, S_IREAD | S_IWRITE)) < 0) {
	perror("Save failed");
	return;
    }
    /*
     * Chown file to one who launched us
     */
/*    chown(fname, getuid(), getgid());
*/
    /*
     * writing signature 
     */
    write(fd, version, 32);
    for (i = 0; i < n; i++) {
	if (effects[i]->proc_save != NULL) {
	    write(fd, &effects[i]->id, sizeof(unsigned short));
	    write(fd, &effects[i]->toggle, sizeof(short));
	    effects[i]->proc_save(effects[i], fd);
	}
    }
    close(fd);
    fprintf(stderr, "ok\n");
}

void
load_pump(char *fname)
{
    int             fd = 0;
    short           effect_tag = -1;
    char            rc_version[32];

    if (!(fd = open(fname, O_RDONLY, S_IREAD | S_IWRITE))) {
	perror("Load failed");
	return;
    }

    /*
     * reading signature and compare with our version 
     */
    read(fd, rc_version, 32);
    if (strncmp(version, rc_version, 7) != 0) {
	fprintf(stderr, "\nThis is not my rc file.");
	close(fd);
	return;
    }

    gtk_clist_clear(GTK_CLIST(processor));
    audio_lock = 1;
    pump_stop();

    n = 0;
    while (read(fd, &effect_tag, sizeof(short)) > 0) {
	assert((effect_tag >= 0 && effect_tag <= EFFECT_AMOUNT));

	fprintf(stderr, "\nloading %s", effect_list[effect_tag].str);

	effects[n] = (struct effect *) calloc(1, sizeof(struct effect));
	effect_list[effect_tag].create_f(effects[n]);
	read(fd, &effects[n]->toggle, sizeof(unsigned short));
	effects[n]->proc_load(effects[n], fd);
	effects[n]->proc_init(effects[n]);
	gtk_clist_append(GTK_CLIST(processor),
			 &effect_list[effect_tag].str);
	n++;
    }
    audio_lock = 0;
}
