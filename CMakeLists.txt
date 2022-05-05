cmake_minimum_required(VERSION 3.13)
project(mxp CXX C)


#-----------------------------------------------------------------------------------------
function(add_git_submodule dir)

find_package(Git REQUIRED)

if(NOT EXISTS ${dir}/CMakeLists.txt)
  execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- ${dir}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMAND_ERROR_IS_FATAL ANY)
endif()

add_subdirectory(${dir})

endfunction(add_git_submodule)
#-----------------------------------------------------------------------------------------

add_git_submodule(portaudio)


set(SRC
    src/mxp.cpp
    portable_mdx/src/mdx_util.c
    portable_mdx/src/mxdrv/mxdrv.cpp
    portable_mdx/src/mxdrv/mxdrv_context.cpp
    portable_mdx/src/mxdrv/sound_iocs.cpp
    portable_mdx/src/x68sound/x68sound.cpp
    portable_mdx/src/x68sound/x68sound_adpcm.cpp
    portable_mdx/src/x68sound/x68sound_context.cpp
    portable_mdx/src/x68sound/x68sound_lfo.cpp
    portable_mdx/src/x68sound/x68sound_op.cpp
    portable_mdx/src/x68sound/x68sound_opm.cpp
    portable_mdx/src/x68sound/x68sound_pcm8.cpp
)

include_directories(
    portable_mdx/include
    portaudio/include
)

add_executable(mxp ${SRC})
target_link_libraries(mxp PortAudio)
target_compile_features(mxp PUBLIC cxx_std_11)
