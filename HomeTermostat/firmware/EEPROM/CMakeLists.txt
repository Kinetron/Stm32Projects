﻿cmake_minimum_required(VERSION 3.19)

target_include_directories(${PROJECT_NAME} PRIVATE Inc )

target_sources(${PROJECT_NAME} PRIVATE
    Src/stm32_eeprom.c
)
