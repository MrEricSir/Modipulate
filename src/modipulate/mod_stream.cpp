#include "mod_stream.h"
#include "libmodplug-hacked/include/modplug.h"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
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

static void mod_stream_cb_on_pattern_changed(unsigned pattern, void* user_data) {
    ((ModStream*) user_data)->on_pattern_changed(pattern);
}

static void mod_stream_cb_on_tempo_changed(int tempo, void* user_data) {
    ((ModStream*) user_data)->on_tempo_changed(tempo);
}

static void mod_stream_cb_on_note_change(unsigned channel, int note, int instrument, int sample, int volume, void* user_data) {
    ((ModStream*) user_data)->on_note_change(channel, note, instrument, sample, volume);
}

static void mod_stream_cb_on_row_changed(int row, void* user_data) {
    ((ModStream*) user_data)->on_row_changed(row);
}

static void mod_stream_cb_increase_sample_count(int add, void* user_data) {
    ((ModStream*) user_data)->increase_sample_count(add);
}

static bool mod_stream_cb_is_volume_command_enabled(int channel, int volume_command, void* user_data) {
    return ((ModStream*) user_data)->is_volume_command_enabled(channel, volume_command);
}
static bool mod_stream_cb_is_volume_command_pending(unsigned channel, void* user_data) {
    return ((ModStream*) user_data)->is_volume_command_pending(channel);
}

static unsigned mod_stream_cb_pop_volume_command(unsigned channel, void* user_data) {
    return ((ModStream*) user_data)->pop_volume_command(channel);
}

static unsigned mod_stream_cb_pop_volume_parameter(unsigned channel, void* user_data) {
    return ((ModStream*) user_data)->pop_volume_parameter(channel);
}

static bool mod_stream_cb_is_effect_command_enabled(int channel, int effect_command, void* user_data) {
    return ((ModStream*) user_data)->is_effect_command_enabled(channel, effect_command);
}

static bool mod_stream_cb_is_effect_command_pending(unsigned channel, void* user_data) {
    return ((ModStream*) user_data)->is_effect_command_pending(channel);
}

static unsigned mod_stream_cb_pop_effect_command(unsigned channel, void* user_data) {
    return ((ModStream*) user_data)->pop_effect_command(channel);
}

static unsigned mod_stream_cb_pop_effect_parameter(unsigned channel, void* user_data)  {
    return ((ModStream*) user_data)->pop_effect_parameter(channel);
}


ModStreamRow::ModStreamRow() :
    samples_since_last(0),
    change_tempo(-1),
    change_pattern(-1)
{}


void ModStreamRow::add_note(ModStreamNote* n) {
    notes.push_back(n);
}


ModStreamNote::ModStreamNote() :
    channel(0),
    note(-1),
    instrument(-1),
    sample(-1),
    volume(-1)
{}

ModStream::ModStream() :
    modplug_file(NULL),
    file_length(0),
    stream_started(false),
    last_tempo_read(-1),
    tempo_override(-1),
    
    pattern_cb(NULL),
    pattern_user_data(NULL),
    
    row_cb(NULL),
    row_user_data(NULL),
    
    note_cb(NULL),
    note_user_data(NULL),
    
    volume_command_enabled(MAX_CHANNELS, VOLCMD_PORTADOWN),
    effect_command_enabled(MAX_CHANNELS, CMD_MIDI),
    
    stream(NULL)
{
    // Init vol and effect command enable/ignore arrays.
    volume_command_enabled.setAll(true);
    effect_command_enabled.setAll(true);
    
    // Zero out our effect arrays.
    for (int i = 0; i < MAX_CHANNELS; i++)
        volCommand[i] = volParameter[i] = effectCommand[i] = effectParameter[i] = 0;
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
    
    // Allocate the current row.
    current_row = new ModStreamRow();
    
    samples_played = 0;
    
    modplug_file = HackedModPlug_Load(buffer, file_length + 1,
        this,
        mod_stream_cb_increase_sample_count,
        mod_stream_cb_is_volume_command_enabled,
        mod_stream_cb_is_volume_command_pending,
        mod_stream_cb_pop_volume_command,
        mod_stream_cb_pop_volume_parameter,
        mod_stream_cb_is_effect_command_enabled,
        mod_stream_cb_is_effect_command_pending,
        mod_stream_cb_pop_effect_command,
        mod_stream_cb_pop_effect_parameter
        );

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
    
    HackedModPlug_SetOnPatternChanged(modplug_file, mod_stream_cb_on_pattern_changed);
    HackedModPlug_SetOnRowChanged(modplug_file, mod_stream_cb_on_row_changed);
    HackedModPlug_SetOnNoteChange(modplug_file, mod_stream_cb_on_note_change);

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
		timer.start();
        stream_started = true;
    } else if (!play) {
        check_error(__LINE__, Pa_StopStream(stream));
        timer.stop();
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
    for (int instrument = 1; instrument <= info->num_instruments; instrument++)
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
    HackedModPlug_SetChannelEnabled(modplug_file, channel, is_enabled);
}


bool ModStream::get_channel_enabled(int channel) {
    return HackedModPlug_GetChannelEnabled(modplug_file, channel);
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
    
    // Convert time to sample number.
	// 1 second / one million = 1 micro second
	unsigned long long samples_since_start = (timer.getElapsedTimeInMicroSec() * (((double) sampling_rate) / 1000000.0));
    
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
    return string("");
    // TODO: why is this causing a crash?
//    const char* message = ModPlug_GetMessage(modplug_file);
//    return (message != NULL) ? string(message) : string("");
}


double ModStream::get_volume() {
    return ((double) (ModPlug_GetMasterVolume(modplug_file) - 1)) / 511.0;
}


void ModStream::set_volume(double vol) {
    if (vol < 0.)
        vol = 0.;
    else if (vol > 1.)
        vol = 1.;
    ModPlug_SetMasterVolume(modplug_file, (int) (vol * 511) + 1);
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
    HackedModPlug_SetTransposition(modplug_file, channel, offset);
}


int ModStream::get_transposition(int channel) {
    return HackedModPlug_GetTransposition(modplug_file, channel);
}


void ModStream::enable_volume_command(int channel, int volume_command, bool enable) {
    volume_command_enabled.set(channel, volume_command, enable);
}


bool ModStream::is_volume_command_enabled(int channel, int volume_command) {
    return volume_command_enabled.get(channel, volume_command);
}


void ModStream::enable_effect_command(int channel, int effect_command, bool enable) {
    effect_command_enabled.set(channel, effect_command, enable);
}


bool ModStream::is_effect_command_enabled(int channel, int effect_command) {
    return effect_command_enabled.get(channel, effect_command);
}


void ModStream::issue_effect_command(unsigned channel, unsigned effect_command, unsigned effect_param) {
    effectCommand[channel] = effect_command;
    effectParameter[channel] = effect_param;
}

bool ModStream::is_effect_command_pending(unsigned channel) {
    return effectCommand[channel] != 0;
}


unsigned ModStream::pop_effect_command(unsigned channel) {
    unsigned cmd = effectCommand[channel];
    effectCommand[channel] = 0;
    
    return cmd;
}


unsigned ModStream::pop_effect_parameter(unsigned channel) {
    unsigned cmd = effectParameter[channel];
    volParameter[channel] = 0;
    
    return cmd;
}


void ModStream::issue_volume_command(unsigned channel, unsigned volume_command, unsigned volume_param) {
    volCommand[channel] = volume_command;
    volParameter[channel] = volume_param;
}


bool ModStream::is_volume_command_pending(unsigned channel) {
    return volCommand[channel] != 0;
}


unsigned ModStream::pop_volume_command(unsigned channel) {
    unsigned cmd = volCommand[channel];
    volCommand[channel] = 0;
    
    return cmd;
}


unsigned ModStream::pop_volume_parameter(unsigned channel) {
    unsigned cmd = volParameter[channel];
    volParameter[channel] = 0;
    
    return cmd;
}
