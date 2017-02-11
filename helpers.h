#ifndef LUAZERO_HELPERS_H
#define LUAZERO_HELPERS_H

#include <stddef.h>
#include <stdint.h>

#include </usr/local/include/libbracket.h>

void abort(void) __attribute__((noreturn));

intptr_t time(intptr_t *tout);

void lua_writestring_default(const char *s, size_t len);

void lua_writeerror_default(const char *s, size_t len);

typedef void *deepptr;

int deepcall(void (*target)(void *), void *param, deepptr *saveaddr);

void deepret(deepptr loadaddr) __attribute__((noreturn));

#endif //LUAZERO_HELPERS_H
