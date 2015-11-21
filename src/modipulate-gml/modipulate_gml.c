/* modipulate_gml.c
 *
 * Copyright 2015 Eric Gregory and Stevie Hryciw
 */

/* ------------------------------------------------------------------------ */

#include <assert.h>
#include <stdio.h>
#include "modipulate.h"
#include "modipulate_gml.h"

/* ------------------------------------------------------------------------ */

#define NOSONG ((ModipulateSong)0)

#define MAX_SONGS 100

typedef enum {
    ERR_OK = 0,
    LOADED_OK = -1000,

    ERR_MIN = -100,
    ERR_ASSERT,
    ERR_FAIL,
    ERR_INVALIDSONGID,
    ERR_SONGOUTOFRANGE,
    ERR_CHANNELOUTOFRANGE,
    ERR_AMPOUTOFRANGE,
    ERR_VALUEOUTOFRANGE,
    ERR_FULL,
} MgmlError;

#define CMD_VOL 0
#define CMD_FX  1

/* ------------------------------------------------------------------------ */

static char errbuf[1024] = "";
static ModipulateSong songs[MAX_SONGS] = {0};

/* Set song pointer. Return error if applicable.
 */
static int get_song(double songid, ModipulateSong* song) {
    unsigned int id;

    if (songid < 0.0 || songid >= MAX_SONGS) {
        return ERR_SONGOUTOFRANGE;
    }
    id = songid;
    if (songs[id] == NOSONG) {
        return ERR_INVALIDSONGID;
    }
    *song = songs[id];

    return ERR_OK;
}

/* ---- Global ------------------------------------------------------------ */

double modipulategml_global_init(void) {
    ModipulateErr err = modipulate_global_init();

    return (err == MODIPULATE_ERROR_NONE ? LOADED_OK : ERR_FAIL);
}

double modipulategml_global_deinit(void) {
    ModipulateErr err = modipulate_global_deinit();

    return (err == MODIPULATE_ERROR_NONE ? ERR_OK : ERR_FAIL);
}

char* modipulategml_error_to_string(double errno) {
    char* moderr = modipulate_global_get_last_error_string();

    switch ((int)errno) {
        case ERR_OK:
        case LOADED_OK:
            return "Everything is OK";
        case ERR_ASSERT:
            return "Error in modipulate-gml code";
        case ERR_FAIL:
#ifdef _WIN32
            /* VS 2010 did not like snprintf() */
            _snprintf_s(errbuf, sizeof (errbuf), sizeof (errbuf),
                "Internal Modipulate error: %s", moderr ? moderr : "?");
#else
            snprintf(errbuf, sizeof (errbuf),
                "Internal Modipulate error: %s", moderr ? moderr : "?");
#endif
            return errbuf;
        case ERR_INVALIDSONGID:
            return "Song ID does not point to a loaded song";
        case ERR_SONGOUTOFRANGE:
            return "Song ID is outside of valid range";
        case ERR_CHANNELOUTOFRANGE:
            return "Song channel is outside of valid range";
        case ERR_AMPOUTOFRANGE:
            return "Amplitude is outside of valid range";
        case ERR_VALUEOUTOFRANGE:
            return "Supplied value is outside of valid range";
        case ERR_FULL:
            return "No more capacity to load songs";
        default:
            assert(0);
            return "Unknown error";
    }
}

double modipulategml_global_update(void) {
    ModipulateErr err = modipulate_global_update();

    return (err == MODIPULATE_ERROR_NONE ? ERR_OK : ERR_FAIL);
}

double modipulategml_global_get_volume(void) {
    return modipulate_global_get_volume();
}

double modipulategml_global_set_volume(double vol) {
    if (vol < 0.0 || vol > 1.0) {
        return ERR_AMPOUTOFRANGE;
    }

    modipulate_global_set_volume(vol);

    return ERR_OK;
}

/* ---- Song -------------------------------------------------------------- */

double modipulategml_song_load(const char* filename) {
    unsigned int id = 0;

    /* Find next available song ID */
    for (id = 0; id < MAX_SONGS; id++) {
        if (songs[id] == NOSONG) {
            break;
        }
    }
    /* All song slots filled */
    if (id == MAX_SONGS) {
        return ERR_FULL;
    }

    if (modipulate_song_load(filename, &songs[id]) != MODIPULATE_ERROR_NONE) {
        return ERR_FAIL;
    }

    return id;
}

double modipulategml_song_unload(double songid) {
    ModipulateSong song;

    int err = get_song(songid, &song);
    if (err != ERR_OK) {
        return err;
    }

    if (modipulate_song_unload(song) != MODIPULATE_ERROR_NONE) {
        return ERR_FAIL;
    }

    return ERR_OK;
}

/* Internal helper */
static double song_play(double songid, int play) {
    ModipulateSong song;

    int err = get_song(songid, &song);
    if (err != ERR_OK) {
        return err;
    }

    if (modipulate_song_play(song, play) != MODIPULATE_ERROR_NONE) {
        return ERR_FAIL;
    }

    return ERR_OK;
}

double modipulategml_song_play(double songid) {
    return song_play(songid, 1);
}

double modipulategml_song_stop(double songid) {
    return song_play(songid, 0);
}

double modipulategml_song_get_volume(double songid) {
    ModipulateSong song;

    int err = get_song(songid, &song);
    if (err != ERR_OK) {
        return err;
    }

    return modipulate_song_get_volume(song);
}

double modipulategml_song_set_volume(double songid, double volume) {
    ModipulateSong song;

    int err = get_song(songid, &song);
    if (err != ERR_OK) {
        return err;
    }
    if (volume < 0.0 || volume > 1.0) {
        return ERR_AMPOUTOFRANGE;
    }

    modipulate_song_set_volume(song, volume);

    return ERR_OK;
}

/* ---- Song channel ------------------------------------------------------ */

/* Internal helper */
static double song_command(double songid, double channel, double command,
    double value, int type) {

    ModipulateSong song;
    unsigned int ch;
    int cmd, val;

    int err = get_song(songid, &song);
    if (err != ERR_OK) {
        return err;
    }
    if (channel < 0) {
        return ERR_CHANNELOUTOFRANGE;
    }
    ch = channel;
    cmd = command;
    val = value;

    switch (type) {
        case CMD_VOL:
            err = modipulate_song_volume_command(song, ch, cmd, val);
            if (err != MODIPULATE_ERROR_NONE) {
                return ERR_FAIL;
            }
            break;
        case CMD_FX:
            err = modipulate_song_effect_command(song, ch, cmd, val);
            if (err != MODIPULATE_ERROR_NONE) {
                return ERR_FAIL;
            }
            break;
        default:
            return ERR_ASSERT;
    }

    return ERR_OK;
}

double modipulategml_song_volume_command(double songid, double ch,
    double cmd, double val) {

    return song_command(songid, ch, cmd, val, CMD_VOL);
}

double modipulategml_song_effect_command(double songid, double ch,
    double cmd, double val) {

    return song_command(songid, ch, cmd, val, CMD_FX);
}

/* Internal helper */
static double song_enable_cmd(double songid, double channel,
    double command, int type, int enabled) {

    ModipulateSong song;
    unsigned int ch;
    int cmd;

    int err = get_song(songid, &song);
    if (err != ERR_OK) {
        return err;
    }
    if (channel < 0) {
        return ERR_CHANNELOUTOFRANGE;
    }
    ch = channel;
    cmd = command;

    switch (type) {
        case CMD_VOL:
            err = modipulate_song_enable_volume(song, ch, command, enabled);
            if (err != MODIPULATE_ERROR_NONE) {
                return ERR_FAIL;
            }
            break;
        case CMD_FX:
            err = modipulate_song_enable_effect(song, ch, command, enabled);
            if (err != MODIPULATE_ERROR_NONE) {
                return ERR_FAIL;
            }
            break;
        default:
            return ERR_ASSERT;
    }

    return ERR_OK;
}

double modipulategml_song_enable_volume(double songid, double ch, double cmd) {

    return song_enable_cmd(songid, ch, cmd, CMD_VOL, 1);
}

double modipulategml_song_disable_volume(double songid, double ch, double cmd) {

    return song_enable_cmd(songid, ch, cmd, CMD_VOL, 0);
}

double modipulategml_song_enable_effect(double songid, double ch, double cmd) {

    return song_enable_cmd(songid, ch, cmd, CMD_FX, 1);
}

double modipulategml_song_disable_effect(double songid, double ch, double cmd) {

    return song_enable_cmd(songid, ch, cmd, CMD_FX, 0);
}

double modipulategml_song_play_sample(double songid, double sample,
    double note, double channel, double modulus, double offset,
    double volume_command, double volume_value,
    double effect_command, double effect_value) {

    ModipulateSong song;
    unsigned int ch, off;

    int err = get_song(songid, &song);
    if (err != ERR_OK) {
        return err;
    }
    if (channel < 0) {
        return ERR_CHANNELOUTOFRANGE;
    }
    if (offset < 0) {
        return ERR_VALUEOUTOFRANGE;
    }
    ch = channel;
    off = offset;

    err = modipulate_song_play_sample(song, sample, note, ch, modulus, off,
        volume_command, volume_value, effect_command, effect_value);
    if (err != MODIPULATE_ERROR_NONE) {
        return ERR_FAIL;
    }

    return ERR_OK;
}

double modipulategml_song_fade_channel(double songid, double msec,
    double channel, double destination_amp) {
    ModipulateSong song;

    int err = get_song(songid, &song);
    if (err != ERR_OK) {
        return err;
    }
    if (destination_amp < 0.0 || destination_amp > 1.0) {
        return ERR_AMPOUTOFRANGE;
    }

    err = modipulate_song_fade_channel(song, msec, channel, destination_amp);
    if (err != MODIPULATE_ERROR_NONE) {
        return ERR_FAIL;
    }

    return ERR_OK;
}

/* ------------------------------------------------------------------------ */
