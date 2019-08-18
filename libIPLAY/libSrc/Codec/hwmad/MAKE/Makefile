ROOTPATH=../../../../
include $(ROOTPATH)config.mak

G_INC = $(ROOTPATH)include/
INC = ../include/
LINC = ../../../include/common
S_INC = ../../../stream/include/
OBJ = ../object/
LIB = $(ROOTPATH)lib/

SRC = ../source/


TARGET = $(LIB)libhwmad.a

 
#CC_OPTION  += $(INLINE_OPTION)
CC_OPTION_1  = $(CC_OPTION) $(OPT2) -I$(LINC) -I$(INC) -I$(S_INC) -I../../include/ -I$(G_INC)  -DHAVE_AV_CONFIG_H \
			-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE   -Wa,-al=
			
CC_OPTION  += $(MAIN_FLAG) -O2 -I$(LINC) -I$(INC) -I$(S_INC) -I../../include/ -I$(G_INC)  -DHAVE_AV_CONFIG_H \
			-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE   -Wa,-al=


 
AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld
 
AR = $(BIN)ar
 
S_OBJS =


C_OBJS =  $(OBJ)ad_libmad.o $(OBJ)bit.o $(OBJ)fixed.o $(OBJ)huffman.o \
		  $(OBJ)frame.o  $(OBJ)layer12.o  $(OBJ)libmad_stream.o 	  

C_OBJS_1 =  $(OBJ)layer3.o 
		  
ALL_OBJS =  $(C_OBJS) $(C_OBJS_1)

all: $(TARGET)

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

	
$(C_OBJS_1):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION_1)$(@:.o=.lst) -o $@ $<	

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(SRC)/*.bak  $(TARGET)

