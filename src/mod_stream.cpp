#include "mod_stream.h"
#include "modipulate_common.h"
#include "libmodplug-hacked/modplug.h"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <malloc.h>
#include <sstream>
#include <math.h>
#include <errno.h>

#include <AL/al.h>
#include <ogg/ogg.h>

using namespace std;

extern void call_note_changed(unsigned channel, int note, int instrument, int sample, int volume);
extern void call_pattern_changed(unsigned pattern);
extern void call_row_changed(int row);
extern void call_tempo_changed(int tempo);


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
    ModPlug_SetSettings(&settings);

    // Stereo.
    format = AL_FORMAT_STEREO16;
    
    // Initialize buffers.
    alGenBuffers(NUM_BUFFERS, buffers);
    check_error(__LINE__);
    alGenSources(1, &source);
    check_error(__LINE__);
    
    alSource3f(source, AL_POSITION,        0.0, 0.0, 0.0);
    alSource3f(source, AL_VELOCITY,        0.0, 0.0, 0.0);
    alSource3f(source, AL_DIRECTION,       0.0, 0.0, 0.0);
    alSourcef (source, AL_ROLLOFF_FACTOR,  0.0          );
    alSourcei (source, AL_SOURCE_RELATIVE, AL_TRUE      );
    
    set_playing(true);
}

void ModStream::close() {
    alSourceStop(source);
    check_error(__LINE__);
    empty();
    alDeleteBuffers(NUM_BUFFERS, buffers);
    check_error(__LINE__);
    alDeleteSources(1, &source);
    check_error(__LINE__);
    
    ModPlug_Unload(modplug_file);
}

bool ModStream::playback() {
    if (!playing)
        return true;
    
    if (is_playing())
        return true;
    
    for (int i = 0; i < NUM_BUFFERS; i++)
        if (!stream(buffers[i]))
            return false;
    
    alSourceQueueBuffers(source, NUM_BUFFERS, buffers);
    alSourcePlay(source);
    
    if (clock_gettime(CLOCK_MONOTONIC, &song_start) == -1)
        DPRINT("Error getting time: %d", errno);
    
    perform_callbacks();
    
    return true;
}

void ModStream::set_playing(bool is_playing) {
    playing = is_playing;
    if (is_playing)
        alSourcePlay(source);
    else
        alSourcePause(source);
    
    check_error(__LINE__);
}

bool ModStream::is_playing() {
    ALenum state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    check_error(__LINE__);
    return AL_PLAYING == state && playing;
}

bool ModStream::update() {
    static bool in_update = false; // anti-reentrancy hack
    int processed;
    bool active = true;
    
    if (!is_playing())
        return false;
    
    if (in_update)
        return true;
    
    in_update = true;
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
    while (processed--) {
        ALuint buffer;
        
        alSourceUnqueueBuffers(source, 1, &buffer);
        check_error(__LINE__);

        active = stream(buffer);
        
        alSourceQueueBuffers(source, 1, &buffer);
        check_error(__LINE__);
    }
    
    // Activate cached callbacks.
    perform_callbacks();
    
    in_update = false;
    return active;
}

bool ModStream::stream(ALuint buffer) {
    if (!playing)
        return false;
    char data[BUFFER_SIZE];
    
    int size = HackedModPlug_Read(modplug_file, data, BUFFER_SIZE);
    if (size < 0) {
        stringstream ss;
        ss << "Error reading from modplug " << size;
        throw string(ss.str());
    } else if (size == 0)
        return false; // end of stream
    
    alBufferData(buffer, format, data, size, sampling_rate);
    check_error(__LINE__);
    
    return true;
}

void ModStream::empty() {
    int queued;
    
    alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
    
    while(queued--) {
        ALuint buffer;
    
        alSourceUnqueueBuffers(source, 1, &buffer);
        check_error(__LINE__);
    }
}

void ModStream::check_error(int line) {
    int error = alGetError();
    string serror = "";
    
    switch(error) {
        break;

        case AL_INVALID_NAME:
            serror = "AL_INVALID_NAME";
        break;

        case AL_INVALID_ENUM:
            serror = "AL_INVALID_ENUM";
        break;

        case AL_INVALID_VALUE:
            serror = "AL_INVALID_VALUE";
        break;

        case AL_INVALID_OPERATION:
            serror = "AL_INVALID_OPERATION";
        break;

        case AL_OUT_OF_MEMORY:
            serror = "AL_OUT_OF_MEMORY";
        break;
    }
    
    if (error != AL_NO_ERROR) {
        stringstream ss;
        ss << "OpenAL error was raised: ";
        if (serror != "")
            ss << serror;
        else
            ss << error;
        ss << " at line " << line;
        throw string(ss.str());
    }
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

void ModStream::perform_callbacks() {
    // Make sure there's something to do!
    if (rows.empty())
        return;
    
    // Check time since we started the music.
    timespec current_time; // Current time.
    timespec since_start;  // Time since start.
    if (clock_gettime(CLOCK_MONOTONIC, &current_time) == -1)
        DPRINT("Error getting time: %d", errno);
    
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
        if (r->change_pattern != -1)
            call_pattern_changed(r->change_pattern);
        
        // 2. Row change callback.
        call_row_changed(r->row);
        
        // 3. Tempo change callback.
        if (r->change_tempo != -1)
            call_tempo_changed(r->change_tempo);
        
        // 4. Note change callbacks.
        if (r->notes.size() > 0) {
            list<ModStreamNote*>::iterator it;
            
            for (it = r->notes.begin(); it != r->notes.end(); it++) {
                ModStreamNote* n = (*it);
                
                call_note_changed(n->channel, n->note, n->instrument, n->sample, n->volume);
                
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

int ModStream::get_current_row() {
    return ModPlug_GetCurrentRow(modplug_file);
}

int ModStream::get_current_pattern() {
    return ModPlug_GetCurrentPattern(modplug_file);
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
