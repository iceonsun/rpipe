cmake_minimum_required(VERSION 3.8)
project(RPipe)

set(CMAKE_CXX_STANDARD 11)

include_directories(/Users/robert/workspace/cpp/CLIon/sockNM)
link_directories(../sockNM/cmake-build-debug/)

set(SOURCE_FILES IPipe.cpp IPipe.h rcommon.h rcommon.c TopStreamPipe.cpp TopStreamPipe.h NMQPipe.cpp NMQPipe.h BtmDGramPipe.cpp BtmDGramPipe.h RApp.cpp RApp.h RClient.cpp RClient.h Config.h Config.cpp RServer.cpp RServer.h BridgePipe.cpp BridgePipe.h debug.h debug.c SessionPipe.cpp SessionPipe.h RTimer.cpp RTimer.h ITopContainerPipe.cpp ITopContainerPipe.h BtmPipe.h BtmPipe.cpp  UdpBtmPipe.h UdpBtmPipe.cpp
    RstSessionPipe.h RstSessionPipe.cpp IRdWriter.h IRdWriter.cpp TcpRdWriter.h TcpRdWriter.cpp FdUtil.h FdUtil.cpp)
add_executable(RPipe ${SOURCE_FILES} main.cpp )

add_executable(rclient ${SOURCE_FILES} client_main.cpp)

add_executable(rserver ${SOURCE_FILES} server_main.cpp)

set(TEST_TCP_ECHO_SERVER_SRC test/test_tcp_echo_server.cpp rcommon.h rcommon.c)
add_executable(test_tcp_echo_server ${TEST_TCP_ECHO_SERVER_SRC})

set(PIPE_TEST ${SOURCE_FILES} test/bridge_nmq_pipe_test/pipe_test.cpp test/bridge_nmq_pipe_test/EchoBtmPipe.cpp test/bridge_nmq_pipe_test/EchoBtmPipe.h test/bridge_nmq_pipe_test/RawPipe.cpp test/bridge_nmq_pipe_test/RawPipe.h)
add_executable(pipe_test ${PIPE_TEST})

### add libuv. g++ -luv
# /usr/local/lib is path for libuv.dylib  on mac or so on other platforms

find_library(UV_PATH uv /usr/local/lib NO_DEFAULT_PATH)
link_libraries($UV_PATH)
target_link_libraries(RPipe uv nmq)
target_link_libraries(rclient uv nmq)
target_link_libraries(rserver uv nmq)
target_link_libraries(test_tcp_echo_server uv nmq)
target_link_libraries(pipe_test uv nmq)