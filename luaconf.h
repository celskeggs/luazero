/*
** $Id: luaconf.h,v 1.255 2016/05/01 20:06:09 roberto Exp $
** Configuration file for Lua
** See Copyright Notice in lua.h
*/


#ifndef luaconf_h
#define luaconf_h

#include <stddef.h>
#include <stdint.h>
#include "helpers.h"

// might be supposed to be 32 instead
#define LUAI_BITSINT    64

#if !defined(LUA_INT_TYPE)
#define LUA_INT_TYPE    int64_t
#endif

#if !defined(LUA_FLOAT_TYPE)
#define LUA_FLOAT_TYPE    double
#endif

#define LUA_API        extern

/* more often than not the libs go together with the core */
#define LUALIB_API    LUA_API
#define LUAMOD_API    LUALIB_API

/*
@@ LUAI_FUNC is a mark for all extern functions that are not to be
** exported to outside modules.
@@ LUAI_DDEF and LUAI_DDEC are marks for all extern (const) variables
** that are not to be exported to outside modules (LUAI_DDEF for
** definitions and LUAI_DDEC for declarations).
** CHANGE them if you need to mark them in some special way. Elf/gcc
** (versions 3.2 and later) mark them as "hidden" to optimize access
** when Lua is compiled as a shared library. Not all elf targets support
** this attribute. Unfortunately, gcc does not offer a way to check
** whether the target offers that support, and those without support
** give a warning about it. To avoid these warnings, change to the
** default definition.
*/
#if defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 302) && \
    defined(__ELF__)        /* { */
#define LUAI_FUNC    __attribute__((visibility("hidden"))) extern
#else				/* }{ */
#define LUAI_FUNC	extern
#endif				/* } */

#define LUAI_DDEC    LUAI_FUNC
#define LUAI_DDEF    /* empty */

/*
** {==================================================================
** Configuration for Numbers.
** Change these definitions if no predefined LUA_FLOAT_* / LUA_INT_*
** satisfy your needs.
** ===================================================================
*/

/*
@@ LUA_NUMBER is the floating-point type used by Lua.
@@ LUAI_UACNUMBER is the result of an 'usual argument conversion'
@@ over a floating number.
@@ l_mathlim(x) corrects limit name 'x' to the proper float type
** by prefixing it with one of FLT/DBL/LDBL.
@@ LUA_NUMBER_FRMLEN is the length modifier for writing floats.
@@ LUA_NUMBER_FMT is the format for writing floats.
@@ lua_number2str converts a float to a string.
@@ l_mathop allows the addition of an 'l' or 'f' to all math operations.
@@ l_floor takes the floor of a float.
@@ lua_str2number converts a decimal numeric string to a number.
*/


/* The following definitions are good for most cases here */

#define l_floor(x)        (l_mathop(floor)(x))

#define lua_number2str(s, sz, n)    (dtostr((n), (s), (sz)))

/*
@@ lua_numbertointeger converts a float number to an integer, or
** returns 0 if float is not within the range of a lua_Integer.
** (The range comparisons are tricky because of rounding. The tests
** here assume a two-complement representation, where MININTEGER always
** has an exact representation as a float; MAXINTEGER may not have one,
** and therefore its conversion to float may have an ill-defined value.)
*/
#define lua_numbertointeger(n, p) \
  ((n) >= (LUA_NUMBER)(LUA_MININTEGER) && \
   (n) < -(LUA_NUMBER)(LUA_MININTEGER) && \
      (*(p) = (LUA_INTEGER)(n), 1))

/* now the variable definitions */

#define LUA_NUMBER    double

#define l_mathlim(n)        (DBL_##n)

#define LUAI_UACNUMBER    double

#define LUA_NUMBER_FRMLEN    ""
#define LUA_NUMBER_FMT        "%.14g"

#define l_mathop(op)        op

#define lua_str2number(s, p)    strtod((s), (p))

/*
@@ LUA_INTEGER is the integer type used by Lua.
**
@@ LUA_UNSIGNED is the unsigned version of LUA_INTEGER.
**
@@ LUAI_UACINT is the result of an 'usual argument conversion'
@@ over a lUA_INTEGER.
@@ LUA_INTEGER_FRMLEN is the length modifier for reading/writing integers.
@@ LUA_INTEGER_FMT is the format for writing integers.
@@ LUA_MAXINTEGER is the maximum value for a LUA_INTEGER.
@@ LUA_MININTEGER is the minimum value for a LUA_INTEGER.
@@ lua_integer2str converts an integer to a string.
*/

/* The following definitions are good for most cases here */

#define lua_integer2str(s, sz, n)    (itostr((n), (s), (sz)))

#define LUAI_UACINT        int64_t
#define LUA_UNSIGNED        uint64_t
#define LUA_INTEGER int64_t
#define LUA_INTEGER_FRMLEN "ll"
#define LUA_MAXINTEGER        INT64_MAX
#define LUA_MININTEGER        INT64_MIN

#define LUA_KCONTEXT    intptr_t

/*
@@ LUA_USE_APICHECK turns on several consistency checks on the C API.
** Define it as a help when debugging C code.
*/
#if defined(LUA_USE_APICHECK)
#include <assert.h>
#define luai_apicheck(l,e)	assert(e)
#endif

/*
@@ LUAI_MAXSTACK limits the size of the Lua stack.
** CHANGE it if you need a different limit. This limit is arbitrary;
** its only purpose is to stop Lua from consuming unlimited stack
** space (and to reserve some numbers for pseudo-indices).
*/
#define LUAI_MAXSTACK        1000000

/*
@@ LUA_EXTRASPACE defines the size of a raw memory area associated with
** a Lua state with very fast access.
** CHANGE it if you need a different size.
*/
#define LUA_EXTRASPACE        (sizeof(void *))

/*
@@ LUA_IDSIZE gives the maximum size for the description of the source
@@ of a function in debug information.
** CHANGE it if you want a different size.
*/
#define LUA_IDSIZE    60

#define LUAL_BUFFERSIZE   ((int)(0x80 * sizeof(void*) * sizeof(lua_Integer)))

#endif

