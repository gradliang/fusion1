ROOTPATH=../../../../
include $(ROOTPATH)config.mak

G_INC = $(ROOTPATH)include/
LINC = ../../../include/common
S_INC = ../../../stream/include/
LIB = $(ROOTPATH)lib/

TARGET = $(LIB)libutilities.a


#CC_OPTION  += $(MAIN_FLAG) -ggdb -O0 -I$(LINC) -I$(INC) -I$(S_INC) -I../../include/ -I$(G_INC)  -DHAVE_AV_CONFIG_H \
#			-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE   -Wa,-al=

CC_OPTION  += $(MAIN_FLAG) -O2 -I$(LINC) -I$(INC) -I$(S_INC) -I../../include/ -I$(G_INC)  -DHAVE_AV_CONFIG_H \
			-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE

CC_OPTION  += -I../../inc -I../../common

CC_OPTION  += -Wa,-al=
 
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

 
S_OBJS =


C_OBJS =  $(OBJ)dma_cache.o $(OBJ)pp_adapter.o

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




