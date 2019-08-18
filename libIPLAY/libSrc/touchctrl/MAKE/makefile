#ROOTPATH=../../../
include ../../../config.mak


TARGET = $(LIB)libtouchctrl.a

CC = $(BIN)gcc
CC_OPTION += -march=r3000 -O2 -c -mno-abicalls -fno-strict-aliasing -G 8 -msoft-float -I$(INC) -I$(G_INC) -Wa,-ahl=
#CC_OPTION = -march=r3000 -O0 -ggdb -c -mno-abicalls -G 8 -msoft-float -I$(INC) -I$(G_INC) -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS = $(OBJ)touchCtrller.o $(OBJ)tcapi.o 
C_OBJS += $(OBJ)tc_gt911.o 
#C_OBJS += $(OBJ)tc_ak4183.o 
#C_OBJS += $(OBJ)tc_a043vl01.o 
#C_OBJS += $(OBJ)tc_ft5306.o
#C_OBJS += $(OBJ)tc_icn8502.o 

ALL_OBJS =  $(S_OBJS) $(C_OBJS)

all: $(TARGET)

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)


$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst  $(TARGET)


