/* modipulate-gml
 *
 * Modipulate wrapper layer which is compatible with GameMaker.
 * Only doubles and char* are used for argument and return types.
 */

/* ------------------------------------------------------------------------ */

#include "modipulate.h"

/* ------------------------------------------------------------------------ */

#define NOSONG ((ModipulateSong)0)

#define MAX_SONGS 100

#define ERR_OK              0.0
#define ERR_OUTOFRANGE     -1.0
#define ERR_INVALIDSONGID  -2.0
#define ERR_FULL           -3.0
#define ERR_FAIL           -4.0

/* ------------------------------------------------------------------------ */

static ModipulateSong songs[MAX_SONGS] = {0};

static double invalid_song(double songid) {
    unsigned int id;

    if (songid < 0.0 || songid >= MAX_SONGS) {
        return ERR_OUTOFRANGE;
    }
    id = songid;
    if (songs[id] == NOSONG) {
        return ERR_INVALIDSONGID;
    }

    return ERR_OK;
}

/* ------------------------------------------------------------------------ */

double modipulategml_global_init(void) {
    return modipulate_global_init();
}

double modipulategml_global_deinit(void) {
    return modipulate_global_deinit();
}

char* modipulategml_global_get_last_error_string(void) {
    return modipulate_global_get_last_error_string();
}

double modipulategml_global_update(void) {
    return modipulate_global_update();
}

double modipulategml_global_get_volume(void) {
    return modipulate_global_get_volume();
}

double modipulategml_global_set_volume(double vol) {
    modipulate_global_set_volume(vol);

    return 0.0;
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
    unsigned int id;

    if (invalid_song(songid)) {
        return invalid_song(songid);
    }
    id = songid;

    if (modipulate_song_unload(songs[id]) != MODIPULATE_ERROR_NONE) {
        return ERR_FAIL;
    }

    return ERR_OK;
}

double modipulategml_song_play(double songid) {
    ModipulateSong song;
    unsigned int id;

    if (invalid_song(songid)) {
        return invalid_song(songid);
    }
    id = songid;

    if (modipulate_song_play(songs[id], 1) != MODIPULATE_ERROR_NONE) {
        return ERR_FAIL;
    }

    return ERR_OK;
}

double modipulategml_song_stop(double songid) {
    ModipulateSong song;
    unsigned int id;

    if (invalid_song(songid)) {
        return invalid_song(songid);
    }
    id = songid;

    if (modipulate_song_play(songs[id], 0) != MODIPULATE_ERROR_NONE) {
        return ERR_FAIL;
    }

    return ERR_OK;
}

/* ------------------------------------------------------------------------ */
