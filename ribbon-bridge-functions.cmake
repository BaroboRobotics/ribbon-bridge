##############################################################################
# Set up nanopb

if(NOT DEFINED NANOPB_ROOT)
    if(DEFINED ENV{NANOPB_ROOT})
        set(NANOPB_ROOT "$ENV{NANOPB_ROOT}")
    else()
        message(FATAL_ERROR "NANOPB_ROOT must be set")
    endif()
endif()

file(TO_CMAKE_PATH "${NANOPB_ROOT}" NANOPB_ROOT)

set(NANOPB_INCLUDE_DIR ${NANOPB_ROOT} CACHE PATH "Location of pb.h")

set(PROTOC_INCLUDE_DIR
    "${NANOPB_ROOT}/generator/proto"
    CACHE PATH "protocol buffer compiler's include directory")

set(PROTOC_EXECUTABLE
    "${NANOPB_ROOT}/generator-bin/protoc"
    CACHE FILEPATH "protocol buffer compiler")

# nanopb_compile_proto(<c-filepath-result-var>
#                   <h-filepath-result-var>
#                   <proto-filepath>)
#
# Compile a .proto file to a .c and a .h using nanopb's protocol buffer
# compiler. Set <c-filepath-result-var> and <h-filepath-result-var> to the
# absolute paths of the generated .c and .h files, respectively.
# <proto-filepath> is the input file; its path will be interpreted relative to
# the current source directory, if it is not absolute. <c-filepath-result-var>
# and <h-filepath-result-var>
function(nanopb_compile_proto cFilepathResultVar hFilepathResultVar protoFilepath)
    # Get the input .proto file's name, stripped of the last extension. Note:
    # using get_filename_component(... NAME_WE) instead of the funky regex
    # business would incorrectly strip all extensions. protoc only strips the
    # last, and we want to mimic its behavior.
    get_filename_component(_name ${protoFilepath} NAME)
    string(REGEX REPLACE "^(.+)(\\.[^.]+)$" "\\1" _basename ${_name})
    set(cOutput ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.pb.c)
    set(hOutput ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.pb.h)

    set(${cFilepathResultVar} ${cOutput} PARENT_SCOPE)
    set(${hFilepathResultVar} ${hOutput} PARENT_SCOPE)

    get_filename_component(_absProto ${protoFilepath} ABSOLUTE)
    get_filename_component(_dir ${_absProto} DIRECTORY)

    add_custom_command(OUTPUT ${cOutput} ${hOutput}
        COMMAND ${PROTOC_EXECUTABLE} -I${_dir} -I${PROTOC_INCLUDE_DIR} -I${RPC_PROTO_INCLUDE_DIR}
            --nanopb_out=${CMAKE_CURRENT_BINARY_DIR} ${_absProto}
        MAIN_DEPENDENCY ${protoFilepath}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.pb.{c,h} from ${protoFilepath}"
        VERBATIM)
endfunction()

# nanopb_add_proto(<target-name> <proto-filepath> [<other-source-files>...])
#
# Define a target to represent a nanopb protobuf library. You can use
# <target-name> in the rest of your code as a static library target (but
# containing position-independent code, and include its headers by using the
#
#   $<TARGET_PROPERTY:<target-name>,INCLUDE_DIRECTORIES>
#
# generator expression in a suitable command's arguments.
function(nanopb_add_proto _targetName _protoFile)
    nanopb_compile_proto(cFile hFile ${_protoFile})
    get_filename_component(protoName ${_protoFile} NAME_WE)
    add_library(${_targetName} STATIC ${cFile} ${ARGN})
    get_filename_component(protoDir ${hFile} DIRECTORY)
    target_include_directories(${_targetName}
        PRIVATE ${NANOPB_INCLUDE_DIR}
        PRIVATE ${protoDir}
        PRIVATE .)
    set_target_properties(${_targetName} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endfunction()
