cmake_minimum_required(VERSION 3.5)
project(Gemesis)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "build")
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address -g -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra")
# Explicitly list the test source code and headers. The Catch header-only unit
# test framework is stored in with the test source.
set(CMake_SRC
  src/main.cpp
)
set(CMake_HEADER
  src/utils.h
  src/settings.h
  src/agent.h
  src/log.h
  src/move.h
  src/types.h
)

# set(CMake_GEN_SRC
#   src/generate/main.cpp
# )

# Make an executable target that depends on the test source code we specified
# above.
add_executable(gemesis ${CMake_SRC} ${CMake_HEADER})
# add_executable(generator ${CMake_GEN_SRC})

# add_custom_target(generator_run
#   COMMAND build/generator
#   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#   COMMENT "Running settings generator in ${CMAKE_CURRENT_SOURCE_DIR}"
#   SOURCES ${CMake_GEN_SRC}
# )

# add_dependencies(gemesis generator_run)
# add_dependencies(generator_run generator)

# Enable testing via CTest
enable_testing()
# Add our test as runnable via CTest

# Link our unit tests against the library we compiled
# target_link_libraries(TestCMakeDemo CMakeDemo)
