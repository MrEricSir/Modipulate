#include "mod_stream.h"
#include "modipulate_common.h"
#include "libmodplug-hacked/modplug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <malloc.h>
#include <sstream>
#include <math.h>

#include <AL/al.h>
#include <ogg/ogg.h>

using namespace std;

extern void call_note_changed(unsigned channel, int note, int instrument, int sample);
extern void call_pattern_changed(unsigned pattern);
extern void call_row_changed();

ModStream::ModStream() {
    modplug_file = NULL;
    file_length = 0;
    cache_pattern_change = -1;
    cache_row_change = false;
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
    
    modplug_file = HackedModPlug_Load(buffer, file_length + 1);
    if (modplug_file == NULL)
        throw("File was loaded, modplug couldn't parse it.");
    
    // Setup playback format.
    ModPlug_Settings settings;
    ModPlug_GetSettings(&settings);
    settings.mFrequency = sampling_rate;
    ModPlug_SetSettings(&settings);

    // Stereo.
    format = AL_FORMAT_STEREO16;
    
    // Initialize buffers.
    alGenBuffers(2, buffers);
    check_error(__LINE__);
    alGenSources(1, &source);
    check_error(__LINE__);
    
    alSource3f(source, AL_POSITION,        0.0, 0.0, 0.0);
    alSource3f(source, AL_VELOCITY,        0.0, 0.0, 0.0);
    alSource3f(source, AL_DIRECTION,       0.0, 0.0, 0.0);
    alSourcef (source, AL_ROLLOFF_FACTOR,  0.0          );
    alSourcei (source, AL_SOURCE_RELATIVE, AL_TRUE      );
    
    // Save pointer.
    modplug_file->mSoundFile.mod_stream = this;
    
    set_playing(true);
}

void ModStream::close() {
    alSourceStop(source);
    check_error(__LINE__);
    empty();
    alDeleteBuffers(2, buffers);
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
    
    if (!stream(buffers[0]))
        return false;
    
    if(!stream(buffers[1]))
        return false;
    
    alSourceQueueBuffers(source, 2, buffers);
    alSourcePlay(source);
    
    return true;
}

void ModStream::set_playing(bool is_playing) {
    playing = is_playing;
    //DPRINT("set playing: %d", (int) is_playing);
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
    // Pattern change.
    if (cache_pattern_change >= 0) {
        call_pattern_changed((unsigned) cache_pattern_change);
        cache_pattern_change = -1;
    }
    
    // Note change.
    if (cache_note_change.size() > 0) {
        list<NoteChange>::iterator it;
        
        for (it = cache_note_change.begin(); it != cache_note_change.end(); it++) {
            call_note_changed(it->channel, it->note, it->instrument, it->sample);
        }
        cache_note_change.clear();
    }
    
    if (cache_row_change) {
        call_row_changed();
        cache_row_change = false;
    }
}

void ModStream::on_note_change(unsigned channel, int note, int instrument, int sample) {
    NoteChange n;
    n.channel = channel;
    n.note = note;
    n.instrument = instrument;
    n.sample = sample;
    cache_note_change.push_back(n);
}

void ModStream::on_pattern_changed(unsigned pattern) {
    cache_pattern_change = pattern;
}

void ModStream::on_row_changed() {
    cache_row_change = true;
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