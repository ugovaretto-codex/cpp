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

# Separate debug-release paths
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)

# Targets
add_executable(index_sequence index_sequence.cpp)
set_property(TARGET index_sequence
             PROPERTY CXX_STANDARD 17)   

add_executable(index_sequence_generator index_sequence_generator.cpp)
set_property(TARGET index_sequence_generator
             PROPERTY CXX_STANDARD 17)   

add_executable(variadic_templates variadic-templates.cpp)
set_property(TARGET variadic_templates
             PROPERTY CXX_STANDARD 17)  

add_executable(zip-variadic-fold zip-variadic-fold.cpp)
set_property(TARGET zip-variadic-fold
            PROPERTY CXX_STANDARD 17)

add_executable(float_constexpr float_constexpr.cpp)
set_property(TARGET float_constexpr                
             PROPERTY CXX_STANDARD 17) 

add_executable(vector_allocation vector_allocation.cpp)             

add_executable(tuple tuple.cpp)
set_property(TARGET tuple                
            PROPERTY CXX_STANDARD 20) 

add_executable(tuple2 tuple2.cpp)
set_property(TARGET tuple2             
            PROPERTY CXX_STANDARD 20)
            
             