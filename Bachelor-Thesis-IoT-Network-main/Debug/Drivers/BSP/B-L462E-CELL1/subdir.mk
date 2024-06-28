################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1.c \
../Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_bus.c \
../Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_eeprom.c \
../Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_env_sensors.c \
../Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_env_sensors_ex.c \
../Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_lcd.c \
../Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_motion_sensors.c \
../Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_motion_sensors_ex.c 

OBJS += \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1.o \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_bus.o \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_eeprom.o \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_env_sensors.o \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_env_sensors_ex.o \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_lcd.o \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_motion_sensors.o \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_motion_sensors_ex.o 

C_DEPS += \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1.d \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_bus.d \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_eeprom.d \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_env_sensors.d \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_env_sensors_ex.d \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_lcd.d \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_motion_sensors.d \
./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_motion_sensors_ex.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/B-L462E-CELL1/%.o Drivers/BSP/B-L462E-CELL1/%.su Drivers/BSP/B-L462E-CELL1/%.cyclo: ../Drivers/BSP/B-L462E-CELL1/%.c Drivers/BSP/B-L462E-CELL1/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L462xx -c -I../Core/Inc -I../Rtosal/Inc -I../Rtosal/Src -I../Utilities/Fonts -I../Drivers/BSP/Components/ssd1315 -I../Drivers/BSP/Components/w25q80ew -I../Drivers/BSP/Components/mt25ql512abb -I../Drivers/BSP/Components/lsm303agr -I../Drivers/BSP/Components/Common -I"C:/Users/islem/STM32CubeIDE/workspace_1.14.0/Temperature/Drivers/BSP/B-L462E-CELL1" -I"C:/Users/islem/STM32CubeIDE/workspace_1.14.0/Temperature/Drivers/BSP/Components/hts221" -I"C:/Users/islem/STM32CubeIDE/workspace_1.14.0/Temperature/Drivers/BSP/Components/lps22hh" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-B-2d-L462E-2d-CELL1

clean-Drivers-2f-BSP-2f-B-2d-L462E-2d-CELL1:
	-$(RM) ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1.cyclo ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1.d ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1.o ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1.su ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_bus.cyclo ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_bus.d ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_bus.o ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_bus.su ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_eeprom.cyclo ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_eeprom.d ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_eeprom.o ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_eeprom.su ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_env_sensors.cyclo ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_env_sensors.d ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_env_sensors.o ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_env_sensors.su ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_env_sensors_ex.cyclo ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_env_sensors_ex.d ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_env_sensors_ex.o ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_env_sensors_ex.su ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_lcd.cyclo ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_lcd.d ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_lcd.o ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_lcd.su ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_motion_sensors.cyclo ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_motion_sensors.d ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_motion_sensors.o ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_motion_sensors.su ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_motion_sensors_ex.cyclo ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_motion_sensors_ex.d ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_motion_sensors_ex.o ./Drivers/BSP/B-L462E-CELL1/stm32l462e_cell1_motion_sensors_ex.su

.PHONY: clean-Drivers-2f-BSP-2f-B-2d-L462E-2d-CELL1

