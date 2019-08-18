include ../../../config.mak

TARGET = $(LIB)libperipheral.a

CC = $(BIN)gcc
CC_OPTION += -march=r3000 -c -mno-abicalls -O2 -fno-strict-aliasing -G 8 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
#CC_OPTION += -march=r3000 -c -mno-abicalls -O1 -g -G 8 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS = $(OBJ)hal_timer.o $(OBJ)hal_iicm.o \
		$(OBJ)hal_gpio.o $(OBJ)hal_mmcp.o $(OBJ)hal_rtc.o

# $(OBJ)KeyPad.o $(OBJ)hal_ir.o $(OBJ)hal_iics.o $(OBJ)hal_adc.o

ALL_OBJS =  $(C_OBJS)

all: $(TARGET)

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(TARGET) $(OBJ)/*.o $(OBJ)/*.lst $(SRC)/*.bak
