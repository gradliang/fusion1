include ../../../config.mak

ROOTPATH=../../../

TARGET = $(LIB)libRecord.a

CC = $(BIN)gcc
CC_OPTION = -O2 -fno-strict-aliasing -march=r3000 -c -mno-abicalls -G 8 -I$(INC) -I$(G_INC) -I../../sensor/include -Wa,-al=
#CC_OPTION = -O1 -g -march=r3000 -c -mno-abicalls -G 8 -I$(INC) -I$(G_INC) -I../../sensor/include -Wa,-al=

MK_WIFI = $(word $(words $(shell grep 'NETWARE_ENABLE' $(ROOTPATH)include\corelib.h)),$(shell grep 'NETWARE_ENABLE' $(ROOTPATH)include\corelib.h))
MK_USB = $(word $(words $(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h))
MK_ADHOC = $(word $(words $(shell grep 'Make_ADHOC' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_ADHOC' $(ROOTPATH)include\corelib.h))

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -al=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

ifeq ($(MK_ADHOC),1)
ifeq ($(MK_WIFI),1)
ifeq ($(MK_USB),3)
C_OBJS += $(OBJ)record_task_wifi.o
else
C_OBJS += $(OBJ)record_task.o
endif
else
C_OBJS += $(OBJ)record_task.o
endif
else
C_OBJS += $(OBJ)record_task.o
endif

#C_OBJS +=  $(OBJ)setting_recording.o
#	$(OBJ)motion_detection.o 

ALL_OBJS =  $(S_OBJS) $(C_OBJS)

all: $(TARGET) makefile


$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst  $(TARGET)
