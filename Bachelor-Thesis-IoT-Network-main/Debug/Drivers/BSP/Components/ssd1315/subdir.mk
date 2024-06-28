################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Components/ssd1315/ssd1315.c \
../Drivers/BSP/Components/ssd1315/ssd1315_reg.c 

OBJS += \
./Drivers/BSP/Components/ssd1315/ssd1315.o \
./Drivers/BSP/Components/ssd1315/ssd1315_reg.o 

C_DEPS += \
./Drivers/BSP/Components/ssd1315/ssd1315.d \
./Drivers/BSP/Components/ssd1315/ssd1315_reg.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/ssd1315/%.o Drivers/BSP/Components/ssd1315/%.su Drivers/BSP/Components/ssd1315/%.cyclo: ../Drivers/BSP/Components/ssd1315/%.c Drivers/BSP/Components/ssd1315/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L462xx -c -I../Core/Inc -I../Rtosal/Inc -I../Rtosal/Src -I../Utilities/Fonts -I../Drivers/BSP/Components/ssd1315 -I../Drivers/BSP/Components/w25q80ew -I../Drivers/BSP/Components/mt25ql512abb -I../Drivers/BSP/Components/lsm303agr -I../Drivers/BSP/Components/Common -I"C:/Users/islem/STM32CubeIDE/workspace_1.14.0/Temperature/Drivers/BSP/B-L462E-CELL1" -I"C:/Users/islem/STM32CubeIDE/workspace_1.14.0/Temperature/Drivers/BSP/Components/hts221" -I"C:/Users/islem/STM32CubeIDE/workspace_1.14.0/Temperature/Drivers/BSP/Components/lps22hh" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components-2f-ssd1315

clean-Drivers-2f-BSP-2f-Components-2f-ssd1315:
	-$(RM) ./Drivers/BSP/Components/ssd1315/ssd1315.cyclo ./Drivers/BSP/Components/ssd1315/ssd1315.d ./Drivers/BSP/Components/ssd1315/ssd1315.o ./Drivers/BSP/Components/ssd1315/ssd1315.su ./Drivers/BSP/Components/ssd1315/ssd1315_reg.cyclo ./Drivers/BSP/Components/ssd1315/ssd1315_reg.d ./Drivers/BSP/Components/ssd1315/ssd1315_reg.o ./Drivers/BSP/Components/ssd1315/ssd1315_reg.su

.PHONY: clean-Drivers-2f-BSP-2f-Components-2f-ssd1315

