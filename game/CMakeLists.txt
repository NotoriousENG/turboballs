# CMakeList.txt : CMake project for TinksWorld, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.12)

project ("game")

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(USE_SYSTEM_JSON ON CACHE BOOL "Use system Nlohmann JSON")
SET(NO_EXAMPLES ON CACHE BOOL "Disable examples")

option(COPY_GAME_ASSETS "Copy assets to build directory" ON) # when prototyping, set to OFF

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    # if CLANG/ GCC use GNU99 (This ensures that aflecs ddons that rely on time & socket functions are compiled correctly.)
    if (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
    endif()
    find_package(SDL2 REQUIRED)
    find_package(sdl2-mixer REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(OpenCV REQUIRED)
endif()

add_subdirectory(lib/glad)

# Set the TMXLITE_STATIC_LIB option to TRUE to request static linking
SET(TMXLITE_STATIC_LIB TRUE CACHE BOOL "Should tmxlite be built as a static or shared lib?")
add_subdirectory(lib/json)

# set INCLUDE_DIRS for modules
set(GLM_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/lib/glm)
set(JSON_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/lib/json/include)
set(TMX_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/lib/tmxlite/tmxlite/include)
set(GLAD_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/lib/glad/include)

# add engine modules
add_subdirectory(modules/input)
add_subdirectory(modules/mixer)
add_subdirectory(modules/render)

# project includes
include_directories(include)

# set the library type based on the BUILD_SHARED_LIBS option
if(BUILD_SHARED_LIBS)
  set(LIB_TYPE SHARED)
else()
  set(LIB_TYPE STATIC)
endif()

add_library (${PROJECT_NAME} ${LIB_TYPE} "src/game.cpp"
  "src/asset-manager-aggregates.cpp"
  )

# dependencies

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/modules/reload)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/modules/asset)

target_include_directories(${PROJECT_NAME} PUBLIC ${GLM_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/modules/input/include)
target_link_libraries(${PROJECT_NAME} PUBLIC input)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/modules/mixer/include)
target_link_libraries(${PROJECT_NAME} PUBLIC mixer)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/modules/render/include)
target_link_libraries(${PROJECT_NAME} PUBLIC render)

target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS} )
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS} )

# set a variable for the directory where the assets are copied to, on non windows it is CMAKE_RUNTIME_OUTPUT_DIRECTORY, on windows it is CMAKE_RUNTIME_OUTPUT_DIRECTORY/(Debug|Release)
if(WIN32)
    if (WIN_VSCODE_APPEND_CONFIG)
        set(FINAL_BINARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>) #VSCODE
    else()
        set(FINAL_BINARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}) # Visual Studio
    endif()
else()
    set(FINAL_BINARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

if (COPY_GAME_ASSETS)
  # copy assets to the build directory
  if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    add_custom_target(copy_assets
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/../assets ${FINAL_BINARY_DIR}/assets
            # log the command so it is visible in the build output
            COMMAND ${CMAKE_COMMAND} -E echo "copying assets to ${FINAL_BINARY_DIR}/assets"
        )
    add_dependencies(${PROJECT_NAME} copy_assets)
  else()
    add_custom_target(copy_assets
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/../assets ${CMAKE_CURRENT_BINARY_DIR}/../assets
        )
    add_dependencies(${PROJECT_NAME} copy_assets)
  endif()
endif()