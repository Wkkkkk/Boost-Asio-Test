cmake_minimum_required(VERSION 3.14)
project(HelloBoostAsio)

add_definitions(-DBOOST_ASIO_NO_DEPRECATED)
add_definitions(-DBOOST_BEAST_ALLOW_DEPRECATED)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_CXX_STANDARD 17)
set(BUILD_SHARED_LIBS ON)
set(Boost_USE_STATIC_LIBS OFF)  # only find static libs
set(Boost_USE_DEBUG_LIBS OFF) # ignore debug libs and
set(Boost_USE_RELEASE_LIBS ON)  # only find release libs
set(Boost_USE_MULTITHREADEDON)
set(Boost_USE_STATIC_RUNTIMEOFF)
set(BUILD_SHARED_LIBS OFF)

set(OPENSSL_USE_STATIC_LIBS ON)
set(OPENSSL_MSVC_STATIC_RT ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
IF (UNIX)
    set(PTHREAD_LIB pthread)
ENDIF ()

find_package(OpenSSL)
find_package(Boost COMPONENTS date_time thread filesystem system iostreams)

if(Boost_FOUND)
    include_directories(
            ${OPENSSL_INCLUDE_DIR}
            ${Boost_INCLUDE_DIRS}
    )
    link_directories(${Boost_LIBRARY_DIRS})
endif()

set(TARGETS
		http_client
		http_server
		websocket_client
		websocket_server
        ref_buf
        asio_threadpool
        chat_client
        chat_server
        echo_tcp_server
        priority_handler_queue
        linux_tcp_server
        )

foreach(name ${TARGETS})
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${PTHREAD_LIB} dl)
endforeach()