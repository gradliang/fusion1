
include ../../../config.mak


TARGET = $(LIB)libSensor.a

CC = $(BIN)gcc
CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls $(OPT2) -G 8 -I$(INC) -I$(G_INC) -I../../display/include -Wa,-ahl=
#CC_OPTION = $(MAIN_FLAG) -march=r3000 -c -mno-abicalls $(OPT2) -G 8 -O0 -ggdb -I$(INC) -I$(G_INC) -I../../display/include -Wa,-ahl=


AS = $(BIN)as
AS_OPTION = -march=r3000 -I$(INC) -I$(G_INC) -ahl=

LD = $(BIN)ld

AR = $(BIN)ar
 
S_OBJS =


#MK_SENSOR = $(word $(words $(shell grep 'SENSOR_TYPE' ..\..\..\..\STD_DPF\include\Platform_MP663.h)),$(shell grep 'SENSOR_TYPE' ..\..\..\..\STD_DPF\include\Platform_MP663.h))
	
#C_OBJS =	$(OBJ)sensor_$(MK_SENSOR).o $(OBJ)sensor_driver_$(MK_SENSOR).o 

#C_OBJS = $(OBJ)sensor.o $(OBJ)sensor_driver.o
C_OBJS = $(OBJ)sensor_manager.o 
#C_OBJS += $(OBJ)sensor_OV5642.o $(OBJ)sensor_driver_OV5642.o 
#C_OBJS += $(OBJ)sensor_OV2643.o $(OBJ)sensor_driver_OV2643.o  
#C_OBJS += $(OBJ)sensor_OV2643_JH43A.o $(OBJ)sensor_driver_OV2643_JH43A.o 
#C_OBJS += $(OBJ)sensor_S5K6AAFX13.o $(OBJ)sensor_driver_S5K6AAFX13.o
#C_OBJS += $(OBJ)sensor_SensorTool.o $(OBJ)sensor_driver_SensorTool.o
C_OBJS += $(OBJ)sensor_driver_NT99140.o $(OBJ)sensor_NT99140.o $(OBJ)sensor_driver_NT99141.o
#C_OBJS += $(OBJ)sensor_driver_cvbs.o $(OBJ)sensor_cvbs.o
#C_OBJS += $(OBJ)sensor_HM1355_AWA.o $(OBJ)sensor_driver_HM1355_AWA.o

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


