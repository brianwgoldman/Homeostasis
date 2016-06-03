################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Cycles.cpp \
../src/Enumeration.cpp \
../src/Model.cpp \
../src/MonteCarloCycles.cpp \
../src/Utilities.cpp \
../src/WalkCycle.cpp \
../src/main.cpp 

OBJS += \
./src/Cycles.o \
./src/Enumeration.o \
./src/Model.o \
./src/MonteCarloCycles.o \
./src/Utilities.o \
./src/WalkCycle.o \
./src/main.o 

CPP_DEPS += \
./src/Cycles.d \
./src/Enumeration.d \
./src/Model.d \
./src/MonteCarloCycles.d \
./src/Utilities.d \
./src/WalkCycle.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++11 -O0 -g3 -pg -pedantic -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


