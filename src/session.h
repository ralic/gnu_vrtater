/* session.h
   Copyright (C) 2012, 2013 J. A. Green <green8@sdf-eu.org>
   license: GNU GPL v3, see COPYING, otherwise see vrtater.c
*/

#ifndef VRT_SESSION_H
#define VRT_SESSION_H

#include "hmap.h"

#define VRT_SCALE_SESSIONS_REQ 1 /* depends on system resources */
/* set these to taste */
#define VRT_OUTBOUND_SESSIONS_MIN 5
#define VRT_INBOUND_SESSIONS_MAX 15
/* calculate the rest
   adjust factors to taste */
#define VRT_MAX_RUNNING_SESSIONS \
(VRT_OUTBOUND_SESSIONS_MIN + VRT_INBOUND_SESSIONS_MAX * VRT_SCALE_SESSIONS_REQ)
#define VRT_MAX_CUED_SESSIONS (10 * VRT_MAX_RUNNING_SESSIONS)
#define VRT_MAX_CALLED_SESSIONS (100 * VRT_MAX_RUNNING_SESSIONS)
#define VRT_MAX_PREV_CALLER_SESSIONS (1000 * VRT_MAX_RUNNING_SESSIONS)

struct complextimate { /* these stats kept by code in generator.c */
	int hmap_count;
	int avg_vertices;
	int avg_map_sz;
};
typedef struct complextimate complextimate_t;

struct session_desc {
	session_t session;
	btoggles_t level;
	char adesc[81]; /* if called, cued */
	complextimate_t partial_vobspace;
};
typedef struct session_desc session_desc_t;

enum { /* session_desc_t level */
	VRT_ORDINAL_SESSION_CALLED,
#define VRT_MASK_SESSION_CALLED (1 << VRT_ORDINAL_SESSION_CALLED)
	VRT_ORDINAL_SESSION_CUED,
#define VRT_MASK_SESSION_CUED (1 << VRT_ORDINAL_SESSION_CUED)
	VRT_ORDINAL_SESSION_PARTIAL,
#define VRT_MASK_SESSION_PARTIAL (1 << VRT_ORDINAL_SESSION_PARTIAL)
	VRT_ORDINAL_SESSION_INBOUND
#define VRT_MASK_SESSION_INBOUND (1 << VRT_ORDINAL_SESSION_INBOUND)
};

struct prev_caller_sessions {
	session_t session;
};
typedef struct prev_caller_sessions prev_caller_sessions_t;

void init_sessions(void);
void session_sync(void);
session_desc_t *p_session_desc(void);
prev_caller_sessions_t *p_prev_caller_sessions(void);
int in_node_session(session_t *);
void list_nodes(char *);
int call_session(char *);
int on_node_session(session_t *);
int close_sessions(session_desc_t *);
void sendmaps_to_partial(session_t *s, hmapf_t *maps);
hmapf_t *recievemaps_from_partial(session_t *s, hmapf_t *maps);
void publish_partial(session_t *s, hmapf_t **maps);
hmapf_t *recieve_partial(session_t *s);

#endif /* VRT_SESSION_H */
