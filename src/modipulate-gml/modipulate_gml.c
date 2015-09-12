/* modipulate_gml.c
 *
 * Copyright 2015 Eric Gregory and Stevie Hryciw
 */

/* ------------------------------------------------------------------------ */

#include <stdio.h>
#include "modipulate.h"

/* ------------------------------------------------------------------------ */

#define NOSONG ((ModipulateSong)0)

#define MAX_SONGS 100

#define ERR_OK              0
#define ERR_OUTOFRANGE     -1
#define ERR_INVALIDSONGID  -2
#define ERR_FULL           -3
#define ERR_FAIL           -4

#define LOADED_OK          -1000

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

/* ------------------------------------------------------------------------ */

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

double modipulategml_song_play(double songid) {
    ModipulateSong song;

    int err = get_song(songid, &song);
    if (err != ERR_OK) {
        return err;
    }

    if (modipulate_song_play(song, 1) != MODIPULATE_ERROR_NONE) {
        return ERR_FAIL;
    }

    return ERR_OK;
}

double modipulategml_song_stop(double songid) {
    ModipulateSong song;

    int err = get_song(songid, &song);
    if (err != ERR_OK) {
        return err;
    }

    if (modipulate_song_play(song, 0) != MODIPULATE_ERROR_NONE) {
        return ERR_FAIL;
    }

    return ERR_OK;
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

/* ------------------------------------------------------------------------ */
