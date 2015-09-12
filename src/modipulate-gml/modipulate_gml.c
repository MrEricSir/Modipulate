/* modipulate_gml.c
 *
 * Copyright 2015 Eric Gregory and Stevie Hryciw
 */

/* ------------------------------------------------------------------------ */

#include <stdio.h>
#include "modipulate.h"
#include "modipulate_gml.h"

/* ------------------------------------------------------------------------ */

#define NOSONG ((ModipulateSong)0)

#define MAX_SONGS 100

#define ERR_OK              0
#define ERR_OUTOFRANGE     -1
#define ERR_INVALIDSONGID  -2
#define ERR_FULL           -3
#define ERR_FAIL           -4
#define ERR_NEG            -5
#define ERR_ASSERT         -6

#define LOADED_OK          -1000

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
        return ERR_OUTOFRANGE;
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
        case ERR_OUTOFRANGE:
            return "Song ID is outside of valid range";
        case ERR_INVALIDSONGID:
            return "Song ID does not point to a loaded song";
        case ERR_FULL:
            return "No more capacity to load songs";
        case ERR_FAIL:
            snprintf(errbuf, sizeof (errbuf),
                "Internal Modipulate error: %s", moderr ? moderr : "?");
            return errbuf;
        case ERR_NEG:
            return "Negative values not allowed";
        case ERR_ASSERT:
            return "Error in modipulate-gml code";
        default:
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
        return ERR_NEG;
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
            return ERR_OK;
        case CMD_FX:
            err = modipulate_song_effect_command(song, ch, cmd, val);
            if (err != MODIPULATE_ERROR_NONE) {
                return ERR_FAIL;
            }
            return ERR_OK;
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
static double song_enable_volume(double songid, double channel,
    double volume_command, int enabled) {
    ModipulateSong song;
    unsigned int ch;
    int cmd;

    int err = get_song(songid, &song);
    if (err != ERR_OK) {
        return err;
    }
    if (channel < 0) {
        return ERR_NEG;
    }
    ch = channel;
    cmd = volume_command;

    err = modipulate_song_enable_volume(song, channel, volume_command, enabled);
    if (err != MODIPULATE_ERROR_NONE) {
        return ERR_FAIL;
    }

    return ERR_OK;
}

double modipulategml_song_enable_volume(double songid, double channel,
    double volume_command) {

    return song_enable_volume(songid, channel, volume_command, 1);
}

double modipulategml_song_disable_volume(double songid, double channel,
    double volume_command) {

    return song_enable_volume(songid, channel, volume_command, 0);
}

/* ------------------------------------------------------------------------ */
