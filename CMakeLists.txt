cmake_minimum_required(VERSION 3.8)
project(RPipe)

set(CMAKE_CXX_STANDARD 11)

include_directories(/Users/robert/workspace/cpp/CLIon/sockNM)
link_directories(../sockNM/cmake-build-debug/)

set(SOURCE_FILES
    IPipe.cpp rcommon.c TopStreamPipe.cpp NMQPipe.cpp BtmDGramPipe.cpp RApp.cpp
    Config.cpp BridgePipe.cpp SessionPipe.cpp SessionPipe.h ITopContainerPipe.cpp BtmPipe.cpp  UdpBtmPipe.cpp
    RstSessionPipe.cpp IRdWriter.cpp TcpRdWriter.cpp
    ISessionPipe.cpp
    thirdparty/debug.c thirdparty/json11.cpp
    util/FdUtil.cpp util/RTimer.cpp
    server/rserver_main.cpp server/RServer.cpp
    client/rclient_main.cpp client/RClient.cpp )

add_executable(rclient ${SOURCE_FILES}  client/main.cpp)
add_executable(rserver ${SOURCE_FILES} server/main.cpp)
add_executable(test_client ${SOURCE_FILES} test/test_client.cpp)
add_executable(test_server ${SOURCE_FILES} test/test_server.cpp)


set(TEST_TCP_ECHO_SERVER_SRC test/test_tcp_echo_server.cpp rcommon.c)
add_executable(test_tcp_echo_server ${TEST_TCP_ECHO_SERVER_SRC})

set(PIPE_TEST ${SOURCE_FILES} test/bridge_nmq_pipe_test/pipe_test.cpp test/bridge_nmq_pipe_test/EchoBtmPipe.cpp test/bridge_nmq_pipe_test/EchoBtmPipe.h test/bridge_nmq_pipe_test/RawPipe.cpp test/bridge_nmq_pipe_test/RawPipe.h)
add_executable(pipe_test ${PIPE_TEST})

### add libuv. g++ -luv
# /usr/local/lib is path for libuv.dylib  on mac or so on other platforms

find_library(UV_PATH uv /usr/local/lib NO_DEFAULT_PATH)
link_libraries($UV_PATH)
target_link_libraries(rclient uv nmq)
target_link_libraries(rserver uv nmq)
target_link_libraries(test_tcp_echo_server uv nmq)
target_link_libraries(pipe_test uv nmq)
target_link_libraries(test_client uv nmq)
target_link_libraries(test_server uv nmq)
