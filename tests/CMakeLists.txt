rpc_compile_proto(WIDGET_PROTO_C_FILE WIDGET_PROTO_H_FILE proto/widget.proto)

set_source_files_properties(
    fire.cpp
    broadcast.cpp
    gen-widget.pb.cpp
    PROPERTIES
    COMPILE_FLAGS "-std=c++11 -ggdb ${FEATURE_DEFS}")

add_executable(fire fire.cpp gen-widget.pb.cpp ${WIDGET_PROTO_C_FILE})
target_include_directories(fire
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(fire rpc)
add_test(NAME fire COMMAND fire)

add_executable(broadcast broadcast.cpp gen-widget.pb.cpp ${WIDGET_PROTO_C_FILE})
target_include_directories(broadcast
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(broadcast rpc)
add_test(NAME broadcast COMMAND broadcast)

set_target_properties(
    fire
    broadcast
    PROPERTIES LINK_FLAGS "-pthread")
