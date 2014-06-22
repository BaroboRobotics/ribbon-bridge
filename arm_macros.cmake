# Extract the binary from the .elf file
macro(EXTRACT_BIN_FILE EXE_NAME)
 set(FO ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.bin)
 set(FI ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME})
 message(STATUS ${FO})
 add_custom_command(
  OUTPUT "${FO}"
  COMMAND ${CMAKE_OBJCOPY}
  ARGS -O binary -R .eeprom ${FI} ${FO}
  DEPENDS ${FI})
 get_filename_component(TGT "${EXE_NAME}" NAME)
 add_custom_target("${TGT}.bin" ALL DEPENDS ${FO} VERBATIM)
 get_directory_property(extra_clean_files ADDITIONAL_MAKE_CLEAN_FILES)
 set_directory_properties(
  PROPERTIES
  ADDITIONAL_MAKE_CLEAN_FILES "${extra_clean_files};${FO}")
 set_source_files_properties("${FO}" PROPERTIES GENERATED TRUE)
endmacro(EXTRACT_BIN_FILE)

# Extract the hex from the .elf file
macro(EXTRACT_IHEX_FILE EXE_NAME)
 set(FO ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.hex)
 set(FI ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME})
 message(STATUS ${FO})
 add_custom_command(
  OUTPUT "${FO}"
  COMMAND ${CMAKE_OBJCOPY}
  ARGS -O ihex -R .eeprom ${FI} ${FO}
  DEPENDS ${FI})
 get_filename_component(TGT "${EXE_NAME}" NAME)
 add_custom_target("${TGT}.hex" ALL DEPENDS ${FO} VERBATIM)
 get_directory_property(extra_clean_files ADDITIONAL_MAKE_CLEAN_FILES)
 set_directory_properties(
  PROPERTIES
  ADDITIONAL_MAKE_CLEAN_FILES "${extra_clean_files};${FO}")
 set_source_files_properties("${FO}" PROPERTIES GENERATED TRUE)
endmacro(EXTRACT_IHEX_FILE)

# Create a source/assembly listing
macro(EXTRACT_LISTING EXE_NAME)
 set(FO ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.list)
 set(FI ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME})
 message(STATUS ${FO})
 add_custom_command(
  OUTPUT "${FO}"
  COMMAND ${CMAKE_OBJDUMP}
  ARGS -h -S -C -r ${FI} > ${FO}
  DEPENDS ${FI})
 get_filename_component(TGT "${EXE_NAME}" NAME)
 add_custom_target("${TGT}.list" ALL DEPENDS ${FO} VERBATIM)
 get_directory_property(extra_clean_files ADDITIONAL_MAKE_CLEAN_FILES)
 set_directory_properties(
  PROPERTIES
  ADDITIONAL_MAKE_CLEAN_FILES "${extra_clean_files};${FO}")
 set_source_files_properties("${FO}" PROPERTIES GENERATED TRUE)
endmacro(EXTRACT_LISTING)

macro(SYMBOL_TABLE EXE_NAME)
 set(FO ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.sym)
 set(FI ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME})
 message(STATUS ${FO})
 add_custom_command(
  OUTPUT "${FO}"
  COMMAND ${CMAKE_NM}
  ARGS -n ${FI} > ${FO}
  DEPENDS ${FI})
 get_filename_component(TGT "${EXE_NAME}" NAME)
 add_custom_target("${TGT}.sym" ALL DEPENDS ${FO} VERBATIM)
 get_directory_property(extra_clean_files ADDITIONAL_MAKE_CLEAN_FILES)
 set_directory_properties(
  PROPERTIES
  ADDITIONAL_MAKE_CLEAN_FILES "${extra_clean_files};${FO}")
 set_source_files_properties("${FO}" PROPERTIES GENERATED TRUE)
endmacro(SYMBOL_TABLE)
