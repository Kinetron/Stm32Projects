cmake_minimum_required(VERSION 3.19)
file(TO_CMAKE_PATH "$ENV{STM32Cube_DIR}" CUBE_PATH)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../../modules/stm32-cmake/cmake/stm32_gcc.cmake")


get_filename_component(BASE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PROJECT_NAME ${BASE_FOLDER})

project(${PROJECT_NAME} C CXX ASM)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../modules/stm32-cmake/cmake")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../tools")

include(stm32_utils)


set(STM32_CMSIS_F1_PATH "${CUBE_PATH}/Drivers/CMSIS" CACHE PATH "" FORCE)
set(STM32_HAL_F1_PATH   "${CUBE_PATH}/Drivers/STM32F1xx_HAL_Driver" CACHE PATH "" FORCE)
set(STM32_CUBE_F1_PATH  "${CUBE_PATH}" CACHE PATH "" FORCE)


set(STM32_CMSIS_F1_NO_FETCH ON)
set(STM32_HAL_F1_NO_FETCH ON)


set(MCU STM32F103C8T)
set(MCU_FAMILY F1)
set(MCU_TYPE F103xB)


find_package(CMSIS COMPONENTS ${MCU} REQUIRED)
find_package(HAL COMPONENTS STM32F1 REQUIRED)


add_executable(${PROJECT_NAME})


add_subdirectory(Core)
add_subdirectory(User)
add_subdirectory(ControlLogic)
add_subdirectory(LED)


target_link_libraries(${PROJECT_NAME}
    HAL::STM32::F1::RCC
    HAL::STM32::F1::RCCEx
    HAL::STM32::F1::GPIO
    HAL::STM32::F1::I2C
    HAL::STM32::F1::UART
    HAL::STM32::F1::USART
    HAL::STM32::F1::DMA
    HAL::STM32::F1::ADC
    HAL::STM32::F1::ADCEx
    HAL::STM32::F1::IWDG
    HAL::STM32::F1::TIM
    HAL::STM32::F1::TIMEx
    HAL::STM32::F1::CORTEX
    HAL::STM32::F1::FLASH
    HAL::STM32::F1::FLASHEx
	CMSIS::STM32::${MCU_TYPE}
    STM32::Nano
    STM32::NoSys
)


target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE USE_HAL_DRIVER STM32F103xB)
target_compile_options(${PROJECT_NAME} PRIVATE 
    $<$<CONFIG:DEBUG>:-Os> 
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti -fno-exceptions>
)
target_link_options(${PROJECT_NAME} PRIVATE 
    LINKER:--no-warn-rwx-segments,--print-memory-usage, -u _printf_float
)


stm32_add_linker_script(${PROJECT_NAME} PRIVATE ${MCU}x_FLASH.ld)

stm32_print_size_of_target(${PROJECT_NAME})
stm32_generate_binary_file(${PROJECT_NAME})
stm32_generate_hex_file(${PROJECT_NAME})
