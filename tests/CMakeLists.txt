nanopb_add_proto(widget-interface proto/widget.proto gen-widget.pb.cpp)

target_link_libraries(widget-interface PRIVATE rpc)

set_source_files_properties(
    asio.cpp
    fire.cpp
    broadcast.cpp
    gen-widget.pb.cpp
    PROPERTIES
    COMPILE_FLAGS "-std=c++11 -ggdb ${FEATURE_DEFS}")

add_executable(asio asio.cpp)
target_include_directories(asio PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(asio widget-interface sfp rpc cxx-util ${Boost_LIBRARIES} pthread rt)
add_test(NAME asio COMMAND asio)

add_executable(fire fire.cpp)
target_include_directories(fire
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(fire widget-interface rpc ${Boost_LIBRARIES})
add_test(NAME fire COMMAND fire)

add_executable(broadcast broadcast.cpp)
target_include_directories(broadcast
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(broadcast widget-interface rpc ${Boost_LIBRARIES})
add_test(NAME broadcast COMMAND broadcast)

set_target_properties(
    fire
    broadcast
    PROPERTIES LINK_FLAGS "-pthread")
