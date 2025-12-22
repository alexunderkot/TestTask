cmake_minimum_required(VERSION 3.10)
project(calculator-server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SERVER_SOURCES 
    "server/*.cpp"
    "src/*.cpp"
)

list(FILTER SERVER_SOURCES EXCLUDE REGEX ".*CalculatorClient\\.cpp$")
list(FILTER SERVER_SOURCES EXCLUDE REGEX ".*HttpClient\\.cpp$")
list(FILTER SERVER_SOURCES EXCLUDE REGEX ".*JsonHandler\\.cpp$")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(calculator-server ${SERVER_SOURCES})
target_link_libraries(calculator-server pthread)

if(WIN32)
    target_link_libraries(calculator-server ws2_32)
endif()