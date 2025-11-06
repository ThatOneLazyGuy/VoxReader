add_executable(ParseFileExample "Source.cpp")
set_target_properties(ParseFileExample PROPERTIES CXX_STANDARD 17)

target_link_libraries(ParseFileExample VoxReader)
target_include_directories(ParseFileExample PRIVATE "${CMAKE_SOURCE_DIR}/Source/")