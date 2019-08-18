include ../../../config.mak


TARGET = $(LIB)libMcard.a

CC = $(BIN)gcc
CC_OPTION += -march=r3000 -O2 -c -mno-abicalls -fno-strict-aliasing -G 8 -I$(INC) -I$(G_INC) -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS =	$(OBJ)mcard.o \
			$(OBJ)SimpleNandLog.o $(OBJ)NandPhy_Mp650.o \
			$(OBJ)NandFTL.o $(OBJ)NandPhyAdv_Mp650.o \
			$(OBJ)SD.o $(OBJ)SdPhy_Mp650.o \
			$(OBJ)Uti.o \
			$(OBJ)MP650_SPI.o $(OBJ)NandIdIdentify.o  \


#			$(OBJ)NandDebug_MP650.o
#			$(OBJ)MP615_ms.o $(OBJ)MP615_ata.o \
# $(OBJ)NandPhy_Mp615.o $(OBJ)NandPhyAdv_Mp615.o $(OBJ)SdPhy_Mp615.o $(OBJ)CF.o $(OBJ)CFPhy_Mp615.o $(OBJ)CFPhy_Mp650.o \
# $(OBJ)xD.o $(OBJ)MP650_MS.o 			$(OBJ)EthernetPhy.o \
#$(OBJ)SD2.o $(OBJ)Sd2ndPhy_Mp650.o

ALL_OBJS =	$(S_OBJS)	$(C_OBJS)

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


