set_source_files_properties(register-object.cpp PROPERTIES COMPILE_FLAGS -std=c++11)
add_executable(register-object register-object.cpp)
add_test(NAME register-object COMMAND register-object)
