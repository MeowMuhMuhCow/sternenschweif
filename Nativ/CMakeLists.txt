cmake_minimum_required(VERSION 3.16)
project(SchweifPort VERSION 0.1.0 LANGUAGES C CXX)

# ============================================================================
# DSA 2: Sternenschweif (C1.02 de) - Nativer Windows-Port (Etappe 1)
# ============================================================================
# Spielcode: Rekonstruierter C/C++-Quelltext (Quelle/, inkl. s_fix.h mit
#            allen Bugfixes FIX_BUGS 0/1/2, FIX_EXPLOITS, FIX_nnnn, s_plog.h
#            FIX_PROBENLOG).
# Plattform: Gemeinsame SDL2-Plattformschicht der Nordlandtrilogie
#            (Port/platform/dsa_platform_sdl2.cpp + Port/platform/schweif_asm_and_platform.cpp)
#            sowie C-Ersatz der 16 .ASM-Module (inkl. ppDecrunch nach
#            BrightEyes-SDL2, GPL v3).
# ============================================================================

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(FIX_BUGS "2" CACHE STRING "Fehlerbehebungsstufe fuer s_fix.h: 0=Original, 1=Wichtig, 2=Alles")
set(FIX_PROBENLOG "0" CACHE STRING "Probenlog (s_plog.h) aktivieren (1=ja, 0=nein)")
set(PORT_SKIP_COPYPROT "1" CACHE STRING "Kopierschutzabfrage standardmaessig ueberspringen (1=ja, 0=nein)")

# 1. Port/src/ aus Quelle/ vorbereiten, falls noch nicht geschehen
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/inc/schweif.h")
    message(STATUS "Bereite Sternenschweif-Quellen in Port/src/ vor...")
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/prepare_sources.py"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE PREP_RES
    )
    if(NOT PREP_RES EQUAL 0)
        message(FATAL_ERROR "prepare_sources.py fehlgeschlagen")
    endif()
endif()

# 2. Alle 301 C/C++-Module des Spiels sammeln (SEG001..SEG048 + Overlays/OVL000..OVL271)
file(GLOB SCHWEIF_C_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/SEG*.C"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Overlays/OVL*.C"
)
file(GLOB SCHWEIF_CPP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/SEG*.CPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Overlays/OVL*.CPP"
)

set_source_files_properties(${SCHWEIF_C_SOURCES} PROPERTIES LANGUAGE C)
set_source_files_properties(${SCHWEIF_CPP_SOURCES} PROPERTIES LANGUAGE CXX)

find_package(SDL2 QUIET)

add_executable(SCHWEIF_PORT
    ${SCHWEIF_C_SOURCES}
    ${SCHWEIF_CPP_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/port_bridges.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/schweif_asm_and_platform.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/dsa_platform_sdl2.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2/sdl2_win32_provider.c"
)

target_include_directories(SCHWEIF_PORT PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/compat"
    "${CMAKE_CURRENT_SOURCE_DIR}/platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2/include/SDL2"
)

target_compile_definitions(SCHWEIF_PORT PRIVATE
    FIX_BUGS=${FIX_BUGS}
    FIX_PROBENLOG=${FIX_PROBENLOG}
    PORT_SKIP_COPYPROT=${PORT_SKIP_COPYPROT}
    far=
    huge=
    near=
    interrupt=
    cdecl=
)

target_compile_options(SCHWEIF_PORT PRIVATE
    -m32
    -mabi=sysv
    -O2
    -fno-toplevel-reorder
    -fno-zero-initialized-in-bss
    -fno-common
    -fno-aggressive-loop-optimizations
    -fno-strict-aliasing
    -fwrapv
    -fno-delete-null-pointer-checks
    -fpack-struct=1
    -funsigned-char
    $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>
    -w
)

target_link_options(SCHWEIF_PORT PRIVATE
    -m32
    -Wl,--allow-multiple-definition
    -Wl,--wrap=_open
    -Wl,--wrap=open
    -Wl,--wrap=_creat
    -Wl,--wrap=creat
    -Wl,--wrap=_read
    -Wl,--wrap=read
    -Wl,--wrap=_write
    -Wl,--wrap=write
    -Wl,--wrap=_lseek
    -Wl,--wrap=lseek
    -Wl,--wrap=_close
    -Wl,--wrap=close
    -Wl,--wrap=_filelength
    -Wl,--wrap=filelength
    -Wl,--wrap=_tell
    -Wl,--wrap=tell
    -Wl,--wrap=_unlink
    -Wl,--wrap=unlink
    -Wl,--wrap=_chdir
    -Wl,--wrap=chdir
    -Wl,--wrap=_mkdir
    -Wl,--wrap=mkdir
    -Wl,--wrap=font_print_wrapped
    -Wl,--wrap=random_schweif
    "${CMAKE_CURRENT_SOURCE_DIR}/build/data_aliases.ld"
)

if(WIN32)
    target_link_libraries(SCHWEIF_PORT PRIVATE winmm gdi32 user32 shell32)
endif()

if(SDL2_FOUND)
    target_link_libraries(SCHWEIF_PORT PRIVATE ${SDL2_LIBRARIES})
endif()
