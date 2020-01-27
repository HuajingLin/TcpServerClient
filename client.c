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

#include "client.h"
#include "server.h"
/*
int recvfile(int socket, char* filename, char* dir)
{
    // Create file where data will be stored
    FILE *fp;
    fp = fopen(filename, "ab");
    if(NULL == fp)
    {
        printf("Error opening file");
        return 1;
    }
    
    // Receive data in chunks of 256 bytes
    while((bytesReceived = read(sockfd, recvBuff, 256)) > 0)
    {
        printf("Bytes received %d\n",bytesReceived);
        // recvBuff[n] = 0;
        fwrite(recvBuff, 1,bytesReceived,fp);
        // printf("%s \n", recvBuff);
    }
    
    if(bytesReceived < 0)
    {
        printf("\n Read Error \n");
    }
}*/


void func(int sockfd, char* filename, char* dir)
{
    int nReceive;
    int nTotal = 0;
    int i50K = 0;
    char buff[MAX];
    FILE *fp =  NULL;
    
    bzero(buff, sizeof(buff));
    strcpy(buff, filename);
    write(sockfd, buff, sizeof(buff));
    while (1) {
    
        bzero(buff, MAX);
        nReceive = read(sockfd, buff, MAX);
        //printf("From Server : %s (%d)\n", buff, n);
        if(nReceive <= 0)
            break;
        if (nReceive < 10)
        {
            if ((strncmp(buff, "not exist", 9)) == 0)
            {
                printf("File %s does not exist in the server\n", filename);
                
            }
            else
                printf("From server: %s\n", buff);
            break;
        }
        else //receive file
        {
            nTotal += nReceive;
            if (fp == NULL) {
                char pathname[PATH_NAME_LANE];
                bzero(pathname, PATH_NAME_LANE);
                if(dir[0] == '.')
                    fp = fopen( filename , "w" );
                else
                {
                    sprintf( pathname, "%s/%s", dir, filename );
                    fp = fopen( pathname , "w" );
                }
            }
            
            fwrite(buff , 1 , nReceive , fp );
            i50K += 1;
            if(i50K == 50)
            {
                printf(".");
                i50K = 0;
            }
        }
    }
    if(nTotal > 0){
        printf("\n");
        printf("%s saved. (%d)\n",filename, nTotal);
    }
    if (fp)
        fclose(fp);
}

int main(int argc, char *argv[])
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
    
    if(argc != 5){
        printf("usage: ~ server port file_name directory\n");
        exit(0);
    }
    printf("argument\n\t server:%s\n\t port:%s\n",argv[1],argv[2]);
    printf("\t file:%s\n\t dir:%s\n",argv[3],argv[4]);
    char* server = argv[1];
    int port = atoi(argv[2]);
    char* filename = argv[3];
    char* dir = argv[4];

    // socket create and varification
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
    servaddr.sin_addr.s_addr = inet_addr(server);
    servaddr.sin_port = htons(port);
    
    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0); 
    } 
    else
        printf("connected to the server..\n"); 
    
    // function for chat 
    func(sockfd, filename, dir);
    
    // close the socket 
    close(sockfd);
    printf("client exit\n");
} 
