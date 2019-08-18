ROOTPATH=../../../../
include $(ROOTPATH)config.mak

G_INC = $(ROOTPATH)include/
INC = ../include/
OBJ = ../object/
LIB = $(ROOTPATH)lib/
SRC = ../source/


TARGET = $(LIB)libmp4ff.a

#CC_OPTION  += -DUSE_TAGGING=1
CC_OPTION  += $(MAIN_FLAG) -O2 -I$(INC) -I../../include/ -I$(G_INC) -DHAVE_AV_CONFIG_H \
				-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE $(INLINE_OPTION) -Wa,-al=

#CC_OPTION  += $(MAIN_FLAG) -O0 -ggdb -I$(INC) -I../../include/ -I$(G_INC) -DHAVE_AV_CONFIG_H \
#				-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE $(INLINE_OPTION) -Wa,-al=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=
LD = $(BIN)ld
AR = $(BIN)ar

C_OBJS =  $(OBJ)mp4ff.o $(OBJ)mp4atom.o $(OBJ)mp4meta.o $(OBJ)mp4sample.o $(OBJ)mp4util.o \
		  $(OBJ)mp4tagupdate.o

S_OBJS = 

ALL_OBJS =  $(C_OBJS)

all: $(TARGET)

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(SRC)/*.bak  $(TARGET)
