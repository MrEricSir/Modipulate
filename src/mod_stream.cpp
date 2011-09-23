#include "mod_stream.h"
#include "modipulate_common.h"
#include "libmodplug-hacked/modplug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <malloc.h>
#include <sstream>

#include <AL/al.h>
#include <ogg/ogg.h>

using namespace std;

extern void call_note_changed(unsigned channel, int note);
extern void call_pattern_changed(unsigned pattern);

ModStream::ModStream() {
    modplug_file = NULL;
    file_length = 0;
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
    
    DPRINT("Name of track: %s", ModPlug_GetName(modplug_file));
    
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
    DPRINT("set playing: %d", (int) is_playing);
    // TODO: seems like we need a better way to start/stop
    //alSourcei(source, AL_SOURCE_STATE, is_playing ? AL_PLAYING : AL_PAUSED);
    check_error(__LINE__);
}

bool ModStream::is_playing() {
    ALenum state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    check_error(__LINE__);
    return AL_PLAYING == state;
}

bool ModStream::update() {
    int processed;
    bool active = true;
    
    if (!is_playing())
        return false;
    
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
    while (processed--) {
        ALuint buffer;
        
        alSourceUnqueueBuffers(source, 1, &buffer);
        check_error(__LINE__);

        active = stream(buffer);

        alSourceQueueBuffers(source, 1, &buffer);
        check_error(__LINE__);
    }
    
    return active;
}

bool ModStream::stream(ALuint buffer) {
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
    if (error != AL_NO_ERROR) {
        stringstream ss;
        ss << "OpenAL error was raised: " << error << " at line " << line;
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

void ModStream::on_note_change(unsigned channel, int note) {
    call_note_changed(channel, note);
}

void ModStream::on_pattern_changed(unsigned pattern) {
    call_pattern_changed(pattern);
}

