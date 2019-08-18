
include ../../../config.mak


TARGET = $(LIB)libImage.a

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -fno-strict-aliasing -G 8 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -g -G 8 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS =  $(OBJ)jpeg_hal.o $(OBJ)IMG_jpeg.o $(OBJ)jpegEncode.o \
          
# $(OBJ)IMG_bmp.o $(OBJ)IMG_gif.o $(OBJ)IMG_png.o $(OBJ)IMG_tiff.o $(OBJ)IMG_mpo.o

ALL_OBJS =  $(C_OBJS)

all: $(TARGET) makefile

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(TARGET)

	
