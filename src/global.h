/* global.h
   Copyright (C) 2012, 2013 J. A. Green <green8@sdf-eu.org>
   license: GNU GPL v3, see COPYING, otherwise see vrtater.c
*/

#ifndef VRT_GLOBAL_H
#define VRT_GLOBAL_H

#define VRT_X_SUPPORT
#define VRT_RENDERER_GL
#define VRT_US101KBD_X11_DIAG
#define PI_180 0.017453292519943
#define ANG_AFS 1.697652 /* angular acc factor for regular solid sphere */

/* Diagnostics.  Used with keypress f, g, or h when in positional mode. */
#define DIAG_DIALOG /* Proposed Free hmapwrapf Vertice Data Disclaimer */
/* session.c diagnostics.  Simulate working session's + hopefully loginkeys. */
#define DIAG_NODEKEYS /* for DIAG_CONTINUING and DIAG_FLEXIBLE diagnostics */
#define DIAG_CONTINUING_ENABLE_OFF /* exclusive vs. DIAG_FLEXIBLE_ENABLE */
#define DIAG_CONTINUING_SESSION /* parts of diagnostic  */
#define DIAG_CONTINUING_OTHRHOLDKEY_OFF /* set holdkey already existing */
#define DIAG_CONTINUING_SETHOLDKEY_OFF /* add new holdkey on continuing */
/* note: FLEXIBLE_SENDS_* tests are exclusive. */
#define DIAG_CONTINUING_FLEXIBLE_SENDS_RETRY_OFF /* redundant key. flexible */
#define DIAG_CONTINUING_FLEXIBLE_SENDS_LASTKEYERR_OFF /* sync's contingent */
#define DIAG_CONTINUING_FLEXIBLE_SENDS_BOTHKEYERR_OFF /* exclusive pair err */
#define DIAG_CONTINUING_FLEXIBLE_SENDS_NEWREPUTED_OFF /* new on flexible */
#define DIAG_FLEXIBLE_ENABLE /* exclusive vs. DIAG_CONTINUING_ENABLE */
#define DIAG_FLEXIBLE_SESSION /* parts of diagnostic */
#define DIAG_FLEXIBLE_LASTKEYERR_OFF /* sync's contingent */
#define DIAG_FLEXIBLE_CONTINGENTKEYERR_OFF /* both sync keys mismatch */
#define DIAG_FLEXIBLE_NEWREPUTED_OFF /* both sync keys null */
#define DIAG_FLEXIBLE_MAPKEY_REDUNDANT_OFF
#define DIAG_FLEXIBLE_SETHOLDKEY_OFF
#define DIAG_FLEXIBLE_HOLDKEY_REDUNDANT_OFF /* use with DIAG_FLEXIBLE_SETHOLDKEY */
#define DIAG_FLEXIBLE_CMPLXT
#define DIAG_RECEIVE_MAP_OFF
/* Misc. diagnostics. */
#define DIAG_FEED_SESSION_OFF
#define DIAG_SEARCH_VOHSPACE_OFF
#define DIAG_GROUPS_OFF
#define DIAG_INTERFACE_OFF
#define DIAG_TIME_OFF
#define DIAG_INTERSECTION_OFF
#define DIAG_EFFECT
#define DIAG_HMAP_MESSAGES
/* hapmap wrap/unwrap diagnostics. */
#define DIAG_HMAPWRAP_OFF
#define DIAG_STF_OFF /* single to file */
#define DIAG_CTF /* compounded to file */
#define DIAG_STB_OFF /* single to buffer */
#define DIAG_CTB_OFF /* compounded to buffer */
#define DIAG_FF_OFF /* from file */
#define DIAG_FFS /* from file using given session name */
#define DIAG_FB_OFF /* from buffer */

#endif /* VRT_GLOBAL_H */
