cmake_minimum_required(VERSION 3.10)
project(apee)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -std=c++14 -pthread -Wno-unknown-pragmas")

set(Boost_USE_STATIC_LIBS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# BOOST libs
set(BOOST_LIBRARYDIR /home/me/Projects/boost/boost_1_69_0/libs)
set (BOOST_ROOT /home/me/Projects/boost/boost_1_69_0)
find_package(Boost 1.69.0 REQUIRED COMPONENTS log REQUIRED)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
endif()


include_directories(include)

set(
    SOURCES
    src/apee.cpp
    src/log.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC ${Boost_LIBRARIES}
)

#enable_testing()

#function (make_test TEST_NAME) 
    #add_executable(${TEST_NAME} test/${TEST_NAME}.cpp)

    #target_link_libraries(
        #${TEST_NAME}
        #${PROJECT_NAME}
        #${Boost_LIBRARIES}
    #)

    #target_compile_definitions(
        #${TEST_NAME}
        #PRIVATE BOOST_TEST_DYN_LINK
    #)

    #add_test(${TEST_NAME} ${TEST_NAME})

#endfunction()


#make_test(applicationdescription_test)
#make_test(endpointdescription_test)
