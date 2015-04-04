/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2009             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

    Vorbis decoder ripped off from the sample code
    provided on the Ogg Vorbis website.

 ********************************************************************/


#include <vorbis/codec.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "decode_vorbis.h"

#define BLOCK_SIZE 4096

int decode_vorbis(const char* const inbuf, std::size_t bufsize, int16_t* outbuf)
{
    ogg_int16_t convbuffer[BLOCK_SIZE];
    int convsize = BLOCK_SIZE;

    ogg_sync_state   oy; /* sync and verify incoming physical bitstream */
    ogg_stream_state os; /* take physical pages, weld into a logical stream of packets */
    ogg_page         og; /* one Ogg bitstream page. Vorbis packets are inside */
    ogg_packet       op; /* one raw packet of data for decode */
    vorbis_info      vi; /* struct that stores all the static vorbis bitstream settings */
    vorbis_comment   vc; /* struct that stores all the bitstream user comments */
    vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
    vorbis_block     vb; /* local working space for packet->PCM decode */

    char* buffer;
    int   bytes;
    const char* bufpos = inbuf;

    /********** Decode setup ************/

    ogg_sync_init(&oy);
    
    while (1) {
        int eos = 0;
        int i;

        /* submit a 4k block to libvorbis' Ogg layer */
        buffer = ogg_sync_buffer(&oy, BLOCK_SIZE);
        bytes = BLOCK_SIZE > bufsize ? bufsize : BLOCK_SIZE;
        memcpy(buffer, bufpos, bytes);
        bufpos += bytes;
        bufsize -= bytes;
        ogg_sync_wrote(&oy, bytes);
        
        /* Get the first page. */
        if (ogg_sync_pageout(&oy, &og) != 1) {
            if (bytes < BLOCK_SIZE)
                break;
            // fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");
            exit(1);
        }
    
        /* Get the serial number and set up the rest of decode. */
        ogg_stream_init(&os, ogg_page_serialno(&og));

        vorbis_info_init(&vi);
        vorbis_comment_init(&vc);
        if (ogg_stream_pagein(&os, &og) < 0) { 
            // fprintf(stderr,"Error reading first page of Ogg bitstream data.\n");
            exit(1);
        }
        
        if (ogg_stream_packetout(&os, &op) != 1) { 
            // fprintf(stderr,"Error reading initial header packet.\n");
            exit(1);
        }
        
        if (vorbis_synthesis_headerin(&vi, &vc, &op) < 0) { 
            // fprintf(stderr,"This Ogg bitstream does not contain Vorbis "
                            // "audio data.\n");
            exit(1);
        }
        
        i = 0;
        while (i < 2) {
            while(i < 2) {
                int result = ogg_sync_pageout(&oy, &og);
                if (result == 0)
                    break;
                if (result == 1) {
                    ogg_stream_pagein(&os, &og);
                    while (i < 2) {
                        result = ogg_stream_packetout(&os, &op);
                        if (result == 0)
                            break;
                        if (result < 0) {
                            // fprintf(stderr,"Corrupt secondary header.  Exiting.\n");
                            exit(1);
                        }
                        result = vorbis_synthesis_headerin(&vi, &vc, &op);
                        if (result < 0) {
                            // fprintf(stderr,"Corrupt secondary header.  Exiting.\n");
                            exit(1);
                        }
                        i++;
                    }
                }
            }
            buffer = ogg_sync_buffer(&oy, BLOCK_SIZE);
            bytes = BLOCK_SIZE > bufsize ? bufsize : BLOCK_SIZE;
            memcpy(buffer, bufpos, bytes);
            bufpos += bytes;
            bufsize -= bytes;
            if (bytes == 0 && i < 2) {
                // fprintf(stderr,"End of file before finding all Vorbis headers!\n");
                exit(1);
            }
            ogg_sync_wrote(&oy, bytes);
        }
        
        {
            char **ptr = vc.user_comments;
            while (*ptr) {
                // fprintf(stderr, "%s\n", *ptr);
                ++ptr;
            }
            // fprintf(stderr, "\nBitstream is %d channel, %ldHz\n", vi.channels, vi.rate);
            // fprintf(stderr, "Encoded by: %s\n\n", vc.vendor);
        }
        
        convsize = BLOCK_SIZE / vi.channels;

        if (vorbis_synthesis_init(&vd, &vi) == 0) {
            vorbis_block_init(&vd, &vb);

            /* The rest is just a straight decode loop until end of stream */
            while (!eos) {
                while (!eos) {
                    int result = ogg_sync_pageout(&oy, &og);
                    if (result == 0)
                        break;
                    if (result < 0) {
                        // fprintf(stderr,"Corrupt or missing data in bitstream; "
                        //                 "continuing...\n");
                    } else {
                        ogg_stream_pagein(&os, &og);
                        while(1) {
                            result = ogg_stream_packetout(&os, &op);
                            
                            if (result == 0)
                                break;
                            if (result < 0) {
                                /* nothing */
                            } else {
                                /* we have a packet.  Decode it */
                                float **pcm;
                                int samples;
                                
                                if (vorbis_synthesis(&vb, &op) == 0)
                                    vorbis_synthesis_blockin(&vd, &vb);

                                while ((samples = vorbis_synthesis_pcmout(&vd, &pcm)) > 0) {
                                    int j;
                                    int clipflag = 0;
                                    int bout = (samples < convsize ? samples : convsize);

                                    for (i = 0; i < vi.channels; i++) {
                                        ogg_int16_t *ptr = convbuffer + i;
                                        float *mono = pcm[i];
                                        for (j = 0; j < bout; j++) {
                                            int val = floor(mono[j] * 32767.f + .5f);
                                            /* guard against clipping */
                                            if (val > 32767) {
                                                val = 32767;
                                                clipflag = 1;
                                            }
                                            if (val < -32768) {
                                                val = -32768;
                                                clipflag = 1;
                                            }
                                            *ptr = val;
                                            ptr += vi.channels;
                                        }
                                    }
                                    
                                    // if (clipflag)
                                    //     fprintf(stderr, "Clipping in frame %ld\n", (long)(vd.sequence));

                                    memcpy(outbuf, convbuffer, vi.channels * bout * sizeof (int16_t));
                                    outbuf += vi.channels * bout;

                                    vorbis_synthesis_read(&vd, bout);
                                }
                            }
                        }
                        if (ogg_page_eos(&og))
                            eos = 1;
                    }
                }
                if (!eos) {
                    buffer = ogg_sync_buffer(&oy, BLOCK_SIZE);
                    bytes = BLOCK_SIZE > bufsize ? bufsize : BLOCK_SIZE;
                    memcpy(buffer, bufpos, bytes);
                    bufpos += bytes;
                    bufsize -= bytes;
                    ogg_sync_wrote(&oy, bytes);
                    if (bytes == 0)
                        eos = 1;
                }
            }

            vorbis_block_clear(&vb);
            vorbis_dsp_clear(&vd);
        } else {
            // fprintf(stderr,"Error: Corrupt header during playback initialization.\n");
        }

        ogg_stream_clear(&os);
        vorbis_comment_clear(&vc);
        vorbis_info_clear(&vi);
    }

    ogg_sync_clear(&oy);
    
    // fprintf(stderr,"Done.\n");
    return 0;
}
