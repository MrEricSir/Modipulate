/* Copyright 2011-2012 Eric Gregory and Stevie Hryciw
 *
 * Modipulate-Lua.
 * https://github.com/MrEricSir/Modipulate/
 *
 * This software is licensed under the GNU LGPL (version 3 or later).
 * See the COPYING.LESSER file in this distribution.
 */

#include <modipulate.h>
#include <string>
#include "utils.h"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
}

extern "C" {
    int LUA_API luaopen_libmodipulatelua(lua_State *L);
    static int modipulateLua_init(lua_State *L);
    static int modipulateLua_deinit(lua_State *L);
    static int modipulateLua_update(lua_State *L);
    static int modipulateLua_getVolume(lua_State *L);
    static int modipulateLua_setVolume(lua_State *L);
    static int modipulateLua_loadSong(lua_State *L);
}

#define MODIPULATE_LUA_ERROR(state, err) if (!MODIPULATE_OK(err)) \
    return luaL_error(state, "Modipulate error: %d :: %s", err, modipulate_global_get_last_error_string());

#define MODIPULATE_SONG_T "ModipulateLuaSongClass"

typedef struct {
    ModipulateSong      song;
    ModipulateSongInfo* song_info;
    char*               title;
    char*               message;
    int                 num_channels;
    int                 num_instruments;
    int                 num_samples;
    int                 num_patterns;
} modipulate_song_t;


////////////////////////////////////////////////////////////

// Song Functions.

////////////////////////////////////////////////////////////


static modipulate_song_t *check_modipulate_song_t(lua_State *L, int index) {
    modipulate_song_t *yd;
    luaL_checktype(L, index, LUA_TUSERDATA);
    yd = (modipulate_song_t *)luaL_checkudata(L, index, MODIPULATE_SONG_T);
    if (yd == NULL) 
        luaL_typerror(L, index, MODIPULATE_SONG_T);
    
    return yd;
}


static modipulate_song_t *push_modipulate_song_t(lua_State *L) {
    modipulate_song_t *yd = (modipulate_song_t*)lua_newuserdata(L, sizeof(modipulate_song_t));
    luaL_getmetatable(L, MODIPULATE_SONG_T);
    lua_setmetatable(L, -2);
    
    return yd;
}


static int modipulateLua_song_destroy(lua_State *L) {
    modipulate_song_t* lua_song = (modipulate_song_t*) lua_touserdata(L, 1);
    
    // Attempt to free the song and song info.
    MODIPULATE_LUA_ERROR(L, modipulate_song_info_free(lua_song->song_info));
    MODIPULATE_LUA_ERROR(L, modipulate_song_unload(&lua_song->song));
    
    return 0;
}


static int modipulateLua_song_play(lua_State *L) {
    const char* usage = "Usage: setVolume(volume) where volume is from 0.0 to 1.0";
    luaL_argcheck(L, lua_gettop(L) == 2, 0, usage);
    modipulate_song_t* lua_song = check_modipulate_song_t(L, 1);
    luaL_argcheck(L, lua_isboolean(L, 2), 2, usage);
    
    MODIPULATE_LUA_ERROR(L, modipulate_song_play(&lua_song->song, (int) lua_toboolean(L, 2)));
    
    return 0;
}


static int modipulateLua_song_volume_command(lua_State *L) {
    // TODO
    return 0;
}


static int modipulateLua_song_volume_ignore(lua_State *L) {
    // TODO
    return 0;
}


static int modipulateLua_song_effect_command(lua_State *L) {
    // TODO
    return 0;
}


static int modipulateLua_song_effect_ignore(lua_State *L) {
    // TODO
    return 0;
}


static int modipulateLua_song_set_transposition(lua_State *L) {
    // TODO
    return 0;
}


static int modipulateLua_song_get_transposition(lua_State *L) {
    // TODO
    return 0;
}


static int modipulateLua_song_get_channel_enabled(lua_State *L) {
    // TODO
    return 0;
}


static int modipulateLua_song_set_channel_enabled(lua_State *L) {
    // TODO
    return 0;
}


static int modipulateLua_song_on_pattern_change(lua_State *L) {
    // TODO
    return 0;
}


static int modipulateLua_song_on_row_change(lua_State *L) {
    // TODO
    return 0;
}


static int modipulateLua_song_on_note(lua_State *L) {
    // TODO
    return 0;
}


static const luaL_reg modipulate_song_meta_methods[] = {
    {"__gc", modipulateLua_song_destroy },
    {0,0}
};


static const luaL_reg modipulate_song_methods[] = {
{"play",                   modipulateLua_song_play},
{"volumeCommand",          modipulateLua_song_volume_command},
{"volumeIgnore",           modipulateLua_song_volume_ignore},
{"effectCommand",          modipulateLua_song_effect_command},
{"effectIgnore",           modipulateLua_song_effect_ignore},
{"setTransposition",       modipulateLua_song_set_transposition},
{"getTransposition",       modipulateLua_song_get_transposition},
{"getChannelEnabled",      modipulateLua_song_get_channel_enabled},
{"setChannelEnabled",      modipulateLua_song_set_channel_enabled},
{"onPatternChange",        modipulateLua_song_on_pattern_change},
{"onRowChange",            modipulateLua_song_on_row_change},
{"onNote",                 modipulateLua_song_on_note},
{0,0}
};


static const Xet_reg_pre modipulate_song_getters[] = {
{"title",           get_string,     offsetof(modipulate_song_t, title)},
{"message",         get_string,     offsetof(modipulate_song_t, message)},
{"numChannels",     get_int,        offsetof(modipulate_song_t, num_channels)},
{"numInstruments",  get_int,        offsetof(modipulate_song_t, num_instruments)},
{"numSamples",      get_int,        offsetof(modipulate_song_t, num_samples)},
{"numPatterns",     get_int,        offsetof(modipulate_song_t, num_patterns)},
{0,0}
};

static const Xet_reg_pre modipulate_song_setters[] = {
{0,0}
};

int modipulateLua_song_register(lua_State *L) {
    int metatable, methods;
    
    /* create methods table, & add it to the table of globals */
    luaL_openlib(L, MODIPULATE_SONG_T, modipulate_song_methods, 0);
    methods = lua_gettop(L);
    
    /* create metatable for your_t, & add it to the registry */
    luaL_newmetatable(L, MODIPULATE_SONG_T);
    luaL_openlib(L, 0, modipulate_song_meta_methods, 0);  /* fill metatable */
    metatable = lua_gettop(L);
    
    lua_pushliteral(L, "__metatable");
    lua_pushvalue(L, methods);    /* dup methods table*/
    lua_rawset(L, metatable);     /* hide metatable:
                                     metatable.__metatable = methods */
    lua_pushliteral(L, "__index");
    lua_pushvalue(L, metatable);  /* upvalue index 1 */
    Xet_add(L, modipulate_song_getters);     /* fill metatable with getters */
    lua_pushvalue(L, methods);    /* upvalue index 2 */
    lua_pushcclosure(L, index_handler, 2);
    lua_rawset(L, metatable);     /* metatable.__index = index_handler */
    
    lua_pushliteral(L, "__newindex");
    lua_newtable(L);              /* table for members you can set */
    Xet_add(L, modipulate_song_setters);     /* fill with setters */
    lua_pushcclosure(L, newindex_handler, 1);
    lua_rawset(L, metatable);     /* metatable.__newindex = newindex_handler */
    
    lua_pop(L, 1);                /* drop metatable */
    
    return 1;                     /* return methods on the stack */
}


////////////////////////////////////////////////////////////

// Global Functions.

////////////////////////////////////////////////////////////


// Initalizes Modipulate.
static int modipulateLua_init(lua_State *L) {
    const char* usage = "Usage: init()";
    luaL_argcheck(L, lua_gettop(L) == 0, 0, usage);
    
    MODIPULATE_LUA_ERROR(L, modipulate_global_init());
    
    return 0;
}


// Cleans up.
static int modipulateLua_deinit(lua_State *L) {
    const char* usage = "Usage: deinit()";
    luaL_argcheck(L, lua_gettop(L) == 0, 0, usage);
    
    MODIPULATE_LUA_ERROR(L, modipulate_global_deinit());
    
    return 0;
}


// Triggers callbacks.
// Call this from your update function.
static int modipulateLua_update(lua_State *L) {
    modipulate_global_update();
    
    return 0;
}


static int modipulateLua_setVolume(lua_State *L) {
    const char* usage = "Usage: setVolume(volume) where volume is from 0.0 to 1.0";
    luaL_argcheck(L, lua_gettop(L) == 1, 0, usage);
    luaL_argcheck(L, lua_isnumber(L, 1), 1, usage);
    
    modipulate_global_set_volume(lua_tonumber(L, 1));
    
    return 0;
}


static int modipulateLua_getVolume(lua_State *L) {
    const char* usage = "Usage: getVolume() returns volume value from 0.0 to 1.0";
    luaL_argcheck(L, lua_gettop(L) == 0, 0, usage);
    
    lua_pushnumber(L, modipulate_global_get_volume());
    
    return 1;
}


static int modipulateLua_loadSong(lua_State *L) {
    const char* usage = "Usage: loadSong(filename) where filename is a path of a MOD, S3M, IT, etc. file";
    luaL_argcheck(L, lua_gettop(L) == 1, 0, usage);
    luaL_argcheck(L, lua_isstring(L, 1), 1, usage);
    
    ModipulateSong song;
    ModipulateSongInfo* song_info;
    modipulate_song_t* lua_song = NULL;
    
    // Load song and song info.
    MODIPULATE_LUA_ERROR(L, modipulate_song_load(lua_tostring(L, 1), &song));
    MODIPULATE_LUA_ERROR(L, modipulate_song_get_info(song, &song_info));
    
    // Push a new song onto the stack and set it up.
    lua_song = push_modipulate_song_t(L);
    
    lua_song->song = song;
    lua_song->song_info = song_info;
    
    lua_song->title = song_info->title;
    lua_song->message = song_info->message;
    lua_song->num_channels = song_info->num_channels;
    lua_song->num_instruments = song_info->num_instruments;
    lua_song->num_samples = song_info->num_samples;
    lua_song->num_patterns = song_info->num_patterns;
    
    return 1;
}


////////////////////////////////////////////////////////////

// Luaopen Function.

////////////////////////////////////////////////////////////

int LUA_API luaopen_libmodipulatelua(lua_State *L) {
    struct luaL_reg driver[] = {
        { "luaopen_libmodipulatelua", luaopen_libmodipulatelua },
        { "init", modipulateLua_init },
        { "deinit", modipulateLua_deinit },
        { "update", modipulateLua_update },
        { "setVolume", modipulateLua_setVolume },
        { "getVolume", modipulateLua_getVolume },
        { "loadSong", modipulateLua_loadSong },
        { NULL, NULL }
    };
    luaL_openlib (L, "modipulate", driver, 0);
    
    modipulateLua_song_register(L); // TODO: does this go here? I have no idea.
    
    return 1;
}
