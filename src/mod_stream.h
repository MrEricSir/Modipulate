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

#include <time.h>
#include <string>
#include <list>
#include <queue>
#include <AL/al.h>
#include "libmodplug-hacked/modplug.h"


class ModStreamNote {
public:
    ModStreamNote();
    unsigned channel;
    int note;
    int instrument;
    int sample;
};

class ModStreamRow {
public:
    ModStreamRow();
    void add_note(ModStreamNote* n);
    
    int row;                         // Row #
    
    unsigned samples_since_last;     // # samples since last row change.
    
    int change_tempo;                // Positive on tempo change: represents new tempo.
    int change_pattern;              // Positive on pattern change: represents new pattern number.
    
    std::list<ModStreamNote*> notes;  // List of notes for this row.
};



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
    void on_row_changed(int row);
    void increase_sample_count(int add);
    
private:
    bool stream(ALuint buffer);
    void empty();
    void check_error(int line);
    void perform_callbacks();
    void time_diff(timespec& result, const timespec& start, const timespec& end);
    
    HackedModPlugFile* modplug_file; // handle to file
    unsigned long file_length;  // length of file
    char* buffer; // file data
    const static int sampling_rate = 44100; // don't change this directly, need to call modplug for that
    const static int NUM_BUFFERS = 2;
    bool playing;
    unsigned long long samples_played; // Samples played thus far.
    timespec song_start; // Time the song started.
    
    ALuint buffers[NUM_BUFFERS];
    ALuint source;
    ALenum format;
    
    // Current row for building data structures. 
    ModStreamRow* current_row;
    
    // Cached data for upcoming callbacks.
    std::queue<ModStreamRow*> rows;
    
    friend class HackedCSoundFile;
};

#endif // MODSTREAM_H
