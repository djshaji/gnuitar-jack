/*
 * GNUitar
 * Distortion effect
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

#ifndef _DISTORT2_H_
#define _DISTORT2_H_ 1

#include "pump.h"
#include "rcfilter.h"
#include "chebyshev.h"

extern void     distort2_create(struct effect *);

struct distort2_params {
    int         r2;
    double      mUt;
    int		noisegate;
    int		treble;
    struct filter_data noise;
    double	c0,d1,lyf;
    double 	last[MAX_CHANNELS];
    double	lastupsample;
    struct Biquad cheb, cheb1;  /* 2 chebyshev filters */
};

#endif
