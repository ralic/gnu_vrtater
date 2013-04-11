/* generator.h 
   Copyright (C) 2012, 2013 J. A. Green <green8@sdf-eu.org>
   license: GNU GPL v3, see COPYING, otherwise see vrtater.c
*/

#ifndef VRT_GENERATOR_H
#define VRT_GENERATOR_H

void init_generator(void);
int generate_node(void);
void generate_vobspace(void);
void regenerate_scene(int *);
void close_vobspace(double time_till_closed);
void close_node(void);
/* generator supported calls for tug input */
void generator_hapticNormill(void);
void generator_intersection(void);

#endif /* VRT_GENERATOR_H */
