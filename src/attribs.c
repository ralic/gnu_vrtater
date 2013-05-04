/* attribs.c: store and maintain hmaps vs. node feedback creating vobspace.
   Copyright (C) 2012, 2013 J. A. Green <green8@sdf-eu.org>
   license: GNU GPL v3, see COPYING, otherwise see vrtater.c
*/

#include <math.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "attribs.h" /* !! */
#include "hmap.h"
#include "vectors.h"
#include "rotation.h"
#include "transform.h"

#ifdef VRT_RENDERER_GL
#include "rendergl.h"
#endif /* VRT_RENDER_GL */

/* lod envelopes */
#define VRT_SORT_PERIF_RATIO 8
#define VRT_SORT_FAR_RATIO 64
#define VRT_NEAR_THRESH 10000.0f /* for now, 10000 meters */
#define VRT_PERIF_THRESH 100000.0f

/* positioning, time, mass, factors, for now, also some to ease things */
#define SQRT_3 1.732050807
#define RANDOM_OFFSET 123456789
#define SMALLRANDOM 0.0000001
#define TINYRANDOM 0.000000001
#define SLOW1_10 .1
#define SLOW1_100 .01

unsigned int vrt_hmaps_max; /* external */
float vrt_render_cyc; /* external */

/* hmap memory, sorting */
hmapf_t *ap_hmap_near;
hmapf_t *ap_hmap_perif;
hmapf_t *ap_hmap_far;
hmapf_t **pp_lr_n;
hmapf_t **pp_rl_n;
hmapf_t **pp_lr_p;
hmapf_t **pp_rl_p;
hmapf_t **pp_lr_f;
hmapf_t **pp_rl_f;
int dlr_n, dlr_p, dlr_f;
unsigned int passes;

void proc_hmapf(hmapf_t *, int lodval);
void flow_over(btoggles_t *balance_criteria);
float estimate_radiusf(hmapf_t *);
void wanderf(hmapf_t *hmap, float e, float m, float r);

/* selection buffer */
hmapf_t *selectf_a; /* external */
hmapf_t *selectf_b; /* external */

/* set up hmap arrays, pointers thereto, nullify their attribs */
void
init_vohspace(void)
{
	int i;
	hmapf_t *p, **pap_hmap_n;

	if((a_hmaps = (hmapf_t *) malloc(vrt_hmaps_max * sizeof(hmapf_t))) == NULL) {
		__builtin_fprintf(stderr,  "vrtater:%s:%d: "
			"Error: Could not malloc for a_hmaps\n",
			__FILE__, __LINE__);
		abort();
	}
	if((ap_hmap_near = (hmapf_t *) malloc(vrt_hmaps_max * sizeof(hmapf_t *))) == NULL) {
		__builtin_fprintf(stderr,  "vrtater:%s:%d: "
			"Error: Could not malloc for ap_hmap_near\n",
			__FILE__, __LINE__);
		abort();
	}
	if((ap_hmap_perif = (hmapf_t *) malloc(vrt_hmaps_max * sizeof(hmapf_t *))) == NULL) {
		__builtin_fprintf(stderr,  "vrtater:%s:%d: "
			"Error: Could not malloc for ap_hmap_perif\n",
			__FILE__, __LINE__);
		abort();
	}
	if((ap_hmap_far = (hmapf_t *) malloc(vrt_hmaps_max * sizeof(hmapf_t *))) == NULL) {
		__builtin_fprintf(stderr,  "vrtater:%s:%d: "
			"Error: Could not malloc for ap_hmap_far\n",
			__FILE__, __LINE__);
		abort();
	}

	p = &a_hmaps[0];
	pap_hmap_n = (hmapf_t **)&ap_hmap_near[0];

	/* point out of bounds when buffer empty */
	pp_lr_n = (hmapf_t **) &ap_hmap_near[0];
	pp_lr_n--;
	pp_rl_n = (hmapf_t **) &ap_hmap_near[vrt_hmaps_max - 1];
	pp_rl_n++;
	pp_lr_p = (hmapf_t **) &ap_hmap_perif[0];
	pp_lr_p--;
	pp_rl_p = (hmapf_t **) &ap_hmap_perif[vrt_hmaps_max - 1];
	pp_rl_p++;
	pp_lr_f = (hmapf_t **) &ap_hmap_far[0];
	pp_lr_f--;
	pp_rl_f = (hmapf_t **) &ap_hmap_far[vrt_hmaps_max - 1];
	pp_rl_f++;

	passes = -1;
	dlr_n = 1;
	dlr_p = 1;
	dlr_f = 1;

	/* put vrt_hmaps_max hmap pointers in near buf */
	for(i=0;i<vrt_hmaps_max;i++, ++pp_lr_n, pap_hmap_n++)
		*pap_hmap_n = p++;
	/* set hmaps to null default */
	p = a_hmaps;
	for(i=0;i<vrt_hmaps_max;i++, p++) {
		p->name = (session_t)0;
		p->v_pos.x = 0;
		p->v_pos.y = 0;
		p->v_pos.z = 0;
		p->v_pos.m = VRT_PERIF_THRESH + 1;
		p->v_vel.x = 0;
		p->v_vel.y = 0;
		p->v_vel.z = 0;
		p->v_vel.m = 0;
		p->v_axi.x = 0;
		p->v_axi.y = 0;
		p->v_axi.z = 0;
		p->v_axi.m = 0;
		p->v_rel.x = 0;
		p->v_rel.y = 0;
		p->v_rel.z = 0;
		p->v_rel.m = 0;
		p->v_pre.x = 0;
		p->v_pre.y = 0;
		p->v_pre.z = 0;
		p->v_pre.m = 0;
		p->ang_spd = 0; /* radians/second */
		p->ang_dpl = 0; /* radians */
		p->mass.kg = 0;
		p->mass.kfactor = 0;
		p->kfactor = 1;
		p->bounding.geom = VRT_BOUND_NONE;
		p->bounding.v_sz.x = 0;
		p->bounding.v_sz.z = 0;
		p->bounding.v_sz.y = 0;
		p->bounding.v_sz.m = 0;
		p->draw.geom = VRT_DRAWGEOM_NONE;
		p->attribs.bits = VRT_MASK_FLOW_OVER;
		p->attribs.session_filter = 0;
		p->attribs.balance_filter = 0;
		p->p_data_vf = NULL;
		p->vf_total = 0;
		p->p_dialog = NULL;
		p->dialog_total = 0;
	}
}

/* from given finite set, find then attach unattached hmap */
hmapf_t *
attach_hmapf(void)
{
	int i;
	hmapf_t *p = a_hmaps;

	for(i=0;i<vrt_hmaps_max;i++,p++) {
		if(p->p_data_vf == NULL) {
			p->index = i; /* caller index */
			p = &a_hmaps[i];
			return p;
		}
	}
	return NULL;
}

/* re-initialize hmap referenced by p.  free any associated memory */
void
detach_hmapf(hmapf_t *p)
{
	hmapf_t *sb = &a_hmaps[0];
	hmapf_t *eb = &a_hmaps[vrt_hmaps_max];

	if((p >= sb) && (p <= eb)) {
		p->name = (session_t)0;
		p->v_pos.x = 0;
		p->v_pos.y = 0;
		p->v_pos.z = 0;
		p->v_pos.m = 0;
		p->v_vel.x = 0;
		p->v_vel.y = 0;
		p->v_vel.z = 0;
		p->v_vel.m = 0;
		p->v_axi.x = 0;
		p->v_axi.y = 0;
		p->v_axi.z = 0;
		p->v_axi.m = 0;
		p->v_rel.x = 0;
		p->v_rel.y = 0;
		p->v_rel.z = 0;
		p->v_rel.m = 0;
		p->v_pre.x = 0;
		p->v_pre.y = 0;
		p->v_pre.z = 0;
		p->v_pre.m = 0;
		p->ang_spd = 0;
		p->ang_dpl = 0;
		p->mass.kg = 0;
		p->mass.kfactor = 1;
		p->kfactor = 1;
		p->bounding.geom = VRT_BOUND_NONE;
		p->bounding.v_sz.x = 0;
		p->bounding.v_sz.z = 0;
		p->bounding.v_sz.y = 0;
		p->bounding.v_sz.m = 0;
		p->draw.geom = VRT_DRAWGEOM_NONE;
		p->attribs.bits = VRT_MASK_FLOW_OVER;
		p->attribs.session_filter = 0;
		p->attribs.balance_filter = 0;
		p->vf_total = 0;
		free(p->p_data_vf);
		p->p_dialog = NULL;
		p->dialog_total = 0;
		p->p_data_vf = NULL;
		free(p->p_dialog);
		return;
	}
	__builtin_fprintf(stderr, "vrtater:%s:%d: "
		"Error: Can not detach hmap\n "
		"Pointer reference NULL or outside of buffer\n",
		__FILE__, __LINE__);
}

/* sort hmaps and cue them for drawing */
void
sort_proc_hmaps(void)
{
	int i, count;

	/* conditional for multiple hmap types */
	/* ... */

	/* keep passthrough number for proximity prioritization */
	passes++;

	/* fixed pointers to array ends */
	hmapf_t **pp_b_n = (hmapf_t **) &ap_hmap_near[0];
	hmapf_t **pp_e_n = (hmapf_t **) &ap_hmap_near[vrt_hmaps_max - 1];
	hmapf_t **pp_b_p = (hmapf_t **) &ap_hmap_perif[0];
	hmapf_t **pp_e_p = (hmapf_t **) &ap_hmap_perif[vrt_hmaps_max - 1];
	hmapf_t **pp_b_f = (hmapf_t **) &ap_hmap_far[0];
	hmapf_t **pp_e_f = (hmapf_t **) &ap_hmap_far[vrt_hmaps_max - 1];

	/* sort for near, perif, far */

	/* near hmaps */
	if(dlr_n) { /* lr */

		/* determine near count based on direction */
		count = (pp_lr_n + 1) - pp_b_n;

		/* set rl pointer to empty position */
		pp_rl_n = pp_e_n + 1;

		/* sort */
		for(i=0;i<count;i++) {
			if((*pp_lr_n)->v_pos.m <= VRT_NEAR_THRESH) {
				proc_hmapf(*pp_lr_n, LOD_NEAR);
				*(--pp_rl_n) = *(pp_lr_n--); /* move in near */
			} else {
				if(dlr_p) { /* move to perif left to right */
					proc_hmapf(*pp_lr_n, LOD_PERIF);
					*(++pp_lr_p) = *(pp_lr_n--);
				} else { /* move to perif right to left */
					proc_hmapf(*pp_lr_n, LOD_PERIF);
					*(--pp_rl_p) = *(pp_lr_n--);
				}
			}
		}
		dlr_n = (dlr_n + 1) % 2; /* flip direction */

	} else { /* rl */

		/* determine near count based on direction */
		count = (pp_e_n + 1) - pp_rl_n;

		/* set lr pointer to empty position */
		pp_lr_n = pp_b_n - 1;

		/* sort */
		for(i=0;i<count;i++) {
			if((*pp_rl_n)->v_pos.m <= VRT_NEAR_THRESH) {
				proc_hmapf(*pp_rl_n, LOD_NEAR);
				*(++pp_lr_n) = *(pp_rl_n++); /* moves in near */
			} else {
				if(dlr_p) { /* moves to perif left to right */
					proc_hmapf(*pp_rl_n, LOD_PERIF);
					*(++pp_lr_p) = *(pp_rl_n++);
				} else { /* moves to perif right to left */
					proc_hmapf(*pp_rl_n, LOD_PERIF);
					*(--pp_rl_p) = *(pp_rl_n++);
				}
			}
		}
		dlr_n = (dlr_n + 1) % 2; /* flip direction */
	}

	/* perif hmaps */
	if(dlr_p) { /* lr */

		/* ratio */
		if(!(passes % VRT_SORT_PERIF_RATIO)) {

			/* determine perif count based on direction */
			count = (pp_lr_p + 1) - pp_b_p;

			/* set rl pointer to empty position */
			pp_rl_p = pp_e_p + 1;

			/* sort */
			for(i=0;i<count;i++) {
				if(((*pp_lr_p)->v_pos.m > VRT_NEAR_THRESH) & ((*pp_lr_p)->v_pos.m <= VRT_PERIF_THRESH)) {
					proc_hmapf(*pp_lr_p, LOD_PERIF);
					*(--pp_rl_p) = *(pp_lr_p--); /* move in perif */
				} else {
					if((*pp_lr_p)->v_pos.m <= VRT_NEAR_THRESH) {
						if(dlr_n) { /* moves to near left to right */
							proc_hmapf(*pp_lr_p, LOD_NEAR);
							*(++pp_lr_n) = *(pp_lr_p--);
						} else { /* moves to near right to left */
							proc_hmapf(*pp_lr_p, LOD_NEAR);
							*(--pp_rl_n) = *(pp_lr_p--);
						}
					} else { /* far */
						if(dlr_f) { /* moves to far left to right */
							proc_hmapf(*pp_lr_p, LOD_FAR);
							*(++pp_lr_f) = *(pp_lr_p--);
						} else { /* moves to far right to left */
							proc_hmapf(*pp_lr_p, LOD_FAR);
							*(--pp_rl_f) = *(pp_lr_p--);
						}
					}
				}
			}
			dlr_p = (dlr_p + 1) % 2; /* flip direction */
		}

	} else { /* rl */

		/* ratio */
		if(!(passes % VRT_SORT_PERIF_RATIO)) {

			/* determine perif count based on direction */
			count = (pp_e_p + 1) - pp_rl_p;

			/* set lr pointer to empty position */
			pp_lr_p = pp_b_p - 1;

			/* sort */
			for(i=0;i<count;i++) {
				if(((*pp_rl_p)->v_pos.m > VRT_NEAR_THRESH) & ((*pp_rl_p)->v_pos.m <= VRT_PERIF_THRESH)) {
					proc_hmapf(*pp_rl_p, LOD_PERIF);
					*(++pp_lr_p) = *(pp_rl_p++); /* moves in perif */
				} else {
					if((*pp_rl_p)->v_pos.m <= VRT_NEAR_THRESH) {
						if(dlr_n) { /* moves to near left to right */
							proc_hmapf(*pp_rl_p, LOD_NEAR);
							*(++pp_lr_n) = *(pp_rl_p++);

						} else { /* moves to near right to left */
							proc_hmapf(*pp_rl_p, LOD_NEAR);
							*(--pp_rl_n) = *(pp_rl_p++);
						}
					} else { /* far */
						if(dlr_f) { /* moves to far left to right */
							proc_hmapf(*pp_rl_p, LOD_FAR);
							*(++pp_lr_f) = *(pp_rl_p++);
						} else { /* moves to far right to left */
							proc_hmapf(*pp_rl_p, LOD_FAR);
							*(--pp_rl_f) = *(pp_rl_p++);
						}
					}
				}
			}
			dlr_p = (dlr_p + 1) % 2; /* flip direction */
		}
	}

	/* far hmaps */
	if(dlr_f) { /* lr */

		/* ratio */
		if(!(passes % VRT_SORT_FAR_RATIO)) {

			/* determine far count based on direction */
			count = (pp_lr_f + 1) - pp_b_f;

			/* set rl pointer to empty position */
			pp_rl_f = pp_e_f + 1;

			/* sort */
			for(i=0;i<count;i++) {
				if((*pp_lr_f)->v_pos.m > VRT_PERIF_THRESH) {
					proc_hmapf(*pp_lr_f, LOD_FAR);
					*(--pp_rl_f) = *(pp_lr_f--); /* moves in far */
				} else {
					/* note: could save cycles heretofore by moving
					   all non far to perif */
					if(((*pp_lr_f)->v_pos.m > VRT_NEAR_THRESH) & ((*pp_lr_f)->v_pos.m <= VRT_PERIF_THRESH)) {
						if(dlr_f) { /* moves to perif left to right */
							proc_hmapf(*pp_lr_f, LOD_PERIF);
							*(++pp_lr_p) = *(pp_lr_f--);
						} else { /* moves to perif right to left */
							proc_hmapf(*pp_lr_f, LOD_PERIF);
							*(--pp_rl_p) = *(pp_lr_f--);
						}
					} else { /* near */
						if(dlr_n) { /* moves to near left to right */
							proc_hmapf(*pp_lr_f, LOD_NEAR);
							*(++pp_lr_n) = *(pp_lr_f--);
						} else { /* moves to near right to left */
							proc_hmapf(*pp_lr_f, LOD_NEAR);
							*(--pp_rl_n) = *(pp_lr_f--);
						}
					}
				}
			}
			dlr_f = (dlr_f + 1) % 2; /* flip direction */
		}

	} else { /* rl */

		/* ratio */
		if(!(passes % VRT_SORT_FAR_RATIO)) {

			/* determine far count based on direction */
			count = (pp_e_f + 1) - pp_rl_f;

			/* set lr pointer to empty position */
			pp_lr_f = pp_b_f - 1;

			/* sort */
			for(i=0;i<count;i++) {
				if((*pp_rl_f)->v_pos.m > VRT_PERIF_THRESH) {
					proc_hmapf(*pp_rl_f, LOD_FAR);
					*(++pp_lr_f) = *(pp_rl_f++); /* moves in far */
				} else {
					if(((*pp_rl_f)->v_pos.m > VRT_NEAR_THRESH) & ((*pp_rl_f)->v_pos.m <= VRT_PERIF_THRESH)) {
						if(dlr_f) { /* moves to perif left to right */
							proc_hmapf(*pp_rl_f, LOD_PERIF);
							*(++pp_lr_p) = *(pp_rl_f++);
						} else { /* moves to perif right to left */
							proc_hmapf(*pp_rl_f, LOD_PERIF);
							*(--pp_rl_p) = *(pp_rl_f++);
						}
					} else { /* near */
						if(dlr_n) { /* moves to near left to right */
							proc_hmapf(*pp_rl_f, LOD_NEAR);
							*(++pp_lr_n) = *(pp_rl_f++);
						} else { /* moves to near right to left */
							proc_hmapf(*pp_rl_f, LOD_NEAR);
							*(--pp_rl_n) = *(pp_rl_f++);
						}
					}
				}
			}
			dlr_f = (dlr_f + 1) % 2; /* flip direction */
		}
	}
}

/* per hmap referenced by p, per frame in frequency, process humdrum attribs.
   foreward lod prerequisite to draw function */
void
proc_hmapf(hmapf_t *p, int lod)
{
	vf_t tmp, *q = &tmp;
	hmapf_t **a, **b;

	/* adjust hmap via kbase if set */

	/* note: vobs in 'partial vobspaces', also need to be written thru
           session_filter to set of all derived sessions vob is attached to. */

	/* set vob position
	   note: velocity mode is always on.  for position mode, set velocity
	   to 0 and then adjust the position manually.  a switch may be added
	   based on cycle saveing vs. average n voh vobs have fixed pos */
	a = (hmapf_t **)selectf_a;
	*a = p_hmapf(0);
	b = (hmapf_t **)selectf_b;
	*b = p;
	select_t t = { 0, 0, (hmapf_t **)selectf_a, 0, (hmapf_t **)selectf_b };
	intersection(&t);
	cp_vf(&(p->v_vel), q); /* take a copy of direction/velocity */
	factor_vf(q, q, vrt_render_cyc); /* create a delta vector given freq */
	sum_vf(&(p->v_pos), q, &(p->v_pos)); /* new pos = delta vector + pos */
	/* set vob angular displacement
	   note: v_ang_vel will be pseudovector.  on fly calc moreso optimal */
	p->ang_dpl += p->ang_spd * vrt_render_cyc;
	/* wraparound for 2 * M_PI, without this a glitch occurs on wrap */
	if(fabs(p->ang_dpl) >= 2 * M_PI)
		p->ang_dpl = fmodf(p->ang_dpl, 2 * M_PI);
	/* build renderer's buffer */
	if(p->p_data_vf)
		draw_hmapf(p, lod);
}

/* vs. overload, release any hmaps with balance_filter over balance_criteria.
   needs further implementation/considerations */
void
flow_over(btoggles_t *balance_criteria)
{
	int i;
	hmapf_t *p = &a_hmaps[0];

	for(i=0;i<vrt_hmaps_max;i++,p++) {
		if( \
		(p->attribs.bits & VRT_MASK_FLOW_OVER) & \
		(p->attribs.balance_filter &= *balance_criteria)) \
			p->attribs.bits |= VRT_MASK_RECYCLE;
	}
	/* then in positional round these are recycled */
}

/* free all dynamic memory associated with vohspace and selection buffer */
void
free_vohspace_memory(void)
{
	int i;
	for(i=0;i<vrt_hmaps_max;i++) {
		free((&a_hmaps[i])->p_data_vf);
		(&a_hmaps[i])->p_data_vf = NULL;
		free((&a_hmaps[i])->p_dialog);
		(&a_hmaps[i])->p_dialog = NULL;
	}
	free(a_hmaps);
	free(ap_hmap_near);
	free(ap_hmap_perif);
	free(ap_hmap_far);
	free(selectf_a);
	free(selectf_b);
}

/* echo_in_node_partial_vobs() as processed */
/* ... */

/* search vobspace */
/* ... */

/* tag vobspace */
/* ... */

/* vob in hmap referenced by p, appears at loc
   this will be moved to transform less wander */
void
nportf(hmapf_t *p, vf_t *loc)
{
	float r;

	/* diag */
	if(!p) {
		__builtin_fprintf(stderr, "vrtater:%s:%d: "
			"Error: Attempted nport of NULL vob\n"
			"Nport destination was %f %f %f\n",
			__FILE__, __LINE__, loc->x, loc->y, loc->z);
		return;
	}

	/* nport */
	cp_vf(loc, &(p->v_pos));
	form_mag_vf(&(p->v_pos));

	/* arrival */
	p->mass.kg = 1.0;
	r = estimate_radiusf(p);
	/* for now */
	wanderf(p, ((float)rand() * TINYRANDOM), p->mass.kg, r);
}

/* guestimate for hmap p, a proportion vs. where it were if not a sphere */
float
estimate_radiusf(hmapf_t *p)
{
	float r;
	switch(p->bounding.geom) {
		case VRT_BOUND_NONE:
			r = 0;
		break;
		case VRT_BOUND_SPHERE:
			r = p->bounding.v_sz.x;
		break;
		case VRT_BOUND_CYL:
			r = 0;
		break;
		case VRT_BOUND_RCUBOID:
			r = M_SQRT1_2 * sqrt(
				p->bounding.v_sz.x * p->bounding.v_sz.x +
				p->bounding.v_sz.y * p->bounding.v_sz.y +
				p->bounding.v_sz.z * p->bounding.v_sz.z);
		break;
		case VRT_BOUND_CUBE:
			r = 0;
		break;
	}
	return r;
}

/* set vob in hmap referenced by p to arbitrary but energy factored initial
   conditions vs. given energy e.  v_vel, v_axi, v_pre, ang_spd, and ang_dpl,
   are set vs. given determinate or estimated radius r and given mass m */
void
wanderf(hmapf_t *p, float e, float m, float r)
{
	float kinetic_energy;
	float avg_absorbed_ke;
	float avg_reflected_ke;
	float avg_orginv_ke;
	float avg_tangentv_ke;
	float rnd, rnd1, rnd2, rnd3;
	static float sign = 1.0;

	/* note: vobspace is represented as 1 default renderer unit per m.
	   e, will be represented in joule units, 1 ampere/s, or 1 neuton/m.
	   this function will be moved to transform.  e can be there derived
	   and redistributed as can r */

	/* pseudo random reciprocal part factors */
	rnd1 = (float)rand(); rnd2 = (float)rand(); rnd3 = rnd1 + rnd2;

	/* for spherical regular solid vohs
	   distribute input energy */
	kinetic_energy = fabs(e) / m; /* kinetic energy per mass */
	avg_absorbed_ke = avg_reflected_ke = kinetic_energy / 2;
	avg_orginv_ke = avg_absorbed_ke * M_SQRT1_2 * (rnd1 / rnd3);
	avg_tangentv_ke = avg_absorbed_ke * (1 - M_SQRT1_2) * (rnd2 / rnd3);

	/* arbitrary direction vector */
	rnd = (float)rand() * SMALLRANDOM;
	p->v_vel.x = rnd = ((rnd + (float)rand()) * SMALLRANDOM);
	if(((int)rnd) % 2) { sign = sign * -1.0; }
	p->v_vel.x = copysign(p->v_vel.x, sign);
	p->v_vel.y = rnd = ((rnd + (float)rand()) * SMALLRANDOM);
	if(((int)rnd) % 2) { sign = sign * -1.0; }
	p->v_vel.y = copysign(p->v_vel.y, sign);
	p->v_vel.z = rnd = ((rnd + (float)rand()) * SMALLRANDOM);
	if(((int)rnd) % 2) { sign = sign * -1.0; }
	p->v_vel.z = copysign(p->v_vel.z, sign);
	normz_vf(&(p->v_vel), &(p->v_vel));

	/* velocity thereapon */
	tele_mag_vf(&(p->v_vel), &(p->v_vel), avg_orginv_ke); /* m/s */

	/* arbitrary v_pre vector axis of mass distribution */
	p->v_pre.x = rnd = ((rnd + (float)rand()) * SMALLRANDOM);
	if(((int)rnd) % 2) { sign = sign * -1.0; }
	p->v_pre.x = copysign(p->v_pre.x, sign);
	p->v_pre.y = rnd = ((rnd + (float)rand()) * SMALLRANDOM);
	if(((int)rnd) % 2) { sign = sign * -1.0; }
	p->v_pre.y = copysign(p->v_pre.y, sign);
	p->v_pre.z = rnd = ((rnd + (float)rand()) * SMALLRANDOM);
	if(((int)rnd) % 2) { sign = sign * -1.0; }
	p->v_pre.z = copysign(p->v_pre.z, sign);
	normz_vf(&(p->v_pre), &(p->v_pre));

	/* arbitrary v_axi and thus relative rotation/sense when v_axi is
	   combined with ang_dpl.  for now v_pre is unused, so use that */
	cp_vf(&(p->v_pre), &(p->v_axi));

	/* arbitrary signed rotation speed and initial displacement */
	p->ang_spd = ANG_AFS * avg_tangentv_ke / r; /* r/s */
	rnd = (rnd + (float)rand()) / 2;
	if(((int)rnd) % 2) { sign = sign * -1.0; }
	p->ang_spd = copysign(p->ang_spd, sign);
	p->ang_dpl = rnd + (float)rand();
}
