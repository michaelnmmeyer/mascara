#include <lua.h>
#include <lauxlib.h>
#include "../mascara.h"

#define MR_MT "mascara"

struct mr_lua {
   struct mascara *mr;
   int str_ref;
};

static int mr_lua_new(lua_State *lua)
{
   const char *lang = luaL_checkstring(lua, 1);
   const char *const modes[] = {"token", "sentence", NULL};
   enum mr_mode mode = luaL_checkoption(lua, 2, "token", modes);
   
   struct mr_lua *mr = lua_newuserdata(lua, sizeof *mr);

   mr->mr = mr_alloc(lang, mode);
   if (!mr->mr) {
      lua_pushfstring(lua, "unknown model: '%s'", lang);
      return lua_error(lua);
   }
   mr->str_ref = LUA_NOREF;

   luaL_getmetatable(lua, MR_MT);
   lua_setmetatable(lua, -2);
   return 1;
}

static int mr_lua_set_text(lua_State *lua)
{
   struct mr_lua *mr = luaL_checkudata(lua, 1, MR_MT);
   size_t len;
   const char *str = luaL_checklstring(lua, 2, &len);

   mr_set_text(mr->mr, str, len);
   luaL_unref(lua, LUA_REGISTRYINDEX, mr->str_ref);
   lua_pushvalue(lua, 2);
   mr->str_ref = luaL_ref(lua, LUA_REGISTRYINDEX);
   
   return 0;
}

static void mr_lua_put_token(lua_State *lua, const struct mr_token *tk)
{
   lua_createtable(lua, 0, 3);

   lua_pushlstring(lua, tk->str, tk->len);
   lua_setfield(lua, -2, "str");
   lua_pushinteger(lua, tk->offset);
   lua_setfield(lua, -2, "offset");
   lua_pushstring(lua, mr_token_type_name(tk->type));
   lua_setfield(lua, -2, "type");
}

static void mr_lua_put_sentence(lua_State *lua, const struct mr_token *tks,
                                size_t len)
{
   lua_createtable(lua, len, 0);
   
   for (size_t i = 0; i < len; i++) {
      mr_lua_put_token(lua, &tks[i]);
      lua_rawseti(lua, -2, i + 1);
   }
}

static int mr_lua_next(lua_State *lua)
{
   struct mr_lua *mr = luaL_checkudata(lua, 1, MR_MT);
   
   if (mr->str_ref == LUA_NOREF)
      return luaL_error(lua, "no text set");
   
   struct mr_token *tks;
   size_t len = mr_next(mr->mr, &tks);
   if (len) {
      if (mr_mode(mr->mr) == MR_TOKEN)
         mr_lua_put_token(lua, tks);
      else
         mr_lua_put_sentence(lua, tks, len);
   } else {
      lua_pushnil(lua);
   }
   return 1;
}

static int mr_lua_free(lua_State *lua)
{
   struct mr_lua *mr = luaL_checkudata(lua, 1, MR_MT);
   mr_dealloc(mr->mr);
   luaL_unref(lua, LUA_REGISTRYINDEX, mr->str_ref);
   return 0;
}

int luaopen_mascara(lua_State *lua)
{
   const luaL_Reg fns[] = {
      {"set_text", mr_lua_set_text},
      {"next", mr_lua_next},
      {"__gc", mr_lua_free},
      {NULL, 0},
   };
   luaL_newmetatable(lua, MR_MT);
   lua_pushvalue(lua, -1);
   lua_setfield(lua, -2, "__index");
   luaL_setfuncs(lua, fns, 0);

   const luaL_Reg lib[] = {
      {"new", mr_lua_new},
      {NULL, 0},
   };
   luaL_newlib(lua, lib);
   lua_pushstring(lua, MR_VERSION);
   lua_setfield(lua, -2, "VERSION");
   lua_pushnumber(lua, MR_MAX_SENTENCE_LEN);
   lua_setfield(lua, -2, "MAX_SENTENCE_LEN");
   return 1;
}
