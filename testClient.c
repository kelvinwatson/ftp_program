#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

int sendAll(int s, char *buf, int *len){
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;
    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
    *len = total;
    return n==-1?-1:0; 
}

int main(){

    struct addrinfo hints2, *res2;
    int dataSocket,connectStatus;
    // first, load up address structs with getaddrinfo():
    memset(&hints2, 0, sizeof hints2);
    hints2.ai_family = AF_UNSPEC;
    hints2.ai_socktype = SOCK_STREAM;
    getaddrinfo("128.193.54.226", "30026", &hints2, &res2);
    // make a socket:
    if( (dataSocket = socket(res2->ai_family, res2->ai_socktype, res2->ai_protocol))==-1){
        perror("client: socket");
    } 
    printf("dataSocket=%d\n",dataSocket);
    // connect!
    if( (connectStatus = connect(dataSocket, res2->ai_addr, res2->ai_addrlen)) == -1){
        close(dataSocket);
        printf("%d",errno);
        perror("client: connect");
    }
    printf("connectStatus=%d\n",connectStatus);
    
    char* requestedFileName = "hello.txt";
    int requestedFileNameLen = strlen(requestedFileName);
    if (sendAll(dataSocket, requestedFileName, &requestedFileNameLen) == -1) {
        perror("sendall");
        printf("We only sent %d bytes because of the error!\n", requestedFileNameLen);
    }    

    return 0;
}