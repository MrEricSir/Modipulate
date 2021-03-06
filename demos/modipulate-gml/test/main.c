/* Demo test app for modipulate-gml */

#include <stdio.h>
#include "modipulate_gml.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif


#define PRINT_RES_D printf("%.3f\n", res_d)
#define PRINT_RES_S printf("%s\n", res_s)
#define CHECK(err) \
    if (err < 0.0) {\
        printf("\n-- Error\n"); \
        printf("   %s\n", modipulategml_error_to_string(err)); \
        goto end;\
    }

#define N_SONGS 6


static double songs[N_SONGS];

static void wait(unsigned ms) {
    #ifdef _WIN32
    Sleep(ms);
    #else
    usleep(ms * 1000);
    #endif
}


int main(int argc, char* argv[])
{
    int i = 0;
    double res_d = 0.0;
    char*  songfile = 0;

    /* -- Parse args -- */

    if (argc < 2) {
        printf("Usage:\n    %s <mod file>\n", argv[0]);

        return 1;
    }
    songfile = argv[1];

    /* -- Setup -- */

    printf("-- Initializing: ");
    res_d = modipulategml_global_init();
    PRINT_RES_D;

    /* -- Volume -- */

    printf("-- Getting global volume: ");
    res_d = modipulategml_global_get_volume();
    PRINT_RES_D;

    printf("-- Setting global volume to 0.25: ");
    res_d = modipulategml_global_set_volume(0.25);
    PRINT_RES_D;

    printf("-- Getting global volume again: ");
    res_d = modipulategml_global_get_volume();
    PRINT_RES_D;

    /* -- Song loading -- */

    printf("-- Loading song to slot: ");
    for (i = 0; i < N_SONGS; i++) {
        res_d = modipulategml_song_load(songfile);
        CHECK(res_d);
        songs[i] = res_d;
    }
    PRINT_RES_D;

    printf("-- Unloading first song: ");
    res_d = modipulategml_song_unload(songs[0]);
    PRINT_RES_D;
    CHECK(res_d);

    /* -- Song playing -- */

    printf("-- Playing song: ");
    res_d = modipulategml_song_play(songs[1]);
    PRINT_RES_D;
    CHECK(res_d);

    printf("-- Waiting...\n");
    wait(3000);

    printf("-- Pausing song: ");
    res_d = modipulategml_song_stop(songs[1]);
    PRINT_RES_D;
    CHECK(res_d);

    printf("-- Waiting...\n");
    wait(1000);

    printf("-- Resuming song: ");
    res_d = modipulategml_song_play(songs[1]);
    PRINT_RES_D;
    CHECK(res_d);

    /* -- Song volume -- */

    printf("-- Setting global volume to 0.90: ");
    res_d = modipulategml_global_set_volume(0.90);
    PRINT_RES_D;
    CHECK(res_d);

    printf("-- Waiting...\n");
    wait(3000);

    printf("-- Setting song volume to 0.40: ");
    res_d = modipulategml_song_set_volume(songs[1], 0.40);
    PRINT_RES_D;

    printf("-- Getting song volume: ");
    res_d = modipulategml_song_get_volume(songs[1]);
    PRINT_RES_D;

    printf("-- Waiting...\n");
    wait(1000);

    printf("-- Setting song volume back to 1.00: ");
    res_d = modipulategml_song_set_volume(songs[1], 1.00);
    PRINT_RES_D;

    /* -- Execute FX -- */

    printf("-- Executing FX command: ");
    res_d = modipulategml_song_effect_command(songs[1], 1, 17, 200);
    PRINT_RES_D;

    printf("-- Updating: ");
    res_d = modipulategml_global_update();
    PRINT_RES_D;
    CHECK(res_d);

    printf("-- Waiting...\n");
    wait(2000);

    /* -- Samples -- */

    printf("-- Playing some samples: ");
    for (i = 0; i < 12; i++) {
        res_d = modipulategml_song_play_sample(songs[1], 1, 64 + i, 1, 2, i,
            -1, -1, -1, -1);
    }
    PRINT_RES_D;

    printf("-- Updating: ");
    res_d = modipulategml_global_update();
    PRINT_RES_D;
    CHECK(res_d);

    printf("-- Waiting...\n");
    wait(2000);

    /* -- Channel fading -- */

    printf("-- Fading all channels out: ");
    res_d = modipulategml_song_fade_channel(songs[1], 2000, -1, 0.0);
    PRINT_RES_D;

    printf("-- Updating a few times: ");
    for (i = 0; i < 120; i++) {
        res_d = modipulategml_global_update();
        if (res_d < 0) {
            break;
        }
        wait(1 / 60 * 1000);
    }
    PRINT_RES_D;
    CHECK(res_d);

    printf("-- Waiting...\n");
    wait(2200);

    /* -- Cleanup -- */

    printf("-- Unloading song: ");
    res_d = modipulategml_song_unload(songs[1]);
    PRINT_RES_D;
    CHECK(res_d);

    end:

    printf("-- Deinitializing: ");
    res_d = modipulategml_global_deinit();
    PRINT_RES_D;
    CHECK(res_d);

    return 0;
}
