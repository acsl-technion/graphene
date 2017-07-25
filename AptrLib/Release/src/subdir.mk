################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Aptr.cpp \
../src/C_warpper.cpp \
../src/SyncUtils.cpp \
../src/mem.cpp

OBJS += \
./src/Aptr.o \
./src/C_warpper.o \
./src/SyncUtils.o \
./src/mem.o

CPP_DEPS += \
./src/Aptr.d \
./src/C_warpper.d \
./src/SyncUtils.d \
./src/mem.d


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I/usr/local/include -I/opt/intel/sgxsdk/include -O3 -Wall -c -fmessage-length=0 -mavx -msse4 -maes -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


