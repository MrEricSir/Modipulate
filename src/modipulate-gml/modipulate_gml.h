/* modipulate_gml.h
 * Wrapper layer for Modipulate that is compatible with Game Maker: Studio.
 *
 * Copyright 2015 Eric Gregory and Stevie Hryciw
 *
 * Due to the type limitations of Game Maker Language (GML), the only types
 * which may be passed as arguments or returned are `string` and `double`.
 *
 * About Modipulate:
 * Modipulate is a library for modipulating games and MOD-style music.
 *
 * Modipulate home page:
 * https://github.com/MrEricSir/Modipulate/
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize Modipulate
 */
double modipulategml_global_init(void);

/* Shut down Modipulate
 */
double modipulategml_global_deinit(void);

/* Get the last error message (if it was set)
 */
char*  modipulategml_global_get_last_error_string(void);

/* Main update routine: call this every cycle
 */
double modipulategml_global_update(void);

/* Get global mixer volume
 */
double modipulategml_global_get_volume(void);

/* Set global mixer volume
 */
double modipulategml_global_set_volume(double vol);

/* Load a song file into a song ID
 */
double modipulategml_song_load(const char* filename, double songid);

/* Unload a song by its ID
 */
double modipulategml_song_unload(double songid);

/* Play a song by its ID. When play is 0, stop; otherwise, play
 */
double modipulategml_song_play(double songid, double play);

/*char*  modipulategml_song_get_info(double songid);
double modipulategml_song_get_volume(double songid);
double modipulategml_song_set_volume(double songid, double volume);
double modipulategml_song_volume_command(double songid, double channel,
    double volume_command, double volume_value);
double modipulategml_song_enable_volume(double songid, double channel,
    double volume_command, double enable);
double modipulategml_song_effect_command(double songid, double channel,
    double effect_command, double effect_value);
double modipulategml_song_play_sample(double songid, double sample,
    double note, double channel, double modulus, double offset,
    double volume_command, double volume_value,
    double effect_command, double effect_value);
double modipulategml_song_fade_channel(double songid, double msec,
    double channel, double destination_amp);*/

/* "C" */
#ifdef __cplusplus
}
#endif
