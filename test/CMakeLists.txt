cmake_minimum_required(VERSION 3.12)
project(test)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 1)
set(CMAKE_CXX_STANDARD 20)

# 添加编译选项，启用协程支持
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-std=c++20 -fcoroutines-ts)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-std=c++20 -fcoroutines)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/std:c++latest)
endif()

set(CMAKE_PREFIX_PATH E:/Env/QT/6.2.4/msvc2019_64)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # 添加build目录
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Concurrent)

set(project_sources
	main.cpp 
    mainWindow.h
    mainWindow.cpp
    mainWindow.ui
    gui.qrc
    ../QControlBinder.hpp
    ../utils/log.hpp
    )

# 设置分组
source_group("src" FILES 
    main.cpp 
    mainWindow.h
    mainWindow.cpp
    mainWindow.ui
)

source_group("resources" FILES gui.qrc)

source_group("utils" FILES 
    ../QControlBinder.hpp
    ../utils/log.hpp
)

# compiler check
if(MSVC)
  list(APPEND my_cflags /wd4828 )
  if (MSVC_VERSION GREATER 1925)
		add_compile_options(/Zc:preprocessor /wd5105)
    else()
		add_compile_options(/experimental:preprocessor /wd5105)
	endif()
endif()

add_executable(${PROJECT_NAME} ${project_sources})
# Use the widgets module from Qt 6
target_link_libraries(${PROJECT_NAME}
	PUBLIC
	Qt6::Core
	Qt6::Gui
	Qt6::Widgets
    Qt6::Concurrent)