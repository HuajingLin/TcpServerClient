//
//  server.h
//  
//
//  Created by huajing lin on 1/24/20.
//
//

#ifndef server_h
#define server_h

#include <stdio.h>
#include <dirent.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#define MAX 1024
#define FILE_NAME_LEN 32
#define PATH_NAME_LANE 128
#define CACHE_MAX 67108864  //64M
#define SA struct sockaddr

struct FILE_NODE
{
    char filename[FILE_NAME_LEN];
    int filesize;
    //unsigned char* position;     //position of cache
    struct FILE_NODE* next;
};

struct PARAM
{
    int sockfd;
    char* dir;
    unsigned char* pCache;
    int* pUsedCache;
    struct sockaddr_in* client;
};

#endif /* server_h */
