/* rendergl.c: render vobs through gl calls.
   Copyright (C) 2012, 2013 J. A. Green <green8@sdf-eu.org>
   license: GNU GPL v3, see COPYING, otherwise see vrtater.c
*/

#include <GL/gl.h>
#include "rendergl.h"
#include "vectors.h"
#include "rotation.h"
#include "progscope.h"

hmapf_t *fov0;
vf_t oa_fp, *vpt = &oa_fp;

/* Based on options, a calling function may want these maintained. */
unsigned int sp_ratio;
unsigned int sf_ratio;
float near_thresh;
float perif_thresh;

/* Set defaults. */
void
init_renderer(void)
{
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_CULL_FACE);

	/* Set frustum. */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-.3, .3, -.3, .3, .3, 10000.0); /* for now */

	/* Assume modeling transforms. */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/* Setup rendering for this frame. */
void
init_next_buffer(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/* For now, set a color. */
	glColor3f(.1,.5,0);
}

/* Called per hmap per frame, draw hmap vs. lod given VRT_DRAWGEOM_* support. */
void
render_hmapf(hmapf_t *hmap, int lod)
{
	int i, j;
	vf_t v, nv, edge, plane, *vmap = hmap->vmap;
	GLfloat glv[3][3], gln[3];

	/* Tend to lod issues, possibly buffering hmaps, vs. lod value. */
	switch (lod) {

		case VRT_MASK_LOD_INF:
		fov0 = hmap; /* vs. filter in proc_hmapf() sent once, first */
		vpt = &(hmap->vpos);
		break;

		case VRT_MASK_LOD_NEAR:
		break;

		case VRT_MASK_LOD_PERIF:
		break;

		case VRT_MASK_LOD_FAR:
		break;
	}

	/* Translate to where the optical axis eminates from the focal plane. */
	glTranslatef(-vpt->x, -vpt->y, -vpt->z);

	switch (hmap->draw.geom) {

		case VRT_DRAWGEOM_NONE:
		break;

		case VRT_DRAWGEOM_TRIANGLES:
		
		for (i = 0; i < hmap->vmap_total / 3; i++) {
			for (j = 0; j < 3; j++, vmap++) {

				cp_vf(vmap, &v);
				rotate_vf(&v, &(hmap->vaxi), hmap->ang_dpl);

				/* Format vertices for rendering. */
				glv[j][0] = (GLfloat) (&v)->x + hmap->vpos.x;
				glv[j][1] = (GLfloat) (&v)->y + hmap->vpos.y;
				glv[j][2] = (GLfloat) (&v)->z + hmap->vpos.z;

				/* Add colors/effects for diagnostic use. */
				if ((hmap->index >= 0) && (hmap->index <= 5)) {
					if (hmap->index == 0) {
						YEL();
						if (i == 0)
							RED();
					}
					if (hmap->index == 1) {
						GRN();
						if (i == 0)
							RED();
						if (i == 1)
							YEL();
					}
					if (hmap->index == 2) {
						YEL();
						if (i == 0)
							RED();
						if (i == 1)
							GRN();
					}
					if (hmap->index == 3)
						ORN();
					if (hmap->index == 4)
						BLU();
					if (hmap->index == 5)
						VLT();
				} else
					GRN();
#ifdef DIAG_EFFECT
				if ((hmap->index >= 20) & (hmap->index <= 23)) {
					static int osc = 0;
					if ((osc++) % 2) {
						GRN();
						glPolygonMode(GL_FRONT, GL_LINE);
					} else {
						RED();
						glPolygonMode(GL_FRONT, GL_FILL);
					}
				}
#endif
			}

			/* Set surface normal then draw triangle. */
			(&edge)->x = (float) glv[1][0] - glv[0][0];
			(&edge)->y = (float) glv[1][1] - glv[0][1];
			(&edge)->z = (float) glv[1][2] - glv[0][2];
			(&plane)->x = (float) glv[2][0] - glv[1][0];
			(&plane)->y = (float) glv[2][1] - glv[1][1];
			(&plane)->z = (float) glv[2][2] - glv[1][2];
			cprod_vf(&edge, &plane, &nv); /* adds (&nv)->m */
			gln[0] = (GLfloat) (&nv)->x / (&nv)->m;
			gln[1] = (GLfloat) (&nv)->y / (&nv)->m;
			gln[2] = (GLfloat) (&nv)->z / (&nv)->m;
			glNormal3fv(gln);

			glBegin(GL_TRIANGLES);
				glVertex3fv(&glv[0][0]);
				glVertex3fv(&glv[1][0]);
				glVertex3fv(&glv[2][0]);
			glEnd();

		}
		break;

		case VRT_DRAWGEOM_LINES:
		for (i = 0; i < hmap->vmap_total; i++) {
			;
		}
		break;

		default:
		__builtin_printf("renderer: err, unsupported hmap geom.\n");
		break;
	}
	glTranslatef(vpt->x, vpt->y, vpt->z);
}

/* Render apon hmaps and also therein and thereout of those.  Where
   fov_available is true, caller specifies that the renderer may modify fov0
   attribs.  This function is called immediately before buffer is drawn to dpy0.
   Viewpoint vpt is the position of hmap fov0. */
void
render_vobspace(int fov0_available)
{
	glTranslatef(-vpt->x, -vpt->y, -vpt->z);
	;
	glTranslatef(vpt->x, vpt->y, vpt->z);
}
