include ../../../config.mak



TARGET = $(LIB)libtcons.a

CC = $(BIN)gcc
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O1 -G 8 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 8 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
CC_OPTION += -I$(INC) -I$(G_INC) -I$(BT_INC) -I$(B_INC) -DBOOT_ROM_TYPE -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS = $(OBJ)kmodule.o

C_OBJS = $(OBJ)tcons.o $(OBJ)tagents.o $(OBJ)elf-reloc.o

ALL_OBJS =  $(S_OBJS) $(C_OBJS)

all: $(TARGET)


$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile $(G_INC)UtilRegFile.h
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(TARGET)
	
