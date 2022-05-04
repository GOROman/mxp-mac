#include "mdxplayer.h"

#define MXP_VERSION "0.01"

int main( int argc, char **argv )
{

	char*   mdxFilePath = NULL;		// 再生対象のファイルパス
//	int     maxLoops    = 0;		// ループ数上限。0 は無制限を意味する
	float   maxDuration = 0.0f;		// 再生時間（秒）上限。0 は無制限を意味する
    
    // 引数解析
	if (argc == 1) {
		printf(
			"Usage:\n"
			"	%s [Options] <MDX Filename>\n"
			"Option:\n"
//			"	-l <number>\n"
//			"		Specify the maximum number of loops.\n"
//			"		0 means infinite.\n"
			"	-d <seconds>\n"
			"		Specify the maximum playback length in seconds.\n"
			"		0 means infinite.\n"
			"Credit:\n"
            "\tX68k MXDRV music driver version 2.06+17 Rel.X5-S (c)1988-92 milk.,K.MAEKAWA, Missy.M, Yatsube\n"
            "\tConverted for Win32 [MXDRVg] V2.00a Copyright (C) 2000-2002 GORRY.\n"
            "\tX68Sound_src020615 Copyright (C) m_puusan.\n"
            "\tPorted for 64bit environments Copyright (C) 2018 Yosshin.\n\n",

			argv[0]
		);
		exit(EXIT_SUCCESS);
	} else {
		int i = 1;
		while (i < argc) {
			if (argv[i][0] == '-') {
				if (strcmp(argv[i], "-l") == 0) {
					i++;
					if (i >= argc) {
						printf("ERROR : No arg for '%s'.\n", argv[i - 1]);
						return EXIT_FAILURE;
					}
					char *endptr = NULL;
//					maxLoops = strtol(argv[i], &endptr, 0);
					if (*endptr != '\0') {
						printf("ERROR : Invalid arg for '%s'.\n", argv[i - 1]);
						return EXIT_FAILURE;
					}
				} else
				if (strcmp(argv[i], "-d") == 0) {
					i++;
					if (i >= argc) {
						printf("ERROR : No arg for '%s'.\n", argv[i - 1]);
						return EXIT_FAILURE;
					}
					char *endptr = NULL;
					maxDuration = strtof(argv[i], &endptr);
					if (*endptr != '\0') {
						printf("ERROR : Invalid arg for '%s'.\n", argv[i - 1]);
						return EXIT_FAILURE;
					}
				} else {
					printf("ERROR : Invalid arg '%s'.\n", argv[i]);
					return EXIT_FAILURE;
				}
			} else {
				if (mdxFilePath != NULL) {
					printf("ERROR : '%s' is already specified as a mdxfilename.\n", mdxFilePath);
					return EXIT_FAILURE;
				}
				mdxFilePath = argv[i];
			}
			i++;
		}
	}

    if ( maxDuration == 0.0f ) {
        maxDuration = 99999999.9f;
    }

	// 引数エラーチェック
	if (mdxFilePath == NULL) {
		printf("ERROR : Please specify a input mdx filepath.\n");
		exit(EXIT_FAILURE);
	}

    MDXPlayer       mdx;
    ScopedPaHandler paInit;

//    printf("PortAudio Test: output sine wave. SR = %d, BufSize = %d\n", SAMPLE_RATE, FRAMES_PER_BUFFER);

    if( paInit.result() != paNoError ) {
        printf("Error:paInit\n");
		exit(EXIT_FAILURE);
    }
    if ( mdx.open(Pa_GetDefaultOutputDevice()))
    {
        printf("X68k MXDRV music driver version 2.06+17 Rel.X5-S (c)1988-92 milk.,K.MAEKAWA, Missy.M, Yatsube\n");
        printf("music player version %s\n", MXP_VERSION);
        if ( mdx.load(mdxFilePath) ) {
            if ( mdx.start() )
            {

//                printf("\n\nPlay for %4.2f seconds.\n", maxDuration );
    
                Pa_Sleep( maxDuration * 1000 );

                mdx.fadeout();

                Pa_Sleep( 1000 );

                mdx.stop();
            }
            mdx.close();
        }
    }

	exit(EXIT_SUCCESS);
}

