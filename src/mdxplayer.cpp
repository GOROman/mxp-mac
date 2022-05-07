#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <libgen.h> // for dirname()
#include <unistd.h>

#include "mdxplayer.h"

#define MDX_BUFFER_SIZE 1 * 1024 * 1024
#define PDX_BUFFER_SIZE 2 * 1024 * 1024
#define MEMORY_POOL_SIZE 8 * 1024 * 1024

// メモリ確保した領域にファイルを読み込む
static void *mallocReadFile(
    const char *fileName,
    uint32_t *sizeRet)
{
    FILE *fd = fopen(fileName, "rb");
    if (fd == NULL)
        return NULL;
    struct stat stbuf;
    if (fstat(fileno(fd), &stbuf) == -1)
    {
        fclose(fd);
        return NULL;
    }

    assert(stbuf.st_size < 0x100000000LL);
    uint32_t size = (uint32_t)stbuf.st_size;
    void *buffer = malloc(size);
    if (buffer == NULL)
    {
        fclose(fd);
        return NULL;
    }
    fread(buffer, 1, size, fd);
    *sizeRet = size;
    fclose(fd);
    return buffer;
}

MDXPlayer::MDXPlayer() : stream(0), mdxBuffer(NULL), pdxBuffer(NULL)
{
    // コンテキストの初期化
    if (MxdrvContext_Initialize(&context, MEMORY_POOL_SIZE) == false)
    {
        printf("MxdrvContext_Initialize failed.\n");
    }
}

MDXPlayer::~MDXPlayer()
{
    MxdrvContext_Terminate(&context);
}

bool MDXPlayer::open()
{
    PaDeviceIndex index = Pa_GetDefaultOutputDevice();

    if (paInit.result() != paNoError)
    {
        printf("Error:paInit\n");
        return false;
    }

    PaStreamParameters outputParameters;

    outputParameters.device = index;
    if (outputParameters.device == paNoDevice)
    {
        return false;
    }

    const PaDeviceInfo *pInfo = Pa_GetDeviceInfo(index);
    if (pInfo != 0)
    {
        //            printf("Output device name: %s\n", pInfo->name);
    }

    outputParameters.channelCount = 2; // stereo output
    outputParameters.sampleFormat = paInt16;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    PaError err = Pa_OpenStream(
        &stream,
        NULL, // no input
        &outputParameters,
        SAMPLE_RATE,
        paFramesPerBufferUnspecified,
        paClipOff, // we won't output out of range samples so don't bother clipping them
        &MDXPlayer::paCallback,
        this // Using 'this' for userData so we can cast to MDXPlayer* in paCallback method
    );

    if (err != paNoError)
    {
        // Failed to open stream to device !!!
        return false;
    }

    err = Pa_SetStreamFinishedCallback(stream, &MDXPlayer::paStreamFinished);

    if (err != paNoError)
    {
        Pa_CloseStream(stream);
        stream = 0;

        return false;
    }

    return true;
}

bool MDXPlayer::load(char *mdxFilePath)
{
    {
        // MDX ファイルパスが "" で括られている場合の補正
        size_t len = strlen(mdxFilePath);
        if (len > 0)
        {
            if (mdxFilePath[0] == '\"' && mdxFilePath[len - 1] == '\"')
            {
                mdxFilePath[len - 1] = '\0';
                mdxFilePath++;
            }
        }

        // MDX ファイルの読み込み
        //            printf("MDX:%s\n", mdxFilePath);
        uint32_t mdxFileImageSizeInBytes = 0;
        void *mdxFileImage = mallocReadFile(mdxFilePath, &mdxFileImageSizeInBytes);
        if (mdxFileImage == NULL)
        {
            printf("mallocReadFile '%s' failed.\n", mdxFilePath);
            return false;
        }

        // MDX タイトルの取得
        if (
            MdxGetTitle(
                mdxFileImage, mdxFileImageSizeInBytes,
                mdxTitle, sizeof(mdxTitle)) == false)
        {
            printf("MdxGetTitle failed.\n");
            return false;
        }
        printf("TITLE:%s\n", mdxTitle);

        // PDX ファイルを要求するか？
        bool hasPdx;
        if (
            MdxHasPdxFileName(
                mdxFileImage, mdxFileImageSizeInBytes,
                &hasPdx) == false)
        {
            printf("MdxHasPdxFileName failed.\n");
            return false;
        }

        // PDX ファイルの読み込み
        uint32_t pdxFileImageSizeInBytes = 0;
        void *pdxFileImage = NULL;
        if (hasPdx)
        {
            char pdxFileName[FILENAME_MAX] = {0};
            if (
                MdxGetPdxFileName(
                    mdxFileImage, mdxFileImageSizeInBytes,
                    pdxFileName, sizeof(pdxFileName)) == false)
            {
                printf("MdxGetPdxFileName failed.\n");
                return false;
            }
            printf("%s\n", pdxFileName);

            char mdxFilePathTmp[FILENAME_MAX] = {0};
            strncpy(mdxFilePathTmp, mdxFilePath, sizeof(mdxFilePathTmp));
            mdxFilePathTmp[FILENAME_MAX - 1] = '\0';
            const char *mdxDirName = dirname(mdxFilePathTmp);

            /*
                ファイル名の大文字小文字が区別される環境では
                    大文字ファイル名 + 大文字拡張子
                    大文字ファイル名 + 小文字拡張子
                    小文字ファイル名 + 大文字拡張子
                    小文字ファイル名 + 小文字拡張子
                の 4 通りで PDX ファイル読み込みを試す必要がある。
            */
            for (int retryCount = 0; retryCount < 4; retryCount++)
            {
                char modifiedPdxFileName[FILENAME_MAX];
                memcpy(modifiedPdxFileName, pdxFileName, FILENAME_MAX);
                if (retryCount & 1)
                {
                    /* ファイル名部分の大文字小文字反転 */
                    for (char *p = modifiedPdxFileName; *p != '\0' && *p != '.'; p++)
                    {
                        if ('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z')
                            *p ^= 0x20;
                    }
                }
                if (retryCount & 2)
                {
                    /* 拡張子部分の大文字小文字反転 */
                    char *p = modifiedPdxFileName;
                    while (strchr(p, '.') != NULL)
                        p = strchr(p, '.') + 1;
                    for (; *p != '\0'; p++)
                    {
                        if ('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z')
                            *p ^= 0x20;
                    }
                }

                char pdxFilePath[FILENAME_MAX];
                sprintf(pdxFilePath, "%s/%s", mdxDirName, modifiedPdxFileName);
                //                    printf("read %s ... ", pdxFilePath);
                pdxFileImage = mallocReadFile(pdxFilePath, &pdxFileImageSizeInBytes);
                if (pdxFileImage != NULL)
                {
                    //                        printf("succeeded.\n");
                    break;
                }
                else
                {
                    printf("failed.\n");
                }
            }
        }

        /* MDX PDX バッファの要求サイズを求める */
        if (
            MdxGetRequiredBufferSize(
                mdxFileImage,
                mdxFileImageSizeInBytes, pdxFileImageSizeInBytes,
                &mdxBufferSizeInBytes, &pdxBufferSizeInBytes) == false)
        {
            printf("MdxGetRequiredBufferSize failed.\n");
            return false;
        }
        //			printf("mdxBufferSizeInBytes = %d\n", mdxBufferSizeInBytes);
        //			printf("pdxBufferSizeInBytes = %d\n", pdxBufferSizeInBytes);

        /* MDX PDX バッファの確保 */
        if (mdxBuffer != NULL)
            free(mdxBuffer);
        mdxBuffer = (uint8_t *)malloc(mdxBufferSizeInBytes);
        if (mdxBuffer == NULL)
        {
            printf("malloc mdxBuffer failed.\n");
            return false;
        }
        if (hasPdx)
        {
            if (pdxBuffer != NULL)
                free(pdxBuffer);
            pdxBuffer = (uint8_t *)malloc(pdxBufferSizeInBytes);
            if (pdxBuffer == NULL)
            {
                printf("malloc pdxBuffer failed.\n");
                return false;
            }
        }

        /* MDX PDX バッファを作成 */
        if (
            MdxUtilCreateMdxPdxBuffer(
                mdxFileImage, mdxFileImageSizeInBytes,
                pdxFileImage, pdxFileImageSizeInBytes,
                mdxBuffer, mdxBufferSizeInBytes,
                pdxBuffer, pdxBufferSizeInBytes) == false)
        {
            printf("MdxUtilCreateMdxPdxBuffer failed.\n");
            return false;
        }

        /* この時点でファイルイメージは破棄してよい */
        if (pdxFileImage != NULL)
            free(pdxFileImage);
        free(mdxFileImage);
    }

    // MXDRV の初期化
    {
        int ret = MXDRV_Start(
            &context,
            SAMPLE_RATE,
            0, 0, 0,
            MDX_BUFFER_SIZE,
            PDX_BUFFER_SIZE,
            0);
        if (ret != 0)
        {
            printf("MXDRV_Start failed. return code = %d\n", ret);
            return false;
        }
    }

    /* PCM8 を有効化 */
    uint8_t *pcm8EnableFlag = (uint8_t *)MXDRV_GetWork(&context, MXDRV_WORK_PCM8);
    *(pcm8EnableFlag) = 1;

    // 音量設定
    MXDRV_TotalVolume(&context, 256);
#if 0
        // 再生時間を求める
        {
            float songDurationInSeconds = MXDRV_MeasurePlayTime(
                &context,
                mdxBuffer, mdxBufferSizeInBytes,
                pdxBuffer, pdxBufferSizeInBytes,
                1, 0
            ) / 1000.0f;
            printf("song duration %.1f(sec)\n", songDurationInSeconds);
        }
#endif

    return true; // NoError
}

bool MDXPlayer::close()
{
    if (stream == 0)
        return false;

    PaError err = Pa_CloseStream(stream);
    stream = 0;

    MXDRV_End(&context);

    if (pdxBuffer != NULL)
    {
        free(pdxBuffer);
        pdxBuffer = NULL;
    }
    if (mdxBuffer != NULL)
    {
        free(mdxBuffer);
        mdxBuffer = NULL;
    }

    return (err == paNoError);
}

bool MDXPlayer::play()
{
    if (stream == 0)
    {
        return false;
    }

    // MDX 再生
    MXDRV_Play(
        &context,
        mdxBuffer, mdxBufferSizeInBytes,
        pdxBuffer, pdxBufferSizeInBytes);

    PaError err = Pa_StartStream(stream);

    return (err == paNoError);
}

bool MDXPlayer::stop()
{
    if (stream == 0)
    {
        return false;
    }

    PaError err = Pa_StopStream(stream);

    return (err == paNoError);
}

bool MDXPlayer::fadeout()
{
    MXDRV_Fadeout2(&context, 1);

    return true;
}

int MDXPlayer::paCallbackMethod(const void *inputBuffer, void *outputBuffer,
                     unsigned long framesPerBuffer,
                     const PaStreamCallbackTimeInfo *timeInfo,
                     PaStreamCallbackFlags statusFlags)
{

    (void)timeInfo; // Prevent unused variable warnings.
    (void)statusFlags;
    (void)inputBuffer;

    MXDRV_GetPCM(&context, outputBuffer, framesPerBuffer);

    return paContinue;
}

// This routine will be called by the PortAudio engine when audio is needed.
// It may called at interrupt level on some machines so don't do anything
// that could mess up the system like calling malloc() or free().
int MDXPlayer::paCallback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo *timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData)
{
    // Here we cast userData to MDXPlayer* type so we can call the instance method paCallbackMethod, we can do that since
    //   we called Pa_OpenStream with 'this' for userData
    return ((MDXPlayer *)userData)->paCallbackMethod(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

void MDXPlayer::paStreamFinishedMethod()
{
}

//
// This routine is called by portaudio when playback is done.
//
void MDXPlayer::paStreamFinished(void *userData)
{
    return ((MDXPlayer *)userData)->paStreamFinishedMethod();
}
