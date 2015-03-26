cmake_minimum_required(VERSION 3.2.1)
project(rpc C CXX)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 3)
set(VERSION_PATCH 0)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set(AVR OFF)
if(CMAKE_SYSTEM_NAME MATCHES "AVR")
    set(AVR ON)
endif()

option(RPC_BUILD_TESTS "Build ribbon-bridge tests" OFF)

if(AVR)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

include(LinkbotLabs)
ll_add_dependency(cxx-util cxx-util)

##############################################################################
# Configure our project given the available features

include(CheckCXXSourceCompiles)
include(CheckLibraryExists)

set(CMAKE_REQUIRED_FLAGS "-std=c++11")

# Visual Studio 2015 Preview fails this test, gcc and clang pass
check_cxx_source_compiles("template <unsigned N>
constexpr unsigned f (const char (&)[N]) { return N; }
int main () { switch (2) { case f(\"a\"): break; } }"
    HAVE_CONSTEXPR_FUNCTION_TEMPLATES)

check_cxx_source_compiles("int main () { throw int(); }" HAVE_EXCEPTIONS)

check_cxx_source_compiles("#include <typeinfo>
int main () { (void)typeid(void); }" HAVE_RTTI)

if(WIN32 OR APPLE)
    set(HAVE_STDLIB ON)
else()
    check_library_exists(stdc++ exit "" HAVE_STDLIB)
endif()

configure_file(config.hpp.in include/rpc/config.hpp)

##############################################################################
# Boost

# Linking statically to Boost is easier for prototyping. For multi-module
# projects we'll want to use shared libraries and defined -DBOOST_ALL_DYN_LINK
# or the like.
set(Boost_USE_STATIC_LIBS ON)

# Boost.Log was introduced in Boost version 1.54.0.
# system is required for asio
find_package(Boost 1.54.0 REQUIRED COMPONENTS system filesystem log thread date_time regex)


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
        COMMAND ${PROTOC_EXECUTABLE} -I${_dir} -I${PROTOC_INCLUDE_DIR}
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
        PUBLIC ${protoDir}
        PUBLIC .)
    set_target_properties(${_targetName} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endfunction()

# Eat own dog food.
nanopb_add_proto(rpc-proto proto/rpc.proto)

##############################################################################

set(SOURCES
    src/message.cpp
    src/status.cpp
    ${NANOPB_ROOT}/pb_decode.c
    ${NANOPB_ROOT}/pb_encode.c
    ${NANOPB_ROOT}/pb_common.c)

if(HAVE_EXCEPTIONS)
    list(APPEND SOURCES src/system_error.cpp)
endif()


add_library(rpc STATIC ${SOURCES})

set_target_properties(rpc PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    COMPILE_FLAGS "-D__STDC_FORMAT_MACROS")

if(MSVC)
    target_compile_options(rpc PRIVATE /W4)
else()
    target_compile_options(rpc PRIVATE -Wall)
endif()
target_include_directories(rpc
    PUBLIC ${NANOPB_ROOT}
    PUBLIC include
    PRIVATE src
    PUBLIC ${PROJECT_BINARY_DIR}
    PUBLIC ${PROJECT_BINARY_DIR}/include
    PUBLIC ${Boost_INCLUDE_DIRS}
    )

# Our asio code is header-only, so no need for us to link with Boost/Winsock
# ourselves. Consumers of our headers will have to, though.
target_link_libraries(rpc PRIVATE rpc-proto INTERFACE ${Boost_LIBRARIES})
if(WIN32)
    target_link_libraries(rpc INTERFACE ws2_32)
endif()
set_target_properties(rpc PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
    SOVERSION ${VERSION_MAJOR}
    MACOSX_RPATH ON
)

##############################################################################

if(RPC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
