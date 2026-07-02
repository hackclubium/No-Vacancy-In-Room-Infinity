cmake_minimum_required(VERSION 3.16)
project(NoVacancyInRoomInfinity CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(no_vacancy
    src/main.cpp
)

target_include_directories(no_vacancy PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

if(EMSCRIPTEN)
    target_compile_options(no_vacancy PRIVATE
        -sUSE_SDL=2
        -sUSE_SDL_TTF=2
        -sUSE_SDL_IMAGE=2
        -sSDL2_IMAGE_FORMATS=["png"]
        -sALLOW_MEMORY_GROWTH=1
        -sWASM=1
        -sEXIT_RUNTIME=0
        -O3
    )

    target_link_options(no_vacancy PRIVATE
        -sUSE_SDL=2
        -sUSE_SDL_TTF=2
        -sUSE_SDL_IMAGE=2
        -sSDL2_IMAGE_FORMATS=["png"]
        -sALLOW_MEMORY_GROWTH=1
        -sWASM=1
        -sEXIT_RUNTIME=0
        --shell-file ${CMAKE_SOURCE_DIR}/shell.html
        --preload-file ${CMAKE_SOURCE_DIR}/assets@assets
    )

    set(CMAKE_EXECUTABLE_SUFFIX ".html")
else()
    find_package(SDL2 REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(SDL2_image REQUIRED)

    if(TARGET SDL2::SDL2_ttf)
        set(SDL2_TTF_TARGET SDL2::SDL2_ttf)
    else()
        set(SDL2_TTF_TARGET SDL2_ttf::SDL2_ttf)
    endif()

    if(TARGET SDL2::SDL2_image)
        set(SDL2_IMAGE_TARGET SDL2::SDL2_image)
    else()
        set(SDL2_IMAGE_TARGET SDL2_image::SDL2_image)
    endif()

    if(WIN32 AND TARGET SDL2::SDL2main)
        target_link_libraries(no_vacancy SDL2::SDL2main)
    endif()
    target_link_libraries(no_vacancy SDL2::SDL2 ${SDL2_TTF_TARGET} ${SDL2_IMAGE_TARGET})

    add_custom_command(TARGET no_vacancy POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:no_vacancy>/assets
    )
endif()

install(TARGETS no_vacancy RUNTIME DESTINATION .)
