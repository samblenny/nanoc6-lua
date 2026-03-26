# components/lua-5.4.8/CMakeLists.txt

idf_component_register(
    SRCS
        src/lapi.c
        src/lauxlib.c
        src/lbaselib.c
        src/lcode.c
        src/ldebug.c
        src/ldo.c
        src/lfunc.c
        src/lgc.c
        src/llex.c
        src/lmem.c
        src/lobject.c
        src/lopcodes.c
        src/lparser.c
        src/lstate.c
        src/lstring.c
        src/ltable.c
        src/ltm.c
        src/lundump.c
        src/lvm.c
        src/lzio.c
        src/linit.c        # customized init file
#        src/lua.c          # interpreter
#        src/ldblib.c       # optional (debug module)
        src/lutf8lib.c     # optional
#        src/loslib.c       # optional (os module)
        src/ltablib.c
#        src/liolib.c       # optional (io module)
#        src/loadlib.c      # optional (package module for require, etc)
    INCLUDE_DIRS "src"     # for .h includes
)

# Make headers available to other components
target_include_directories(${COMPONENT_LIB} PUBLIC src)

