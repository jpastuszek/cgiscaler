/***************************************************************************
 *   Copyright (C) 2007, 2008, 2008 by Jakub Pastuszek   *
 *   jpastuszek@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "config.h"

#include <math.h>

#include "assert.h"
#include "geometry_math.h"
#include "debug.h"

/* This function will return dimensions that will be result of fitting rectangle of dimensions a in to rectangle of dimensions b without loosing it's aspect ratio */
struct dimensions resize_to_fit_in(struct dimensions a, struct dimensions b) {
	double wf, hf, f;
	struct dimensions out;

	wf = (double) b.w / a.w;
	hf = (double) b.h / a.h;

	if (wf > hf)
		f = hf;
	else 
		f = wf;

	out.w = a.w * f;
	out.h = a.h * f;
	
	return out;
}

/* This function will return dimension a with reduced field to value of field parameter without loosing it's aspect ratio */
struct dimensions reduce_filed(struct dimensions a, int field) {
	int in_field;
	struct dimensions ret;
	double f;

	debug(DEB,"Reducing rectangle field %d x %d to %d pixels", a.w, a.h, field);

	in_field = a.w * a.h;
	if (in_field <= field)
		return a;

	f = sqrt((double) field / in_field);

	ret.w = a.w * f;
	ret.h = a.h * f;

	debug(DEB,"After reduction %d x %d", ret.w, ret.h);
	return ret;
}
