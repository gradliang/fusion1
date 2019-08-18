ROOTPATH=../../../../
include $(ROOTPATH)config.mak

G_INC = $(ROOTPATH)include/
INC = ../include/

S_INC = ../../../stream/include/
D_INC = ../../../demux/include/
OBJ = ../object/
LIB = $(ROOTPATH)lib/
SRC = ../source/
#SRC = ../../../../libsrc/FFMPEG/

TARGET = $(LIB)libffmpeg.a

CC_OPTION  += -O2 -I$(INC) -I../../include/  -I$(G_INC) \
			 -DHAVE_AV_CONFIG_H -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE \
			-D_GNU_SOURCE $(INLINE_OPTION)  -Wa,-al=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS =  $(OBJ)amr.o $(OBJ)ad_ffmpeg.o $(OBJ)sram_init.o

ALL_OBJS =  $(C_OBJS)

all: $(TARGET)

$(TARGET)	: $(ALL_OBJS)
	rm -f $(TARGET)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(SRC)/*.bak  $(TARGET)




