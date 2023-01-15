#ifndef INF_H
#define INF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_READ_MSGS 10
#define SHORT_MSG_LEN 16
#define LONG_MSG_LEN 300

#define MSG_SIZE SHORT_MSG_LEN+LONG_MSG_LEN+1
#define LMSG_SIZE SHORT_MSG_LEN*2 + sizeof(int)
#define LCMSG_SIZE sizeof(int)+1

#define MAX_USERS 15
#define MAX_BAD_PIDS 100
#define MAX_GROUPS 5

struct msgbuf {
    long mtype;
    char msgCode;
    char shortMsg[SHORT_MSG_LEN];
    char longMsg[LONG_MSG_LEN];
};

struct loginbuf {
    long mtype;
    int pid;
    char nick[SHORT_MSG_LEN];
    char pswd[SHORT_MSG_LEN];
};

struct loginconfbuf {
    long mtype;
    char msgCode;
    int ipcID;
};

typedef struct msgbuf MSGBUF;
typedef struct loginbuf LBUF;
typedef struct loginconfbuf LCBUF;

#endif