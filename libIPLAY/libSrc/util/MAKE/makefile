
include ../../../config.mak

TEXT_INC = ../../text/include

TARGET = $(LIB)libutil.a

CC = $(BIN)gcc
CC_OPTION += -march=r3000 -O2 -fno-strict-aliasing -c -mno-abicalls -G 8 -I$(TEXT_INC) -I$(INC) -I$(G_INC) -Wa,-ahl=
#CC_OPTION += -march=r3000 -O1 -g -c -mno-abicalls -G 8 -I$(TEXT_INC) -I$(INC) -I$(G_INC) -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

#C_OBJS = $(OBJ)util_Asc2Uni.o  $(OBJ)util_AscCatAsc.o  $(OBJ)util_Num2Str.o  $(OBJ)util_U16ToU08.o  $(OBJ)util_Uni2Asc.o  $(OBJ)util_Uni2Big5.o \
#         $(OBJ)util_UniCatAsc.o  $(OBJ)util_UniCatUni.o  $(OBJ)util_UniCompare.o  $(OBJ)util_UniUpperCase.o  $(OBJ)util_Big5ToUni.o  $(OBJ)util_Utf8ToUni.o \
#         $(OBJ)util_FbOverlap.o  $(OBJ)util_CharString.o $(OBJ)util_ConfigParser.o $(OBJ)util_EndianSwap.o \

C_OBJS = $(OBJ)util_CharString.o $(OBJ)util_Utf8ToUni.o $(OBJ)util_U16ToU08.o $(OBJ)util_Asc2Uni.o

ALL_OBJS =  $(S_OBJS) $(C_OBJS)

all: $(TARGET)


$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)


$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst  $(TARGET)


