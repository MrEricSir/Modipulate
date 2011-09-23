/* Copyright 2011 Eric Gregory
 *
 * This software is licensed under the GNU LGPL (version 3 or later).
 * See the COPYING.LESSER file in this distribution. 
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 

#ifndef __MODIPULATE_H__
#define __MODIPULATE_H__


// Size of audio buffer.
#define MIN_BUFFER_SIZE 4096
#define BUFFER_SIZE (MIN_BUFFER_SIZE * 8)

// For printing. When release mode becomes available, this will likely do nothing.
#define DPRINT(...) {\
printf("Modipulate (%s:%d): ", __FUNCTION__, __LINE__);\
printf(__VA_ARGS__);\
printf("\n");\
}

#endif
