include ../../../config.mak

TARGET = $(LIB)libcurl.a

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 8 -I$(INC) -I$(G_INC) -I../../../libSrc/lwip/include -I../../../libSrc/sdio/include -I../../../libSrc/openssl/include -DBOOT_ROM_TYPE 
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 8 -I$(INC) -I$(G_INC) -I../../../libSrc/lwip/include -I../../../libSrc/sdio/include -I../../../libSrc/openssl/include  -DBOOT_ROM_TYPE -Wa,-ahl=
CC_OPTION +=  -I../../../libSrc/os_linux/include 
CC_OPTION +=  -I../../../libSrc/zlib/include
CC_OPTION +=  -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS =  $(OBJ)net_curl_inet_pton.o $(OBJ)net_curl_hostip4.o $(OBJ)net_curl_strtok.o $(OBJ)net_curl_netrc.o \
		  $(OBJ)net_curl_content_encoding.o $(OBJ)net_curl_splay.o $(OBJ)net_curl_multi.o $(OBJ)net_curl_escape.o \
		  $(OBJ)net_curl_md5.o $(OBJ)net_curl_mprintf.o $(OBJ)net_curl_base64.o $(OBJ)net_curl_http_digest.o $(OBJ)net_curl_http.o \
		  $(OBJ)net_curl_http_chunks.o $(OBJ)net_curl_select.o $(OBJ)net_curl_timeval.o $(OBJ)net_curl_progress.o $(OBJ)net_curl_formdata.o \
		  $(OBJ)net_curl_sslgen.o $(OBJ)net_curl_parsedate.o $(OBJ)net_curl_share.o $(OBJ)net_curl_strtoofft.o $(OBJ)net_curl_strequal.o \
		  $(OBJ)net_curl_cookie.o $(OBJ)net_curl_getinfo.o $(OBJ)net_curl_sendf.o $(OBJ)net_curl_llist.o \
		  $(OBJ)net_curl_hash.o $(OBJ)net_curl_transfer.o $(OBJ)net_curl_hostip.o $(OBJ)net_curl_url.o $(OBJ)net_curl_easy.o \
		  $(OBJ)net_curl_connect.o $(OBJ)net_curl_memdebug.o $(OBJ)net_curl_simple.o $(OBJ)net_curl_ftp.o\
		  $(OBJ)net_curl_ssluse.o
                   		  
ALL_OBJS =  $(S_OBJS) $(C_OBJS)

all: $(TARGET) makefile


$(TARGET)	: $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: $(SRC)%.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: $(SRC)%.c makefile 
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(TARGET)
