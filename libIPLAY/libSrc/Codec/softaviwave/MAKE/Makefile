ROOTPATH=../../../../
include $(ROOTPATH)config.mak

G_INC = $(ROOTPATH)include/
INC = ../include/
LINC = ../../../include/common
S_INC = ../../../stream/include/
OBJ = ../object/
LIB = $(ROOTPATH)lib/

SRC = ../source/


TARGET = $(LIB)libsoftaviwave.a

 
#CC_OPTION  += $(MAIN_FLAG) -ggdb -O0 -I$(LINC) -I$(INC) -I$(S_INC) -I../../include/ -I$(G_INC)  -DHAVE_AV_CONFIG_H \
#			-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE   -Wa,-al=

CC_OPTION  += $(MAIN_FLAG) -O2 -I$(LINC) -I$(INC) -I$(S_INC) -I../../include/ -I$(G_INC)  -DHAVE_AV_CONFIG_H \
			-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE   -Wa,-al=


 
AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld
 
AR = $(BIN)ar
 
S_OBJS =


C_OBJS =  $(OBJ)softaviwave.o $(OBJ)mips_wave.o


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

