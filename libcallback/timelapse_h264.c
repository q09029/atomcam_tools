#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

struct FrameCtrlSt {
  unsigned char *buf;
  int size;
  unsigned int reserve2;
  unsigned int reserve3;
  unsigned int reserve4;
  unsigned int stat;
  unsigned int reserve6;
  unsigned int reserve7;
  unsigned int reserve8;
  unsigned int reserve9;
};

extern int video_get_frame(int ch, int lockId, int mode, unsigned char *buf, struct FrameCtrlSt *frameCtrl);
extern void CommandResponse(int fd, const char *res);

static int Busy = 0;
static int NumOfTimes = 0;
static int Interval = 0;
static int Count = 0;
static char File[256];
static char ResBuf[256];
static pthread_mutex_t TimelapseMutex = PTHREAD_MUTEX_INITIALIZER;

// timelapse <file> <interval> <count>
// timelapse stop
char *Timelapse(int fd, char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(p && !strcmp(p, "stop")) {
    if(!Busy) return "error";
    Busy = 0;
    return "ok";
  }

  if(!p) {
    if(!Busy) return "not operating.";
    snprintf(ResBuf, 255, "file: %s\ninterval: %dsec, count: %d/%d\n", File, Interval, Count, NumOfTimes);
    return ResBuf;
  }
  if(Busy) return "error :　Already in operation.";

  strncpy(File, p, 255);
  FILE *fp = fopen(File, "w");
  if(!fp) return strerror(errno);
  fclose(fp);

  p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) return "error";
  Interval = atoi(p);
  if(Interval < 1) Interval = 1;

  p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) return "error";
  NumOfTimes = atoi(p);
  if(NumOfTimes < 1) NumOfTimes = 1;
  Count = 0;

  Busy = 1;
  pthread_mutex_unlock(&TimelapseMutex);
  return "ok";
}

static void *TimelapseThread() {

  while(1) {
    pthread_mutex_lock(&TimelapseMutex);
    struct FrameCtrlSt frameCtrl;
    memset(&frameCtrl, 0, sizeof(frameCtrl));
    frameCtrl.buf = malloc(0x50000);
    if(!frameCtrl.buf) {
      fprintf(stderr, "[timelapse] video_get_frame : malloc error\n");
      Busy = 0;
    }
    int initial = 1;
    while(Busy) {
      fprintf(stderr, "TimelapseThread\n");
      int ret = video_get_frame(0, 0, 2, frameCtrl.buf, &frameCtrl);
      fprintf(stderr, "[timelapse] video_get_frame : %d %d\n", ret, frameCtrl.stat);
      if(!ret && !frameCtrl.stat) {
        FILE *fp = fopen(File, "a");
        if(!fp) {
          fprintf(stderr, "[timelapse] error : fopen %s : %s\n", File, strerror(errno));
        } else {
          /*
          if(initial) {
            const unsigned char sei0[] = {
              0x00, 0x00, 0x00, 0x01, 0x06, 0x05, 0xff, 0xff,
              0xaa, 0xdc, 0x45, 0xe9, 0xbd, 0xe6, 0xd9, 0x48,
              0xb7, 0x96, 0x2c, 0xd8, 0x20, 0xd9, 0x23, 0xee,
              0xef
            };
            const char *sei1 = "x264 - core 163 r3060 5db6aa6 - H.264/MPEG-4 AVC codec - Copyleft 2003-2021 - http://www.videolan.org/x264.html - options: cabac=1 ref=3 deblock=1:0:0 analyse=0x3:0x113 me=hex subme=7 psy=1 psy_rd=1.00:0.00 mixed_ref=1 me_range=16 chroma_me=1 trellis=1 8x8dct=1 cqm=0 deadzone=21,11 fast_pskip=1 chroma_qp_offset=-2 threads=6 lookahead_threads=1 sliced_threads=0 nr=0 decimate=1 interlaced=0 bluray_compat=0 constrained_intra=0 bframes=3 b_pyramid=2 b_adapt=1 b_bias=0 direct=1 weightb=1 open_gop=0 weightp=2 keyint=250 keyint_min=20 scenecut=40 intra_refresh=0 rc_lookahead=40 rc=crf mbtree=1 crf=23.0 qcomp=0.60 qpmin=0 qpmax=69 qpstep=4 ip_ratio=1.40 aq=1:1.00";
            const unsigned char sei2[] = { 0x80 };

            fwrite(sei0, sizeof(sei0), 1, fp);
            fwrite(sei1, strlen(sei1) + 1, 1, fp);
            fwrite(sei2, sizeof(sei2), 1, fp);
            initial = 0;
          }
          */
          fwrite(frameCtrl.buf, frameCtrl.size, 1, fp);
          fclose(fp);
          /*
          int zcount = 0;
          int nalFlag = 0;
          for(int i = 0; i < frameCtrl.size; i++) {
            if(nalFlag) {
              int nal = frameCtrl.buf[i];
              nalFlag = 0;
              fprintf(stderr, "NAL %02x\n", nal);
            }
            if((zcount >= 3) && (frameCtrl.buf[i] == 1)) {
              nalFlag = 1;
            }
            if(frameCtrl.buf[i]) {
              zcount = 0;
            } else {
              zcount++;
            }
          }
          */

          Count++;
          if(Count >= NumOfTimes) Busy = 0;
          fprintf(stderr, "Count %d Num %d %d\n", Count, NumOfTimes, Busy);
        }
      }
      if(Busy) sleep(Interval);
    }
    free(frameCtrl.buf);
  }
}

static void __attribute ((constructor)) JpegInit(void) {

  if(getppid() != 1) return;
  pthread_mutex_lock(&TimelapseMutex);
  pthread_t thread;
  if(pthread_create(&thread, NULL, TimelapseThread, NULL)) {
    fprintf(stderr, "pthread_create error\n");
    pthread_mutex_unlock(&TimelapseMutex);
    return;
  }
}
