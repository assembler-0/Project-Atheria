cmake_minimum_required(VERSION 3.30)
project(AtheriaCompiler CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use find_package for configuration
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Configure LLVM
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST UNIX_COMMAND "${LLVM_DEFINITIONS}")
add_definitions(${LLVM_DEFINITIONS_LIST})

# Use llvm-config to get the actual libraries since LLVM:: targets don't work
execute_process(
        COMMAND llvm-config --libs core support mc target targetparser
        OUTPUT_VARIABLE LLVM_LIBS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
        COMMAND llvm-config --ldflags
        OUTPUT_VARIABLE LLVM_LD_FLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Add LLVM flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LLVM_LD_FLAGS}")

file(GLOB SOURCES "src/*.cpp")

add_executable(ac ${SOURCES})

target_include_directories(ac PUBLIC src)

# Use the actual library names from llvm-config
target_link_libraries(ac PRIVATE ${LLVM_LIBS})