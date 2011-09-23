/* Copyright 2011 Eric Gregory
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
    static int set_playing(lua_State *L);
    static int get_num_channels(lua_State *L);
    static int set_channel_enabled(lua_State *L);
    static int get_channel_enabled(lua_State *L);
    static int set_on_note_changed(lua_State *L);
    static int set_on_pattern_changed(lua_State *L);
}

////////////////////////////////////////////////////////////

// Variables.

////////////////////////////////////////////////////////////

ModStream mod;

int on_note_changed = - 1;
lua_State *on_note_changed_state = NULL;


int on_pattern_changed = - 1;
lua_State *on_pattern_changed_state = NULL;

////////////////////////////////////////////////////////////

// Function calls.

////////////////////////////////////////////////////////////

// Initalizes Modipulate.
// Call this from love.load()
static int load(lua_State *L) {
    DPRINT("Loading");
    
    // Init OpenAL.
    alutInit(0, 0);
    
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
    alutExit();
    
    return 0;
}

// Updates the audio, triggers callbacks.
// Call this from love.update()
static int update(lua_State *L) {
    //DPRINT("Update loop"); // For testing only: this printf may eat CPU time.
    
    try {
        if (!mod.update()) {
            if (!mod.is_playing()) {
                DPRINT("MOD is not playing");
                if (!mod.playback())
                    throw std::string("Mod abruptly stopped.");
            }
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
    DPRINT(b ? "Playing" : "Not playing");
    mod.set_playing(b);  // TODO: Fix this. The function doesn't work.
    
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
    lua_pushboolean(L, true);
    return 1;
}

// Callbacks.
//
// TODO
// I'm thinking for starters, something like this.
//
// note_changed(void* callback, int channel, int note, int instrument)
// note_changed_callback(int channel, int note, int instrument) <-- callback sig
// -1's for ints that don't matter (want all changes)

// and another for
// pattern_changed(void* callback, int pattern_number)



static int set_on_note_changed(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 1) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("bool set_on_note_changed(function)");
        DPRINT("  function: void your_func(int channel, int note)");
        return 0;
    }
    
    on_note_changed = luaL_ref(L, LUA_REGISTRYINDEX);
    on_note_changed_state = L;
    return 0;
}

void call_note_changed(unsigned channel, int note) {
    if (on_note_changed == -1)
        return;
    
    lua_getglobal(on_note_changed_state, "note_changed");
    lua_pushnumber(on_note_changed_state, channel);
    lua_pushnumber(on_note_changed_state, note);
    lua_call(on_note_changed_state, 2, 0);
}


static int set_on_pattern_changed(lua_State *L) {
    int argc = lua_gettop(L);
    
    if (argc != 1) {
        DPRINT("ERROR: Function parameters incorrect.");
        DPRINT("bool set_on_pattern_changed(function)");
        DPRINT("  function: void your_func(int pattern)");
        return 0;
    }
    
    on_pattern_changed = luaL_ref(L, LUA_REGISTRYINDEX);
    on_pattern_changed_state = L;
    return 0;
}

void call_pattern_changed(unsigned pattern) {
    if (on_pattern_changed == -1)
        return;
    
    lua_getglobal(on_pattern_changed_state, "pattern_changed");
    lua_pushnumber(on_pattern_changed_state, pattern);
    lua_call(on_pattern_changed_state, 2, 0);
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
        { "set_playing", set_playing },
        { "get_num_channels", get_num_channels },
        { "set_channel_enabled", set_channel_enabled },
        { "get_channel_enabled", get_channel_enabled },
        { "set_on_note_changed", set_on_note_changed },
        { "set_on_pattern_changed", set_on_pattern_changed },
        { NULL, NULL },
    };
    luaL_openlib (L, "modipulate", driver, 0);
    return 1;
}
