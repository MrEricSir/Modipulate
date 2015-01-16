#ifndef _DECODE_VORBIS_H
#define _DECODE_VORBIS_H

#include <cstdint>
#include <cstdio>
#include <cstddef>

int decode_vorbis(const char* const inbuf, std::size_t bufsize, int16_t* outbuf);
int encode_vorbis(const char* const inbuf, std::size_t len, FILE* outfile,
    const uint8_t channels, const uint8_t bitdepth, const uint32_t rate, float quality);

#endif
