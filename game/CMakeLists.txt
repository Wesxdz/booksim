cmake_minimum_required(VERSION 3.15)
project(agent_loop)

# Add the main executable sources
set(SOURCES main.c log.c)

# Add the executable target
add_executable(agent ${SOURCES})
include_directories("include" "SDL2_ttf-2.20.2")
set(CMAKE_BUILD_TYPE Debug)

# Add the subdirectory for flecs
add_subdirectory(flecs-3.2.4)
add_subdirectory(SDL2-2.0.20)
add_subdirectory(SDL2_ttf-2.20.2)
add_subdirectory(SDL_image-2.6.0)

# Include FetchContent module
# include(FetchContent)

# Tell FetchContent to fetch SDL2
# FetchContent_Declare(
#   SDL2
#   URL https://www.libsdl.org/release/SDL2-2.20.2.tar.gz  # replace with the URL to the version you're using
# )
# FetchContent_MakeAvailable(SDL2)

# Link the flecs library and SDL2 to the executable
target_link_libraries(agent flecs SDL2::SDL2 SDL2_ttf SDL2_image ${CMAKE_DL_LIBS} m pthread)

