include ../../../config.mak

TARGET = $(LIB)libxpg.a

CC = $(BIN)gcc
CC_OPTION += -march=r3000 -c -mno-abicalls -O2 -fno-strict-aliasing -G 4 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
#CC_OPTION += -march=r3000 -O0 -ggdb -c -mno-abicalls -G 8 -I$(INC) -I$(G_INC) -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS =  $(OBJ)xpg.o $(OBJ)xpgMovie.o $(OBJ)xpgPage.o $(OBJ)xpgUtil.o \
		  $(OBJ)xpgSprite.o $(OBJ)xpgRole.o $(OBJ)xpgReader.o $(OBJ)xpgDraw.o $(OBJ)xpgUi.o \
#		  $(OBJ)xpgRoleFunc.o
		  
		#	$(OBJ)xpgReadPng.o $(OBJ)xpgTheme.o 
		
ALL_OBJS =  $(C_OBJS)

all: $(TARGET)

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(TARGET) $(ALL_OBJS) $(OBJ)\*.lst $(SRC)\*.bak 
