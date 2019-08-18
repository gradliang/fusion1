
include ../../../config.mak

TARGET = $(LIB)libos.a

CC = $(BIN)gcc
CC_OPTION += -O2 -fno-strict-aliasing -march=r3000 -c -mno-abicalls -G 8 -I$(INC) -I$(G_INC) -I$(G_INC) -Wa,-al=
#CC_OPTION += -O1 -g -march=r3000 -c -mno-abicalls -G 8 -I$(INC) -I$(G_INC) -I$(G_INC) -Wa,-al=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -al=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS =  	$(OBJ)cosmos.o  $(OBJ)task.o   $(OBJ)object.o	$(OBJ)event.o   $(OBJ)semaphore.o  $(OBJ)mailbox.o \
			$(OBJ)message.o $(OBJ)memory.o $(OBJ)systimer.o $(OBJ)interrupt.o

ALL_OBJS =  $(S_OBJS) $(C_OBJS)

all: $(TARGET) makefile


$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)


$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile $(INC)osinner.h $(INC)osconfig.h
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(G_INC)utilregfile.h	$(G_INC)utiltypedef.h	$(G_INC)devio.h

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst  $(TARGET)
