
include ../../../config.mak


TARGET = $(LIB)libtext.a

CC = $(BIN)gcc
CC_OPTION += -march=r3000 -O2 -fno-strict-aliasing -c -mno-abicalls -G 8 -I$(INC) -I$(G_INC) -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS = $(OBJ)txt_Big5ToUnicode.o  $(OBJ)txt_Gb2312ToUnicode.o  $(OBJ)txt_Utf8ToUnicode.o 

ALL_OBJS =  $(S_OBJS) $(C_OBJS)

all: $(TARGET) makefile
	$(MAKE) -C ../make/txt

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)


$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.o $(OBJ)/*.lst  $(TARGET)


