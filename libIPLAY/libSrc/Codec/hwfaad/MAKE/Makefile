ROOTPATH=../../../../
include $(ROOTPATH)config.mak

G_INC = $(ROOTPATH)include/
INC = ../include/
OBJ = ../object/
LIB = $(ROOTPATH)lib/
SRC = ../source/


TARGET = $(LIB)libhwfaad.a


CC_OPTION  += $(MAIN_FLAG) -I$(INC) -I../../include/ -I$(G_INC)  -DHAVE_AV_CONFIG_H -D_FILE_OFFSET_BITS=64  \
				-D_LARGEFILE_SOURCE -D_GNU_SOURCE $(INLINE_OPTION) -Wa,-al=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS =  $(OBJ)ad_faad.o $(OBJ)bits.o	 $(OBJ)mp4.o   \
          $(OBJ)common.o   $(OBJ)huffman.o	 $(OBJ)decoder.o  $(OBJ)syntax.o \
	        $(OBJ)pulse.o 	  

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



