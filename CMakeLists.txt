project(VoxReader CXX)

add_library(VoxReader "Source/VoxReader.cpp")
set_target_properties(VoxReader PROPERTIES CXX_STANDARD 17)

add_subdirectory("Examples/ParseFile/" EXCLUDE_FROM_ALL)