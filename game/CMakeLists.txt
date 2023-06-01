cmake_minimum_required(VERSION 3.15)
project(agent_loop)

# Add the main executable sources
set(SOURCES main.c)

# Add the executable target
add_executable(agent ${SOURCES})
include_directories("include")

# Add the subdirectory for flecs
add_subdirectory(flecs)

# Include FetchContent module
include(FetchContent)

# Tell FetchContent to fetch SDL2
FetchContent_Declare(
  SDL2
  URL https://www.libsdl.org/release/SDL2-2.0.16.tar.gz  # replace with the URL to the version you're using
)
FetchContent_MakeAvailable(SDL2)

# Link the flecs library and SDL2 to the executable
target_link_libraries(agent flecs SDL2::SDL2 ${CMAKE_DL_LIBS} m)
