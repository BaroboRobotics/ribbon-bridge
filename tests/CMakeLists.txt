set(ROBOT_PROTO_SOURCES robot.proto)
#set(ROBOT_PROTO_OPTIONS robot.options)
set(ROBOT_PROTO_C_FILE robot.pb.c)
set(ROBOT_PROTO_H_FILE robot.pb.h)

add_custom_command(OUTPUT ${ROBOT_PROTO_C_FILE} ${ROBOT_PROTO_H_FILE}
    COMMAND ${PROTOC} --nanopb_out=${CMAKE_CURRENT_BINARY_DIR} ${ROBOT_PROTO_SOURCES}
    MAIN_DEPENDENCY ${PROTO_SOURCE_DIR}/${ROBOT_PROTO_SOURCES}
    #    DEPENDS ${PROTO_SOURCE_DIR}/${ROBOT_PROTO_OPTIONS}
    WORKING_DIRECTORY ${PROTO_SOURCE_DIR}
    COMMENT "Generating Robot protocol buffers ..."
    VERBATIM)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
set_source_files_properties(method-fire.cpp PROPERTIES COMPILE_FLAGS -std=c++11)
add_executable(method-fire method-fire.cpp ${ROBOT_PROTO_C_FILE})
target_link_libraries(method-fire rpc)
add_test(NAME method-fire COMMAND method-fire)
