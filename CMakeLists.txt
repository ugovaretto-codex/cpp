#https://cliutils.gitlab.io/modern-cmake/chapters/basics.html
cmake_minimum_required(VERSION 3.1...3.17)

if(${CMAKE_VERSION} VERSION_LESS 3.16)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.16)
endif()
project(cppscratch VERSION 1.0
                   DESCRIPTION "C++ scratchpad"
                   LANGUAGES CXX)
#packages
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Separate debug-release paths
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)

add_executable(cache cache.cpp)
set_property(TARGET cache
             PROPERTY CXX_STANDARD 17)   
target_link_libraries(cache PRIVATE Threads::Threads)     

add_executable(index_sequence index_sequence.cpp)
set_property(TARGET index_sequence
             PROPERTY CXX_STANDARD 17)   

add_executable(index_sequence_generator index_sequence_generator.cpp)
set_property(TARGET index_sequence_generator
             PROPERTY CXX_STANDARD 17)   

add_executable(variadic_templates variadic-templates.cpp)
set_property(TARGET variadic_templates
             PROPERTY CXX_STANDARD 17)  

# get_target_property(TEMP ${THE_TARGET} COMPILE_FLAGS)
# if(TEMP STREQUAL "TEMP-NOTFOUND")
#   SET(TEMP "") # Set to empty string
# else()
#   SET(TEMP "${TEMP} ") # A space to cleanly separate from existing content
# endif()
# # Append our values
# SET(TEMP "${TEMP}${GCC_COVERAGE_COMPILE_FLAGS}" )
# set_target_properties(${THE_TARGET} PROPERTIES COMPILE_FLAGS ${TEMP} )

