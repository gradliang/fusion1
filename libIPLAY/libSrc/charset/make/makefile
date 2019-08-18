include ../../../config.mak

TARGET = $(LIB)libcharset.a

CC = $(BIN)gcc
CC_OPTION += -march=r3000 -c -mno-abicalls -O2 -fno-strict-aliasing -G 4 -I$(INC) -I$(G_INC) -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS =  $(OBJ)charset_main.o $(OBJ)charset_int.o  $(OBJ)charset_clib_wcslen.o  $(OBJ)charset_clib_wcscpy.o \
			$(OBJ)charset_clib_wcscat.o  $(OBJ)charset_clib_wcsncpy.o  $(OBJ)charset_clib_wcschr.o \
			$(OBJ)charset_clib_wcscmp.o  $(OBJ)charset_clib_wcsncmp.o  $(OBJ)charset_clib_wcsncat.o \
			$(OBJ)charset_clib_wcsrchr.o $(OBJ)charset_clib_wcsstr.o   $(OBJ)charset_clib_wcsicmp.o \
			$(OBJ)charset_clib_stricmp.o $(OBJ)charset_clib_strnicmp.o  \
			$(OBJ)cp1252.o  $(OBJ)cp1250.o  $(OBJ)cp1251.o $(OBJ)gbk.o  $(OBJ)big5.o  $(OBJ)shift_jis.o \
			$(OBJ)korean.o $(OBJ)iso_8859_15.o
			
ALL_OBJS =  $(C_OBJS)

all: $(TARGET)

$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(OBJ)/*.o $(OBJ)/*.lst $(SRC)/*.bak 
