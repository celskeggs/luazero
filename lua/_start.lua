local math = {}

function math.type(x)
    if type(x) == "number" then
        if isintegerraw(x) then
            return "integer"
        else
            return "float"
        end
    end -- otherwise, nil
end

function assert(cond, message, ...)
    if cond then
        return cond, message, ...
    else
        error(message or "assertion failed!", 1)
    end
end

function print(...)
    local n = select("#", ...)
    for i = 1, n do
        local str = tostring(select(i, ...))
        if type(str) ~= "string" then
            error("'tostring' must return a string to 'print'")
        end
        if i > 1 then printraw("\t") end
        printraw(str)
    end
    printraw("\n")
end

local ALNUM = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"

local function isalnum1(char, si) -- just checks first char
    local n = getbyteraw(char, si)
    return n and ((n >= 48 and n <= 57) or (n >= 65 and n <= 90) or (n >= 97 and n <= 122))
end

local function asdigit(char, si) -- just checks first char
    local n = getbyteraw(char, si)
    return n and ((n >= 48 and n <= 57 and (n - 48)) or (n >= 65 and n <= 90 and (n - 55)) or (n >= 97 and n <= 122 and (n - 87)))
end

local function isspace(char, si) -- just checks first char
    local n = getbyteraw(char, si)
    return n and (n == 32 or (n >= 9 and n <= 13))
end

local function b_str2int(s, base)
    local si = 1
    while #s >= si and isspace(s, si) do
        si = si + 1
    end
    local neg = false
    if getbyteraw(s, si) == getbyteraw("-", 1) then
        si = si + 1
        neg = true
    end
    if not isalnum1(s, si) then print("FAIL1", s, si) return nil end
    local n = 0
    repeat
        local digit = asdigit(s, si)
        if digit >= base then print("FAIL2", s, si) return nil end
        n = n * base + digit
        print("LEND", s, digit, si, n)
        si = si + 1
    until not isalnum1(s, si)
    while #s >= si and isspace(s, si) do
        si = si + 1
    end
    if #s >= si then print("FAIL3", s, si) return nil end
    if neg then n = -n end
    return n
end

function tonumber(e, base)
    if not base then
        if type(e) == "number" then
            return e
        elseif type(e) == "string" then
            return tonumberraw(e)
        end -- otherwise return nil
    else
        assert(type(base) == "number" and math.type(base) == "integer", "non-integer base to tonumber")
        assert(type(e) == "string", "non-string argument to tonumber with a base")
        if base < 2 or base > 36 then
            error("base out of range")
        end
        return b_str2int(e, base)
    end
end

function getmetatable(obj)
    local mt = getmetatableraw(obj)
    if mt == nil then
        return nil
    else
        local tt = rawget(mt, "__metatable")
        if tt == nil then
            return mt
        else
            return tt
        end
    end
end

function setmetatable(table, mt)
    if type(table) ~= "table" or (mt ~= nil and type(mt) ~= "table") then
        error("bad argument to setmetatable")
    end
    local mt2 = getmetatableraw(table)
    if mt2 ~= nil and rawget(mt2, "__metatable") ~= nil then
        error("cannot change a protected metatable")
    end
    setmetatableraw(table, mt)
    return table
end

function pairs(t) end

static int luaB_pairs(lua_State *L) {
    if (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
        luaL_checktype(L, 1, LUA_TTABLE);  /* argument must be a table */
        lua_pushcfunction(L, luaB_next);  /* will return generator, */
        lua_pushvalue(L, 1);  /* state, */
        lua_pushnil(L);
    } else {
        lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
        lua_call(L, 1, 3);  /* get 3 values from metamethod */
    }
    return 3;
}

print("Hello, World!", 10)
print(tonumber(12345), tonumber("54321", 9))
