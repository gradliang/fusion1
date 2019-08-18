
include ../../../../config.mak

TARGET = ../$(LIB)libpdbtool.a

PDBTOOL_OBJ = ../$(OBJ)
SRC = ../../source/pdbtool/


CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -fno-strict-aliasing -mno-abicalls -msoft-float -O2 -G 4 -I../$(INC) -I../$(G_INC) -I../$(INC)pdbtool -I../$(INC)lua -DBOOT_ROM_TYPE -Wa,-ahl=
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -fno-strict-aliasing -mno-abicalls -msoft-float -O -ggdb -G 0 -I../$(INC) -I../$(G_INC) -I../$(INC)pdbtool -I../$(INC)lua -DBOOT_ROM_TYPE -Wa,-ahl=
LD = $(BIN)ld

AR = $(BIN)ar

#------------------------------------------------------------------------
	
C_OBJS = \
	$(PDBTOOL_OBJ)pdbio.o \
	$(PDBTOOL_OBJ)pdblua.o \
	$(PDBTOOL_OBJ)pdbrep.o \
	$(PDBTOOL_OBJ)pushchar.o \
	$(PDBTOOL_OBJ)shop.o \
	$(PDBTOOL_OBJ)jfile.o \
	$(PDBTOOL_OBJ)mobile.o \
	$(PDBTOOL_OBJ)pdbconv.o \
  $(PDBTOOL_OBJ)pdb.o \
	$(PDBTOOL_OBJ)todo.o \
	$(PDBTOOL_OBJ)pdb_mpx.o \
	

ALL_OBJS = $(C_OBJS)
	
#------------------------------------------------------------------------
	
all: $(TARGET)

$(TARGET): $(ALL_OBJS)
	rm -f $(TARGET)
	$(AR) -rv $(TARGET) $(ALL_OBJS)

$(ALL_OBJS):	$(PDBTOOL_OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

#------------------------------------------------------------------------

clean:
	rm -f $(ALL_OBJS) $(TARGET)

