cmake_minimum_required(VERSION 3.1.2)
project(tradingsystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# If on windows, uncomment and set the path to the boost library
# set(CMAKE_PREFIX_PATH "D:/boost_1_82_0")

# Find the Boost library
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

# four servers: price, orderbook, trade, and inquiry
add_executable(server main.cpp)
target_link_libraries(server ${Boost_LIBRARIES})

# two servers for publish: stream and execution
add_executable(stream OutputStreamConnector.cpp)
add_executable(execution OutputExecutionConnector.cpp)

# four main clients: price, orderbook, trade, and inquiry
add_executable(price InputPriceConnector.cpp)
add_executable(market InputMarketConnector.cpp)
add_executable(trade InputTradeConnector.cpp)
add_executable(inquiry InputInquiryConnector.cpp)
