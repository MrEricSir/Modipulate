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
#include "modipulate_common.h"
#include <portaudio.h>
#include "libmodplug-hacked/modplug.h"
#include "modipulate.h"


class ModStreamNote {
public:
    ModStreamNote();
    unsigned channel;
    int note;
    int instrument;
    int sample;
    int volume;
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
    
    void get_info(ModipulateSongInfo** info);
    void free_info(ModipulateSongInfo* info);
    
    // Returns the number of channels.
    int get_num_channels();
    
    // Gets title of song.
    std::string get_title();
    
    // Gets the "message" text associated with the song.
    std::string get_message();
    
    int get_default_tempo();
    
    // Get instrument info.
    unsigned get_num_instruments();
    std::string get_instrument_name(unsigned number);
    
    // Get sample info.
    unsigned get_num_samples();
    std::string get_sample_name(unsigned number);
    
    // Get/set volume. Between 0 and 1.0
    double get_volume();
    void set_volume(double vol);
    
    // Gets the # of patterns in this song.
    int get_num_patterns();
    
    // Gets the total rows in a given pattern.
    int get_rows_in_pattern(int pattern);
    
    // Override the tempo.
    void set_tempo_override(int tempo);
    int get_tempo_override();
    
    // Transposition offset.
    void set_transposition(int channel, int offset);
    int get_transposition(int channel);
    
    void set_pattern_change_cb(modipulate_song_pattern_change_cb cb, void* user_data);
    
    void set_row_change_cb(modipulate_song_row_change_cb cb, void* user_data);
    
    void set_note_change_cb(modipulate_song_note_cb cb, void* user_data);
    
    void perform_callbacks();
    
protected:
    void on_note_change(unsigned channel, int note, int instrument, int sample, int volume);
    void on_pattern_changed(unsigned pattern);
    void on_row_changed(int row);
    void increase_sample_count(int add);
    void on_tempo_changed(int tempo);
    
private:
    void check_error(int line, PaError err);
    
    int audio_callback(const void *input, void *output, unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
    
    void stream_finished_callback();
    
    void get_current_time(timespec& time);
    void time_diff(timespec& result, const timespec& start, const timespec& end);
    void time_add(timespec& result, const timespec& time1, const timespec& time2);
    
    HackedModPlugFile* modplug_file; // handle to file
    unsigned long file_length;  // length of file
    char* buffer; // file data
    const static int sampling_rate = 44100; // don't change this directly, need to call modplug for that
    bool stream_started;
    unsigned long long samples_played; // Samples played thus far.
    timespec song_start; // Time the song started.
    timespec pause_start; // Time when we started being paused.
    int last_tempo_read; // Last tempo we encountered.
    int tempo_override; // tempo override (-1 means disabled)
    
    int default_tempo;
    
    modipulate_song_pattern_change_cb pattern_cb;
    void* pattern_user_data;
    
    modipulate_song_row_change_cb row_cb;
    void* row_user_data;
    
    modipulate_song_note_cb note_cb;
    void* note_user_data;
    
    PaStream *stream;
    
    // Current row for building data structures. 
    ModStreamRow* current_row;
    
    // Cached data for upcoming callbacks.
    std::queue<ModStreamRow*> rows;
    
    friend class HackedCSoundFile;
    
    friend int mod_stream_callback(const void *input, void *output, unsigned long frameCount, 
        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
    
    friend void mod_stream_callback_finished(void* userData);
};

#endif // MODSTREAM_H