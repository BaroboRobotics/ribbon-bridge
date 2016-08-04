##############################################################################
# Set up nanopb

set(oldCfrpmi ${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE})
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NO_CMAKE_FIND_ROOT_PATH)
# The CMAKE_FIND_ROOT_PATH_MODE_INCLUDE grotesquery is required to make the find_path(... pb.h)
# call succeed in FindNanopb.cmake under cross-compilation. Resetting it via the oldCfrpmi
# temporary variable might not strictly be required.
find_package(Nanopb REQUIRED)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ${oldCfrpmi})

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
    list(APPEND NANOPB_IMPORT_DIRS ${RPC_PROTO_INCLUDE_DIR})
    nanopb_generate_cpp(sources headers ${_protoFile})

    if(MSVC)
        # Disable warning C4127: conditional expression is constant
        set_source_files_properties(${sources}
            PROPERTIES COMPILE_FLAGS /wd4127
        )
    endif()

    add_library(${_targetName} STATIC ${sources} ${headers} ${ARGN})
    set(headerDirs)
    foreach(header ${headers})
        get_filename_component(headerDir ${header} DIRECTORY)
        list(APPEND headerDirs ${headerDir})
    endforeach()
    list(REMOVE_DUPLICATES headerDirs)
    target_include_directories(${_targetName}
        PUBLIC
            ${NANOPB_INCLUDE_DIRS}
        PRIVATE
            ${headerDirs}
            .
    )
    if(NOT CMAKE_SYSTEM_NAME MATCHES "AVR")
        set_target_properties(${_targetName} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()
endfunction()
