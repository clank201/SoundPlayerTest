#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stb_vorbis.h"
#include <3ds.h>

u32 Samples;stb_vorbis *vorbisFile = NULL;
//----------------------------------------------------------------------------
void fill_buffer(void *audioBuffer, size_t size) {
//----------------------------------------------------------------------------

	u32 *dest = (u32*)audioBuffer;

	stb_vorbis_get_samples_short_interleaved(vorbisFile, 2, (short*)dest, Samples * 2);

	DSP_FlushDataCache(audioBuffer,size);

}

//----------------------------------------------------------------------------
int main(int argc, char **argv) {
//----------------------------------------------------------------------------

	PrintConsole topScreen;
	ndspWaveBuf waveBuf[2];

	gfxInitDefault();

	consoleInit(GFX_TOP, &topScreen);

	consoleSelect(&topScreen);

	printf("libctru streaming audio\n");
	
	
	
	stb_vorbis_info info;
	int error;


      vorbisFile = stb_vorbis_open_filename("/mau5.ogg", &error, NULL);
      info = stb_vorbis_get_info(vorbisFile);
      Samples = info.sample_rate;
   
	

	u32 *audioBuffer = (u32*)linearAlloc(Samples*sizeof(s16)*2);

	bool fillBlock = false;

	ndspInit();

	ndspSetOutputMode(NDSP_OUTPUT_STEREO);

	ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
	ndspChnSetRate(0, Samples);
	ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);

	float mix[12];
	memset(mix, 0, sizeof(mix));
	mix[0] = 1.0;
	mix[1] = 1.0;
	ndspChnSetMix(0, mix);

	int note = 4;

	memset(waveBuf,0,sizeof(waveBuf));
	waveBuf[0].data_vaddr = &audioBuffer[0];
	waveBuf[0].nsamples = Samples;
	waveBuf[1].data_vaddr = &audioBuffer[Samples];
	waveBuf[1].nsamples = Samples;

	ndspChnWaveBufAdd(0, &waveBuf[0]);
	ndspChnWaveBufAdd(0, &waveBuf[1]);

	printf("Press up/down to change tone\n");

	
	
	
	
	
	
	while(aptMainLoop()) {

		gfxSwapBuffers();
		gfxFlushBuffers();
		gspWaitForVBlank();

		hidScanInput();
		u32 kDown = hidKeysDown();

		if (kDown & KEY_START)
			break; // break in order to return to hbmenu


		if (waveBuf[fillBlock].status == NDSP_WBUF_DONE) {

			fill_buffer(waveBuf[fillBlock].data_pcm16, waveBuf[fillBlock].nsamples);

			ndspChnWaveBufAdd(0, &waveBuf[fillBlock]);

			fillBlock = !fillBlock;
		}
	}

	ndspExit();

	linearFree(audioBuffer);

	gfxExit();
	return 0;
}
