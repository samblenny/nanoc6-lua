# Makefile for building Lua 5.4 as one file

SRC= ..
include $(SRC)/Makefile

F= one
ALL= $F
ALL_O= 
LUA_A= liblua.o
THECFLAGS= $(CFLAGS) -I$(SRC)
THELDFLAGS= $(LDFLAGS) $(LIBS)

$F:
	$(CC) -o $(LUA_A)  $(THECFLAGS) -DMAKE_LIB -c $F.c
	$(CC) -o $(LUA_T)  $(THECFLAGS) -DMAKE_LUA  $F.c $(THELDFLAGS)
	$(CC) -o $(LUAC_T) $(THECFLAGS) -DMAKE_LUAC $F.c $(THELDFLAGS)
