/* Copyright 2011 Eric Gregory
 *
 * This software is licensed under the GNU LGPL (version 3 or later).
 * See the COPYING.LESSER file in this distribution. 
 */

#ifndef MODSTREAM_H
#define MODSTREAM_H

#include <string>
#include <AL/al.h>
#include "libmodplug-hacked/modplug.h"

// Uses modplug-hacked to stream audio via OpenAL.
// Same pattern as the ogg_stream class from: 
//    http://www.devmaster.net/articles/openal-tutorials/lesson8.php

class ModStream {

public:
    ModStream();
    ~ModStream();
    
    // Opens a file.
    void open(std::string path);
    
    // Closes the file.
    void close();
    
    // Updates audio stream. Must be called repeatedly.
    bool update();
    
    // True if we're playing, false if we're stopped for
    // any reason.
    bool playback();
    
    // Enable/disable playing (default is enabled)
    void set_playing(bool is_playing);
    
    // Checks if we're supposed to be playing or not.
    bool is_playing();
    
    // Enable or disable channels.
    void set_channel_enabled(int channel, bool enabled);
    bool get_channel_enabled(int channel);
    
    // Returns the number of channels.
    int get_num_channels();
    
    void print(unsigned u, bool b);
    
protected:
    void on_note_change(unsigned channel, int note);
    void on_pattern_changed(unsigned pattern);
    
    
private:
    bool stream(ALuint buffer);
    void empty();
    void check_error(int line);
    
    HackedModPlugFile* modplug_file; // handle to file
    unsigned long file_length;  // length of file
    char* buffer; // file data
    const static int sampling_rate = 44100; // don't change this directly, need to call modplug for that

    ALuint buffers[2];
    ALuint source;
    ALenum format;
    
    friend class HackedCSoundFile;
};

#endif // MODSTREAM_H
