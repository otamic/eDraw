cmake_minimum_required(VERSION 3.20)
project(eDraw)

set(CMAKE_CXX_STANDARD 17)

find_package(BISON)
find_package(FLEX)
find_package(OpenGL REQUIRED)

BISON_TARGET(
        Parser
        ary.yy ${CMAKE_CURRENT_BINARY_DIR}/ary.tab.cpp
)

FLEX_TARGET(
        Lexer
        ary_zh.l ${CMAKE_CURRENT_BINARY_DIR}/ary.lex.cpp
)

ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${GLUT_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
        "libraries/includes"
)
find_library(GLFW_LIB libglfw.3.dylib "libraries/libs")

add_executable(eDraw ${BISON_Parser_OUTPUTS} ${FLEX_Lexer_OUTPUTS} m_type.h m_type.cpp m_scanner.h m_driver.h m_driver.cpp main.cpp glad.c)
target_link_libraries(eDraw ${OPENGL_LIBRARY} ${GLFW_LIB})
