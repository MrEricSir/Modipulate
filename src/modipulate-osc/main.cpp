/* Copyright 2011-2014 Eric Gregory and Stevie Hryciw
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
#include <cstring>  // strcmp() etc.
#include <cstdlib>  // atoi()
#include <string>   // std::string

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
#define MSG_MATCH(s1, s2) (std::strcmp(s1, s2) == 0)
#define USAGE_MSG \
"Usage:\n" \
"  " APPNAME " -h\n" \
"  " APPNAME " receive_port send_address send_port\n" \
"Options:\n" \
"  -h        View this help message and exit\n" \
"Example:\n" \
"  " APPNAME " 7071 127.0.0.1 8009\n"


// Modipulate: globals
ModipulateErr err = MODIPULATE_ERROR_NONE;
ModipulateSong song = 0;
ModipulateSongInfo* song_info;

// oscpkt: globals
oscpkt::UdpSocket socketSend;
oscpkt::UdpSocket socketReceive;
int rcv_port = 0;
int snd_port = 0;
std::string snd_address = "";
char snd_buffer[OUTPUT_BUFFER_SIZE] = "";


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

    std::cout << PFX_OSCOUT << "Modipulate: Pattern change: " << pattern_number << "\n";
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
 * Ping/pong handshake.
 * input: /ping [n]
 * output: /pong [n+1]
 */
bool processPing(oscpkt::Message *msg) 
{
    oscpkt::int32_t port = 0;
    if (!msg->match("/ping").popInt32(port).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << "** pong **\n";

    oscpkt::Message reply;
    oscpkt::PacketWriter pw;
    reply.init("/pong").pushInt32(port + 1);
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
 * Load a song
 * input: /modipulate/song/load
 */
bool processSongLoad(oscpkt::Message *msg) 
{
    std::string filename;
    if (!msg->match("/modipulate/song/load").popStr(filename).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Loading song '" << filename << "'\n";
    err = modipulate_song_load(filename.c_str(), &song);
    modipulate_song_on_pattern_change(song, on_pattern_change, NULL);
    modipulate_song_on_row_change(song, on_row_change, NULL);
    modipulate_song_on_note(song, on_note, NULL);

    return true;
}

/**
 * Unloads a song
 * input: /modipulate/song/unload
 */
bool processSongUnload(oscpkt::Message *msg) 
{
    if (!msg->match("/modipulate/song/unload").isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Unloading song\n";
    err = modipulate_song_unload(song);
    song = NULL;

    return true;
}

/**
 * Gets song info.
 * input: /modipulate/song/unload
 */
bool processSongInfo(oscpkt::Message *msg) 
{
    if (!msg->match("/modipulate/song/info").isOkNoMoreArgs())
    {
        return false;
    }

    // TODO: send song info
    std::cout << PFX_CMD << "Modipulate: Requesting song info\n";
    return true;
}

/**
 * Plays a song
 * input: /modipulate/transport/play
 */
bool processSongPlay(oscpkt::Message *msg) 
{
    if (!msg->match("/modipulate/transport/play").isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Playing song\n";
    err = modipulate_song_play(song, 1);
    return true;
}

/**
 * Stops a song
 * input: /modipulate/transport/stop
 */
bool processSongStop(oscpkt::Message *msg) 
{
    if (!msg->match("/modipulate/transport/stop").isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Stopping song\n";
    err = modipulate_song_play(song, 0);
    return true;
}

/**
 * Sets global volume
 * input: /modipulate/mixer/setvolume [float]
 */
bool processSetVolume(oscpkt::Message *msg) 
{
    float volume = 0.0;
    if (!msg->match("/modipulate/mixer/setvolume").popFloat(volume).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Setting volume to " << volume << "\n";
    modipulate_global_set_volume(volume);
    return true;
}

/**
 * Disables a channel.
 * input: /modipulate/song/channel/disable [int32]
 */
bool processSetSongChannelDisable(oscpkt::Message *msg) 
{
    oscpkt::int32_t channel = 0;
    if (!msg->match("/modipulate/song/channel/disable").popInt32(channel).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Disabling channel " << channel << "\n";
    modipulate_song_set_channel_enabled(song, channel, 0);
    return true;
}

/**
 * Enables a channel.
 * input: /modipulate/song/channel/enable [int32]
 */
bool processSetSongChannelEnable(oscpkt::Message *msg) 
{
    oscpkt::int32_t channel = 0;
    if (!msg->match("/modipulate/song/channel/enable").popInt32(channel).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Enabling channel " << channel << "\n";
    modipulate_song_set_channel_enabled(song, channel, 1);
    return true;
}

/**
 * Sets an effect on a channel.
 * input: /modipulate/song/channel/effect [int32] [int32] [int32]
 */
bool processSetSongChannelEffect(oscpkt::Message *msg) 
{
    oscpkt::int32_t channel = 0;
    oscpkt::int32_t effect_command = 0;
    oscpkt::int32_t effect_value = 0;
    if (!msg->match("/modipulate/song/channel/effect").popInt32(channel).popInt32(effect_command).popInt32(effect_value).isOkNoMoreArgs())
    {
        return false;
    }

    std::cout << PFX_CMD << "Modipulate: Executing command " << effect_command
                << " value " << effect_value << " on channel " << channel << "\n";
    err = modipulate_song_effect_command(song, channel, effect_command, effect_value);
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
    oscpkt::PacketReader pr(socketReceive.packetData(), socketReceive.packetSize());
    oscpkt::Message *msg;
    while (pr.isOk() && (msg = pr.popMessage()) != 0)
    {
        err = MODIPULATE_ERROR_NONE;

        // Print whole received message
        std::cout << PFX_OSCIN << *msg << "\n";

        // Check each command.
        if (processPing(msg)) goto runtimeErrorCheck;
        if (processQuit(msg)) break;
        if (processSongLoad(msg)) goto runtimeErrorCheck;
        if (processSongUnload(msg)) goto runtimeErrorCheck;
        if (processSongInfo(msg)) goto runtimeErrorCheck;
        if (processSongPlay(msg)) goto runtimeErrorCheck;
        if (processSongStop(msg)) goto runtimeErrorCheck;
        if (processSetVolume(msg)) goto runtimeErrorCheck;
        if (processSetSongChannelDisable(msg)) goto runtimeErrorCheck;
        if (processSetSongChannelEnable(msg)) goto runtimeErrorCheck;
        if (processSetSongChannelEffect(msg)) goto runtimeErrorCheck;


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
    if (argc > 1 && std::strcmp(argv[1], "-h") == 0)
    {
        std::cout << USAGE_MSG;
        return 0;
    }
    if (argc < 4)
    {
        std::cout << "Not enough parameters\n";
        std::cout << USAGE_MSG;
        return 1;
    }
    rcv_port = std::atoi(argv[1]);
    snd_address = argv[2];
    snd_port = std::atoi(argv[3]);
    if (!rcv_port || !snd_port || snd_address.length() == 0)
    {
        std::cout << "Error parsing parameters (invalid address or port number?)\n";
        return 1;
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
    std::cout << PFX_INFO << "Listening for OSC messages on UDP port " << rcv_port << "\n";
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
