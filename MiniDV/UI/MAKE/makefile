include ../../config.mak


INC = ../include/
OBJ = ../object/
SRC = ../source/

VPATH   = $(SRC)
 
TARGET = $(LIBMAIN)libui.a

CC = $(BIN)gcc
#CC_OPTION += -O1 -g
CC_OPTION += -O1 -fno-strict-aliasing
CC_OPTION += -I$(INC) -I$(G_INC) -I../../main/INCLUDE -I../../../libIPLAY/libSrc/xpg/include -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

#C_OBJS = $(OBJ)Ir.o $(OBJ)uigpio.o $(OBJ)KeyPad.o $(OBJ)alarm.o  $(OBJ)rtc.o $(OBJ)rtcData.o $(OBJ)rtcFont.o \
#		 $(OBJ)uiTouchCtrller.o $(OBJ)Clock.o $(OBJ)Calendar.o
          
C_OBJS =$(OBJ)uigpio.o $(OBJ)uiTouchCtrller.o

ALL_OBJS =  $(C_OBJS) $(S_OBJS)

all: $(TARGET) makefile

$(TARGET)	: $(ALL_OBJS) makefile
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: %.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: %.c  makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst  $(TARGET)

