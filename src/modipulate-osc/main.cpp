/* modipulate-osc
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
 * - Uses the free OSC library "oscpack"
 * TODO:
 * - More intelligent message routing with MappingOscPacketListener
 * - Handle oscpack exceptions wherever available
 */


#include <iostream> // cout
#include <cstring>  // strcmp() etc.
#include <cstdlib>  // atoi()
#include <pthread.h>

#ifdef _WIN32
#include <windows.h> // Sleep()
#else
#include <unistd.h>  // usleep()
#endif

#include <modipulate.h>
#include "oscpack/ip/UdpSocket.h"                 // network socket
#include "oscpack/ip/IpEndpointName.h"            // IP address and port no.
#include "oscpack/osc/OscPacketListener.h"        // receive OSC frames
#include "oscpack/osc/OscReceivedElements.h"      // parse received OSC frames
#include "oscpack/osc/OscPrintReceivedElements.h" // print received OSC frames
#include "oscpack/osc/OscOutboundPacketStream.h"  // send OSC frames


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


using namespace std;


// Modipulate: globals
ModipulateErr err = MODIPULATE_ERROR_NONE;
ModipulateSong song = 0;
ModipulateSongInfo* song_info;

// oscpack: globals
int rcv_port = 0;
int snd_port = 0;
char *snd_address = 0;
char snd_buffer[OUTPUT_BUFFER_SIZE] = "";
UdpListeningReceiveSocket *rcv_socket = 0;
UdpTransmitSocket *snd_socket = 0;
osc::OutboundPacketStream *snd_stream = 0;

// Flow control vars
bool sendloop = true;
bool emptybundle = true;

// Platform-independent sleep
void wait(unsigned ms)
{
#ifdef _WIN32
	Sleep(ms);
#else
	usleep(ms * 1000);
#endif
}

// Thread task for running the socket listener
void *listenertask(void *)
{
	rcv_socket->Run();
	return NULL;
}

// Modipulate: pattern change callback
void on_pattern_change(ModipulateSong song, int pattern_number, void* user_data)
{
	emptybundle = false;
	*snd_stream << osc::BeginBundleImmediate;
	*snd_stream << osc::BeginMessage("/modipulate/cb/patternchange")
		<< pattern_number
		<< osc::EndMessage;
	snd_socket->Send(snd_stream->Data(), snd_stream->Size());
	snd_stream->Clear();
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


// oscpack: Implement an OSC packet listener to process incoming frames
class ModipulatePacketListener : public osc::OscPacketListener
{
public:
	virtual void ProcessMessage(const osc::ReceivedMessage &msg, const IpEndpointName &remoteEndpoint)
	{
		(void) remoteEndpoint; // suppress unused parameter warning

		try
		{
			// Print whole received message
			std::cout << PFX_OSCIN << msg << "\n";
			if (MSG_MATCH(msg.AddressPattern(), "/ping"))
			{
				// TODO: send "/pong" with exact same args as received message
				std::cout << "** pong **\n";
			}
			// Shut down the system and exit
			else if (MSG_MATCH(msg.AddressPattern(), "/modipulate/quit"))
			{
				std::cout << PFX_CMD << "Modipulate: Quitting\n";
				sendloop = false;
			}
			// Load a song
			else if (MSG_MATCH(msg.AddressPattern(), "/modipulate/song/load"))
			{
				osc::ReceivedMessageArgumentStream args = msg.ArgumentStream();
				const char *filename;
				args >> filename >> osc::EndMessage;
				std::cout << PFX_CMD << "Modipulate: Loading song '" << filename << "'\n";
				err = modipulate_song_load(filename, &song);
				modipulate_song_on_pattern_change(song, on_pattern_change, NULL);
				modipulate_song_on_row_change(song, on_row_change, NULL);
				modipulate_song_on_note(song, on_note, NULL);
			}
			// Unload the current song
			else if (MSG_MATCH(msg.AddressPattern(), "/modipulate/song/unload"))
			{
				std::cout << PFX_CMD << "Modipulate: Unloading song\n";
				err = modipulate_song_unload(song);
			}
			// Get info about the current song
			else if (MSG_MATCH(msg.AddressPattern(), "/modipulate/song/info"))
			{
				// TODO: if sending OSC, send song info
				std::cout << PFX_CMD << "Modipulate: Requesting song info\n";
			}
			// Play the current song
			else if (MSG_MATCH(msg.AddressPattern(), "/modipulate/transport/play"))
			{
				std::cout << PFX_CMD << "Modipulate: Playing song\n";
				err = modipulate_song_play(song, 1);
			}
			// Stop the current song
			else if (MSG_MATCH(msg.AddressPattern(), "/modipulate/transport/stop"))
			{
				std::cout << PFX_CMD << "Modipulate: Stopping song\n";
				err = modipulate_song_play(song, 0);
			}
			// Set the global mixer volume
			else if (MSG_MATCH(msg.AddressPattern(), "/modipulate/mixer/setvolume"))
			{
				osc::ReceivedMessageArgumentStream args = msg.ArgumentStream();
				float volume;
				args >> volume >> osc::EndMessage;
				std::cout << PFX_CMD << "Modipulate: Setting volume to " << volume << "\n";
				modipulate_global_set_volume(volume);
				err = MODIPULATE_ERROR_NONE;
			}
			else if (MSG_MATCH(msg.AddressPattern(), "/modipulate/channel/disable"))
			{
				osc::ReceivedMessageArgumentStream args = msg.ArgumentStream();
				int channel;
				args >> channel >> osc::EndMessage;
				std::cout << PFX_CMD << "Modipulate: Disabling channel " << channel << "\n";
				modipulate_song_set_channel_enabled(song, channel, 0);
				err = MODIPULATE_ERROR_NONE;
			}
			else if (MSG_MATCH(msg.AddressPattern(), "/modipulate/channel/enable"))
			{
				osc::ReceivedMessageArgumentStream args = msg.ArgumentStream();
				int channel;
				args >> channel >> osc::EndMessage;
				std::cout << PFX_CMD << "Modipulate: Enabling channel " << channel << "\n";
				modipulate_song_set_channel_enabled(song, channel, 1);
				err = MODIPULATE_ERROR_NONE;
			}
			else if (MSG_MATCH(msg.AddressPattern(), "/modipulate/channel/effect"))
			{
				osc::ReceivedMessageArgumentStream args = msg.ArgumentStream();
				int channel;
				int effect_command;
				int effect_value;
				args >> channel >> effect_command >> effect_value >> osc::EndMessage;
				std::cout << PFX_CMD << "Modipulate: Executing command " << effect_command
					<< " value " << effect_value << " on channel " << channel << "\n";
				err = modipulate_song_effect_command(song, channel, effect_command, effect_value);
			}
			if (!MODIPULATE_OK(err))
				std::cout << PFX_ERR << "Modipulate: " << modipulate_global_get_last_error_string() << "\n";
		}
		catch (osc::Exception &ex)
		{
			std::cout << PFX_ERR << "OSC: Problem parsing incoming message\n";
			std::cout << "  Message:   " << msg << "\n";
			std::cout << "  Exception: " << ex.what() << "\n";
		}
	}
};


// Welcome to Maine
int main(int argc, char *argv[])
{
	ModipulatePacketListener listener;
	pthread_t listenerthread = 0;

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
	if (!rcv_port || !snd_port || !snd_address)
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
		goto cleanup_oscpack;
	}

	// oscpack: set up UDP transmit socket and OSC stream
	try
	{
		snd_socket = new UdpTransmitSocket(IpEndpointName(snd_address, snd_port));
	}
	catch (...)
	{
		std::cout << PFX_ERR << "OSC: Could not connect to transmit socket\n";
		goto cleanup_modipulate;
	}
	try
	{
		snd_stream = new osc::OutboundPacketStream(snd_buffer, OUTPUT_BUFFER_SIZE);
	}
	catch (osc::Exception &ex)
	{
		std::cout << PFX_ERR << "OSC: Could not create outbound stream\n";
		std::cout << PFX_ERR << "  Exception: " << ex.what() << "\n";
		goto cleanup_modipulate;
	}
	std::cout << PFX_INFO << "Sending OSC messages to " << snd_address << " on port " << snd_port << "\n";

	// oscpack: set up UDP receive socket
	try
	{
		rcv_socket = new UdpListeningReceiveSocket(IpEndpointName(IpEndpointName::ANY_ADDRESS, rcv_port), &listener);
	}
	catch (...)
	{
		std::cout << PFX_ERR << "OSC: Could not connect to receive socket\n";
		goto cleanup_modipulate;
	}
	if (pthread_create(&listenerthread, NULL, listenertask, NULL) != 0)
	{
		std::cout << "Abort! Could not create OSC listener thread\n";
		return 1;
	}

	std::cout << PFX_INFO << "Listening for OSC messages on UDP port " << rcv_port << "\n";

	while (sendloop)
	{
		emptybundle = true;

		// Modipulate: update
		err = modipulate_global_update();
		if (!MODIPULATE_OK(err))
		{
			std::cout << PFX_ERR << "Modipulate: " << modipulate_global_get_last_error_string() << "\n";
			return err;
		}

		if (0)
		{
			// // Close the bundle so it can be sent
			// *snd_stream << osc::EndBundle;
			// // Send OSC packet
			// if (!emptybundle)
			// 	snd_socket->Send(snd_stream->Data(), snd_stream->Size());
			;
		}

		// Sleep a bit to reduce CPU usage
		wait(SLEEP_MS);
	}

cleanup_modipulate:
	// Modipulate: shut down
	std::cout << PFX_INFO << "Closing up Modipulate...\n";
	err = modipulate_global_deinit();
	if (!MODIPULATE_OK(err))
		std::cout << PFX_ERR << "Modipulate: " << modipulate_global_get_last_error_string() << "\n";

cleanup_oscpack:
	// oscpack: shut down
	std::cout << PFX_INFO << "Closing up OSC engine...\n";
	if (snd_stream)
		delete snd_stream;
	if (snd_socket)
		delete snd_socket;
	if (rcv_socket)
	{
		rcv_socket->AsynchronousBreak();
		if (pthread_join(listenerthread, NULL) != 0)
		{
			std::cout << "Abort! Could not join listener thread\n";
			return 1;
		}
		delete rcv_socket;
	}

cleanup_post:
	std::cout << "\nHave a nice day.\n\n";

	return 0;
}
