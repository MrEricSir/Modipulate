/*! \mainpage Modipulate home page
 \author Copyright 2011-2012 Eric Gregory and Stevie Hryciw
 \section intro What is Modipulate?
 
 Modipulate is a library for modipulating games and MOD-style music.
 
 Modipulate home page:
 https://github.com/MrEricSir/Modipulate/
 
 \section license License
 This software is licensed under the GNU LGPL (version 3 or later).
 See the COPYING.LESSER file in this distribution.
 */

/*! \file modipulate.h
    \brief API for the Modipulate library.
*/

#ifndef MODIPULATE_H
#define MODIPULATE_H

#ifdef __cplusplus
extern "C" {
#endif 

#define MODIPULATE_ERROR_NONE                   0
#define MODIPULATE_ERROR_GENERAL                1
#define MODIPULATE_ERROR_INVALID_PARAMETERS     2
#define MODIPULATE_ERROR_NOT_IMPLEMENTED        3

/** \ingroup global 
Error checking macro. Returns 0 for error, 1 for no error.
*/
#define MODIPULATE_OK(err) (err == MODIPULATE_ERROR_NONE)

/** \ingroup global
Error result from Modipulate.
*/
typedef unsigned ModipulateErr;

/** \ingroup song
ID (or "handle") of a song loaded by Modipulate.
*/
typedef void* ModipulateSong;


/** \ingroup song
Pattern change callback.

Called when the the pattern has changed.

@param song           Song that triggered this callback.
@param patter_number  Number of pattern that has started.
@param user_data      Arbitrary per-song callback data.
*/
typedef void (*modipulate_song_pattern_change_cb) (ModipulateSong song, int pattern_number,
    void* user_data);

/** \ingroup song
Note change callback.

Called whenever a note is issued.

@param song           Song that triggered this callback.
@param channel        Channel number that this note is on.
@param note           Note number, where each step is a semitone.
@param instrument     Instrument number or -1 if none
@param sample         Sample number of -1 if none
@param volume_command Identifier for the volume command type, or -1 if none
@param volume_value   Value of the command.  Will be set to zero if volume_command is -1
@param effect_command Identifier for the effect command type, or -1 if none
@param effect_value   Value of the command.  Will be set to zero if effect_command is -1
@param user_data      Arbitrary per-song callback data.
*/
typedef void (*modipulate_song_note_cb) (ModipulateSong song, unsigned channel, int note,
    int instrument, int sample, int volume_command, int volume_value,
    int effect_command, int effect_value, void* user_data);

/** \ingroup song
Row change callback

@param song           Song that triggered this callback.
@param row            The new row that has started playing.
@param user_data      Arbitrary per-song callback data.
*/
typedef void (*modipulate_song_row_change_cb) (ModipulateSong song, int row, void* user_data);

/** \ingroup song
Song information struct.  Contains metadata for a song.
*/
typedef struct {
    char** instrument_names; //!< Array of instrument names from 1 to numInstruments
    char** sample_names;  //!< Array of samples names from 0 to numSamples - 1
    int* rows_per_pattern;  //!< Array of rows per pattern.
    char* title;  //!< Song title
    char* message;  //!< Message string metadata
    int default_tempo;  //!< Default tempo. Until a tempo command is issued, this will be the tempo
    int num_channels;  //!< Number of channels.
    int num_instruments;  //!< Number of instruments. For historical reasons, valid range is 1..numInstruments
    int num_samples;  //!< Number of samples
    int num_patterns; //!< Number of patterns
} ModipulateSongInfo;


/** \addtogroup global Global functions in Modipulate.
@{
*/


/**
Initialize Modipulate.

Call this function when you're ready to start audio processing so Modipulate can get set up.

@return Error
*/
ModipulateErr modipulate_global_init(void);


/**
De-init Modipulate.

Lets Modipulate tidy up after itself.

@return Error
*/
ModipulateErr modipulate_global_deinit(void);


/**
Gets the most recent error string.

Use this to get more information on an error. Can be useful for logging and diagnostics.

@return English string with details on the last error.  Do not free this string.
*/
char* modipulate_global_get_last_error_string(void);


/**
Update function (must be called frequently)

Issues all pending callbacks. Call this in your update loop.

@return Error
*/
ModipulateErr modipulate_global_update(void);


/**
Gets the current global volume.

@return global volume from 0.0 to 1.0
*/
float modipulate_global_get_volume(void);

/**
Sets the current global volume.

@oaram vol the global volume from 0.0 to 1.0
*/
void modipulate_global_set_volume(float vol);

/**@}*/


/** \addtogroup song Per-song functions and callbacks in Modipulate.
@{
*/

/**
Loads a song into Modipulate.

Loads a song into memory. To play the song or pause it once it's started, call modipulate_song_play()

@param filename Name of a MOD-style file to open (MOD, IT, XM, S3M, and many more). String must be null terminated.
@param song     [out] Song handle. Must not be null.
@return Error
*/
ModipulateErr modipulate_song_load(const char* filename, ModipulateSong* song);

/**
Unloads a song from Modipulate.

Frees a song from memory. Song will be stopped if playing.

@param song Song to stop.  The song ID is no longer valid after this call.
@param Error
*/
ModipulateErr modipulate_song_unload(ModipulateSong song);


/**
Plays or pauses the song.

All songs start paused, this must be called to start them.  Can also be used for pausing and unpausing audio.

@param song Song to play or pause.
@param play 1 to play, 0 to pause
*/
ModipulateErr modipulate_song_play(ModipulateSong song, int play);


/**
Gets information about song.

Inspect meta information about the song such as the title and instrument names.

@param song      Song to retrieve information on.
@param song_info [out] Metadata struct.  Must not be null. Must free with modipulate_song_info_free()
@return Error
*/
ModipulateErr modipulate_song_get_info(ModipulateSong song, ModipulateSongInfo** song_info);

/**
Frees a ModipulateSongInfo instance.

@param song_info Metadata struct.  Freed by this call; must ONLY be called once with given song_info.
@return Error
*/
ModipulateErr modipulate_song_info_free(ModipulateSongInfo* song_info);

/**
Issues a volume command.

@param song           Song to act on.
@param channel        Channel number to issue the command on.
@param volume_command Command ID to issue
@param volume_command Command value
@return Error
*/
ModipulateErr modipulate_song_volume_command(ModipulateSong song, unsigned channel,
    int volume_command, int volume_value);

/**
Supresses effect commands on a given channel.

@param song Song to change
@param channel Channel to enable or disable effects on
@param volume_command The specific volume command to supress.
@param enable Whether to enable volume commands on this channel. True (1) is default. False (0) disables commands.
@return Error
*/
ModipulateErr modipulate_song_enable_volume(ModipulateSong song, unsigned channel,
    int volume_command, int enable);

/**
Issues an effect command.

@param song     Song to act on.
@param channel  Channel number to issue the command on.
@param effect   Command ID to issue
@param effect   Command value
@return Error
*/
ModipulateErr modipulate_song_effect_command(ModipulateSong song, unsigned channel,
    int effect_command, int effect_value);


/**
Supresses effect commands on a given channel.

Note that commands issued by the user

@param song Song to change
@param channel Channel to enable or disable effects on
@param effect_command The specific effect to supress.
@param enable Whether to enable effect commands on this channel. True (1) is default. False (0) disables commands.
@return Error
*/
ModipulateErr modipulate_song_enable_effect(ModipulateSong song, unsigned channel,
    int effect_command, int enable);

/**
Sets a transposition offset for a given channel.

@param song The song to act on.
@param channel The channel to set the offset on.
@param offset The semitone offset to set on this channel.  Can be positive or negative.
        Zero means no offset.
@return Error
*/
ModipulateErr modipulate_song_set_transposition(ModipulateSong song, unsigned channel,
    int offset);

/**
Returns the transposition offset for a given channel.

@param song The song to act on.
@param channel The channel to set the offset on.
@param offset [out] The current semitone offset on this channel.
@return Error
*/
ModipulateErr modipulate_song_get_transposition(ModipulateSong song, unsigned channel,
    int* offset);

void modipulate_song_set_channel_enabled(ModipulateSong song, unsigned channel, int enabled);

int modipulate_song_get_channel_enabled(ModipulateSong song, unsigned channel);

/**
Sets a callback to be triggered on a pattern change.

@param cb your callback function
@return Error
*/
ModipulateErr modipulate_song_on_pattern_change(ModipulateSong song,
    modipulate_song_pattern_change_cb cb, void* user_data);

/**
Sets a callback to be triggered on a row change.

@param cb your callback function
@return Error
*/
ModipulateErr modipulate_song_on_row_change(ModipulateSong song,
    modipulate_song_row_change_cb cb, void* user_data);

/**
Sets a callback to be triggered on a note event.

@param song Song to set callback for
@param cb Your callback function.
@param user_data Arbitrary data to be passed to callback.
@return Error
*/
ModipulateErr modipulate_song_on_note(ModipulateSong song,
    modipulate_song_note_cb cb, void* user_data);

/**@}*/


#ifdef __cplusplus
}
#endif 

#endif /* MODIPULATE_H */
