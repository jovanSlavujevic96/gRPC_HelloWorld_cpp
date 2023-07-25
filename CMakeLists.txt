cmake_minimum_required(VERSION 3.19)
project(basic_grpc)

set(CMAKE_CXX_STANDARD 14)

find_package(Threads)

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
if(CMAKE_CROSSCOMPILING)
    find_program(_PROTOBUF_PROTOC protoc)
else()
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP gRPC::grpc++)
if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
endif()

set(COMPONENT "sample")

get_filename_component(PROTO "${CMAKE_CURRENT_SOURCE_DIR}/protos/${COMPONENT}.proto" ABSOLUTE)
get_filename_component(PROTO_PATH "${PROTO}" PATH)

set(PROTO_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}.pb.cc")
set(PROTO_HEADERS "${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}.pb.h")
set(GRPC_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}.grpc.pb.cc")
set(GRPC_HEADERS "${CMAKE_CURRENT_BINARY_DIR}/${COMPONENT}.grpc.pb.h")

add_custom_command(
    OUTPUT "${PROTO_SOURCES}" "${PROTO_HEADERS}" "${GRPC_SOURCES}" "${GRPC_HEADERS}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}" --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
    -I "${PROTO_PATH}"
    --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
    "${PROTO}"
    DEPENDS "${PROTO}"
)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

#######################################################################

add_library(${COMPONENT}_grpc_proto
    ${PROTO_SOURCES}
    ${PROTO_HEADERS}
    ${GRPC_SOURCES}
    ${GRPC_HEADERS}
)

target_link_libraries(${COMPONENT}_grpc_proto
    PUBLIC
    ${_PROTOBUF_LIBPROTOBUF}
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
)

#######################################################################

add_executable(client
    ${CMAKE_CURRENT_SOURCE_DIR}/src/client.cc
)
target_link_libraries(client
    ${COMPONENT}_grpc_proto
)

#######################################################################

add_executable(server
    ${CMAKE_CURRENT_SOURCE_DIR}/src/server.cc
)
target_link_libraries(server
    ${COMPONENT}_grpc_proto
)
