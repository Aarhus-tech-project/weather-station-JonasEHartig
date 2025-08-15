cmake_minimum_required(VERSION 3.16)

project(HelloWin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME} ${APP_SOURCES})

# Copy libmariadb.dll after build

# Include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
        "C:/msys64/mingw64/include/mariadb"
)

find_package(nlohmann_json REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        mosquitto
        mariadb
        curl
        nlohmann_json::nlohmann_json
        ws2_32
        secur32
        crypt32
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
