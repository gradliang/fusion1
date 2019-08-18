
include ../../../config.mak

LINC = ../../include/common

VPATH   = $(SRC)

TARGET = $(LIB)libaudio.a

#CC_OPTION += -ggdb 
CC_OPTION += -O0 -DAUDIO_SELF_THREAD
CC_OPTION += -DMX6xx_AUDIO_SUPPORT

CC_OPTION  += $(MAIN_FLAG) -I$(INC) -I$(G_INC) 
CC_OPTION  += $(INLINE_OPTION)  -Wa,-al=

AS_OPTION +=  -I$(G_INC) -ahl=

C_OBJS =  $(OBJ)audio.o $(OBJ)audio_hal.o $(OBJ)audioSimInterface.o $(OBJ)audioDac_CS4334.o

#          $(OBJ)audioDac_ac97.o $(OBJ)audioDac_MPX.o $(OBJ)audioDac_AK4387.o \
#          $(OBJ)audioDac_WM8750L.o $(OBJ)audioDac_WM8961.o $(OBJ)audioVerify.o \
#          $(OBJ)audioRecordVerify.o	$(OBJ)audioDac_AD9889B.o	\
#          $(OBJ)audioDac_WM8960.o $(OBJ)audioDac_ES7240.o $(OBJ)audioDac_WM8904.o \
#          $(OBJ)audioDac_ES8328.o $(OBJ)audioDac_ALC5621.o $(OBJ)audioDac_CS4334.o

ALL_OBJS =  $(C_OBJS)

all: $(TARGET)

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: %.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: %.c  makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<
	
clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst  $(TARGET)



