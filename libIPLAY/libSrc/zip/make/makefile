include ../../../config.mak
 
TARGET = $(LIB)libzip.a

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 4 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 0 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS = \
	$(OBJ)mkstemp.o $(OBJ)zip_add.o $(OBJ)zip_add_dir.o $(OBJ)zip_close.o $(OBJ)zip_delete.o $(OBJ)zip_dirent.o $(OBJ)zip_entry_free.o \
	$(OBJ)zip_entry_new.o $(OBJ)zip_err_str.o $(OBJ)zip_error.o $(OBJ)zip_error_get.o $(OBJ)zip_error_get_sys_type.o \
	$(OBJ)zip_error_strerror.o $(OBJ)zip_error_to_str.o $(OBJ)zip_fclose.o $(OBJ)zip_file_error_clear.o $(OBJ)zip_file_error_get.o $(OBJ)zip_file_get_offset.o \
	$(OBJ)zip_file_strerror.o $(OBJ)zip_filerange_crc.o $(OBJ)zip_fopen.o $(OBJ)zip_fopen_index.o $(OBJ)zip_fread.o $(OBJ)zip_free.o $(OBJ)zip_fs.o \
	$(OBJ)zip_get_archive_comment.o $(OBJ)zip_get_archive_flag.o $(OBJ)zip_get_file_comment.o $(OBJ)zip_get_name.o $(OBJ)zip_get_num_files.o \
	$(OBJ)zip_memdup.o $(OBJ)zip_name_locate.o $(OBJ)zip_new.o $(OBJ)zip_open.o $(OBJ)zip_rename.o $(OBJ)zip_replace.o $(OBJ)zip_set_archive_comment.o $(OBJ)zip_set_archive_flag.o  \
	$(OBJ)zip_set_file_comment.o $(OBJ)zip_source_buffer.o $(OBJ)zip_source_file.o $(OBJ)zip_source_filep.o $(OBJ)zip_source_free.o $(OBJ)zip_source_function.o \
	$(OBJ)zip_source_zip.o $(OBJ)zip_set_name.o $(OBJ)zip_stat.o $(OBJ)zip_stat_index.o $(OBJ)zip_stat_init.o $(OBJ)zip_strerror.o $(OBJ)zip_unchange.o \
	$(OBJ)zip_unchange_all.o $(OBJ)zip_unchange_archive.o $(OBJ)zip_unchange_data.o
#	$(OBJ)zipcmp.o $(OBJ)zipmerge.o 

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