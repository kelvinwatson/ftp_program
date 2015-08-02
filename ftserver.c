/* PROGRAMMED BY: Kelvin Watson
 * OSU ID: 932540242
 * ONID: watsokel
 * FILENAME: ftserver.c
 * DESCRIPTION: CS372 Assignment 2: Server for 2-connection client-server network
 * application that performs simple file transfer. See README.txt for instructions on 
 * compilation and execution.
 * CODE SOURCES: http://stackoverflow.com/questions/12489/how-do-you-get-a-directory-listing-in-c
 * https://www.cs.bu.edu/teaching/c/file-io/intro/
 * http://www.cplusplus.com/reference/cstdio/fread/
 * http://www.programminglogic.com/example-of-client-server-program-in-c-using-sockets-and-tcp/
 * http://stackoverflow.com/questions/3060950/how-to-get-ip-address-from-sock-structure-in-c
 * http://stackoverflow.com/questions/12722904/how-to-use-struct-timeval-to-get-the-execution-time
 * http://pubs.opengroup.org/onlinepubs/007908775/xsh/dirent.h.html
 * http://stackoverflow.com/questions/5297248/how-to-compare-last-n-characters-of-a-string-to-another-string-in-c
 * http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
 * http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
 * http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 * http://cboard.cprogramming.com/c-programming/98138-fflush-stdout.html
 */
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

#define BACKLOG 10
#define HANDLE_SIZE 11
#define MESSAGE_SIZE 500
#define CLEARBUFFER() char ch; while ((ch = getchar()) != '\n' && ch != EOF);

void *addr;
struct sockaddr_in *ipv4;

void signalHandler(int);
int sendAll(int,char*,int*);
char* receiveMessage(char*,int);
void sendMessage(char*,char*,int,size_t);
char* startUp(int,char**);
void *get_in_addr(struct sockaddr *sa);
int establishDataConnection(struct sockaddr_in);
char* getFileData(const char*,int*);
void sendData(const int,char*);


int main(int argc, char* argv[]){
    char *portNumber = startUp(argc,argv);

    int status, welcomeSocket, connectionSocket, yes=1;  // listen on sock_fd, new connection on connectionSocket
    struct addrinfo hints, *serverInfo, *p;
    struct sockaddr_in clientAddr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((status = getaddrinfo(NULL, portNumber, &hints, &serverInfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    for(p = serverInfo; p != NULL; p = p->ai_next) {
        if ((welcomeSocket = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (bind(welcomeSocket, p->ai_addr, p->ai_addrlen) == -1) {
            close(welcomeSocket);
            perror("server: bind");
            continue;
        }
        break;
    }
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }
    freeaddrinfo(serverInfo);

    int waitForConnection=0;
    while(1){
        if (listen(welcomeSocket, BACKLOG) == -1) {
            perror("listen");
            exit(1);
        }
        sa.sa_handler = signalHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGCHLD, &sa, NULL) == -1) {
            perror("sigaction");
            exit(1);
        }

        printf("\nServer: Waiting for client connections on port %s...\n",portNumber);
        fflush(stdout);
        
        char response[MESSAGE_SIZE], input[MESSAGE_SIZE];
        memset(response,0,MESSAGE_SIZE);
        int charsRecv,sendStatus;
        
        while(1) {  //Accept Loop
            sin_size = sizeof clientAddr;
            //printf("IN THE ACCEPT LOOP");fflush(stdout);
            connectionSocket = accept(welcomeSocket, (struct sockaddr *)&clientAddr, &sin_size);        /*open a connection socket*/
            fcntl(connectionSocket, F_SETFL, O_NONBLOCK);
            if(connectionSocket == -1) {
                perror("accept");
                continue;
            }
            while(1){ 
                strcpy(response,receiveMessage(response,connectionSocket));      
                //printf("response truncated?=%s",response);fflush(stdout);

                if(!strncmp(response,"-l",2)){
                    printf("\n------------------------------------------------------");
                    fflush(stdout);
                    printf("\n-l (list) command received from client");
                    fflush(stdout);
                    printf("\nSending names of files in current directory to client.");
                    fflush(stdout);
                    printf("\n------------------------------------------------------");
                    fflush(stdout);
                    //printf("Response was %s",response); fflush(stdout);
                    waitForConnection = 1;
                    DIR *directory;
                    struct dirent *entry;
                    int tempLen=0, existingLen=0;
                    char* existingFileNames = NULL;
                    if ((directory = opendir (".")) != NULL) {
                        while ((entry = readdir (directory)) != NULL) {
                            //printf ("%s\n", entry->d_name);
                            tempLen = strlen(entry->d_name)+2;
                            //printf("tempLen = %d\n",tempLen);               //additional char for comma delimiter
                            char *tempFileName = (char*)malloc(tempLen);
                            strcpy(tempFileName,entry->d_name);
                            strcat(tempFileName,",");
                            //printf("tempFileName = %s\n",tempFileName);     //replace null terminator with comma
                            if(!existingFileNames){                         //first round
                                existingFileNames = (char*)malloc(tempLen+1);
                                assert(existingFileNames);
                                strcpy(existingFileNames,tempFileName);
                            } else{                                         //subsequent rounds
                                //printf("existingFileNames = %s\n",existingFileNames);
                                int existingLen = strlen(existingFileNames);
                                char* tempExistingFileNames = (char*)malloc(existingLen+2);
                                strcpy(tempExistingFileNames,existingFileNames);
                                free(existingFileNames);
                                existingFileNames = (char*)malloc(tempLen+existingLen+2); //reallocate
                                strcpy(existingFileNames,tempExistingFileNames);
                                free(tempExistingFileNames);
                                strcat(existingFileNames,tempFileName);
                            } 
                            free(tempFileName);   
                        }
                        closedir (directory);
                    } else {
                      perror (""); /*Unable to open directory for reading*/
                      return EXIT_FAILURE;
                    }
                    int finalExistingFileNamesLen = ((int)strlen(existingFileNames)) +1;
                    char* finalExistingFileNames = (char*)malloc(finalExistingFileNamesLen+2);
                    strcpy(finalExistingFileNames,existingFileNames);
                    strcat(finalExistingFileNames,"\n");
    
                    if (sendAll(connectionSocket, finalExistingFileNames, &finalExistingFileNamesLen) == -1) {
                        perror("sendall");
                        printf("We only sent %d bytes because of the error!\n", finalExistingFileNamesLen);
                    }
                    free(existingFileNames);
                    free(finalExistingFileNames);
                    break;
                }

                if(!strncmp(response,"-g",2)){
                    waitForConnection = 1;
                    int responseLen = strlen(response);
                    //printf("responseLen=%d\n",responseLen);fflush(stdout);
                    char* dataPortString = &response[responseLen-6];
                    //printf("dataPortString=%s",dataPortString);fflush(stdout);
                    unsigned short int dataPortNumber = atoi(dataPortString);
                    //printf("dataPortNumber=%d",dataPortNumber);fflush(stdout);
                    response[responseLen-6] = '\0';
                    char *requestedFileName = response;
                    requestedFileName += 3;
                    //printf("  requestedFileName=%s", requestedFileName);
                    int requestedFileNameLen = (int)strlen(requestedFileName);
                    //printf("strlen of requestedFileName=%d\n",requestedFileNameLen);
                    requestedFileName[requestedFileNameLen-1] = '\0'; //replace the space with null
                    requestedFileNameLen = (int)strlen(requestedFileName);
                    //printf("new strlen of requestedFileName=%d\n",requestedFileNameLen); //success
                    
                    /*Establish data connection with client*/
                    /*printf("%d.%d.%d.%d\n", (int)(clientAddr.sin_addr.s_addr&0xFF),
                      (int)((clientAddr.sin_addr.s_addr&0xFF00)>>8),
                      (int)((clientAddr.sin_addr.s_addr&0xFF0000)>>16),
                      (int)((clientAddr.sin_addr.s_addr&0xFF000000)>>24));*/

                    int dataSocket = establishDataConnection(clientAddr);
                    
                    DIR *directory;
                    struct dirent *entry;
                    int fileByteSize;
                    char* fileData = NULL;
                    if ((directory = opendir (".")) != NULL) {
                        while ((entry = readdir (directory)) != NULL) {
                            if(strcmp(entry->d_name,requestedFileName)==0){//file found!
                                printf("\n------------------------------------------");
                                fflush(stdout);
                                printf("\n-g (get) command received from client.");
                                fflush(stdout);
                                printf("\nFILE FOUND! Transferring file to client.");
                                fflush(stdout);
                                printf("\n------------------------------------------");
                                fflush(stdout);
                                fileData = getFileData(requestedFileName,&fileByteSize);
                                break;
                            } else continue;    
                        }
                        closedir(directory);
                        if(!fileData){
                            printf("\n--------------------------------------");
                            fflush(stdout);
                            printf("\n-g (get) command received from client.");
                            fflush(stdout);
                            printf("\nFILE NOT FOUND!");
                            fflush(stdout);
                            printf("\n--------------------------------------");
                            fflush(stdout); 
                            char fileNotFound[] = "ERROR: FILE NOT FOUND. Exiting.";
                            sendData(dataSocket,fileNotFound);
                            close(dataSocket);
                            break;
                        }                      
                    } else {
                      perror ("");
                      return EXIT_FAILURE;
                    }
                    strcat(fileData, "\x1A");                        
                    if (sendAll(dataSocket, fileData, &fileByteSize) == -1) {
                        perror("sendall");
                        printf("We only sent %d bytes because of the error!\n", requestedFileNameLen);
                    }
                    close(dataSocket);
                    break;
                }
                else{
                    printf("invalid command received"); fflush(stdout);
                }
            }
            if(waitForConnection==1){
                close(connectionSocket);
                //printf("\nReady and waiting for new client connections on port %s...\n",portNumber);                  
                break;
            }
            else{
                printf("closing connection");
                close(connectionSocket);
            }
        } continue;
    }
    return 0;
}

/* Function: signalHandler
 * Description: reaps dead processes 
 * Source: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 * Parameters: none
 * Preconditions: none
 * Postconditions: no return value
 */
void signalHandler(int s){
  while(waitpid(-1, NULL, WNOHANG) > 0);
}

/* Function: sendAll
 * Description: sends messages to client 
 * Source: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#sendall
 * Parameters: string to send, buffer and string length
 * Preconditions: buffer must be allocated
 * Postconditions: returns -1 on failure, and 0 on success
 */
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

/* Function: receiveMessage
 * Description: receives messages from client and prevents blocking by
 * implementing a time delay, so the function loops until all parts of
 * the response is received 
 * Source: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 * Parameters: string to store response, connectionSocket, and start time
 * Preconditions: TCP connection must be established between client and server
 * Postconditions: returns the response string to main
 */
char* receiveMessage(char* resp,int cSocket){
    struct timeval currTime, startTime;
    //printf("in receive\n");fflush(stdout);
    int transferStarted = 0, charsRecv;
    char chunk[MESSAGE_SIZE];
    memset(resp,0,MESSAGE_SIZE);
    do{
        memset(chunk,0,MESSAGE_SIZE);
        charsRecv=recv(cSocket,chunk,MESSAGE_SIZE,0);
        if(charsRecv==0){
        }
        else if(charsRecv > 0){ //receive success 
            //printf("charsRecv>0"); fflush(stdout);
            if(transferStarted == 0) {
                gettimeofday(&startTime, NULL);
                //printf("startTime=%f",startTime); fflush(stdout);    
                transferStarted = 1;
            }
            strcat(resp,chunk);
        }
        else if(charsRecv < 0 && transferStarted == 1) {
            gettimeofday(&currTime, NULL);
            //printf("took %lu\n", currTime.tv_usec - startTime.tv_usec);fflush(stdout);    
            if((currTime.tv_usec - startTime.tv_usec) > 1000) {
                transferStarted = 0;
                break;    
            }
        }
    }while(1);
    resp[charsRecv-1]= '\0';
    return resp;
}

/* Function: sendMessage
 * Description: callse sendAll to complete the message transfer
 * Source: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 * Parameters: message to be sent, handle, connectionSocket, length of handle
 * Preconditions: TCP connection must be established between client and server
 * Postconditions: frees the dynamically allocated handle+message
 */
void sendMessage(char* input,char* handle,int cSocket,size_t hLen){ 
    int inputLen = (int)strlen(input);
    int messageLen = (int)(hLen+inputLen);
    char* message = (char*)malloc(messageLen);
    strcpy(message,handle);
    strcat(message,input);
    int finalLen = strlen(message);
    if (sendAll(cSocket, message, &finalLen) == -1) {
        perror("sendall");
        printf("We only sent %d bytes because of the error!\n", finalLen);
    } 
    free(message);
}

/* Function: startUp
 * Description: accepts the port number from the commandline
 * Parameters: command line argument count and argument values
 * Preconditions: program must be executed with commandline arguments
 * Postconditions: returns the port number
 */
char* startUp(int argC, char** argV){
    if(argC<2){
        fprintf(stderr,"\n***ERROR: Missing port number in command-line argument: e.g. ./TCPserver port\n\n");
        exit(0);
    }
    else{ 
    printf("\n\nFILE TRANSFER SYSTEM (SERVER)");
    printf("\nProgrammed by Kelvin Watson, OSU ID 932540242, onid: watsokel)");
    printf("\n**************************************************************");   
    printf("\nWelcome to the the Simple File Transfer program. This is the server!\n");
    fflush(stdout);
    return argV[1];
    }
}



void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int establishDataConnection(struct sockaddr_in clientAddr){
    int IP1 = (int)(clientAddr.sin_addr.s_addr&0xFF);
    int IP2 = (int)((clientAddr.sin_addr.s_addr&0xFF00)>>8);
    int IP3 = (int)((clientAddr.sin_addr.s_addr&0xFF0000)>>16);
    int IP4 = (int)((clientAddr.sin_addr.s_addr&0xFF000000)>>24);                        
    char connectToIP[15];
    int numCharsWritten = sprintf(connectToIP,"%d.%d.%d.%d",IP1,IP2,IP3,IP4);
    //printf("numCharsWritten=%d, IPAddress=%s",numCharsWritten,connectToIP);

    struct addrinfo hints2, *res2;
    int dataSocket,connectStatus;
    // first, load up address structs with getaddrinfo():
    memset(&hints2, 0, sizeof hints2);
    hints2.ai_family = AF_UNSPEC;
    hints2.ai_socktype = SOCK_STREAM;
    getaddrinfo(connectToIP, "30026", &hints2, &res2);
    //printf("TESTING");
    if((dataSocket = socket(res2->ai_family, res2->ai_socktype, res2->ai_protocol))==-1){
        perror("client: socket");
    } 
    //printf("dataSocket=%d\n",dataSocket);
    if((connectStatus = connect(dataSocket, res2->ai_addr, res2->ai_addrlen)) == -1){
        close(dataSocket);
        printf("%d",errno);
        perror("client: connect");
    }
    //printf("ConnectStatus=%d\n",connectStatus);
    return dataSocket;
}

char* getFileData(const char* requestedFileName,int* fileByteSize){
    FILE* ifp;
    size_t readStatus; 
    char ch;
    ifp = fopen(requestedFileName,"r"); //open file for reading
    if(!ifp){
        fprintf(stderr, "Unable to open file for reading. Aborting.\n");
        exit(1);
    }
    fseek (ifp,0,SEEK_END);
    (*fileByteSize) = ftell(ifp);
    //printf("fileByteSize=%d",(*fileByteSize));
    rewind(ifp);/*determine file size */
    char* fileData = (char*)malloc(sizeof(char)* (*fileByteSize) );
    if (!fileData) {
        fprintf(stderr, "Unable to allocate memory for file data. Aborting\n"); 
        exit(2);
    }
    readStatus = fread(fileData,1, (*fileByteSize),ifp);
    if (readStatus != (*fileByteSize) ) {
        fputs ("Reading error",stderr); 
        exit(3);
    }
    fclose(ifp);
    return fileData;
}

void sendData(const int dataSocket,char* message){
    int messageLen = (int)strlen(message); 
    //printf("message=%s,messageLen=%d\n",message,messageLen);
    fflush(stdout);
    if (sendAll(dataSocket, message, &messageLen) == -1) {
        perror("sendall");
        printf("We only sent %d bytes because of the error!\n", messageLen);
    } 
}