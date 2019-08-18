ROOTPATH=../../../

include ../../../config.mak
 
TARGET = $(LIB)libxml.a

MK_USB = $(word $(words $(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h)),$(shell grep 'Make_USB' $(ROOTPATH)include\corelib.h))

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O2 -G 4 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE 
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls -O0 -ggdb -G 8 -I$(INC) -I$(G_INC) -DBOOT_ROM_TYPE -Wa,-ahl=
ifeq ($(MK_USB),5)
CC_OPTION += -I../../../libSrc/os_linux27/include 
else
CC_OPTION += -I../../../libSrc/os_linux/include 
endif
CC_OPTION += -I../../../libSrc/lwip/include 
CC_OPTION +=  -Wa,-ahl=

AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar

S_OBJS =

C_OBJS = $(OBJ)xmlrole.o $(OBJ)xmltok.o $(OBJ)xmltok_impl.o $(OBJ)xmltok_ns.o\
				 $(OBJ)xmlutil.o $(OBJ)xmlparse.o $(OBJ)xmlrss.o $(OBJ)xmlflickr.o $(OBJ)xmlpicasa2.o $(OBJ)xmlgce.o \
				 $(OBJ)xmlhtml.o $(OBJ)xmlhttp.o	$(OBJ)xmlmagicpixel.o  $(OBJ)netfs.o $(OBJ)discovery.o  \
				 $(OBJ)xmlyougotphoto.o $(OBJ)xmlshoutcast.o $(OBJ)xmlframechannel.o $(OBJ)xmlvtuner.o $(OBJ)xmlupnp.o $(OBJ)xmlframeit.o\
				 $(OBJ)xmlsnapfish.o $(OBJ)iperf_test.o $(OBJ)xmlyoutube.o $(OBJ)xmlword.o $(OBJ)xmlyahoo.o $(OBJ)xmlyouku3G.o $(OBJ)xmlfacebook.o
#				 $(OBJ)xmlshutterfly.o 
#C_OBJS += $(OBJ)vperf_test.o 
          
# Replace $(OBJ)xmlpicasa.o with $(OBJ)xmlpicasa2.o to access private albums 

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

