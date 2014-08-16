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

// Table of all modules.
#define MAX_MODSTREAMS 16
ModStream* mods[MAX_MODSTREAMS];

// Check if we've initialized.
static bool modipulateIsInitialized = false;

ModipulateErr modipulate_global_init(void) {
    if (modipulateIsInitialized) {
        return MODIPULATE_ERROR_GENERAL;
    }

    DPRINT("Loading Modipulate!");

	for (int i = 0; i < MAX_MODSTREAMS; i++)
		mods[i] = NULL;
    
    modipulateIsInitialized = true;

    // Start PortAudio.
    return modipulate_handle_pa_error(Pa_Initialize());
}

ModipulateErr modipulate_global_deinit(void) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NONE;
    }

    modipulateIsInitialized = false;

    DPRINT("Quiting Modipulate");
    ModipulateErr ret = MODIPULATE_ERROR_NONE;
    
    // Close mod players.
	for (int i = 0; i < MAX_MODSTREAMS; i++) {
		try {
			if (mods[i] != NULL)
				mods[i]->close();
		} catch (std::string e) {
			modipulate_set_error_string_cpp(e);
			ret = MODIPULATE_ERROR_GENERAL;
		}

		// Free memory!
		delete mods[i];
		mods[i] = NULL;
	}
    
    // Stop PortAudio.
    Pa_Terminate();
    
    return ret;
}

char* modipulate_global_get_last_error_string(void) {
    return last_error;
}

ModipulateErr modipulate_global_update(void) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }

    // Call all callbacks.
	for (int i = 0; i < MAX_MODSTREAMS; i++) {
		if (mods[i]) {
			mods[i]->perform_callbacks();
		}
	}

    return MODIPULATE_ERROR_NONE;
}

ModipulateErr modipulate_song_load(const char* filename, ModipulateSong* song) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }

    DPRINT("Opening file: %s", filename);
    ModipulateErr ret = MODIPULATE_ERROR_NONE;

	ModStream* stream = NULL;

	// Find an empty slot.
    int slot = -1;
	for (int i = 0; i < MAX_MODSTREAMS; i++) {
		if (mods[i] == NULL) {
			// We got one!
			stream = new ModStream();
            slot = i;
			mods[slot] = stream;

			break;
		}
	}

	if (!stream) {
		// Oh no!
		modipulate_set_error_string_cpp("Max concurrent songs reached!");

        return MODIPULATE_ERROR_GENERAL;
	}
    
    try {
        stream->open(filename);
		*song = stream;
    } catch (std::string e) {
        modipulate_set_error_string_cpp(e);
        ret = MODIPULATE_ERROR_GENERAL;

        // Cleanup.
        delete stream;
        mods[slot] = NULL;
    }
    
    return ret;
}


ModipulateErr modipulate_song_unload(ModipulateSong song) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }

    modipulate_song_play(song, false);
    
    // Destroy object.
	for (int i = 0; i < MAX_MODSTREAMS; i++) {
		if (mods[i] == song) {
			mods[i]->close();
			delete mods[i];
			mods[i] = NULL;

			break;
		}
	}
    
    return MODIPULATE_ERROR_NONE;
}

ModipulateErr modipulate_song_play(ModipulateSong song, int play) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }

    ModipulateErr ret = MODIPULATE_ERROR_NONE;
	
    try {
        ((ModStream*) song)->set_playing((bool) play);
    } catch (std::string e) {
        modipulate_set_error_string_cpp(e);
        ret = MODIPULATE_ERROR_GENERAL;
    }
    
    return ret;
}

ModipulateErr modipulate_song_get_info(ModipulateSong song, ModipulateSongInfo** song_info) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }

    ModipulateErr ret = MODIPULATE_ERROR_NONE;
    
    try {
        ((ModStream*) song)->get_info(song_info);
    } catch (std::string e) {
        modipulate_set_error_string_cpp(e);
        ret = MODIPULATE_ERROR_GENERAL;
    }
    
    return ret;
}

ModipulateErr modipulate_song_info_free(ModipulateSongInfo* song_info) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }

    ModStream::free_info(song_info);
    
    return MODIPULATE_ERROR_NONE;
}

float modipulate_song_get_volume(ModipulateSong song) {
    if (!modipulateIsInitialized) {
        return -1;
    }

    return ((ModStream*) song)->get_volume();
}

void modipulate_song_set_volume(ModipulateSong song, float volume) {
    if (!modipulateIsInitialized) {
        return;
    }

    ((ModStream*) song)->set_volume(volume);
}

void modipulate_song_set_channel_enabled(ModipulateSong song, unsigned channel, int enabled) {
    if (!modipulateIsInitialized) {
        return;
    }

    DPRINT("Channel %d is set to %s", channel, enabled ? "Enabled" : "Disabled");
    
    ((ModStream*) song)->set_channel_enabled(channel, enabled);
}


int modipulate_song_get_channel_enabled(ModipulateSong song, unsigned channel) {
    if (!modipulateIsInitialized) {
        return -1;
    }

    return ((ModStream*) song)->get_channel_enabled(channel);
}


ModipulateErr modipulate_song_play_sample(ModipulateSong song, int sample, int note,
	unsigned channel, int modulus, unsigned offset, int volume_command, int volume_value,
	int effect_command, int effect_value) {
	
	if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }

    ModipulateErr ret = MODIPULATE_ERROR_NONE;
    
    try {
        ((ModStream*) song)->play_sample(sample, note, channel,
			modulus, offset, volume_command, volume_value,
			effect_command, effect_value);
    } catch (std::string e) {
        modipulate_set_error_string_cpp(e);
        ret = MODIPULATE_ERROR_GENERAL;
    }
    
    return ret;
}


float modipulate_global_get_volume(void) {
    if (!modipulateIsInitialized) {
        return -1.0;
    }

	return ModStream::modipulate_global_volume;
}


void modipulate_global_set_volume(float vol) {
    if (!modipulateIsInitialized) {
        return;
    }

    ModStream::modipulate_global_volume = vol;
}


ModipulateErr modipulate_song_set_transposition(ModipulateSong song, unsigned channel, int offset) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }

    ((ModStream*) song)->set_transposition(channel, offset);
    
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_get_transposition(ModipulateSong song, unsigned channel, int *offset) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }

    *offset = ((ModStream*) song)->get_transposition(channel);
    
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_volume_command(ModipulateSong song, unsigned channel,
    int volume_command, int volume_value) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }
    
    ((ModStream*) song)->issue_volume_command(channel, volume_command, volume_value);
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_enable_volume(ModipulateSong song, unsigned channel,
    int volume_command, int enable) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }

    ((ModStream*) song)->enable_volume_command(channel, volume_command, (bool) enable);
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_effect_command(ModipulateSong song, unsigned channel,
    int effect_command, int effect_value) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }

    ((ModStream*) song)->issue_effect_command(channel, effect_command, effect_value);
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_enable_effect(ModipulateSong song, unsigned channel,
    int effect_command, int enable) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }

    ((ModStream*) song)->enable_effect_command(channel, effect_command, (bool) enable);
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_on_pattern_change(ModipulateSong song,
    modipulate_song_pattern_change_cb cb, void* user_data) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }

    ((ModStream*) song)->set_pattern_change_cb(cb, user_data);
    
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_on_row_change(ModipulateSong song,
    modipulate_song_row_change_cb cb, void* user_data) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }

    ((ModStream*) song)->set_row_change_cb(cb, user_data);
    
    return MODIPULATE_ERROR_NONE;
}


ModipulateErr modipulate_song_on_note(ModipulateSong song,
    modipulate_song_note_cb cb, void* user_data) {
    if (!modipulateIsInitialized) {
        return MODIPULATE_ERROR_NOT_INITIALIZED;
    }

    ((ModStream*) song)->set_note_change_cb(cb, user_data);
    
    return MODIPULATE_ERROR_NONE;
}
