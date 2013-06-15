/* generator.h 
   Copyright (C) 2012, 2013 J. A. Green <green8@sdf-eu.org>
   license: GNU GPL v3, see COPYING, otherwise see vrtater.c
*/

#ifndef VRT_GENERATOR_H
#define VRT_GENERATOR_H

#include "bittoggles.h"
#include "vectors.h"

/* generator_opts_t
   these are all set by ifnode**.c, some through data maintained in dialog.c
   balance_criteria is still in the works, and may be proposed as some hybrid
   of bits vs. a quantity once needed.  vobspace_criteria, still in the works
   is listed below */
struct generator_opts {
	btoggles_t balance_criteria; /* balance_filter options */
	btoggles_t vobspace_criteria; /* context options */
	int why; /* fail, ... */
	int what; /* shutdown, ... */
	double when; /* shutdown, ... */
} genopts;

/* gen_opts_t vobspace_criteria.  effective while held high, some of these
   where noted, will be cleared after use */
enum {
	VRT_ORDINAL_SHUTDOWN,
#define VRT_MASK_SHUTDOWN (1 << VRT_ORDINAL_SHUTDOWN)
	VRT_ORDINAL_DASHF,
#define VRT_MASK_DASHF (1 << VRT_ORDINAL_DASHF)
};


int generate_node(void);
void regenerate_scene(vf_t *);
void close_vobspace(double time_till_closed);
void close_node(void);
int resize_node(int, int);
/* generator supported calls for tug input */
void generator_hapticNormill(void);
void generator_intersection(void);

#endif /* VRT_GENERATOR_H */
