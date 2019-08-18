
include ../../../config.mak


TARGET = $(LIB)libDisplay.a

CC = $(BIN)gcc
CC_OPTION = -O1 $(MAIN_FLAG) -march=r3000 -c -mno-abicalls $(OPT2) -G 8 -I$(INC) -I$(G_INC) -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar
 
S_OBJS =

C_OBJS =	$(OBJ)ipu.o	$(OBJ)idu.o	$(OBJ)osd.o $(OBJ)osd4.o $(OBJ)fontdisp.o	\
		$(OBJ)imgWin.o $(OBJ)imgString.o $(OBJ)IICM.o $(OBJ)IduTable.o \
		$(OBJ)Font.o $(OBJ)hal_idu.o $(OBJ)hal_ipu.o $(OBJ)scaler_650.o \
		$(OBJ)FontExt.o
# $(OBJ)minilzo.o

# $(OBJ)AUOLED.o  $(OBJ)osd8.o $(OBJ)osd2.o	$(OBJ)iplay_font.o
# $(OBJ)MX88V44.o $(OBJ)MX88V430.o  $(OBJ)scaler_615.o $(OBJ)AUO_2_5inch.o $(OBJ)ILI9327.o $(OBJ)ILI9163.o $(OBJ)ILI9342.o
#$(OBJ)edge_laplacian.o $(OBJ)HimaxCtrl.o 		$(OBJ)FontRotation.o \

ALL_OBJS =  $(C_OBJS)

all: $(TARGET) makefile

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(TARGET) $(ALL_OBJS) $(OBJ)\*.lst $(SRC)\*.bak 


