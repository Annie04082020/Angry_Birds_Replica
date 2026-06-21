set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.16)
project(Angry_Birds_Replica)

set(CMAKE_CXX_STANDARD 17)

cmake_policy(SET CMP0135 NEW)

# include(FetchContent)
# set(FETCHCONTENT_QUIET FALSE)

# FetchContent_Declare(
#     ptsd

#     URL         https://github.com/ntut-open-source-club/practical-tools-for-simple-design/archive/refs/tags/v0.2.zip

#     # Or alternatively if you want to use a specific git commit instead of a release:
#     # GIT_REPOSITORY  https://github.com/ntut-open-source-club/practical-tools-for-simple-design.git
#     # GIT_TAG         <git_tag>

#     SOURCE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/PTSD
# )

# FetchContent_MakeAvailable(ptsd)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/PTSD)

include(files.cmake)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
list(TRANSFORM SRC_FILES PREPEND ${SRC_DIR}/)
list(TRANSFORM INCLUDE_FILES PREPEND ${INCLUDE_DIR}/)

# file(GLOB_RECURSE SRC_FILES src/*)
# file(GLOB_RECURSE HEADER_FILES include/*)

add_executable(${PROJECT_NAME}
    ${SRC_DIR}/main.cpp
    ${SRC_FILES}
    ${INCLUDE_FILES}
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Resources")

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${DEPENDENCY_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/PTSD/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE PROJECT_HEADER_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
set(PROJECT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
foreach(header_file ${PROJECT_HEADER_FILES})
    get_filename_component(header_dir ${header_file} DIRECTORY)
    list(APPEND PROJECT_INCLUDE_DIRS ${header_dir})
endforeach()
list(REMOVE_DUPLICATES PROJECT_INCLUDE_DIRS)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
    SDL2::SDL2main
    PTSD
)

add_custom_target(reset_unlock_progress ALL
    COMMAND ${CMAKE_COMMAND}
            -DOUTPUT_PATH=${CMAKE_CURRENT_SOURCE_DIR}/Resources/unlock_progress.json
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/reset_unlock_progress.cmake
    COMMENT "Resetting level unlock progress for this build"
)

add_dependencies(reset_unlock_progress ${PROJECT_NAME})
