##################################################################################################
# PATH
##################################################################################################

CROSS_COMPILER	= lexra-mpx-elf-
BIN = $(CROSS_COMPILER)
OBJ = ../object/
INC = ../include/
SRC = ../source/
LIB = ../../../libIPLAY/lib/
G_INC = ../../../libIPLAY/include/
LIBSRCPATH = ../../../libIPLAY/libsrc/
LIBMAIN = ../../lib/
UI_INC = ../../../MiniDV/UI/INCLUDE

ALL_INCLUDE = -I$(INC) -I$(G_INC) -I$(UI_INC) -I$(LIBSRCPATH)xpg/include

##################################################################################################
# PROGRAM
##################################################################################################

CC              = $(BIN)gcc
AR              = $(BIN)ar
LD              = $(BIN)ld
AS              = $(BIN)as
OC              = $(BIN)objcopy
OBJDUMP         = $(BIN)objdump

##################################################################################################
# CC/AS OPTIONS
##################################################################################################


# only remove one optimization of O2 : -fstrength-reduce \
OPT2 =    -O0 -fforce-mem \
          -foptimize-sibling-calls \
          -fcse-follow-jumps  -fcse-skip-blocks \
          -frerun-cse-after-loop  -frerun-loop-opt \
          -fgcse   -fgcse-lm   -fgcse-sm \
          -fdelete-null-pointer-checks \
          -fexpensive-optimizations \
          -fregmove \
          -fschedule-insns  -fschedule-insns2 \
          -fsched-interblock -fsched-spec \
          -fcaller-saves \
          -fpeephole2 \
          -freorder-blocks  -freorder-functions \
          -fstrict-aliasing \
          -falign-functions  -falign-jumps \
          -falign-loops  -falign-labels
          
###Fix.2005.07.12: GregXu, BrendaLi
###AIU DMA endian mode: 1 - big endian;  0 - little endian
CC_OPTION	+= -DAUDIO_ENDIAN=1 
CC_OPTION	+= -march=r3000 -c -EB -msoft-float 
CC_OPTION	+= -G 8 -D__DEBUG__
CC_OPTION	+= -O2 -DNO_STUB -fno-strict-aliasing			# optimize code, no debug message
#CC_OPTION	+= -O0 -g
CC_OPTION 	+= -DWORDS_BIGENDIAN -DARCH_IS_BIG_ENDIAN
CC_OPTION 	+= -DAMR_NB_ENABLE

CC_OPTION   += -DLAYERII_FORCE_MONO
CC_OPTION   += -DMPEG1_DROP_B_FRAME -DXVID_DROP_B_FRAME
CC_OPTION   += -finline-limit=128 -finline-functions
#CC_OPTION  += -DPURE_VIDEO
######Macros for performance testing#######
####When testing performance, the macros NO_AV_CONTROL should be defined.
#CC_OPTION +=-DNO_AV_CONTROL
#CC_OPTION    +=  -DFS_TIME
#CC_OPTION +=-DTOTAL_FRAMES
#CC_OPTION +=-DAUDIO_TOTAL_TIME

####This Video macro VIDEO_TOTAL_TIME cannot be defined with other macros at the same time
#CC_OPTION +=-DVIDEO_TOTAL_TIME
#CC_OPTION +=-DVIDEO_READ_TIME
#CC_OPTION +=-DVIDEO_DEC_TIME
#CC_OPTION +=-DVIDEO_DISP_TIME

###CC_OPTION +=-DISR_TOTAL_TIME

AS_OPTION = -march=r3000 -I$(INC) -al=$(OBJ)

          
##################################################################################################
# File Format:	0 - MPG;1 - AVI; 2 - MOV; 3 - AAC; 4 - MP3 ; 5-ASF
# Video Codec:	0 - NONE;1 - RAW;2 - MPEG1;3 - MPEG4;4 - MJPG; 5 - DIV3
# Audio Codec:	0 - NONE;1 - PCM;2 - MP3;3 - AAC 
##################################################################################################

#CC_OPTION	+= -D__FILE_FORMAT__=1 -D__V_CODEC__=3 -D__A_CODEC__=0

##################################################################################################
# Rules for compiling
################################################################################################## 

SRCS_C	= $(foreach dir, $(SRCDIR), $(wildcard $(dir)*.c))
SRCS_S	= $(foreach dir, $(SRCDIR), $(wildcard $(dir)*.S))
SRCS	= $(SRCS_C) $(SRCS_S)

##################################################################################################
# Function notdir will remove the dir part of the dir+filename.
################################################################################################## 

OBJS_C_T	= $(notdir $(patsubst %.c, %.o, $(SRCS_C)))
OBJS_S_T	= $(notdir $(patsubst %.S, %.o, $(SRCS_S)))
OBJS_C 		= $(addprefix $(OBJDIR), $(OBJS_C_T))
OBJS_S		= $(addprefix $(OBJDIR), $(OBJS_S_T))
OBJS		= $(OBJS_S) $(OBJS_C)



