/* dialogX11.h
   Copyright (C) 2012, 2013 J. A. Green <green8@sdf-eu.org>
   license: GNU GPL v3, see COPYING, otherwise see vrtater.c
*/

#ifndef DIALOGX11_H
#define DIALOGX11_H

#include "hmap.h"

void refresh_dialog_interfaces(void);
void node_partial_dialog(select_t *, session_t *partial);
int *node_orgin_dialog(select_t *);

#endif /* DIALOGX11_H */
