cmake_minimum_required(VERSION 3.5)
project(ribbon-bridge VERSION 0.3.0 LANGUAGES C CXX)

set(AVR OFF)
if(CMAKE_SYSTEM_NAME MATCHES "AVR")
    set(AVR ON)
endif()

option(RPC_BUILD_TESTS "Build ribbon-bridge tests" OFF)

if(AVR)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
else()
    set(boostComponents system filesystem log thread date_time regex)
endif()

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.54.0 REQUIRED COMPONENTS ${boostComponents})

if(AVR AND Boost_INCLUDE_DIR MATCHES "/usr/include")
    message(FATAL_ERROR "Cannot use Boost installed at /usr/include: host machine headers will override libc-avr's")
endif()

# We need to be able to express an interface dependency on the Boost headers, so set up this fancy
# IMPORTED target.
add_library(boost INTERFACE IMPORTED)
set_property(TARGET boost PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})

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

set(RPC_PROTO_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/proto)
include(${PROJECT_NAME}-functions.cmake)

# Eat own dog food.
nanopb_add_proto(rpc-proto proto/rpc.proto)

##############################################################################

set(SOURCES
    src/message.cpp
    src/status.cpp
)

if(HAVE_EXCEPTIONS)
    list(APPEND SOURCES src/system_error.cpp)
endif()


add_library(rpc STATIC ${SOURCES})

set_target_properties(rpc
    PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
        POSITION_INDEPENDENT_CODE ON
        COMPILE_FLAGS "-D__STDC_FORMAT_MACROS"
)

if(MSVC)
    target_compile_options(rpc PRIVATE /W4)
else()
    target_compile_options(rpc PRIVATE -Wall)
endif()
target_include_directories(rpc
    PRIVATE include
    PRIVATE ${PROJECT_BINARY_DIR}
    PRIVATE ${PROJECT_BINARY_DIR}/include
)

target_link_libraries(rpc PRIVATE rpc-proto INTERFACE boost)
if(HAVE_EXCEPTIONS)
    target_link_libraries(rpc PUBLIC Boost::system)
endif()
if(WIN32)
    # Our asio code is header-only, so no need for us to link with Winsock
    # ourselves. Consumers of our headers will have to, though.
    target_link_libraries(rpc INTERFACE ws2_32)
endif()
set_target_properties(rpc PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    MACOSX_RPATH ON
)

##############################################################################

if(RPC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()


install(TARGETS rpc rpc-proto EXPORT barobo
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY ${PROJECT_BINARY_DIR}/include/ DESTINATION include)
install(DIRECTORY proto/ DESTINATION proto)
install(FILES ${PROJECT_BINARY_DIR}/rpc.pb.h DESTINATION include)
install(FILES ${PROJECT_NAME}-functions.cmake DESTINATION cmake)

# Boilerplate follows

export(EXPORT barobo
    FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake"
    NAMESPACE barobo::
)

install(EXPORT barobo
    FILE ${PROJECT_NAME}-targets.cmake
    NAMESPACE barobo::
    DESTINATION cmake
)

configure_file(${PROJECT_NAME}-config.cmake.in ${PROJECT_NAME}-config.cmake @ONLY)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-configVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-configVersion.cmake"
    DESTINATION cmake
)
