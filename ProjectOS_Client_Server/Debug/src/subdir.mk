################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ProjectOS_Client_Server.c \
../src/StartThread.c \
../src/server.c \
../src/sqlLiteController.c 

OBJS += \
./src/ProjectOS_Client_Server.o \
./src/StartThread.o \
./src/server.o \
./src/sqlLiteController.o 

C_DEPS += \
./src/ProjectOS_Client_Server.d \
./src/StartThread.d \
./src/server.d \
./src/sqlLiteController.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -pthread -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


