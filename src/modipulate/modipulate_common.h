/* Copyright 2011-2015 Eric Gregory and Stevie Hryciw
 *
 * Modipulate.
 * https://github.com/MrEricSir/Modipulate/
 *
 * Modipulate is released under the BSD license.  See LICENSE for details.
 */

#ifndef MODIPULATE_COMMON_H
#define MODIPULATE_COMMON_H

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h> 
#include "modipulate.h"
#include <string>
#include <portaudio.h>

// For printing. When release mode becomes available, this will likely do nothing.
#ifdef DEBUG
#define DPRINT(...) {\
printf("Modipulate (%s:%d): ", __FUNCTION__, __LINE__);\
printf(__VA_ARGS__);\
printf("\n");\
}
#else
#define DPRINT(...)
#endif



// From the man page for snprintf
char* modipulate_make_message(const char *fmt, ...);

void modipulate_set_error_string(const char* fmt, ...);
void modipulate_set_error_string_cpp(std::string err);

// Checks for a PortAudio error, returns a Modipulate error and sets
// the last error string.
ModipulateErr modipulate_handle_pa_error(PaError err);


#endif
