include ../../../config.mak

ROOTPATH=../../../

VPATH   = $(SRC)

TARGET = $(LIB)libmpapi.a

#for mpImagePP.c, include codec library header files, which's not public
#INC += -I../../Codec/inc/ -I../../Codec/pp/INCLUDE/ -I../../Codec/common/ -I../../Codec/config/

CC_OPTION += -I$(INC) -I$(G_INC) -DAUDIO_SELF_THREAD -I../../xpg/include -I../../subtitle/include -I../../demux/include
#CC_OPTION += -O0 -ggdb
CC_OPTION += -DMX6xx_AUDIO_SUPPORT 
CC_OPTION += -finline-limit=256 -finline-functions

CC_OPTION  +=  -DWORDS_BIGENDIAN  -Wa,-al=

AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

S_OBJS = 

MK_BT = $(word $(words $(shell grep 'MAKE_BLUETOOTH' $(ROOTPATH)include\corelib.h)),$(shell grep 'MAKE_BLUETOOTH' $(ROOTPATH)include\corelib.h))


#C_OBJS =  $(OBJ)mpapi.o $(OBJ)mpapi_inter.o $(OBJ)mjpeg_api.o $(OBJ)api_main.o \
#		  $(OBJ)mpTrace.o $(OBJ)mpUtil.o $(OBJ)mpImageApi.o $(OBJ)mpImageFileApi.o \
#		  $(OBJ)mpImageResizeApi.o $(OBJ)mpImageRotateApi.o $(OBJ)mpImageScaler.o

C_OBJS =  $(OBJ)mpapi.o $(OBJ)mpapi_inter.o $(OBJ)api_main.o $(OBJ)mpImageResizeApi.o \
          $(OBJ)mpImageApi.o $(OBJ)mpTrace.o $(OBJ)mpUtil.o $(OBJ)mpImageFileApi.o \
          $(OBJ)mpImageRotateApi.o $(OBJ)mpStreamApi.o $(OBJ)mjpeg_api.o \

ifeq ($(MK_BT),1)
C_OBJS += $(OBJ)BtApi.o
C_OBJS += $(OBJ)btbrowser.o
endif

ALL_OBJS =  $(C_OBJS) $(S_OBJS)



all: $(TARGET) makefile

$(TARGET): $(ALL_OBJS)
	$(AR) -rsv $(TARGET) $(ALL_OBJS)

$(S_OBJS):	$(OBJ)%.o: %.s makefile
	$(AS) $(AS_OPTION)$(@:.o=.lst) -o $@ $<

$(C_OBJS):	$(OBJ)%.o: %.c  makefile
	$(CC) $(CC_OPTION)$(@:.o=.lst) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(OBJ)/*.lst $(TARGET)



