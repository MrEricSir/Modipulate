

// Utility functions.
// Most of this comes from here: http://lua-users.org/wiki/BindingWithMembersAndMethods

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}


typedef int (*Xet_func) (lua_State *L, void *v);

/* member info for get and set handlers */
typedef const struct Xet_reg_type {
  const char *name;  /* member name */
  Xet_func func;     /* get or set function for type of member */
  size_t offset;     /* offset of member within your_t */
}  Xet_reg_pre;

typedef Xet_reg_pre * Xet_reg;

int get_int (lua_State *L, void *v);

int set_int (lua_State *L, void *v);

int get_number (lua_State *L, void *v);

int set_number (lua_State *L, void *v);

int get_string (lua_State *L, void *v);

typedef Xet_reg_pre * Xet_reg;

void Xet_add (lua_State *L, Xet_reg l);

int Xet_call (lua_State *L);

int index_handler (lua_State *L);

int newindex_handler (lua_State *L);
