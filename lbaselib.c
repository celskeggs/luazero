/*
** $Id: lbaselib.c,v 1.313 2016/04/11 19:18:40 roberto Exp $
** Basic library
** See Copyright Notice in lua.h
*/

#define lbaselib_c
#define LUA_LIB

#include "helpers.h"

#include "lua.h"
#include "lctype.h"
#include "lauxlib.h"


static int luaB_printraw(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    lua_writestring(s, l);
    return 0;
}

static int luaB_tonumberraw(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    return s != NULL && lua_stringtonumber(L, s) == l + 1 ? 1 : 0;
}

static int luaB_isintegerraw(lua_State *L) {
    if (lua_type(L, 1) == LUA_TNUMBER) {
        lua_pushboolean(L, lua_isinteger(L, 1));
        return 1;
    } else {
        return luaL_error(L, "parameter to isintegerraw was not an integer");
    }
}

static int luaB_getbyteraw(lua_State *L) {
    size_t l;
    const char *s = luaL_checklstring(L, 1, &l);
    lua_Integer i = luaL_checkinteger(L, 2);
    if (i < 1 || (l < LUA_MAXINTEGER && i > (lua_Integer) l)) {
        return 0;
    } else {
        lua_pushinteger(L, (unsigned char) s[i - 1]);
        return 1;
    }
}

static int luaB_error(lua_State *L) {
    int level = (int) luaL_optinteger(L, 2, 1);
    lua_settop(L, 1);
    if (lua_type(L, 1) == LUA_TSTRING && level > 0) {
        luaL_where(L, level);   /* add extra information */
        lua_pushvalue(L, 1);
        lua_concat(L, 2);
    }
    return lua_error(L);
}

static int luaB_getmetatableraw(lua_State *L) {
    luaL_checkany(L, 1);
    return lua_getmetatable(L, 1);
}

static int luaB_setmetatableraw(lua_State *L) {
    int t = lua_type(L, 2);
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_argcheck(L, t == LUA_TNIL || t == LUA_TTABLE, 2,
                  "nil or table expected");
    lua_settop(L, 2);
    lua_setmetatable(L, 1);
    return 0;
}

static int luaB_rawequal(lua_State *L) {
    luaL_checkany(L, 1);
    luaL_checkany(L, 2);
    lua_pushboolean(L, lua_rawequal(L, 1, 2));
    return 1;
}


static int luaB_rawlen(lua_State *L) {
    int t = lua_type(L, 1);
    luaL_argcheck(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                  "table or string expected");
    lua_pushinteger(L, lua_rawlen(L, 1));
    return 1;
}


static int luaB_rawget(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checkany(L, 2);
    lua_settop(L, 2);
    lua_rawget(L, 1);
    return 1;
}

static int luaB_rawset(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checkany(L, 2);
    luaL_checkany(L, 3);
    lua_settop(L, 3);
    lua_rawset(L, 1);
    return 1;
}


static int luaB_type(lua_State *L) {
    int t = lua_type(L, 1);
    luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
    lua_pushstring(L, lua_typename(L, t));
    return 1;
}


static int luaB_next(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
    if (lua_next(L, 1))
        return 2;
    else {
        lua_pushnil(L);
        return 1;
    }
}

/*
** Traversal function for 'ipairs'
*/
static int ipairsaux(lua_State *L) {
    lua_Integer i = luaL_checkinteger(L, 2) + 1;
    lua_pushinteger(L, i);
    return (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
static int luaB_ipairs(lua_State *L) {
    luaL_checkany(L, 1);
    lua_pushcfunction(L, ipairsaux);  /* iteration function */
    lua_pushvalue(L, 1);  /* state */
    lua_pushinteger(L, 0);  /* initial value */
    return 3;
}


static int load_aux(lua_State *L, int status, int envidx) {
    if (status == LUA_OK) {
        if (envidx != 0) {  /* 'env' parameter? */
            lua_pushvalue(L, envidx);  /* environment for loaded function */
            if (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
                lua_pop(L, 1);  /* remove 'env' if not used by previous call */
        }
        return 1;
    } else {  /* error (message is on top of the stack) */
        lua_pushnil(L);
        lua_insert(L, -2);  /* put before error message */
        return 2;  /* return nil plus error message */
    }
}

/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** reserved slot, above all arguments, to hold a copy of the returned
** string to avoid it being collected while parsed. 'load' has four
** optional arguments (chunk, source name, mode, and environment).
*/
#define RESERVEDSLOT    5


/*
** Reader for generic 'load' function: 'lua_load' uses the
** stack for internal stuff, so the reader cannot change the
** stack top. Instead, it keeps its resulting string in a
** reserved slot inside the stack.
*/
static const char *generic_reader(lua_State *L, void *ud, size_t *size) {
    (void) (ud);  /* not used */
    luaL_checkstack(L, 2, "too many nested functions");
    lua_pushvalue(L, 1);  /* get function */
    lua_call(L, 0, 1);  /* call it */
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);  /* pop result */
        *size = 0;
        return NULL;
    } else if (!lua_isstring(L, -1))
        luaL_error(L, "reader function must return a string");
    lua_replace(L, RESERVEDSLOT);  /* save string in reserved slot */
    return lua_tolstring(L, RESERVEDSLOT, size);
}


static int luaB_load(lua_State *L) {
    int status;
    size_t l;
    const char *s = lua_tolstring(L, 1, &l);
    const char *mode = luaL_optstring(L, 3, "bt");
    int env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
    if (s != NULL) {  /* loading a string? */
        const char *chunkname = luaL_optstring(L, 2, s);
        status = luaL_loadbufferx(L, s, l, chunkname, mode);
    } else {  /* loading from a reader function */
        const char *chunkname = luaL_optstring(L, 2, "=(load)");
        luaL_checktype(L, 1, LUA_TFUNCTION);
        lua_settop(L, RESERVEDSLOT);  /* create reserved slot */
        status = lua_load(L, generic_reader, NULL, chunkname, mode);
    }
    return load_aux(L, status, env);
}

/* }====================================================== */


static int luaB_select(lua_State *L) {
    int n = lua_gettop(L);
    if (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
        lua_pushinteger(L, n - 1);
        return 1;
    } else {
        lua_Integer i = luaL_checkinteger(L, 1);
        if (i < 0) i = n + i;
        else if (i > n) i = n;
        luaL_argcheck(L, 1 <= i, 1, "index out of range");
        return n - (int) i;
    }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
static int finishpcall(lua_State *L, int status, lua_KContext extra) {
    if (status != LUA_OK && status != LUA_YIELD) {  /* error? */
        lua_pushboolean(L, 0);  /* first result (false) */
        lua_pushvalue(L, -2);  /* error message */
        return 2;  /* return false, msg */
    } else
        return lua_gettop(L) - (int) extra;  /* return all results */
}


static int luaB_pcall(lua_State *L) {
    int status;
    luaL_checkany(L, 1);
    lua_pushboolean(L, 1);  /* first result if no errors */
    lua_insert(L, 1);  /* put it in place */
    status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
    return finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
static int luaB_xpcall(lua_State *L) {
    int status;
    int n = lua_gettop(L);
    luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
    lua_pushboolean(L, 1);  /* first result */
    lua_pushvalue(L, 1);  /* function */
    lua_rotate(L, 3, 2);  /* move them below function's arguments */
    status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
    return finishpcall(L, status, 2);
}


static int luaB_tostring(lua_State *L) {
    luaL_checkany(L, 1);
    luaL_tolstring(L, 1, NULL);
    return 1;
}


static const luaL_Reg base_funcs[] = {
        {"getbyteraw",   luaB_getbyteraw},
        {"error",        luaB_error},
        {"getmetatableraw", luaB_getmetatableraw},
        {"ipairs",       luaB_ipairs},
        {"load",         luaB_load},
        {"next",         luaB_next},
        {"pairs",        luaB_pairs},
        {"pcall",        luaB_pcall},
        {"printraw",     luaB_printraw},
        {"rawequal",     luaB_rawequal},
        {"rawlen",       luaB_rawlen},
        {"rawget",       luaB_rawget},
        {"rawset",       luaB_rawset},
        {"select",       luaB_select},
        {"setmetatableraw", luaB_setmetatableraw},
        {"tonumberraw",  luaB_tonumberraw},
        {"isintegerraw", luaB_isintegerraw},
        {"tostring",     luaB_tostring},
        {"type",         luaB_type},
        {"xpcall",       luaB_xpcall},
        /* placeholders */
        {"_G",       NULL},
        {"_VERSION", NULL},
        {NULL,       NULL}
};


LUAMOD_API void luaopen_base(lua_State *L) {
    /* open lib into global table */
    lua_pushglobaltable(L);
    luaL_setfuncs(L, base_funcs, 0);
    /* set global _G */
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "_G");
    /* set global _VERSION */
    lua_pushliteral(L, LUA_VERSION);
    lua_setfield(L, -2, "_VERSION");
}

