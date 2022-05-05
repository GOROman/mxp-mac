#include "mdxplayer.h"

#define MXP_VERSION "0.02"

// コマンドラインオプション
struct MXPOPTION {
	float	duration;
	int		verbose;
	char*	mdxFilePath;

	MXPOPTION() : duration(0), verbose(0), mdxFilePath(NULL) {
		opterr = 0; // getopt()のエラーメッセージを無効にする。
	}

	bool getoption(int argc, char** argv)
	{
		int o;
		while ((o = getopt(argc, argv, "vd:")) != -1) {
			switch(o) {
				case 'v':	verbose = 1;                       break;
				case 'd':	duration = strtof(optarg, NULL);   break;
				default:
					usage();
					return false;
			}
		}

		mdxFilePath = argv[optind];

		return true;
	}

	bool check() {
		if ( duration == 0.0f ) {
			duration = 100000.0f;
		}

		if (verbose) {
			printf("OPT:Duration: %4.2f\n", duration);
		}

		if (mdxFilePath == NULL) {
			printf("ERROR: Please specify an input mdx file.\n");
			return false;
		}
		return true;
	}

	void usage()
	{
		printf(
			"Usage:\n"
			"	mxp [Options] <MDX Filename>\n"
			"Option:\n"
			"	-d <seconds>\n"
			"		Specify the maximum playback length in seconds.\n"
			"		0 means infinite.\n"
			"CREDIT:\n"
			"\tX68k MXDRV music driver version 2.06+17 Rel.X5-S (c)1988-92 milk.,K.MAEKAWA, Missy.M, Yatsube\n"
			"\tConverted for Win32 [MXDRVg] V2.00a Copyright (C) 2000-2002 GORRY.\n"
			"\tX68Sound_src020615 Copyright (C) m_puusan.\n"
			"\tPorted for 64bit environments Copyright (C) 2018 Yosshin.\n\n"
		);

	}
};



int main( int argc, char **argv )
{
    MXPOPTION		opt;
    MDXPlayer       mdx;
        
	printf("X68k MXDRV music driver version 2.06+17 Rel.X5-S (c)1988-92 milk.,K.MAEKAWA, Missy.M, Yatsube\n");
	printf("music player version %s\n", MXP_VERSION);

	if ( opt.getoption( argc, argv ) == false )
	{
		exit(EXIT_SUCCESS);
	}

	if ( opt.check() == false ) {
		exit(EXIT_FAILURE);
	}

    if ( mdx.open(Pa_GetDefaultOutputDevice()))
    {
        if ( mdx.load(opt.mdxFilePath) ) {
            if ( mdx.start() )
            {
                Pa_Sleep( opt.duration * 1000 );
                mdx.fadeout();
                Pa_Sleep( 1000 );
                mdx.stop();
            }
            mdx.close();
        }
    }

	exit(EXIT_SUCCESS);
}

