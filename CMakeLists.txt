cmake_minimum_required(VERSION 3.6)
project(luazero)
enable_language(C ASM)

FIND_LIBRARY(LIBSYSCALL libsyscall)

execute_process(COMMAND ${CMAKE_C_COMPILER} -print-search-dirs
        COMMAND sed -n -e "s/install: \\\(.*\\\)/\\1/p"
        COMMAND tr -s "\n" "/"
        OUTPUT_VARIABLE CC_BUILTIN_ROOT)
set(CC_BUILTIN_INC ${CC_BUILTIN_ROOT}include)
add_definitions("-std=gnu11 -m64 -nostdinc -I ${CC_BUILTIN_INC} -ffreestanding -Wall -Wextra -O2")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -m64")

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/luaembed_final.s
        COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/luaembed.s ${CMAKE_CURRENT_BINARY_DIR}/luaembed_final.s
        DEPENDS luaembed.s lua/_start.lua)

add_library(luazero lmem.c lgc.h lobject.h llimits.h lua.h luaconf.h lstate.h ltm.h lzio.h lmem.h ldebug.h ldo.h
        lauxlib.h lauxlib.c helpers.c helpers.h dlmalloc.c dlmalloc.h dlmalloctweaks.h lstate.c lapi.h lfunc.h llex.h
        lstring.h ltable.h lapi.c lundump.h lvm.h ldebug.c lcode.h lopcodes.h lparser.h ltable.c lobject.c lctype.h
        lstring.c ltm.c llex.c lzio.c lfunc.c lgc.c ldo.c lparser.c lvm.c lctype.c lcode.c lopcodes.c ldump.c lundump.c
        lbaselib.c libs.h)

add_executable(lua main.c ${CMAKE_CURRENT_BINARY_DIR}/luaembed_final.s)
target_include_directories(lua PUBLIC lua)
target_link_libraries(lua luazero -lsyscall -lbracket)
