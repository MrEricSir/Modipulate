/* Copyright 2011 Eric Gregory and Stevie Hryciw
 *
 * Modipulate.
 * https://github.com/MrEricSir/Modipulate/
 *
 * This software is licensed under the GNU LGPL (version 3 or later).
 * See the COPYING.LESSER file in this distribution. 
 */

#include "modipulate_common.h"
#include "mod_stream.h"

#include <AL/alut.h>
#include <string>

////////////////////////////////////////////////////////////

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
}

extern "C" {
    int LUA_API luaopen_modipulate(lua_State *L);
    static int load(lua_State *L);
    static int quit(lua_State *L);
    static int update(lua_State *L);
    static int open_file(lua_State *L);
    static int get_title(lua_State *L);
    static int get_message(lua_State *L);
    static int get_num_instruments(lua_State *L);
    static int get_instrument_name(lua_State *L);
    static int get_num_samples(lua_State *L);
    static int get_sample_name(lua_State *L);
    static int set_playing(lua_State *L);
    static int get_num_channels(lua_State *L);
    static int set_channel_enabled(lua_State *L);
    static int get_channel_enabled(lua_State *L);
    static int get_current_pattern(lua_State *L);
    static int get_current_row(lua_State *L);
    static int get_current_tempo(lua_State *L);
    static int get_rows_in_pattern(lua_State *L);
    static int set_volume(lua_State *L);
    static int get_volume(lua_State *L);
    static int set_transposition(lua_State *L);
    static int get_transposition(lua_State *L);
    static int set_tempo_override(lua_State *L);
    static int set_on_note_changed(lua_State *L);
    static int set_on_pattern_changed(lua_State *L);
    static int set_on_row_changed(lua_State *L);
    static int set_on_tempo_changed(lua_State *L);
}

////////////////////////////////////////////////////////////

// Variables.

////////////////////////////////////////////////////////////

ModStream mod;

bool manage_openal = false;

int on_note_changed = - 1;
lua_State *on_note_changed_state = NULL;

int on_pattern_changed = - 1;
lua_State *on_pattern_changed_state = NULL;

int on_row_changed = - 1;
lua_State *on_row_changed_state = NULL;

int on_tempo_changed = - 1;
lua_State *on_tempo_changed_state = NULL;

int current_pattern = 0;
int current_row = 0;
int current_tempo = 0;

////////////////////////////////////////////////////////////

// Function calls.

////////////////////////////////////////////////////////////

// Initalizes Modipulate.
// Call this from love.load()
static int load(lua_State *L) {
    DPRINT("Loading Modipulate!");
    
    int argc = lua_gettop(L);
    
    if (argc == 1 && lua_toboolean(L, 1) == 1) {
        // Init OpenAL.
        alutInit(0, 0);
        manage_openal = true;
    }
    
    
    
    int error = alGetError();
    if (error != AL_NO_ERROR)
        DPRINT("OpenAL error was raised: %d", error);
    
    return 0;
}

// Cleans up.
// Call before quitting.
static int quit(lua_State *L) {
    DPRINT("Quiting");
    
    // Close mod player.
    try {
        mod.close();
    } catch (std::string e) {
        DPRINT("Error during quit: %s", e.c_str())
    } 
    
    // OpenAL cleanup.
    if (manage_openal)
        alutExit();
    
    return 0;
}

// Updates the audio, triggers callbacks.
// Call this from love.update()
static int update(lua_State *L) {
    try {
        if (!mod.update()) {
            if (!mod.is_playing() && !mod.playback())
                throw std::string("Mod abruptly stopped.");
        }
    }
    catch(std::string e) {
        DPRINT("Exception: %s", e.c_str())
    }
    
    return 0;
}

static int open_file(lua_State *L) {
    DPRINT("Opening file...");
    int argc = lua_gettop(L);
    
    if (argc != 1) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("open_file(string filename)");
        DPRINT("  filename: name of MOD, XM, IT, etc. file to load");
        return 0;
    }
    
    std::string filename = lua_tostring(L, 1);
    DPRINT("Filename is %s", filename.c_str());
    
    try {
        mod.open(filename);
    } catch (std::string e) {
        DPRINT("Problem loading file: %s error: %s", filename.c_str(), e.c_str())
    }
    
    return 0;
}

// Play or pause Modipulate.
static int set_playing(lua_State *L) {
    DPRINT("Set playing called...");
    int argc = lua_gettop(L);
    
    if (argc != 1) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("set_playing(bool playing)");
        DPRINT("  playing: true to play (default) false to pause");
        return 0;
    }
    
    bool b = (bool) lua_toboolean(L, 1);
    mod.set_playing(b);
    
    return 0;
}

static int get_num_channels(lua_State *L) {
    DPRINT("Getting number of channels");
    int argc = lua_gettop(L);
    if (argc != 0) {
        DPRINT("This function doesn't require any arguments!");
    }
    
    lua_pushnumber(L, mod.get_num_channels());
    return 1;
}


static int set_channel_enabled(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 2) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("set_channel_enabled(int channel, bool enabled)");
        DPRINT("  channel: channel to set");
        DPRINT("  enabled: true to play (default) false to mute");
        return 0;
    }
    
    int c = (int) lua_tonumber(L, 1);
    bool b = (bool) lua_toboolean(L, 2);
    DPRINT("Channel %d is set to %s", c, b ? "Enabled" : "Disabled");
    
    mod.set_channel_enabled(c, b);
    
    return 0;
}


static int get_channel_enabled(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 1) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("bool get_channel_enabled(int channel)");
        DPRINT("  channel: channel to get");
        DPRINT("returns: true to play (default) false to mute");
        return 0;
    }
    
    int c = (int) lua_tonumber(L, 1);
    
    lua_pushboolean(L, mod.get_channel_enabled(c));
    
    return 1;
}


static int get_title(lua_State *L) {
    DPRINT("Getting title");
    int argc = lua_gettop(L);
    if (argc != 0) {
        DPRINT("This function doesn't require any arguments!");
    }
    
    lua_pushstring(L, mod.get_title().c_str());
    return 1;
}

static int get_message(lua_State *L) {
    DPRINT("Getting message");
    int argc = lua_gettop(L);
    if (argc != 0) {
        DPRINT("This function doesn't require any arguments!");
    }
    
    lua_pushstring(L, mod.get_message().c_str());
    return 1;
}

static int get_num_instruments(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 0) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("int get_num_instruments()");
        DPRINT("returns: number of instruments");
        return 0;
    }
    
    lua_pushnumber(L, mod.get_num_instruments());
    
    return 1;
}

static int get_instrument_name(lua_State *L) {
     DPRINT("Getting instrument name...");
    int argc = lua_gettop(L);
    
    if (argc != 1) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("string get_instrument_name(int instrument)");
        DPRINT("  instrument: # of instrument to get name of");
        DPRINT("returns: name of instrument");
        return 0;
    }
    
    unsigned instrument = (unsigned)lua_tonumber(L, 1);
    lua_pushstring(L, mod.get_instrument_name(instrument).c_str());
    return 1;
}

static int get_num_samples(lua_State *L){
int argc = lua_gettop(L);
    
    if (argc != 0) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("int get_num_samples()");
        DPRINT("returns: number of samples");
        return 0;
    }
    
    lua_pushnumber(L, mod.get_num_samples());
    
    return 1;
}

static int get_sample_name(lua_State *L)  {
     DPRINT("Getting sample name...");
    int argc = lua_gettop(L);
    
    if (argc != 1) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("string get_sample_name(int sample)");
        DPRINT("  sample: # of sample to get name of");
        DPRINT("returns: name of sample");
        return 0;
    }
    
    unsigned sample = (unsigned)lua_tonumber(L, 1);
    lua_pushstring(L, mod.get_sample_name(sample).c_str());
    return 1;
}


static int set_volume(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 1) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("set_volume(double vol)");
        DPRINT("  vol: volume to set, from 0 to 1.0");
        return 0;
    }
    
    mod.set_volume(lua_tonumber(L, 1));
    
    return 0;
}


static int get_volume(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 0) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("double get_volume()");
        DPRINT("returns: current volume from 0 to 1.0");
        return 0;
    }
    
    lua_pushnumber(L, mod.get_volume());
    
    return 1;
}


static int set_transposition(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 2) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("void set_transposition(int channel, int offset)");
        DPRINT("  channel: channel to set");
        DPRINT("  offset: transposition offset (signed integeter)");
        return 0;
    }
    
    int c = (int) lua_tonumber(L, 1);
    int t = (int) lua_tonumber(L, 2);
    
    mod.set_transposition(c, t);
    
    return 1;
}


static int get_transposition(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 1) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("int get_transposition(int channel)");
        DPRINT("  channel: channel to get");
        DPRINT("returns: transposition offset (signed integeter)");
        return 0;
    }
    
    int c = (int) lua_tonumber(L, 1);
    
    lua_pushnumber(L, mod.get_transposition(c));
    return 1;
}


static int get_current_pattern(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 0) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("int get_current_pattern()");
        DPRINT("returns: current pattern number in the playing song");
        return 0;
    }
    
    lua_pushnumber(L, current_pattern);
    
    return 1;
}

static int get_current_row(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 0) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("int get_current_row()");
        DPRINT("returns: current row number in the playing song");
        return 0;
    }
    
    lua_pushnumber(L, current_row);
    
    return 1;
}

static int get_current_tempo(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 0) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("int get_current_tempo()");
        DPRINT("returns: current tempo number in the playing song");
        return 0;
    }
    
    if (mod.get_tempo_override() != -1)
        lua_pushnumber(L, mod.get_tempo_override());
    else
        lua_pushnumber(L, current_tempo);
    
    return 1;
}

static int get_rows_in_pattern(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 1) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("int get_rows_in_pattern(int pattern)");
        DPRINT("returns: total rows in the pattern given");
        return 0;
    }
    
    int pattern = (int) lua_tonumber(L, 1);
    lua_pushnumber(L, mod.get_rows_in_pattern(pattern));
    
    return 1;
}


static int set_tempo_override(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 1) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("set_tempo_override(int tempo)");
        DPRINT("  tempo: tempo to override. -1 disables override");
        return 0;
    }
    
    mod.set_tempo_override(lua_tonumber(L, 1));
    
    return 0;
}

// Callbacks.



static int set_on_note_changed(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 1) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("bool set_on_note_changed(function)");
        DPRINT("  function: void your_func(int channel, int note, int instrument, int sample)");
        return 0;
    }
    
    on_note_changed = luaL_ref(L, LUA_REGISTRYINDEX);
    on_note_changed_state = L;
    return 0;
}

void call_note_changed(unsigned channel, int note, int instrument, int sample, int volume) {
    if (on_note_changed == -1)
        return;
    
    lua_rawgeti(on_note_changed_state, LUA_REGISTRYINDEX, on_note_changed);
    lua_pushnumber(on_note_changed_state, channel);
    lua_pushnumber(on_note_changed_state, note);
    lua_pushnumber(on_note_changed_state, instrument);
    lua_pushnumber(on_note_changed_state, sample);
    lua_pushnumber(on_note_changed_state, volume);
    
    lua_call(on_note_changed_state, 5, 0);
}


static int set_on_pattern_changed(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 1) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("void set_on_pattern_changed(function)");
        DPRINT("  function: void your_func(int pattern)");
        return 0;
    }
    
    on_pattern_changed = luaL_ref(L, LUA_REGISTRYINDEX);
    on_pattern_changed_state = L;
    return 0;
}

void call_pattern_changed(unsigned pattern) {
    current_pattern = pattern;
    if (on_pattern_changed == -1)
        return;
    
    lua_rawgeti(on_pattern_changed_state, LUA_REGISTRYINDEX, on_pattern_changed);
    lua_pushnumber(on_pattern_changed_state, pattern);
    lua_call(on_pattern_changed_state, 1, 0);
}

static int set_on_row_changed(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 1) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("void set_on_row_changed(function)");
        DPRINT("  function: void your_func(int row)");
        return 0;
    }
    
    on_row_changed = luaL_ref(L, LUA_REGISTRYINDEX);
    on_row_changed_state = L;
    return 0;
}

void call_row_changed(int row) {
    current_row = row;
    if (on_row_changed == -1)
        return;
    
    lua_rawgeti(on_row_changed_state, LUA_REGISTRYINDEX, on_row_changed);
    lua_pushnumber(on_row_changed_state, row);
    lua_call(on_row_changed_state, 1, 0);
}

static int set_on_tempo_changed(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 1) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("void set_on_tempo_changed(function)");
        DPRINT("  function: void your_func(int tempo)");
        return 0;
    }
    
    on_tempo_changed = luaL_ref(L, LUA_REGISTRYINDEX);
    on_tempo_changed_state = L;
    return 0;
}

void call_tempo_changed(int tempo) {
    current_tempo = tempo;
    if (on_tempo_changed == -1)
        return;
    
    lua_rawgeti(on_tempo_changed_state, LUA_REGISTRYINDEX, on_tempo_changed);
    lua_pushnumber(on_tempo_changed_state, tempo);
    lua_call(on_tempo_changed_state, 1, 0);
}


////////////////////////////////////////////////////////////

// Loader.

////////////////////////////////////////////////////////////

int LUA_API luaopen_modipulate(lua_State *L) {
    struct luaL_reg driver[] = {
        { "luaopen_modipulate", luaopen_modipulate },
        { "load", load },
        { "quit", quit },
        { "update", update },
        { "open_file", open_file },
        { "get_title", get_title },
        { "get_message", get_message },
        { "get_num_instruments", get_num_instruments },
        { "get_instrument_name", get_instrument_name },
        { "get_num_samples", get_num_samples },
        { "get_sample_name", get_sample_name },
        { "set_playing", set_playing },
        { "get_num_channels", get_num_channels },
        { "set_channel_enabled", set_channel_enabled },
        { "get_channel_enabled", get_channel_enabled },
        { "get_current_pattern", get_current_pattern },
        { "get_current_row", get_current_row },
        { "get_current_tempo", get_current_tempo },
        { "set_tempo_override", set_tempo_override },
        { "set_volume", set_volume },
        { "get_volume", get_volume },
        { "set_transposition", set_transposition },
        { "get_transposition", get_transposition },
        { "get_rows_in_pattern", get_rows_in_pattern },
        { "set_on_note_changed", set_on_note_changed },
        { "set_on_pattern_changed", set_on_pattern_changed },
        { "set_on_row_changed", set_on_row_changed },
        { "set_on_tempo_changed", set_on_tempo_changed },
        { NULL, NULL },
    };
    luaL_openlib (L, "modipulate", driver, 0);
    return 1;
}
