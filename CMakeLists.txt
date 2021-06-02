cmake_minimum_required(VERSION 3.5)

project(compiler)

find_package(LLVM REQUIRED CONFIG)

link_directories(${LLVM_LIB_DIRS})

message(${LLVM_INCLUDE_DIRS})

add_subdirectory(ast)
add_subdirectory(gen)

add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC ast gen
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)