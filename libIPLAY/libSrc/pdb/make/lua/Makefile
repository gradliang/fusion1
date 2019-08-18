
include ../../../../config.mak

TARGET = ../$(LIB)liblua.a

LUA_OBJ = ../$(OBJ)
SRC = ../../source/lua/


CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -fno-strict-aliasing -mno-abicalls -msoft-float -O2 -G 4 -I../$(INC) -I../$(G_INC) -I../$(INC)lua -DBOOT_ROM_TYPE -Wa,-ahl=

LD = $(BIN)ld

AR = $(BIN)ar

#------------------------------------------------------------------------
	
C_OBJS = \
	$(LUA_OBJ)lapi.o \
	$(LUA_OBJ)lauxlib.o \
	$(LUA_OBJ)lbuffer.o \
	$(LUA_OBJ)lbuiltin.o \
	$(LUA_OBJ)ldo.o \
	$(LUA_OBJ)lfunc.o \
	$(LUA_OBJ)lgc.o \
	$(LUA_OBJ)llex.o \
	$(LUA_OBJ)lmem.o \
	$(LUA_OBJ)lobject.o \
	$(LUA_OBJ)lparser.o \
	$(LUA_OBJ)lstate.o \
	$(LUA_OBJ)lstring.o \
	$(LUA_OBJ)ltable.o \
	$(LUA_OBJ)ltm.o \
	$(LUA_OBJ)lundump.o \
	$(LUA_OBJ)lvm.o \
	$(LUA_OBJ)lzio.o \
  $(LUA_OBJ)linit.o \
  $(LUA_OBJ)ldblib.o \
  $(LUA_OBJ)liolib.o \
  $(LUA_OBJ)lmathlib.o \
  $(LUA_OBJ)lstrlib.o \
  $(LUA_OBJ)lua.o \
  $(LUA_OBJ)luac.o \
  $(LUA_OBJ)dump.o \
  $(LUA_OBJ)opcode.o \
  $(LUA_OBJ)opt.o \
  $(LUA_OBJ)print.o \
  $(LUA_OBJ)test.o \
# $(LUA_OBJ)stubs.o \

ALL_OBJS = $(C_OBJS)
	
#------------------------------------------------------------------------
	
all: $(TARGET)

$(TARGET): $(ALL_OBJS)
	rm -f $(TARGET)
	$(AR) -rv $(TARGET) $(ALL_OBJS)

$(ALL_OBJS):	$(LUA_OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

#------------------------------------------------------------------------

clean:
	rm -f $(ALL_OBJS) $(TARGET)

