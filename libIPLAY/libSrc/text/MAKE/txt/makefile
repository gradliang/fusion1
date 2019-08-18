
include ../../../../config.mak

TARGET = ../$(LIB)libtxt.a

TXT_OBJ = ../$(OBJ)
SRC = ../../source/txt/


CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -fno-strict-aliasing -mno-abicalls -msoft-float -O2 -G 8 -I../$(INC) -I../$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -fno-strict-aliasing -mno-abicalls -msoft-float -O -ggdb -G 0 -I../$(INC) -I../$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
LD = $(BIN)ld

AR = $(BIN)ar

#------------------------------------------------------------------------
	
C_OBJS = \
	$(TXT_OBJ)txt.o \
	$(TXT_OBJ)txt_mpx.o \
	
ALL_OBJS = $(C_OBJS)
	
#------------------------------------------------------------------------
	
all: $(TARGET)

$(TARGET): $(ALL_OBJS)
	rm -f $(TARGET)
	$(AR) -rv $(TARGET) $(ALL_OBJS)

$(ALL_OBJS):	$(TXT_OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

#------------------------------------------------------------------------

clean:
	rm -f $(ALL_OBJS) $(TARGET)

