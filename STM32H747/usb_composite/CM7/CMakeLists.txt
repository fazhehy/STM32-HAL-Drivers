cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Compiler options
set(STM32_MCU_FLAGS  "-mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard " )

#Linker options
set(STM32_LINKER_SCRIPT stm32h747xx_flash_CM7.ld)
set(STM32_LINKER_OPTION  )

# Set the project name
set(CMAKE_PROJECT_NAME STM32H747_CM7)
project(${CMAKE_PROJECT_NAME})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Include mx-generated
include("mx-generated.cmake")


# In order to use CMake for cross-compiling
set(CMAKE_CXX_COMPILER_FORCED "true")

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ../BSP
    ../BSP/SD_CARD
    ../BSP/USB_CDC
    ../BSP/USB_MSC
    ../User
    ../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
    ../Middlewares/ST/STM32_USB_Device_Library/Class/CompositeBuilder/Inc
)

# 复合模式下 usbd_core.c / usbd_ctlreq.c 也会 include 复合构建器的头，
# 它们编在 USB_Device_Library 目标里，所以这个目标同样要加。
target_include_directories(USB_Device_Library PUBLIC
    ../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
    ../Middlewares/ST/STM32_USB_Device_Library/Class/CompositeBuilder/Inc
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ../BSP/SD_CARD/sd_card.c
    ../BSP/USB_CDC/usb_cdc.c
    ../BSP/USB_MSC/usb_msc.c
    ../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c
    ../Middlewares/ST/STM32_USB_Device_Library/Class/CompositeBuilder/Src/usbd_composite_builder.c
    # mx-generated.cmake 里只有 usbd_storage_if.c，CDC 的应用回调要手工加。
    # 加在这里而不是 mx-generated.cmake，是为了 CubeMX 重新生成时不会被覆盖。
    USB_DEVICE/App/usbd_cdc_if.c
    ../User/sd_card_fatfs.c
    ../User/cm7_main.cpp
    ../User/log.c
    ../User/delay.c
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined libraries
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Validate that STM32CubeMX code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()





