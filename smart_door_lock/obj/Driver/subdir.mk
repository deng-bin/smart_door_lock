################################################################################
# MRS Version: 2.2.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Driver/as608.c \
../Driver/audio.c \
../Driver/esp8266.c \
../Driver/iic.c \
../Driver/key.c \
../Driver/lcd.c \
../Driver/timer.c \
../Driver/uart.c 

C_DEPS += \
./Driver/as608.d \
./Driver/audio.d \
./Driver/esp8266.d \
./Driver/iic.d \
./Driver/key.d \
./Driver/lcd.d \
./Driver/timer.d \
./Driver/uart.d 

OBJS += \
./Driver/as608.o \
./Driver/audio.o \
./Driver/esp8266.o \
./Driver/iic.o \
./Driver/key.o \
./Driver/lcd.o \
./Driver/timer.o \
./Driver/uart.o 


EXPANDS += \
./Driver/as608.c.234r.expand \
./Driver/audio.c.234r.expand \
./Driver/esp8266.c.234r.expand \
./Driver/iic.c.234r.expand \
./Driver/key.c.234r.expand \
./Driver/lcd.c.234r.expand \
./Driver/timer.c.234r.expand \
./Driver/uart.c.234r.expand 



# Each subdirectory must supply rules for building sources it contributes
Driver/%.o: ../Driver/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"d:/MounRiver_Studio/MounRiver_Studio_files/smart_door_lock/Debug" -I"d:/MounRiver_Studio/MounRiver_Studio_files/smart_door_lock/Core" -I"d:/MounRiver_Studio/MounRiver_Studio_files/smart_door_lock/User" -I"d:/MounRiver_Studio/MounRiver_Studio_files/smart_door_lock/Peripheral/inc" -I"d:/MounRiver_Studio/MounRiver_Studio_files/smart_door_lock/Driver" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

