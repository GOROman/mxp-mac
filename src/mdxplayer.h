#pragma once
#include <stdint.h>
#include <stdlib.h>

#include <portaudio.h>

#include <mdx_util.h>
#include <mxdrv.h>
#include <mxdrv_context.h>

#include "mdxconfig.h"

class ScopedPaHandler
{
public:
    ScopedPaHandler() : _result(Pa_Initialize()) {
    }

    ~ScopedPaHandler() {
        if (_result == paNoError) {
            Pa_Terminate();
        }
    }

    PaError result() const { 
        return _result; 
    }

private:
    PaError _result;
};

class MDXPlayer
{
    MxdrvContext context;   // MXDRV Context

 	void*   mdxBuffer;
	void*   pdxBuffer;
    uint32_t mdxBufferSizeInBytes = 0;
    uint32_t pdxBufferSizeInBytes = 0;

    char mdxTitle[256];

    ScopedPaHandler paInit;
    PaStream* stream;


public:
    MDXPlayer();
    virtual ~MDXPlayer();

    bool open();
    bool load(char* mdxFilePath);
    bool close();

    bool play();
    bool stop();
    bool fadeout();

private:
    int paCallbackMethod(const void *inputBuffer, void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags);

    // This routine will be called by the PortAudio engine when audio is needed.
    // It may called at interrupt level on some machines so don't do anything
    // that could mess up the system like calling malloc() or free().
    static int paCallback( const void *inputBuffer, void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData );


    void paStreamFinishedMethod();

    // This routine is called by portaudio when playback is done.
    static void paStreamFinished(void* userData);

};

