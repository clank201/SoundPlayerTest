#include <3ds.h>
#include <cstdio>
#include "stb_vorbis.h"

#include <cstdlib>
#include <iostream>
#include <cstring>
#include <dirent.h>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>

using namespace std;

stb_vorbis *v = NULL;

enum {
   STATE_IDLE,
   STATE_FC
};

u32 state = STATE_IDLE;
char *filename;
std::string cur_dir = "";
u32 cursor_pos = 0;
bool draw_ui = true;
int error;
s16 *audiobuf = NULL;
stb_vorbis_info info;
u32 Samples;
u32 audiobuf_size;
bool loop_flag = false;
bool paused = false;

std::string currently_playing;

u32 audiobuf_index = 0;

void play_file_from_filename(const std::string name) {
   currently_playing = std::string(name);
   if (audiobuf) linearFree(audiobuf);

      v = stb_vorbis_open_filename(name.c_str(), &error, NULL);
      info = stb_vorbis_get_info(v);
      Samples = info.sample_rate;
      audiobuf_size = Samples * sizeof(s16) * 2;
      audiobuf = (s16*)linearAlloc(audiobuf_size);

   paused = false;
}

void play_file() {
   DIR *dir;
   struct dirent *ent;
   if ((dir = opendir (cur_dir.c_str())) != NULL) {

      u32 cur = 1;
      while ((ent = readdir (dir)) != NULL) {
         if (strncmp(ent->d_name, ".", 1) == 0) continue;
         if (cur == cursor_pos) {
            if (v) {
               stb_vorbis_close(v);
            }
            std::string name = cur_dir + "/" + ent->d_name;
            play_file_from_filename(name);
            closedir (dir);
            return;
         }
         cur++;
      }
      closedir (dir);
   } else {
      /* could not open directory */
      perror ("");
   }
}

//void *device = NULL;

int main()
{
   // Initialize services
   srvInit();
   aptInit();
   hidInit();
   //ptmInit();
   csndInit();
   gfxInitDefault();
   // fsInit();
   // sdmcInit();
	consoleInit(GFX_BOTTOM, NULL);
   chdir("sdmc:/");

   int frames = 0;
   int channel = 0x8;
	play_file_from_filename("/mau5.ogg");
	
   while (aptMainLoop())
   {
	   hidScanInput();
	   if (hidKeysDown() & KEY_START)break;
      gspWaitForVBlank();
      if (audiobuf && !paused) {
         if(frames >= 60) {
            frames = 0;
            int n = 0;
            n = stb_vorbis_get_samples_short_interleaved(v, 2, audiobuf, Samples * 2);
            if(n == 0) {
                  stb_vorbis_close(v);

               linearFree(audiobuf);
               audiobuf = NULL;
               v = NULL;
               if (loop_flag) play_file_from_filename(currently_playing);
            }
            GSPGPU_FlushDataCache((u8*)audiobuf, audiobuf_size);
            csndPlaySound(SOUND_CHANNEL(channel), SOUND_ONE_SHOT | SOUND_LINEAR_INTERP | SOUND_FORMAT_16BIT, Samples * 2, 10.0, 0.0, (u32*)audiobuf, (u32*)audiobuf, audiobuf_size);

         }
         frames++;
      }
	  gfxFlushBuffers();
      gfxSwapBuffersGpu();
   }

   // Exit services
   csndExit();
   gfxExit();
   //ptmExit();
   hidExit();
   aptExit();
   srvExit();
   return 0;
}
