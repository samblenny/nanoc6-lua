# components/lua-5.4.8/CMakeLists.txt

idf_component_register(
    SRCS
        src/lapi.c
        src/lauxlib.c
        src/lbaselib.c
        src/lcode.c
        src/lcorolib.c
        src/lctype.c
#        src/ldblib.c     # optional (debug module)
        src/ldebug.c     # debug stuff (required)
        src/ldo.c
        src/ldump.c
        src/lfunc.c
        src/lgc.c
        src/linit.c       # customized init file
#        src/liolib.c     # optional (io module)
        src/llex.c
        src/lmathlib.c
        src/lmem.c
#        src/loadlib.c    # optional (package module for require, etc)
        src/lobject.c
        src/lopcodes.c
#        src/loslib.c     # optional (os module)
        src/lparser.c
        src/lstate.c
        src/lstring.c
        src/lstrlib.c
        src/ltable.c
        src/ltablib.c
        src/ltm.c
        src/lundump.c
        src/lutf8lib.c
        src/lvm.c
        src/lzio.c

    INCLUDE_DIRS "src"     # for .h includes
)

# Make headers available to other components
target_include_directories(${COMPONENT_LIB} PUBLIC src)

