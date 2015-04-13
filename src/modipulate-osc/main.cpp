/* Copyright 2011-2015 Eric Gregory and Stevie Hryciw
 *
 * Modipulate-OSC is part of the Modipulate distribution.
 * https://github.com/MrEricSir/Modipulate/
 *
 * Modipulate is released under the BSD license.  See LICENSE for details.
 *
 * modipulate-osc
 * Command-line Modipulate player which can be controlled
 * via Open Sound Control (OSC).
 * ----
 * Jobs:
 * - Run the Modipulate engine
 * - Open a UDP socket and receive OSC messages
 * - Process OSC messages which map to Modipulate functions
 *   - Load/unload song, play/pause, jump position, mixing, etc.
 * - Send OSC messages corresponding to Modipulate events
 *   - Row played, pattern changed, etc.
 * Notes:
 * - Uses the zlib-licensed OSC library "oscpkt"
 * TODO:
 * - Lots.
 */


#include <iostream> // cout
#include <string>   // std::string
#include <cstdint>  // int32_t, etc
#include <map>      // map song IDs to song pointers

// Modipulate
#include <modipulate.h>

// oscpkt
#define OSCPKT_OSTREAM_OUTPUT
#include "oscpkt/oscpkt.hh"
#include "oscpkt/udp.hh"

// Hardcoded constants
#define APPNAME "modipulate-osc"
#define OUTPUT_BUFFER_SIZE 1024 // holds outbound OSC data
#define SLEEP_MS 50             // sleep between cycles (milliseconds)
// Prefixes for messages printed while the engine is running
#define PFX_INFO   "<info>    "
#define PFX_ERR    "<error>   "
#define PFX_CMD    "<execute> "
#define PFX_OSCIN  "<osc-in>  "
#define PFX_OSCOUT "<osc-out> "
// Helper macros
#define USAGE_MSG \
"Usage:\n" \
"  " APPNAME " --help\n" \
"  " APPNAME " [options]\n" \
"Options:\n" \
"  --help             View this help message and exit\n" \
"  --bind-port=port   Port for incoming messages (default: 7070)\n" \
"  --send-port=port   Port for outgoing messages (default: 7071)\n" \
"  --send-addr=addr   Address for outgoing messages (default: localhost)\n" \
"  --skip-bytes=N     Skip N bytes of incoming packets (default: 0)\n" \
"Examples:\n" \
"  " APPNAME "\n" \
"  " APPNAME " --send-addr=192.168.1.15 -send-port=9442\n" \
"  " APPNAME " --bind-port=7071 --skip-bytes=16\n"


// Modipulate: globals
ModipulateErr err = MODIPULATE_ERROR_NONE;
std::map<std::int32_t, ModipulateSong> song_map;

// oscpkt: globals
oscpkt::UdpSocket socketSend;
oscpkt::UdpSocket socketReceive;
int skip_bytes = 0;
int rcv_port = 7070;
int snd_port = 7071;
std::string snd_address = "127.0.0.1";
char snd_buffer[OUTPUT_BUFFER_SIZE] = "";

static ModipulateSong get_song_from_id(std::int32_t song_id)
{
    std::map<std::int32_t, ModipulateSong>::iterator iter = song_map.find(song_id);
    return iter != song_map.end() ? iter->second : NULL;
}

/*****
       Modipulate Callbacks
                            *****/


// Modipulate: pattern change callback
void on_pattern_change(ModipulateSong song, int pattern_number, void* user_data)
{
    oscpkt::Message msg("/modipulate/cb/patternchange");
    msg.pushInt32(pattern_number);

    oscpkt::PacketWriter pw;
    pw.startBundle().startBundle().addMessage(msg).endBundle().endBundle();
    socketSend.sendPacket(pw.packetData(), pw.packetSize());

    // std::cout << PFX_OSCOUT << "Modipulate: Pattern change: " << pattern_number << "\n";
}

// Modipulate: row change callback
void on_row_change(ModipulateSong song, int row, void* user_data)
{
    ; // TODO
    // Note: see below comments in on_note()
}

// Modipulate: note play callback
void on_note(ModipulateSong song, unsigned channel, int note,
        int instrument, int sample, int volume_command, int volume_value,
        int effect_command, int effect_value, void* user_data)
{
    ; // TODO
    // bundle << "modipulate/cb/note" << channel, note, instrument, sample, ...
}


/*****
       Command Handlers
                        *****/


/**
 * Ping/pong handshake to test connection.
 * input: /modipulate/ping [n]
 * output: /modipulate/pong [n+1]
 */
bool processPing(oscpkt::Message *msg)
{
    std::int32_t port = 0;
    if (!msg->match("/modipulate/ping").popInt32(port).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << "** pong **\n";

    oscpkt::Message reply;
    oscpkt::PacketWriter pw;
    reply.init("/modipulate/pong").pushInt32(port + 1);
    pw.init().addMessage(reply);

    socketSend.sendPacketTo(pw.packetData(), pw.packetSize(), socketSend.packetOrigin());

    return true;
}

/**
 * Quit Modipulate-OSC
 * input: /modipulate/quit
 *
 * NOTE: Unlike other functions, this returns true if we should quit.
 */
bool processQuit(oscpkt::Message *msg)
{
    if (!msg->match("/modipulate/quit").isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Quitting\n";
    return true; // Quit
}

/**
 * Set global volume
 * input: /modipulate/set_volume [float]
 */
bool processSetVolume(oscpkt::Message *msg)
{
    float volume = 0.0;
    if (!msg->match("/modipulate/set_volume").popFloat(volume).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Setting global volume to " << volume << "\n";
    modipulate_global_set_volume(volume);
    return true;
}

/**
 * Load a song
 * input: /modipulate/song/load [int32]
 */
bool processSongLoad(oscpkt::Message *msg)
{
    std::string filename;
    std::int32_t song_id = 0;
    ModipulateSong song;
    if (!msg->match("/modipulate/song/load").popStr(filename).popInt32(song_id).isOkNoMoreArgs())
    {
        return false;
    }
    std::cout << PFX_CMD << "Modipulate: Loading song '" << filename << "' into ID " << song_id << "\n";
    song = get_song_from_id(song_id);
    if (song != NULL)
    {
        std::cout << PFX_INFO << "Modipulate: Song ID " << song_id << " occupied; unloading previous song\n";
        err = modipulate_song_unload(song);
    }
    err = modipulate_song_load(filename.c_str(), &song);
    song_map[song_id] = song;
    if (!MODIPULATE_OK(err))
    {
        return false;
    }
    modipulate_song_on_pattern_change(song, on_pattern_change, NULL);
    modipulate_song_on_row_change(song, on_row_change, NULL);
    modipulate_song_on_note(song, on_note, NULL);

    return true;
}

/**
 * Unload a song
 * input: /modipulate/song/unload [int32]
 */
bool processSongUnload(oscpkt::Message *msg)
{
    std::int32_t song_id = 0;
    ModipulateSong song;
    if (!msg->match("/modipulate/song/unload").popInt32(song_id).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Unloading song " << song_id << "\n";
    song = get_song_from_id(song_id);
    if (song == NULL)
    {
        std::cout << PFX_ERR << "Modipulate: No song with ID " << song_id << "\n";
        return true;
    }
    err = modipulate_song_unload(song);
    song_map.erase(song_id);

    return true;
}

/**
 * Get song info
 * input: /modipulate/song/get_info [int32]
 */
bool processSongGetInfo(oscpkt::Message *msg)
{
    std::int32_t song_id = 0;
    if (!msg->match("/modipulate/song/get_info").popInt32(song_id).isOkNoMoreArgs())
    {
        return false;
    }

    // TODO: send song info
    std::cout << PFX_CMD << "Modipulate: Requesting info for song " << song_id << "\n";
    return true;
}

/**
 * Play a song
 * input: /modipulate/song/play [int32]
 */
bool processSongPlay(oscpkt::Message *msg)
{
    std::int32_t song_id = 0;
    ModipulateSong song;
    if (!msg->match("/modipulate/song/play").popInt32(song_id).isOkNoMoreArgs())
    {
        return false;
    }
    std::cout << PFX_CMD << "Modipulate: Playing song " << song_id << "\n";
    song = get_song_from_id(song_id);
    if (song == NULL)
    {
        std::cout << PFX_ERR << "Modipulate: No song with ID " << song_id << "\n";
        return true;
    }
    err = modipulate_song_play(song, 1);
    return true;
}

/**
 * Stop a song
 * input: /modipulate/song/pause [int32]
 */
bool processSongPause(oscpkt::Message *msg)
{
    std::int32_t song_id = 0;
    ModipulateSong song;
    if (!msg->match("/modipulate/song/pause").popInt32(song_id).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Stopping song\n";
    song = get_song_from_id(song_id);
    if (song == NULL)
    {
        std::cout << PFX_ERR << "Modipulate: No song with ID " << song_id << "\n";
        return true;
    }
    err = modipulate_song_play(song, 0);
    return true;
}

/**
 * Set song volume
 * input: /modipulate/song/set_volume [int32] [float]
 */
bool processSongSetVolume(oscpkt::Message *msg)
{
    std::int32_t song_id = 0;
    float volume = 0.0;
    ModipulateSong song;
    if (!msg->match("/modipulate/song/set_volume").popInt32(song_id).popFloat(volume).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Setting volume to " << volume << " on song " << song_id << "\n";
    song = get_song_from_id(song_id);
    if (song == NULL)
    {
        std::cout << PFX_ERR << "Modipulate: No song with ID " << song_id << "\n";
        return true;
    }
    modipulate_song_set_volume(song, volume);
    return true;
}

/**
 * Play a sample
 * input: /modipulate/song/play_sample [int32] [int32] [int32] [int32] [int32] [int32] [int32]
 */
bool processSongPlaySample(oscpkt::Message *msg)
{
    std::int32_t song_id = 0;
    std::int32_t sample_id = 0;
    std::int32_t note = 0;
    std::int32_t channel = 0;
    std::int32_t modulus = 0;
    std::int32_t offset = 0;

    ModipulateSong song;
    if (!msg->match("/modipulate/song/play_sample")
        .popInt32(song_id).popInt32(sample_id)
        .popInt32(note).popInt32(channel)
        .popInt32(modulus).popInt32(offset).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Playing sample " << sample_id << " on song " << song_id << "\n";
    song = get_song_from_id(song_id);
    if (song == NULL)
    {
        std::cout << PFX_ERR << "Modipulate: No song with ID " << song_id << "\n";
        return true;
    }
    modipulate_song_play_sample(song, sample_id, note, channel, modulus, offset, -1, 0, -1, 0);
    return true;
}

/**
 * Disable a channel
 * input: /modipulate/song/channel/disable [int32] [int32]
 */
bool processSongChannelDisable(oscpkt::Message *msg)
{
    std::int32_t song_id = 0;
    std::int32_t channel = 0;
    ModipulateSong song;
    if (!msg->match("/modipulate/song/channel/disable").popInt32(song_id).popInt32(channel).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Disabling channel " << channel << " on song " << song_id << "\n";
    song = get_song_from_id(song_id);
    if (song == NULL)
    {
        std::cout << PFX_ERR << "Modipulate: No song with ID " << song_id << "\n";
        return true;
    }
    modipulate_song_set_channel_enabled(song, channel, 0);
    return true;
}

/**
 * Enables a channel
 * input: /modipulate/song/channel/enable [int32] [int32]
 */
bool processSongChannelEnable(oscpkt::Message *msg)
{
    std::int32_t song_id = 0;
    std::int32_t channel = 0;
    ModipulateSong song;
    if (!msg->match("/modipulate/song/channel/enable").popInt32(song_id).popInt32(channel).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Enabling channel " << channel << " on song " << song_id << "\n";
    song = get_song_from_id(song_id);
    if (song == NULL)
    {
        std::cout << PFX_ERR << "Modipulate: No song with ID " << song_id << "\n";
        return true;
    }
    modipulate_song_set_channel_enabled(song, channel, 1);
    return true;
}

/**
 * Fade channel volume
 * Channel -1 represents all channels
 * input: /modipulate/song/channel/fade [int32] [int32] [float] [int32]
 */
bool processSongChannelFade(oscpkt::Message *msg)
{
    std::int32_t song_id = 0;
    std::int32_t channel = 0;
    float volume = 0.0;
    std::int32_t duration = 0.0;
    ModipulateSong song;
    if (!msg->match("/modipulate/song/channel/fade")
        .popInt32(song_id)
        .popInt32(channel)
        .popFloat(volume)
        .popInt32(duration)
        .isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Fading channel " << channel << " on song " << song_id
        << " to " << volume << " over " << duration << "ms\n";
    song = get_song_from_id(song_id);
    if (song == NULL)
    {
        std::cout << PFX_ERR << "Modipulate: No song with ID " << song_id << "\n";
        return true;
    }
    if (duration < 0)
    {
        std::cout << PFX_ERR << "Modipulate: Duration must be positive number of milliseconds\n";
        return true;
    }
    err = modipulate_song_fade_channel(song, duration, channel, volume);
    return true;
}

/**
 * Issue an effect column command on a channel
 * input: /modipulate/song/channel/fx_cmd [int32] [int32] [int32] [int32]
 */
bool processSongChannelEffectCommand(oscpkt::Message *msg)
{
    std::int32_t song_id = 0;
    std::int32_t channel = 0;
    std::int32_t effect_command = 0;
    std::int32_t effect_value = 0;
    ModipulateSong song;
    if (!msg->match("/modipulate/song/channel/fx_cmd").popInt32(song_id).popInt32(channel)
            .popInt32(effect_command).popInt32(effect_value).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Executing command " << effect_command
                << " value " << effect_value << " on channel " << channel << "\n";
    song = get_song_from_id(song_id);
    if (song == NULL)
    {
        std::cout << PFX_ERR << "Modipulate: No song with ID " << song_id << "\n";
        return true;
    }
    err = modipulate_song_effect_command(song, channel, effect_command, effect_value);
    return true;
}

/**
 * Issue a volume column command on a channel
 * input: /modipulate/song/channel/vol_cmd [int32] [int32] [int32] [int32]
 */
bool processSongChannelVolumeCommand(oscpkt::Message *msg)
{
    std::int32_t song_id = 0;
    std::int32_t channel = 0;
    std::int32_t volume_command = 0;
    std::int32_t volume_value = 0;
    ModipulateSong song;
    if (!msg->match("/modipulate/song/channel/vol_cmd").popInt32(song_id).popInt32(channel)
            .popInt32(volume_command).popInt32(volume_value).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Executing command " << volume_command
                << " value " << volume_value << " on channel " << channel << "\n";
    song = get_song_from_id(song_id);
    if (song == NULL)
    {
        std::cout << PFX_ERR << "Modipulate: No song with ID " << song_id << "\n";
        return true;
    }
    err = modipulate_song_volume_command(song, channel, volume_command, volume_value);
    return true;
}


/*****
       The Good Stuff
                      *****/


/**
 * Receive loop
 * Returns false when it's time to quit.
 */
bool doReceive(void)
{
    oscpkt::PacketReader pr((char*)socketReceive.packetData() + skip_bytes,
        socketReceive.packetSize() - skip_bytes);
    oscpkt::Message *msg;

    while (pr.isOk() && (msg = pr.popMessage()) != 0)
    {
        err = MODIPULATE_ERROR_NONE;

        // Print whole received message
        std::cout << PFX_OSCIN << *msg << "\n";

        // Check each command.
        if (processPing(msg)) goto runtimeErrorCheck;
        if (processQuit(msg)) return false;
        if (processSetVolume(msg)) goto runtimeErrorCheck;
        if (processSongLoad(msg)) goto runtimeErrorCheck;
        if (processSongUnload(msg)) goto runtimeErrorCheck;
        if (processSongGetInfo(msg)) goto runtimeErrorCheck;
        if (processSongPlay(msg)) goto runtimeErrorCheck;
        if (processSongPause(msg)) goto runtimeErrorCheck;
        if (processSongSetVolume(msg)) goto runtimeErrorCheck;
        if (processSongPlaySample(msg)) goto runtimeErrorCheck;
        //if (processSongSetTempo(msg)) goto runtimeErrorCheck; // For the future...
        if (processSongChannelEnable(msg)) goto runtimeErrorCheck;
        if (processSongChannelDisable(msg)) goto runtimeErrorCheck;
        if (processSongChannelFade(msg)) goto runtimeErrorCheck;
        if (processSongChannelEffectCommand(msg)) goto runtimeErrorCheck;
        if (processSongChannelVolumeCommand(msg)) goto runtimeErrorCheck;
        //if (processSongChannelSetVolume(msg)) goto runtimeErrorCheck; // For the future...


runtimeErrorCheck:
        if (!MODIPULATE_OK(err))
            std::cout << PFX_ERR << "Modipulate: " << modipulate_global_get_last_error_string() << "\n";
    }

    return true;
}


// Core loop
void doLoop(void)
{
    while (socketSend.isOk() && socketReceive.isOk())
    {
        // Modipulate: update
        err = modipulate_global_update();
        if (!MODIPULATE_OK(err))
        {
            std::cout << PFX_ERR << "Modipulate: " << modipulate_global_get_last_error_string() << "\n";
            break;
        }

        // Wait a little bit for incoming messages.
        if (socketReceive.receiveNextPacket(SLEEP_MS))
        {
            if (!doReceive())
            {
                return;
            }
        }
    }
}


// Welcome to Maine
int main(int argc, char *argv[])
{
    // Process arguments
    int i;
    for (i = 1; i < argc; i++)
    {
        std::string param = argv[i];
        if (param.compare("--help") == 0)
        {
            std::cout << USAGE_MSG;
            return 0;
        }
        else
        {
            size_t pos = param.find("=");
            if (pos == std::string::npos)
            {
                std::cout << "Error: Expected --option=VALUE (" << param << ")\n\n";
                std::cout << USAGE_MSG;
                return 1;
            }
            std::string option = param.substr(0, pos);
            std::string value;
            try
            {
                value = param.substr(pos + 1);
            }
            catch (const std::out_of_range& oor)
            {
                std::cout << "Error: No value supplied to option (" << option << ")\n";
                return 1;
            }
            if (option.compare("--bind-port") == 0)
            {
                try
                {
                    rcv_port = std::stoi(value);
                }
                catch (const std::invalid_argument& ia)
                {
                    std::cout << "Error: Bad number supplied to option (" << option << ")\n";
                    return 1;
                }
            }
            else if (option.compare("--send-port") == 0)
            {
                try
                {
                    snd_port = std::stoi(value);
                }
                catch (const std::invalid_argument& ia)
                {
                    std::cout << "Error: Bad number supplied to option (" << option << ")\n";
                    return 1;
                }
            }
            else if (option.compare("--send-addr") == 0)
            {
                snd_address = value;
            }
            else if (option.compare("--skip-bytes") == 0)
            {
                try
                {
                    skip_bytes = std::stoi(param.substr(13));
                }
                catch (const std::invalid_argument& ia)
                {
                    std::cout << "Error: Bad number supplied to option (" << option << ")\n";
                    return 1;
                }
                if (skip_bytes < 0)
                    skip_bytes = 0;
            }
            else
            {
                std::cout << "Error: Unknown option (" << option << ")\n\n";
                std::cout << USAGE_MSG;
                return 1;
            }
        }
    }

    // Modipulate: set up
    std::cout << PFX_INFO << "Modipulate: Setting up\n";
    err = modipulate_global_init();
    if (!MODIPULATE_OK(err))
    {
        std::cout << PFX_ERR << "Modipulate: " << modipulate_global_get_last_error_string() << "\n";
        goto cleanup_post;
    }

    // oscpkt: setup
    socketSend.connectTo(snd_address, snd_port);
    if (!socketSend.isOk())
    {
        std::cout << PFX_ERR << "Error opening port " << snd_port << ": " << socketSend.errorMessage() << "\n";
        goto cleanup_modipulate;
    }

    socketReceive.bindTo(rcv_port);
    if (!socketReceive.isOk())
    {
        std::cout << PFX_ERR << "Error opening port " << rcv_port << ": " << socketReceive.errorMessage() << "\n";
        goto cleanup_modipulate;
    }


    // Let's a-go!
    std::cout << PFX_INFO << "Listening for OSC messages on port " << rcv_port << "\n";
    std::cout << PFX_INFO << "Sending OSC messages to " << snd_address << ":" << snd_port << "\n";
    if (skip_bytes > 0)
        std::cout << PFX_INFO << "Skipping " << skip_bytes << " bytes of incoming packets\n";
    doLoop();
    // ...and we're done.


cleanup_modipulate:
    // Modipulate: shut down
    std::cout << PFX_INFO << "Closing up Modipulate...\n";
    err = modipulate_global_deinit();
    if (!MODIPULATE_OK(err))
        std::cout << PFX_ERR << "Modipulate: " << modipulate_global_get_last_error_string() << "\n";

cleanup_post:
    std::cout << "\nHave a nice day.\n\n";

    return 0;
}
