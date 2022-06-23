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

extern int local_sdk_video_get_jpeg_data(int ch, unsigned char *buf, int size);
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
    unsigned char *jpegBuf = malloc(0x50000);
    if(!jpegBuf) {
      fprintf(stderr, "[timelapse] error : malloc\n");
      Busy = 0;
    }
    while(Busy) {
      fprintf(stderr, "TimelapseThread\n");
      FILE *fp = fopen(File, "a");
      if(!fp) {
        fprintf(stderr, "[timelapse] error : fopen %s : %s\n", File, strerror(errno));
        break;
      }
      int ret = local_sdk_video_get_jpeg_data(0, jpegBuf, 0x50000);
      if(ret < 0) {
        fprintf(stderr, "[timelapse] error : get_jpeg\n");
      } else {
        fwrite(jpegBuf, ret, 1, fp);
      }
      fclose(fp);
      Count++;
      if(Count >= NumOfTimes) Busy = 0;
      fprintf(stderr, "Count %d Num %d %d\n", Count, NumOfTimes, Busy);
      if(Busy) sleep(Interval);
    }
    Busy = 0;
    if(jpegBuf) free(jpegBuf);
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
