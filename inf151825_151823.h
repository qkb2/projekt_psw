#ifndef INF_H
#define INF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_READ_MSGS 10
#define SHORT_MSG_LEN 16
#define LONG_MSG_LEN 300
#define MSG_SIZE SHORT_MSG_LEN+LONG_MSG_LEN

struct msgbuf {
    long mtype;
    char shortMsg[SHORT_MSG_LEN];
    char longMsg[LONG_MSG_LEN];
};

typedef struct msgbuf MSGBUF;


#endif