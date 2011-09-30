/* Copyright 2011 Eric Gregory and Stevie Hryciw
 *
 * Modipulate.
 * https://github.com/MrEricSir/Modipulate/
 *
 * This software is licensed under the GNU LGPL (version 3 or later).
 * See the COPYING.LESSER file in this distribution. 
 */

#ifndef MODSTREAM_H
#define MODSTREAM_H

#include <string>
#include <list>
#include <AL/al.h>
#include "libmodplug-hacked/modplug.h"

// Uses modplug-hacked to stream audio via OpenAL.
// Same pattern as the ogg_stream class from: 
//    http://www.devmaster.net/articles/openal-tutorials/lesson8.php

struct NoteChange {
    unsigned channel;
    int note;
    int instrument;
    int sample;
};

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
    
    // Gets title of song.
    std::string get_title();
    
    // Gets the "message" text associated with the song.
    std::string get_message();
    
    // Get instrument info.
    unsigned get_num_instruments();
    std::string get_instrument_name(unsigned number);
    
    // Get sample info.
    unsigned get_num_samples();
    std::string get_sample_name(unsigned number);
    
    
    // Get/set volume. Between 0 and 1.0
    double get_volume();
    void set_volume(double vol);
    
    
    // Gets the current row #.
    int get_current_row();
    
    // Gets the current pattern #.
    int get_current_pattern();
    
    // Gets the total rows in a given pattern.
    int get_rows_in_pattern(int pattern);
    
protected:
    void on_note_change(unsigned channel, int note, int instrument, int sample);
    void on_pattern_changed(unsigned pattern);
    void on_row_changed();
    
private:
    bool stream(ALuint buffer);
    void empty();
    void check_error(int line);
    void perform_callbacks();
    
    HackedModPlugFile* modplug_file; // handle to file
    unsigned long file_length;  // length of file
    char* buffer; // file data
    const static int sampling_rate = 44100; // don't change this directly, need to call modplug for that
    bool playing;

    ALuint buffers[2];
    ALuint source;
    ALenum format;
    
    // Cached callback data.
    long cache_pattern_change;
    std::list<NoteChange> cache_note_change;
    bool cache_row_change;
    
    friend class HackedCSoundFile;
};

#endif // MODSTREAM_H
