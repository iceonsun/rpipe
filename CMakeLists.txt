cmake_minimum_required(VERSION 3.8)
project(RPipe)

set(CMAKE_CXX_STANDARD 11)

include_directories(./include)

include_directories(/Users/robert/workspace/cpp/CLIon/sockNM)
link_directories(../sockNM/cmake-build-debug/)

set(SOURCE_FILES
    IPipe.cpp rcommon.c TopStreamPipe.cpp BtmDGramPipe.cpp RApp.cpp
    Config.cpp BridgePipe.cpp SessionPipe.cpp SessionPipe.h ITopContainerPipe.cpp
    RstSessionPipe.cpp ISessionPipe.cpp
    nmq/NMQPipe.cpp nmq/INMQPipe.cpp
    thirdparty/json11.cpp
    util/FdUtil.cpp util/RTimer.cpp util/RPUtil.cpp util/ProcUtil.cpp
    )

set(SVR_SOURCE_FILES ${SOURCE_FILES} server/rserver_main.cpp server/RServerApp.cpp)
set(CLI_SOURCE_FILES ${SOURCE_FILES} client/rclient_main.cpp client/RClientApp.cpp)

add_executable(rclient ${CLI_SOURCE_FILES}  client/main.cpp)
add_executable(rserver ${SVR_SOURCE_FILES} server/main.cpp)
set_target_properties(
        rserver
        PROPERTIES
        COMPILE_DEFINITIONS RPIPE_IS_SERVER
)

add_executable(test_client ${CLI_SOURCE_FILES} test/test_client.cpp)
add_executable(test_server ${SVR_SOURCE_FILES} test/test_server.cpp)
set_target_properties(
        test_server
        PROPERTIES
        COMPILE_DEFINITIONS RPIPE_IS_SERVER
)

add_executable(echo_client test/tcp_echo_client.cpp rcommon.c)
add_executable(echo_server test/test_tcp_echo_server.cpp rcommon.c)

### add libuv. g++ -luv
# /usr/local/lib is path for libuv.dylib  on mac or so on other platforms

find_library(UV_PATH uv /usr/local/lib NO_DEFAULT_PATH)
link_libraries($UV_PATH)
target_link_libraries(rclient uv nmq)
target_link_libraries(rserver uv nmq)
target_link_libraries(test_client uv nmq)
target_link_libraries(test_server uv nmq)
target_link_libraries(echo_client uv)
target_link_libraries(echo_server uv)