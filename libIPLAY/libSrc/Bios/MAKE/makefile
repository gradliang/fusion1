include ../../../config.mak

SRC = ../source/
#SRC = ../../../libsrc/BIOS/

TARGET = $(LIB)libbios.a

CC_OPTION += -march=r3000 -c -mno-abicalls -freg-struct-return -O2 -fno-strict-aliasing -G 8 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
#CC_OPTION += -march=r3000 -c -mno-abicalls -freg-struct-return -O1 -g -G 8 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
SS_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -freg-struct-return -O0 -G 8 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

S_OBJS =  $(OBJ)startup.o \
		$(OBJ)exception.o \

SS_OBJS =  #$(OBJ)exceptionC.o \

C_OBJS = $(OBJ)agent.o $(OBJ)MP650_uart.o $(OBJ)MP650_BTuart.o $(OBJ)hal_Ckg.o \
		$(OBJ)string.o \
#$(OBJ)MP615_uart.o $(OBJ)MP615_BTuart.o 
ALL_OBJS =  $(S_OBJS) $(SS_OBJS) $(C_OBJS)

all: $(TARGET) makefile

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(SS_OBJS):	$(OBJ)%.o: $(SRC)%.S makefile
	$(CC) $(SS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(OBJ)/*.o $(OBJ)/*.lst $(TARGET)
