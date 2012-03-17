#include "mod_stream.h"
#include "libmodplug-hacked/modplug.h"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <malloc.h>
#include <sstream>
#include <math.h>
#include <errno.h>

#include <portaudio.h>

using namespace std;


// Callback helper functions.
int mod_stream_callback(const void *input, void *output, unsigned long frameCount, 
    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
    return ((ModStream*) userData)->audio_callback(input, output, frameCount, timeInfo, statusFlags);
}

void mod_stream_callback_finished(void* userData) {
    ((ModStream*) userData)->stream_finished_callback();
}


ModStreamRow::ModStreamRow() {
    change_tempo = -1;
    change_pattern = -1;
    samples_since_last = 0;
}


void ModStreamRow::add_note(ModStreamNote* n) {
    notes.push_back(n);
}


ModStreamNote::ModStreamNote() {
    channel = 0;
    note = -1;
    instrument = -1;
    sample = -1;
    volume = -1;
}


ModStream::ModStream() {
    modplug_file = NULL;
    file_length = 0;
    last_tempo_read = -1;
    tempo_override = -1;
    stream_started = false;
    
    pattern_cb = NULL;
    pattern_user_data = NULL;
    
    row_cb = NULL;
    row_user_data = NULL;
    
    note_cb = NULL;
    note_user_data = NULL;
    
    stream = NULL;
}


ModStream::~ModStream() {
}


void ModStream::open(string path) {
    DPRINT("Opening: %s", path.c_str());
    FILE *file;
    
    // Open file and load into memory.
    file = fopen(path.c_str(), "rb");
    if (!file)
        throw string("Unable to open file: " + path);
    
    // Fseek to get file length.
    fseek(file, 0, SEEK_END);
    file_length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate memory
    buffer = (char*)malloc(file_length + 1);
    if (!buffer) {
        fclose(file);
        throw string("Out of memory");
    }
    
    // Read file contents into buffer.
    fread(buffer, file_length, 1, file);
    fclose(file);
    
    DPRINT("Loaded mod! File length is: %lu", file_length);
    
    // Save pointer.
    modplug_file->mSoundFile.mod_stream = this;
    
    // Allocate the current row.
    current_row = new ModStreamRow();
    
    samples_played = 0;
    
    modplug_file = HackedModPlug_Load(buffer, file_length + 1);
    if (modplug_file == NULL)
        throw("File was loaded, modplug couldn't parse it.");
    
    // Setup playback format.
    ModPlug_Settings settings;
    ModPlug_GetSettings(&settings);
    settings.mFrequency = sampling_rate;
    settings.mLoopCount = -1;
    settings.mBits = 32;
    settings.mChannels = 2;
    ModPlug_SetSettings(&settings);
    
    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) {
        DPRINT("Error: No default output device.");
    }
    outputParameters.channelCount = 2;
    outputParameters.sampleFormat = paInt16;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    
    check_error(__LINE__, Pa_OpenStream(
              &stream,
              NULL, /* no input */
              &outputParameters,
              sampling_rate,
              paFramesPerBufferUnspecified,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              mod_stream_callback,
              this));
    
    check_error(__LINE__, Pa_SetStreamFinishedCallback(stream, &mod_stream_callback_finished));
    
    default_tempo = ModPlug_GetCurrentTempo(modplug_file);
}


void ModStream::close() {
    check_error(__LINE__, Pa_StopStream(stream));
    check_error(__LINE__, Pa_CloseStream(stream));
    
    ModPlug_Unload(modplug_file);
}


void ModStream::set_playing(bool play) {
    if (is_playing() == play) {
        // Nothing to do.
        return;
    } else if (play) {
        check_error(__LINE__, Pa_StartStream(stream));
        time_add(song_start, song_start, pause_start); // add paused time to start
        stream_started = true;
    } else if (!play) {
        check_error(__LINE__, Pa_StopStream(stream));
        get_current_time(pause_start);
    }
}

bool ModStream::is_playing() {
    if (!stream_started)
        return false;
    
    return (Pa_IsStreamActive(stream) == 1);
}


int ModStream::audio_callback(const void *input, void *output, unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags) {
    
    int size = HackedModPlug_Read(modplug_file, output, frameCount*4);
    if (size < 0) {
        stringstream ss;
        ss << "Error reading from modplug " << size;
        throw string(ss.str());
    } else if (size == 0)
        return paAbort; // end of stream
    
    return paContinue;
}


void ModStream::stream_finished_callback() {
    DPRINT("PA: Stream finished.");
}


void ModStream::check_error(int line, PaError err) {
    if (err != paNoError) {
        stringstream ss;
        ss << "PortAudio error #";
        ss << err;
        ss << " ";
        ss << Pa_GetErrorText( err );
        ss << " at line " << line;
        throw string(ss.str());
    }
}


void ModStream::get_info(ModipulateSongInfo** _info) {
    ModipulateSongInfo* song_info = new ModipulateSongInfo;
    
    song_info->num_channels = get_num_channels();
    song_info->num_instruments = get_num_instruments();
    song_info->num_samples = get_num_samples();
    song_info->num_patterns = get_num_patterns();
    song_info->default_tempo = get_default_tempo();
    
    song_info->title = modipulate_make_message("%s", get_title().c_str());
    song_info->message = modipulate_make_message("%s", get_message().c_str());
    
    song_info->instrument_names = new char*[song_info->num_instruments + 1];
    song_info->sample_names = new char*[song_info->num_samples];
    song_info->rows_per_pattern = new int[song_info->num_patterns];
    
    for (int instrument = 1; instrument <= song_info->num_instruments; instrument++)
        song_info->instrument_names[instrument] = modipulate_make_message("%s", 
            get_instrument_name(instrument).c_str());
    
    for (int sample = 0; sample < song_info->num_samples; sample++)
        song_info->sample_names[sample] = modipulate_make_message("%s", 
            get_sample_name(sample).c_str());
    
    for (int pattern = 0; pattern < song_info->num_patterns; pattern++)
        song_info->rows_per_pattern[pattern] = get_rows_in_pattern(pattern);
    
    // Return parameter.
    *_info = song_info;
}


void ModStream::free_info(ModipulateSongInfo* info) {
    for (int instrument = 0; instrument < info->num_instruments; instrument++)
        delete info->instrument_names[instrument];
    
    for (int sample = 0; sample < info->num_samples; sample++)
        delete info->sample_names[sample];
    
    delete info->instrument_names;
    delete info->sample_names;
    delete info->rows_per_pattern;
    
    delete info;
}


// Enable or disable channels.
void ModStream::set_channel_enabled(int channel, bool is_enabled) {
    modplug_file->mSoundFile.enabled_channels[channel] = is_enabled;
}


bool ModStream::get_channel_enabled(int channel) {
    return modplug_file->mSoundFile.enabled_channels[channel];
}


// Returns the number of channels.
int ModStream::get_num_channels() {
    return (int) ModPlug_NumChannels(modplug_file);
}

int ModStream::get_default_tempo() {
    return default_tempo;
}

void ModStream::set_pattern_change_cb(modipulate_song_pattern_change_cb cb, void* user_data) {
    pattern_cb = cb;
    pattern_user_data = user_data;
}


void ModStream::set_row_change_cb(modipulate_song_row_change_cb cb, void* user_data) {
    row_cb = cb;
    row_user_data = user_data;
}


void ModStream::set_note_change_cb(modipulate_song_note_cb cb, void* user_data) {
    note_cb = cb;
    note_user_data = user_data;
}


void ModStream::perform_callbacks() {
    // Make sure there's something to do!
    if (rows.empty() || !is_playing())
        return;
    
    // Check time since we started the music.
    timespec current_time; // Current time.
    timespec since_start;  // Time since start.
    get_current_time(current_time);
    time_diff(since_start, song_start, current_time);
    
    // Convert time to sample number.
    unsigned long long samples_since_start = since_start.tv_sec * sampling_rate + 
        (unsigned long long) ( ((double) (since_start.tv_nsec) * ((double)sampling_rate) / 1000000000.0)); // 1 / one billion = 1 ns
    
    while (!rows.empty()) {
        // Process callbacks from row data.
        ModStreamRow* r = rows.front();
        
        unsigned long long sample_counter = samples_played + r->samples_since_last;
        if (sample_counter > samples_since_start)
            break; // done (for now!)
        
        samples_played += r->samples_since_last;
        
        rows.pop();
        
        // 1. Pattern change callback.
        if (r->change_pattern != -1 && pattern_cb != NULL)
            pattern_cb(this, r->change_pattern, pattern_user_data);
        
        // 2. Row change callback.
        if (row_cb != NULL)
            row_cb(this, r->row, row_user_data);
        
        // 3. Note change callbacks.
        if (r->notes.size() > 0) {
            list<ModStreamNote*>::iterator it;
            
            for (it = r->notes.begin(); it != r->notes.end(); it++) {
                ModStreamNote* n = (*it);
                
                if (note_cb != NULL)
                    note_cb(this, n->channel, n->note, n->instrument, n->sample, 0, 0, 0, 0, note_user_data);
                
                // TODO: what is n->volume? do we need it?
                //call_note_changed(n->channel, n->note, n->instrument, n->sample, n->volume);
                
                delete n;
            }
        }
        
        delete r;
    }
}

void ModStream::on_note_change(unsigned channel, int note, int instrument, int sample, int volume) {
    ModStreamNote* n = new ModStreamNote();
    n->channel = channel;
    n->note = note;
    n->instrument = instrument;
    n->sample = sample;
    n->volume = volume;
    current_row->add_note(n);
}


void ModStream::on_pattern_changed(unsigned pattern) {
    current_row->change_pattern = (int) pattern;
}


void ModStream::on_row_changed(int row) {
    rows.push(current_row);
    current_row = new ModStreamRow();
    current_row->row = row;
}


void ModStream::on_tempo_changed(int tempo) {
    if (tempo != last_tempo_read) {
        current_row->change_tempo = tempo;
        last_tempo_read = tempo;
    }
}

void ModStream::increase_sample_count(int add) {
    current_row->samples_since_last += add;
}


std::string ModStream::get_title() {
    const char* name = ModPlug_GetName(modplug_file);
    return name != NULL ? string(name) : string("");
}


std::string ModStream::get_message() {
    const char* message = ModPlug_GetMessage(modplug_file);
    return (message != NULL) ? string(message) : string("");
}


double ModStream::get_volume() {
    return ((double) (ModPlug_GetMasterVolume(modplug_file) - 1)) / 511.0;
}


void ModStream::set_volume(double vol) {
    if (vol < 0.)
        vol = 0.;
    else if (vol > 1.)
        vol = 1.;
    ModPlug_SetMasterVolume(modplug_file, round(vol * 511) + 1);
}

unsigned ModStream::get_num_instruments() {
    return ModPlug_NumInstruments(modplug_file);
}


std::string ModStream::get_instrument_name(unsigned number) {
    std::string ret = "";
    unsigned length = ModPlug_InstrumentName(modplug_file, number, NULL);
    if (length == 0)
        return ret;
    
    char* buffer = new char[length+2];
    for (unsigned i = 0; i < length+2; i++)
        buffer[i] = 0;
    if (length == ModPlug_InstrumentName(modplug_file, number, buffer))
        ret = string(buffer);
    
    delete [] buffer;
    return ret;
}


unsigned ModStream::get_num_samples() {
    return ModPlug_NumSamples(modplug_file);
}


std::string ModStream::get_sample_name(unsigned number) {
    std::string ret = "";
    unsigned length = ModPlug_SampleName(modplug_file, number, NULL);
    if (length == 0)
        return ret;
    
    char* buffer = new char[length+2];
    for (unsigned i = 0; i < length+2; i++)
        buffer[i] = 0;
    if (length == ModPlug_SampleName(modplug_file, number, buffer))
        ret = string(buffer);
    
    delete [] buffer;
    return ret;
}


int ModStream::get_num_patterns() {
    return ModPlug_NumPatterns(modplug_file);
}


int ModStream::get_rows_in_pattern(int pattern) {
    unsigned int num_rows = 0;
    if (NULL == ModPlug_GetPattern(modplug_file, pattern, &num_rows))
        return -1;
    
    return (int) num_rows;
}


void ModStream::set_tempo_override(int tempo) {
    tempo_override = tempo;
}


int ModStream::get_tempo_override() {
    return tempo_override;
}


void ModStream::set_transposition(int channel, int offset) {
    modplug_file->mSoundFile.transposition_offset[channel] = offset;
}

int ModStream::get_transposition(int channel) {
    return modplug_file->mSoundFile.transposition_offset[channel];
}


// Grabs current timespec val.
void ModStream::get_current_time(timespec& time) {
    if (clock_gettime(CLOCK_MONOTONIC, &time) == -1)
        DPRINT("Error getting time: %d", errno);
}


// Based on code from here:
// http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/
void ModStream::time_diff(timespec& result, const timespec& start, const timespec& end) {
    if ((end.tv_nsec-start.tv_nsec)<0) {
        result.tv_sec = end.tv_sec-start.tv_sec-1;
        result.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        result.tv_sec = end.tv_sec-start.tv_sec;
        result.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
}


// Based on code from here:
// http://www.geonius.com/software/libgpl/ts_util.html
void ModStream::time_add(timespec& result, const timespec& time1, const timespec& time2) {
    result.tv_sec = time1.tv_sec + time2.tv_sec;
    result.tv_nsec = time1.tv_nsec + time2.tv_nsec;
    if (result.tv_nsec >= 1000000000L) {
        result.tv_sec++;
        result.tv_nsec = result.tv_nsec - 1000000000L;
    }
}
