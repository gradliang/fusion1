
include ../../../config.mak

LUA_LIB = $(LIB)liblua.a
PDBTOOL_LIB = $(LIB)libpdbtool.a

TARGET = $(LUA_LIB) $(PDBTOOL_LIB)

all:
	$(MAKE) -C ../make/pdbtool
	$(MAKE) -C ../make/lua

clean:
	rm -f $(OBJ)\*.o $(OBJ)\*.lst $(TARGET)
