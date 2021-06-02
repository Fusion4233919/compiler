add_custom_command(
    OUTPUT scanner.l.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scanner.l
    COMMAND flex -o scanner.l.cpp ${CMAKE_CURRENT_SOURCE_DIR}/scanner.l
)

add_custom_command(
    OUTPUT parser.y.cpp parser.y.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
    COMMAND bison --defines=parser.y.h -o parser.y.cpp ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/scanner.l.cpp
)

add_library(ast
    ${CMAKE_CURRENT_BINARY_DIR}/parser.y.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AST.cpp
)
target_include_directories(ast
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET ast PROPERTY CXX_STANDARD 11)