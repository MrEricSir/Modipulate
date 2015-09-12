/* modipulate_gml.h
 * Wrapper layer for Modipulate that is compatible with Game Maker: Studio.
 *
 * Copyright 2015 Eric Gregory and Stevie Hryciw
 *
 * Modipulate home page:
 * https://github.com/MrEricSir/Modipulate/
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize Modipulate
 * As a special case, this function returns -1000 upon success.
 * Since variables in GML are initialized to 0, and Game Maker could silently
 * fail to load the entire library, this makes it easier to catch faiulre than
 * a success code of `0`.
 */
double modipulategml_global_init(void);

/* Shut down Modipulate
 */
double modipulategml_global_deinit(void);

/* Translate an error code to its corresponding message
 */
char*  modipulategml_error_to_string(double errno);

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
 * Returns new song ID, or negative number upon error
 */
double modipulategml_song_load(const char* filename);

/* Unload a song
 * Returns 0, or negative number upon error
 */
double modipulategml_song_unload(double songid);

/* Play a song
 */
double modipulategml_song_play(double songid);

/* Stop (pause) a song
 */
double modipulategml_song_stop(double songid);

/* Get song info in the form of a JSON string
 * In GML, this data structure can be decoded to a ds_map with json_decode()
 */
/*char*  modipulategml_song_get_info(double songid);*/

/* Get song volume
 */
double modipulategml_song_get_volume(double songid);

/* Set song volume
 */
double modipulategml_song_set_volume(double songid, double volume);

/* Execute a volume command on a song's channel
 * See your MOD format reference for command meanings
 */
double modipulategml_song_volume_command(double songid, double channel,
    double volume_command, double volume_value);

/* Enable processing of a specific volume command on a song channel
 */
double modipulategml_song_enable_volume(double songid, double channel,
    double volume_command);

/* Disable processing of a specific volume command on a song channel
 */
double modipulategml_song_disable_volume(double songid, double channel,
    double volume_command);

/* Execute an FX column command on a song's channel
 */
double modipulategml_song_effect_command(double songid, double channel,
    double effect_command, double effect_value);

/* Play a sample stored in a song, referred by its internal sample number
 * Set volume and FX column commands to < 0 in order to bypass commands
 */
double modipulategml_song_play_sample(double songid, double sample,
    double note, double channel, double modulus, double offset,
    double volume_command, double volume_value,
    double effect_command, double effect_value);

/* Fade a channel to a specified volume over a specified duration
 * Passing -1 to channel fades all channels
 */
double modipulategml_song_fade_channel(double songid, double msec,
    double channel, double destination_amp);

/* "C" */
#ifdef __cplusplus
}
#endif
