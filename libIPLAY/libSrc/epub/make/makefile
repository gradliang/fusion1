include ../../../config.mak
 
TARGET = $(LIB)libepub.a

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 4 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 0 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS = $(OBJ)einfo.o $(OBJ)epub.o $(OBJ)linklist.o $(OBJ)list.o $(OBJ)ocf.o\
				 $(OBJ)opf.o $(OBJ)xmlepub.o $(OBJ)avltree.o $(OBJ)xmlepubtag.o

ALL_OBJS =  $(C_OBJS) $(S_OBJS)

all: $(TARGET) makefile

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst  $(TARGET)