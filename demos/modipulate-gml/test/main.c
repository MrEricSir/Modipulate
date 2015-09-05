/* Demo test app for modipulate-gml */

#include <stdio.h>
#include "modipulate_gml.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif


#define PRINT_RES_D printf("   %.3f\n", res_d)
#define PRINT_RES_S printf("   %s\n", res_s)
#define PRINT_ERROR printf("   Last error: %s\n", \
        modipulategml_global_get_last_error_string() ? \
        modipulategml_global_get_last_error_string() : "<none>")

#define SONG_ID (6)


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
    char*  res_s = 0;
    char*  songfile = 0;

    /* -- Parse args -- */

    if (argc < 2) {
        printf("Usage:\n    %s <mod file>\n", argv[0]);

        return 1;
    }
    songfile = argv[1];

    /* -- Setup -- */

    printf("-- Initializing\n");
    res_d = modipulategml_global_init();
    PRINT_RES_D;
    PRINT_ERROR;

    /* -- Volume -- */

    printf("-- Getting volume\n");
    res_d = modipulategml_global_get_volume();
    PRINT_RES_D;
    PRINT_ERROR;

    printf("-- Setting volume to 0.25\n");
    res_d = modipulategml_global_set_volume(0.25);
    PRINT_RES_D;
    PRINT_ERROR;

    printf("-- Getting global mixer volume again\n");
    res_d = modipulategml_global_get_volume();
    PRINT_RES_D;
    PRINT_ERROR;

    /* -- Song loading -- */

    printf("-- Loading song\n");
    res_d = modipulategml_song_load(songfile, SONG_ID);
    PRINT_RES_D;
    PRINT_ERROR;

    printf("-- Attempting to re-load song\n");
    res_d = modipulategml_song_load(songfile, SONG_ID);
    PRINT_RES_D;
    PRINT_ERROR;

    /* -- Song playing -- */

    printf("-- Playing song\n");
    res_d = modipulategml_song_play(SONG_ID, 1.0);
    PRINT_RES_D;
    PRINT_ERROR;

    printf("-- Waiting\n");
    wait(3000);

    printf("-- Setting global mixer volume to 0.90\n");
    res_d = modipulategml_global_set_volume(0.90);
    PRINT_RES_D;
    PRINT_ERROR;

    printf("-- Waiting\n");
    wait(3000);

    printf("-- Updating a few times\n");
    for (i = 0; i < 60; i++) {
        res_d = modipulategml_global_update();
        wait(1000/60);
    }
    PRINT_RES_D;

    printf("-- Unloading song\n");
    res_d = modipulategml_song_unload(SONG_ID);
    PRINT_RES_D;
    PRINT_ERROR;

    /* -- Cleanup -- */

    printf("-- Deinitializing\n");
    res_d = modipulategml_global_deinit();
    PRINT_RES_D;
    PRINT_ERROR;

    return 0;
}
