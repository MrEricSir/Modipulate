/* Checkit
 */

/* ------------------------------------------------------------------------ */

#include "modipulate.h"

/* ------------------------------------------------------------------------ */

#define MAX_SONGS 100

/* ------------------------------------------------------------------------ */

ModipulateSong songs[MAX_SONGS];

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

double modipulategml_song_load(const char* filename, double songid) {
    unsigned int id = songid;

    if (id >= MAX_SONGS) {
        return 0.0;
    }

    return modipulate_song_load(filename, &songs[id]);
}

double modipulategml_song_unload(double songid) {
    unsigned int id = songid;

    if (id >= MAX_SONGS) {
        return 0.0;
    }

    return modipulate_song_unload(songs[id]);
}

double modipulategml_song_play(double songid, double play) {
    unsigned int id = songid;

    if (id >= MAX_SONGS) {
        return 0.0;
    }

    return modipulate_song_play(songs[id], (play != 0.0));
}

/* ------------------------------------------------------------------------ */
