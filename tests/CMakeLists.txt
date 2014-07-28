rpc_compile_proto(WIDGET_PROTO_C_FILE WIDGET_PROTO_H_FILE proto/widget.proto)

set_source_files_properties(
    get-attribute.cpp
    set-attribute.cpp
    broadcast-attribute.cpp
    fire-method.cpp
    broadcast-broadcast.cpp
    gen-widget.pb.cpp
    PROPERTIES
    COMPILE_FLAGS "-std=c++11 -ggdb ${FEATURE_DEFS}")

add_executable(get-attribute get-attribute.cpp gen-widget.pb.cpp ${WIDGET_PROTO_C_FILE})
target_include_directories(get-attribute
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(get-attribute rpc)
add_test(NAME get-attribute COMMAND get-attribute)

add_executable(set-attribute set-attribute.cpp gen-widget.pb.cpp ${WIDGET_PROTO_C_FILE})
target_include_directories(set-attribute
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(set-attribute rpc)
add_test(NAME set-attribute COMMAND set-attribute)

add_executable(broadcast-attribute broadcast-attribute.cpp gen-widget.pb.cpp ${WIDGET_PROTO_C_FILE})
target_include_directories(broadcast-attribute
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(broadcast-attribute rpc)
add_test(NAME broadcast-attribute COMMAND broadcast-attribute)

add_executable(fire-method fire-method.cpp gen-widget.pb.cpp ${WIDGET_PROTO_C_FILE})
target_include_directories(fire-method
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(fire-method rpc)
add_test(NAME fire-method COMMAND fire-method)

add_executable(broadcast-broadcast broadcast-broadcast.cpp gen-widget.pb.cpp ${WIDGET_PROTO_C_FILE})
target_include_directories(broadcast-broadcast
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(broadcast-broadcast rpc)
add_test(NAME broadcast-broadcast COMMAND broadcast-broadcast)

set_target_properties(
    get-attribute
    set-attribute
    broadcast-attribute
    fire-method
    broadcast-broadcast
    PROPERTIES LINK_FLAGS "-pthread")
