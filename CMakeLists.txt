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

list(APPEND MODULES "oneway" "bidirectional")

foreach(MODULE IN LISTS MODULES)
    message(STATUS "module = ${MODULE}")

    get_filename_component(PROTO "${CMAKE_CURRENT_SOURCE_DIR}/protos/${MODULE}.proto" ABSOLUTE)
    get_filename_component(PROTO_PATH "${PROTO}" PATH)

    set(PROTO_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/${MODULE}.pb.cc")
    set(PROTO_HEADERS "${CMAKE_CURRENT_BINARY_DIR}/${MODULE}.pb.h")
    set(GRPC_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/${MODULE}.grpc.pb.cc")
    set(GRPC_HEADERS "${CMAKE_CURRENT_BINARY_DIR}/${MODULE}.grpc.pb.h")

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

    add_library(${MODULE}_grpc_proto
        ${PROTO_SOURCES}
        ${PROTO_HEADERS}
        ${GRPC_SOURCES}
        ${GRPC_HEADERS}
    )

    target_link_libraries(${MODULE}_grpc_proto
        PUBLIC
        ${_PROTOBUF_LIBPROTOBUF}
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
    )

    ######################## CLIENT SIDE ###################################

    add_executable(${MODULE}_client
        ${CMAKE_CURRENT_SOURCE_DIR}/src/${MODULE}_client.cc
    )
    target_link_libraries(${MODULE}_client
        ${MODULE}_grpc_proto
    )

    ######################## SERVER SIDE ####################################

    add_executable(${MODULE}_server
        ${CMAKE_CURRENT_SOURCE_DIR}/src/${MODULE}_server.cc
    )
    target_link_libraries(${MODULE}_server
        ${MODULE}_grpc_proto
    )

endforeach()
