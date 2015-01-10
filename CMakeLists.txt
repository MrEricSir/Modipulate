cmake_minimum_required(VERSION 2.6)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(modipulate_path ${CMAKE_SOURCE_DIR}/src/modipulate)
set(libopenmpt_path ${modipulate_path}/libopenmpt-forked)
set(modipulatelua_path ${CMAKE_SOURCE_DIR}/src/modipulate_lua)
set(modipulateosc_path ${CMAKE_SOURCE_DIR}/src/modipulate-osc)
set(oscpkt_path ${modipulateosc_path}/oscpkt)
set(demo_path ${CMAKE_SOURCE_DIR}/demos)

project(modipulate)

find_package(portaudio REQUIRED)
find_package(Vorbis REQUIRED)
find_package(Lua51)
find_package(Threads)

include_directories(${PORTAUDIO_INCLUDE_DIRS})
include_directories(${VORBIS_INCLUDE_DIRS})
include_directories(${libopenmpt_path} ${libopenmpt_path}/common ${libopenmpt_path}/libopenmpt ${libopenmpt_path}/include ${modipulate_path})

if (UNIX AND NOT APPLE)
    # -fPIC for Linux
    add_definitions(-fPIC)
endif()

# Needed for clang -- TODO: replace with actual Clang check instead of APPLE
if (APPLE)
    set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
endif (APPLE)

# Removes a filename from a list
function(remove_from_list haystack needle)
    SET(i 0)
    foreach(entry ${${haystack}})
        if (entry MATCHES "(${needle})$")
            list(REMOVE_AT ${haystack} ${i})
            break()
        endif ()
        MATH(EXPR i "${i} + 1")
    endforeach()
    set(${haystack} ${${haystack}} PARENT_SCOPE)
endfunction(remove_from_list)


# libopenmpt-forked
add_definitions( -DLIBOPENMPT_BUILD )
add_definitions( -DNO_LIBMODPLUG )
file(GLOB libopenmpt_sources
    "${libopenmpt_path}/common/*.c*"
    "${libopenmpt_path}/include/miniz/miniz.*"
    "${libopenmpt_path}/include/miniz/timer.*"
    "${libopenmpt_path}/include/miniz/tinfl.*"
    "${libopenmpt_path}/libopenmpt/*.c*"
    "${libopenmpt_path}/soundlib/*.c*"
    "${libopenmpt_path}/soundlib/plugins/*.c*"
    "${libopenmpt_path}/soundlib/Tunings/*.c*"
    )
add_library(libopenmpt-forked STATIC ${libopenmpt_sources})

message(status "portaudio libraries" ${PORTAUDIO_LIBRARIES})
message(status "vorbis libraries" ${VORBIS_LIBRARIES})

set_target_properties(libopenmpt-forked PROPERTIES PREFIX "")
target_link_libraries(libopenmpt-forked ${PORTAUDIO_LIBRARIES} ${VORBIS_LIBRARIES})
if(APPLE) # -- TODO: Linux too?
    target_link_libraries(libopenmpt-forked iconv)
endif(APPLE)

# modipulate
file(GLOB modipulate_sources 
    "${modipulate_path}/*.c*" 
	"${modipulate_path}/*.h*"
    "${modipulate_path}/timer/*.c*"
	"${modipulate_path}/timer/*.h*"
    )
add_library(libmodipulate-static STATIC ${modipulate_sources})
add_library(libmodipulate SHARED ${modipulate_sources})
set_target_properties(libmodipulate-static PROPERTIES PREFIX "")
set_target_properties(libmodipulate PROPERTIES PREFIX "")
target_link_libraries(libmodipulate-static
    ${PORTAUDIO_LIBRARIES} 
    ${VORBIS_LIBRARIES}
    libopenmpt-forked
    )
target_link_libraries(libmodipulate 
    ${PORTAUDIO_LIBRARIES}
    ${VORBIS_LIBRARIES}
    libopenmpt-forked
    )


# modipulate-lua
# Optional: will only be built if Lua is present.
if (LUA51_FOUND)
    # Takes care of a bug where lua.h 
    # ends up in the library list.
    remove_from_list(LUA_LIBRARIES "lua.h")
    
    include_directories(${LUA_INCLUDE_DIR})
    file(GLOB modipulatelua_sources 
        "${modipulatelua_path}/*.c*"
        "${modipulatelua_path}/*.def*"
        )
    if (NOT WIN32)
        remove_from_list(modipulatelua_sources "dllmain.cpp")
    endif ()
    
    add_library(libmodipulatelua SHARED ${modipulatelua_sources})
    set_target_properties(libmodipulatelua PROPERTIES PREFIX "")
    target_link_libraries(libmodipulatelua 
        ${PORTAUDIO_LIBRARIES}
        ${LUA_LIBRARIES}
        ${VORBIS_LIBRARIES}
        libopenmpt-forked
        libmodipulate-static
    )
endif ()


# modipulate-osc
if(CMAKE_USE_PTHREADS_INIT OR CMAKE_USE_WIN32_THREADS_INIT)
    include_directories(${modipulateosc_path} ${oscpkt_path})
    set(modipulateosc_sources
        "${modipulateosc_path}/main.cpp"
    )
    add_executable(modipulate-osc ${modipulateosc_sources})
    target_link_libraries(modipulate-osc
        ${PORTAUDIO_LIBRARIES}
        ${VORBIS_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        libmodipulate-static
    )
endif(CMAKE_USE_PTHREADS_INIT OR CMAKE_USE_WIN32_THREADS_INIT)


# demo: console
file(GLOB demo_console_sources 
    "${demo_path}/modipulate/console/*.c*"
    )
add_executable(console ${demo_console_sources})
target_link_libraries(console
    ${PORTAUDIO_LIBRARIES}
    ${VORBIS_LIBRARIES}
    libmodipulate-static
    )

