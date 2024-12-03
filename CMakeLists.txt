cmake_minimum_required(VERSION 3.28)
project(qbasic)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "/home/ayanami/Qt/6.8.0/gcc_64")
set(EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
#        Svg
        Test
#        Multimedia
        REQUIRED)
find_package(fmt CONFIG REQUIRED)
 
add_executable(qbasic main.cpp
        tokenizer.h
        tokenizer.cpp
        util.h
        parser.cpp
        parser.h)
target_link_libraries(qbasic
        Qt::Core
        Qt::Gui
        Qt::Widgets
        fmt::fmt-header-only
)

add_executable(qbasic_test
        tokenizer.cpp
        tokenizer.h
        tokenizer_test.cpp
        tokenizer_test.h
        main_test.cpp
)

add_test(
        NAME qbasic_test
        COMMAND qbasic_test
)
target_link_libraries(qbasic_test
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Test
        fmt::fmt-header-only
)