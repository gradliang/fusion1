include ../../../config.mak

VPATH   = $(SRC)

TARGET = $(LIB)libsubtitle.a


CC = $(BIN)gcc
#CC_OPTION += -O0 -g
CC_OPTION += -O2 -I$(INC) -I$(G_INC) -DAUDIO_SELF_THREAD -I../../xpg/include
CC_OPTION += -DMX6xx_AUDIO_SUPPORT 
CC_OPTION += -finline-limit=256 -finline-functions

CC_OPTION  +=  -DWORDS_BIGENDIAN  -Wa,-al=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS = 

C_OBJS =  $(OBJ)SAMI_parser.o $(OBJ)SubRip_parser.o $(OBJ)sub_common.o $(OBJ)sub_stream.o $(OBJ)sub_stream_file.o \
          


ALL_OBJS =  $(C_OBJS) $(S_OBJS)

all: $(TARGET) makefile

$(TARGET): $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: %.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: %.c  makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(TARGET)



