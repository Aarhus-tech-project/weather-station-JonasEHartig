cmake_minimum_required(VERSION 3.10)

project(MQTTClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Mosquitto paths
include_directories("C:/msys64/ucrt64/include")
link_directories("C:/msys64/ucrt64/lib")

include_directories(${CMAKE_SOURCE_DIR}/include)

# Executable
add_executable(mqtt-client src/main.cpp)

# Link mosquitto
target_link_libraries(mqtt-client mosquitto)
