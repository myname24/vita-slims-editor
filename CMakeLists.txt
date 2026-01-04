cmake_minimum_required(VERSION 3.12)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(slimseditor)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")
# App Information
set(VITA_APP_NAME "Vita Slim's Editor")
set(VITA_TITLEID  "SLMS00001")
set(VITA_VERSION  "01.00")

# Source files
set(SOURCES
    src/main.cpp
    src/app.cpp
    src/core/save_file.cpp
    src/core/game_data.cpp
    src/ui/file_browser.cpp
    src/ui/save_editor.cpp
    src/ui/keyboard.cpp
)

# Header files (for IDE support, not required for building)
set(HEADERS
    src/app.h
    src/core/save_file.h
    src/data/rac_vita_games_data.h
    src/ui/file_browser.h
    src/ui/save_editor.h
    src/ui/keyboard.h
    src/utils/colors.h
    src/utils/input.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    SDL2
    SDL2_ttf
    freetype
    png
    z
    bz2
    SceDisplay_stub
    SceCtrl_stub
    SceAudio_stub
    SceAudioIn_stub
    SceSysmodule_stub
    SceGxm_stub
    SceCommonDialog_stub
    SceTouch_stub
    SceMotion_stub
    SceHid_stub
    ScePgf_stub
    SceIme_stub
    m
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
# VPK Creation with icon
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE sce_sys/icon0.png sce_sys/icon0.png
)
