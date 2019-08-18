ROOTPATH=../../../
include ../../../config.mak


TARGET = $(LIB)libfile.a

CC = $(BIN)gcc
CC_OPTION += -march=r3000 -O2 -c -mno-abicalls -fno-strict-aliasing -G 8 -I$(INC) -I$(G_INC) -Wa,-ahl=
#CC_OPTION += -march=r3000 -O0 -ggdb -c -mno-abicalls -G 8 -I$(INC) -I$(G_INC) -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS =  	$(OBJ)dir.o $(OBJ)chain.o $(OBJ)drive.o $(OBJ)file.o $(OBJ)ffile.o $(OBJ)Format.o $(OBJ)filebrowser.o \
		$(OBJ)fileapp.o 	

# $(OBJ)exFAT.o
ALL_OBJS =  $(S_OBJS) $(C_OBJS)

all: $(TARGET)

ALL_INC =  $(G_INC)Utiltypedef.h   $(G_INC)Utilregfile.h   $(G_INC)taskid.h

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)


$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile $(ALL_INC)
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst  $(TARGET)


