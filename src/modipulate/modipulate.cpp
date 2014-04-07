/* Copyright 2011-2014 Eric Gregory and Stevie Hryciw
 *
 * Modipulate.
 * https://github.com/MrEricSir/Modipulate/
 *
 * Modipulate is released under the BSD license.  See LICENSE for details.
 */

#include "mod_stream.h"
#include "modipulate_common.h"
#include "modipulate.h"

#include <string>
#include <string.h>
#include "portaudio.h"

// Last error string.
char* last_error = NULL;

ModStream mod;

ModipulateErr modipulate_global_init(void) {
    DPRINT("Loading Modipulate!");
    
    // Start PortAudio.
    return modipulate_handle_pa_error(Pa_Initialize());
}

ModipulateErr modipulate_global_deinit(void) {
    DPRINT("Quiting Modipulate");
    ModipulateErr ret = MODIPULATE_ERROR_NONE;
    
    // Close mod player.
    try {
        mod.close();
    } catch (std::string e) {
        modipulate_set_error_string_cpp(e);
        ret = MODIPULATE_ERROR_GENERAL;
    }
    
    // Stop PortAudio.
    Pa_Terminate();
    
    return ret;
}

char* modipulate_global_get_last_error_string(void) {
    return last_error;
}

ModipulateErr modipulate_global_update(void) {
    // TODO: call all mod callbacks
    mod.perform_callbacks();
    
    return MODIPULATE_ERROR_NONE;
}

ModipulateErr modipulate_song_load(const char* filename, ModipulateSong* song) {
    DPRINT("Opening file: %s", filename);
    ModipulateErr ret = MODIPULATE_ERROR_NONE;
    
    try {
        mod.open(filename);
    } catch (std::string e) {
        modipulate_set_error_string_cpp(e);
        ret = MODIPULATE_ERROR_GENERAL;
    }
    
    return ret;
}


ModipulateErr modipulate_song_unload(ModipulateSong song) {
    modipulate_song_play(song, false);
    
    // TODO: destroy object.
    
    return MODIPULATE_ERROR_NONE;
}

ModipulateErr modipulate_song_play(ModipulateSong song, int play) {
    ModipulateErr ret = MODIPULATE_ERROR_NONE;
    
    try {
        mod.set_playing((bool) play);
    } catch (std::string e) {
        modipulate_set_error_string_cpp(e);
        ret = MODIPULATE_ERROR_GENERAL;
    }
    
    return ret;
}

ModipulateErr modipulate_song_get_info(ModipulateSong song, ModipulateSongInfo** song_info) {
    ModipulateErr ret = MODIPULATE_ERROR_NONE;
    
    try {
        mod.get_info(song_info);
    } catch (std::string e) {
        modipulate_set_error_string_cpp(e);
        ret = MODIPULATE_ERROR_GENERAL;
    }
    
    return ret;
}

ModipulateErr modipulate_song_info_free(ModipulateSongInfo* song_info) {
    mod.free_info(song_info);
    
    return MODIPULATE_ERROR_NONE;
}

void modipulate_song_set_channel_enabled(ModipulateSong song, unsigned channel, int enabled) {
    DPRINT("Channel %d is set to %s", channel, enabled ? "Enabled" : "Disabled");
    
    mod.set_channel_enabled(channel, enabled);
}


int modipulate_song_get_channel_enabled(ModipulateSong song, unsigned channel) {
    return mod.get_channel_enabled(channel);
}


float modipulate_global_get_volume(void) {
    return mod.get_volume();
}


void modipulate_global_set_volume(float vol) {
    mod.set_volume(vol);
}


ModipulateErr modipulate_song_set_transposition(ModipulateSong song, unsigned channel, int offset) {
    mod.set_transposition(channel, offset);
    
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_get_transposition(ModipulateSong song, unsigned channel, int *offset) {
    *offset = mod.get_transposition(channel);
    
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_volume_command(ModipulateSong song, unsigned channel,
    int volume_command, int volume_value) {
    
    mod.issue_volume_command(channel, volume_command, volume_value);
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_enable_volume(ModipulateSong song, unsigned channel,
    int volume_command, int enable) {
    
    mod.enable_volume_command(channel, volume_command, (bool) enable);
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_effect_command(ModipulateSong song, unsigned channel,
    int effect_command, int effect_value) {
    
    mod.issue_effect_command(channel, effect_command, effect_value);
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_enable_effect(ModipulateSong song, unsigned channel,
    int effect_command, int enable) {
    
    mod.enable_effect_command(channel, effect_command, (bool) enable);
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_on_pattern_change(ModipulateSong song,
    modipulate_song_pattern_change_cb cb, void* user_data) {
    
    mod.set_pattern_change_cb(cb, user_data);
    
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_on_row_change(ModipulateSong song,
    modipulate_song_row_change_cb cb, void* user_data) {
    
    mod.set_row_change_cb(cb, user_data);
    
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_on_note(ModipulateSong song,
    modipulate_song_note_cb cb, void* user_data) {
    
    mod.set_note_change_cb(cb, user_data);
    
    return MODIPULATE_ERROR_NONE;
}
