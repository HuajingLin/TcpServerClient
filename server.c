//
// CIS4307
// HW2
// Student: Huajing Lin
// TU ID: 915660847
// Spring 2020
//
//  Created by huajing lin on 1/24/20.
//
//

#include "server.h"

/*
int search_file_in_dir(struct DIR* dir, char* filename, FILE** pFile)
{
    char bFoundfile = 0;
    struct dirent *de;
    // for readdir()
    while ((de = readdir(dir)) != NULL){
        if (de->d_type != DT_REG)
            continue;
        
        //printf("%s\n", de->d_name);
        if(strncmp(de->d_name, filename, FILE_NAME_LEN) == 0)
        {
            pFile =
            return 1;
        }
        else
            return 0;
    }
    return bFoundfile;
}*/
int readAndSendFile(FILE* pFile, int sockfd, unsigned char* buff)
{
    int nTotal = 0;
    int nRead = 0;
    int nSent = 0;
    int nTotalSent = 0;
    
    while(1)
    {
        memset(buff, 0, MAX);
        
        int nRead = fread(buff, 1, 1024, pFile);
        //printf("Bytes read %d \n", nRead);
        
        // If read was success, send data.
        if(nRead > 0)
        {
            nTotal += nRead;
            nSent = write(sockfd, buff, nRead);
            //printf("sent %d\n", nSent);
            nTotalSent += nSent;
        }
        if (feof(pFile)){
            printf("read: %d, sent:%d\n", nTotal, nTotalSent);
            break;
        }
    }
    return nTotal;
}

// return file size, if not hit return 0;
int search_file_in_cache(unsigned char* pCache, char* filename, unsigned char** pData)
{
    struct FILE_NODE* pFile = (struct FILE_NODE*) pCache;
    unsigned char bCacheHit = 0;
    while (pFile != 0) {
        if (strncmp(pFile->filename, filename, FILE_NAME_LEN) == 0) {
            bCacheHit = 1;
            *pData = pFile + sizeof(struct FILE_NODE);
            break;
        }
        pFile = pFile->next;
    }
    if(bCacheHit == 1)
        return pFile->filesize;
    else
        return 0;
}

// if success return >0, otherwise return 0
int addFileToCache(FILE* pFile, int filesize, unsigned char* pCache, int* pUsedCache, char* filename)
{
    struct FILE_NODE* pFileNode = (struct FILE_NODE*)pCache;
    struct FILE_NODE* pFileNodeStart = pFileNode;
    struct FILE_NODE* pSecondNode = NULL;
    unsigned char* pFileData = NULL;
    unsigned char* pNode = NULL;
    int sizeNode = sizeof(struct FILE_NODE);
    int sizeFiles = 0; //total size of deleted files.
    
    printf("add file to cache, file size: %dK\n", filesize/1024);
    //if remain space less than file size
    if (filesize > CACHE_MAX - *pUsedCache )
    {
        while(pFileNode != NULL)    //delete file nodes
        {
            sizeFiles += pFileNode->filesize + sizeNode;
            pSecondNode = pFileNode->next;
            pFileNode->next = NULL;
            pFileNode->filesize = 0;
            printf("=== delete file.(%dK)\n", sizeFiles/1024);
            if (sizeFiles >= filesize + sizeNode) {
                break;
            }
            pFileNode = pSecondNode;
        }
        *pUsedCache -= sizeFiles;
        
        //move data
        int n = 0;
        //calculate the size of moving
        int nMoveSize = 0;
        pFileNode = pSecondNode;
        while(pFileNode != NULL){
            nMoveSize += pFileNode->filesize + sizeNode;
            pFileNode = pFileNode->next;
        }
        printf("=== move data %d.\n", nMoveSize);

        //move data
        unsigned char* pSour = (unsigned char*)pSecondNode;
        unsigned char* pDest = pFileNodeStart;//pSour - nMoveSize;
        while (n < nMoveSize) {
            n += 1;
            *pDest = *pSour;
            pDest += 1;
            pSour += 1;
        }
        
        //adjust pointor
        pFileNode = pFileNodeStart;
        while(pFileNode != NULL){
            if(pFileNode->next > 0)
            {
                pNode = (unsigned char*)pFileNode->next;
                pNode -= sizeFiles;
                pFileNode->next = pNode;
                printf("=== adjust file pointor.\n");
            }
            pFileNode = pFileNode->next;
        }
        
    }
    
    //skip to end of file list
    pFileNode = pFileNodeStart;
    while(pFileNode != NULL && pFileNode->filesize > 0)
    {
        printf("skip file\n");
        if(pFileNode->next == NULL){
            pNode = (unsigned char*)pFileNode;
            pFileNode->next = pNode + pFileNode->filesize + sizeNode;
            pFileNode = pFileNode->next;
            break;
        }
        pFileNode = pFileNode->next;
    }
    
    //read file and write to cache
    strncpy(pFileNode->filename, filename, FILE_NAME_LEN);
    pFileNode->filesize = filesize;
    pFileNode->next = NULL;
    pFileData = pFileNode + sizeNode;
    
    fseek(pFile, 0, SEEK_SET);
    if(filesize != fread(pFileData, 1, filesize, pFile))
    {
        printf("addFileToCache: read file error.(%d)\n", filesize);
        pFileNode->filesize = 0;
        return 0;
    }
    else{
        *pUsedCache += filesize + sizeNode;
        return filesize;
    }
}

//list files in cache
void listFileOfCache(unsigned char* pCache)
{
    struct FILE_NODE* pFileNode = (struct FILE_NODE*)pCache;
    printf("\tFile List:\n");
    while(pFileNode != NULL){
        printf("\t\t%s\n", pFileNode->filename);
        pFileNode = pFileNode->next;
    }
}

// connection handler
void connection_handler(void *ptr)
{
    char buff[MAX];
    char strIP[INET_ADDRSTRLEN];
    char pathname[PATH_NAME_LANE];
    struct PARAM* param = (struct PARAM*)ptr;
    
    unsigned char* pFileData = 0;
    FILE* pFile = 0;
    int filesize = 0;
    int nSent = 0;
    
    // read the message from client and copy it in buffer
    bzero(buff, MAX);
    read(param->sockfd, buff, MAX);
    
    // print message from client
    inet_ntop(AF_INET, &(param->client->sin_addr), strIP, INET_ADDRSTRLEN);
    printf("Client %s is requesting %s\n", strIP, buff);
    
    // if client's file name is "exit", server exits.
    if (strncmp(buff,"exit",4) == 0) {
        close(param->sockfd);
        param->sockfd = -1;
        return;
    }
    else if (strncmp(buff,"ls",2) == 0) {
        listFileOfCache(param->pCache);
        write(param->sockfd, "ok", 2);
        close(param->sockfd);
        return;
    }
    
    //search file_name in cache
    filesize = search_file_in_cache(param->pCache, buff, &pFileData);
    if(filesize > 0)  // the file is in the cache
    {
        nSent = write(param->sockfd, pFileData, filesize);
        printf("Cache hit. File %s sent to the client.(%d/%d)\n", buff, nSent, filesize);
    }
    else    // the file is in the cache
    {
        printf("Cache miss. File %s sent to the client\n", buff);
        //search file_name in the destination directory
        bzero(pathname, PATH_NAME_LANE);
        sprintf( pathname, "%s/%s", param->dir, buff );
        char fileName[FILE_NAME_LEN];
        bzero(fileName,FILE_NAME_LEN);
        strcpy(fileName, buff);
        pFile = fopen(pathname, "rb");
        if (pFile) {
            printf("file %s found.\n", buff);
            filesize = readAndSendFile(pFile, param->sockfd, buff);
            if(filesize == 0)
                printf("read file err.\n");
            else
            {
                if(filesize > CACHE_MAX - sizeof(struct FILE_NODE))
                    printf("Warn: File too big to cache.\n");
                else
                {
                    filesize = addFileToCache(pFile, filesize, param->pCache, param->pUsedCache,fileName);
                    if(filesize == 0)
                        printf("Fail to add file to cache.\n");
                    else
                        printf("Added file %s to cache.(used: %dk)\n",fileName,*param->pUsedCache/1024);
                }
            }
        }
        else
        {
            printf("File %s does not exist.\n",buff);
            bzero(buff, MAX);
            strncpy(buff, "not exist", 9);  // send result to client
            write(param->sockfd, buff, 9);
        }
        fclose(pFile);
    }
    close(param->sockfd);
}

// Driver function
int main(int argc, char *argv[])
{
    int sockfd, connfd, len;
    pthread_t thread_id;
    struct sockaddr_in servaddr, cli;
    struct DIR *dir;
    struct PARAM param;
    unsigned char* pCache = 0;
    int nUsedCache = 0;
    
    if(argc != 3){
        printf("usage: ~ port dir\n");
        exit(0);
    }
    printf("argument\n\t port:%s\n\t dir:%s\n",argv[1],argv[2]);
    int port = atoi(argv[1]);
    char* dirName = argv[2];
    
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
    
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    
    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");
    
    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    
    //try to open the Specified directory
    dir = opendir(dirName);
    if (dir == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open directory: %s\n", dirName);
        exit(0);
    }
    
    //create the cache for files
    pCache = (unsigned char*)malloc(CACHE_MAX);
    bzero(pCache, CACHE_MAX);
    
    len = sizeof(cli);
    
    while( 1 )
    {
        printf("Server is listening..\n");
        connfd = accept(sockfd, (SA*)&cli, &len);
        if (connfd < 0) {
            printf("server acccept failed...\n");
            break;
        }
        else
            printf("server acccept the client...\n");
        
        bzero(&param, sizeof(struct PARAM));
        param.sockfd = connfd;
        param.client = &cli;
        param.dir = dirName;
        param.pCache = pCache;
        param.pUsedCache = &nUsedCache;
        
        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &param) < 0)
        {
            perror("could not create thread");
            break;
        }
        
        //Now join the thread , so that we dont terminate before the thread
        pthread_join( thread_id , NULL);
        if (param.sockfd == -1) {
            break;
        }
    }
    
    // After chatting close the socket 
    close(sockfd);
    free(pCache);
    closedir(dir);
    printf("server exit\n");
} 
