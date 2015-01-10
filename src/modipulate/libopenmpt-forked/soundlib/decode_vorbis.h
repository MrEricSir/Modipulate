#ifndef _DECODE_VORBIS_H
#define _DECODE_VORBIS_H

#include <stdint.h>

int decode_vorbis(const char* const inbuf, size_t bufsize, int16_t* outbuf);

#endif
