cmake_minimum_required(VERSION 3.13)
project(mxp CXX C)

add_subdirectory(portaudio)

set(SRC
    src/mxp.cpp
    src/mdxconfig.h
    src/mdxplayer.h
    src/mdxplayer.cpp
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

INSTALL(TARGETS mxp RUNTIME DESTINATION bin)
