
.globl lua_start_lua
.globl lua_start_lua_end
.section .rodata
.align 8

lua_start_lua:
    .incbin "_start.lua"
lua_start_lua_end:
    .byte 0

