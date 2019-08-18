include ../../../config.mak
 
TARGET = $(LIB)libxml2.a

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -msoft-float -O2 -G 4 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -G 0  -ggdb -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS = $(OBJ)c14n.o $(OBJ)catalog.o $(OBJ)chvalid.o $(OBJ)dict.o \
         $(OBJ)DOCBparser.o $(OBJ)encoding.o $(OBJ)entities.o $(OBJ)error.o $(OBJ)globals.o \
         $(OBJ)hash.o $(OBJ)HTMLparser.o $(OBJ)HTMLtree.o $(OBJ)legacy.o $(OBJ)list.o \
         $(OBJ)nanohttp.o $(OBJ)parser.o $(OBJ)parserInternals.o $(OBJ)relaxng.o \
         $(OBJ)SAX2.o $(OBJ)SAX.o $(OBJ)threads.o $(OBJ)trionan.o $(OBJ)tree.o $(OBJ)uri.o \
         $(OBJ)valid.o $(OBJ)xinclude.o $(OBJ)xlink.o $(OBJ)xmlcatalog.o $(OBJ)xmlIO.o $(OBJ)xmlmemory.o \
         $(OBJ)xmlreader.o $(OBJ)xmlregexp.o $(OBJ)xmlschemas.o $(OBJ)xmlschemastypes.o \
         $(OBJ)xmlunicode.o $(OBJ)xpath.o $(OBJ)xpointer.o	 

ALL_OBJS =  $(C_OBJS) $(S_OBJS)

all: $(TARGET) makefile

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst  $(TARGET)