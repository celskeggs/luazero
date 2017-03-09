
#include "lauxlib.h"
#include "llimits.h"
#include </usr/include/libsyscall.h>
#include "libs.h"

extern const char lua_start_lua[];
extern const char lua_start_lua_end[];

LSC_MAIN(argc, argv, env) {
    UNUSED(argc);
    UNUSED(argv);
    UNUSED(env);

    lua_State *L = luaL_newstate();

    luaopen_base(L);

    if (luaL_loadbuffer(L, lua_start_lua, lua_start_lua_end - lua_start_lua, "_start.lua") != LUA_OK) {
        luaL_error(L, "could not init _start.lua");
    }
    lua_call(L, 0, 0);

    lua_close(L);
    return 0;
}
