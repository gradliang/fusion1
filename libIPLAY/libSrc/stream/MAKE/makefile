ROOTPATH=../../../
include $(ROOTPATH)config.mak

TARGET = $(LIB)libstream.a

#CC_OPTION += -D__USE_STREAM_CACHE__
CC_OPTION += -mno-abicalls -freg-struct-return -I../../demux/include -Wa,-ahl=

S_OBJS =

C_OBJS =  $(OBJ)stream.o   $(OBJ)stream_file.o    $(OBJ)stream_cache.o 

ALL_OBJS =  $(C_OBJS) $(S_OBJS)

all: $(TARGET)

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(TARGET) $(ALL_OBJS) $(OBJ)\*.lst $(SRC)\*.bak 

	
