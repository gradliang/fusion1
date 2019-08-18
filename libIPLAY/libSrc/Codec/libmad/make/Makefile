ROOTPATH=../../../../
include $(ROOTPATH)config.mak

G_INC = $(ROOTPATH)include/
INC = ../include/
LINC = ../../../include/common
S_INC = ../../../stream/include/
OBJ = ../object/
LIB = $(ROOTPATH)lib/
SRC = ../source/


TARGET = $(LIB)libmad.a

 
#CC_OPTION  += $(INLINE_OPTION)
#CC_OPTION_1  = $(CC_OPTION) -O0 -ggdb -I$(LINC) -I$(INC) -I$(S_INC) -I../../include/ -I$(G_INC)  -DHAVE_AV_CONFIG_H \
#			-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE   -Wa,-al=

#CC_OPTION  += $(MAIN_FLAG) -O0 -ggdb -I$(LINC) -I$(INC) -I$(S_INC) -I../../include/ -I$(G_INC)  -DHAVE_AV_CONFIG_H \
#			-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE -DHAVE_CONFIG_H  -Wa,-al=

CC_OPTION_1  = $(CC_OPTION) -O2 -I$(LINC) -I$(INC) -I$(S_INC) -I../../include/ -I$(G_INC)  -DHAVE_AV_CONFIG_H \
			-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE   -Wa,-al=

CC_OPTION  += $(MAIN_FLAG) -O2 -I$(LINC) -I$(INC) -I$(S_INC) -I../../include/ -I$(G_INC)  -DHAVE_AV_CONFIG_H \
			-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE -DHAVE_CONFIG_H  -Wa,-al=


#BIN = C:\Magic-Pixel\tools\mips\bin\mips-elf-mpx-
CC = $(BIN)gcc
AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=
LD = $(BIN)ld
AR = $(BIN)ar
 
C_OBJS =  $(OBJ)mp3_info.o $(OBJ)bit.o $(OBJ)fixed.o $(OBJ)huffman.o $(OBJ)synth.o \
		  $(OBJ)frame.o $(OBJ)layer3.o $(OBJ)layer12.o  $(OBJ)stream.o $(OBJ)timer.o $(OBJ)sram.o \
		  $(OBJ)ad_libmad.o $(OBJ)mad_mp3.o

#ad_libmad.c and mad_mp3.c will move to other place in the future!!!		C.W 081008
	      
S_OBJS =

ALL_OBJS =  $(C_OBJS) $(C_OBJS_1) $(S_OBJS)

all: $(TARGET)

$(TARGET)	: $(C_OBJS) $(S_OBJS)
	$(AR) -rsv $(TARGET) $(C_OBJS) $(S_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS_1):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION_1)$(@:.o=.lst) -o $@ $<	

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(SRC)/*.bak  $(TARGET)

