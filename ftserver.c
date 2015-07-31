/* Programmed by: Kelvin Watson
 * OSU ID: 932540242
 * ONID: watsokel
 * FileName: ftserver.c
 * Description: CS372 Assignment 2: Server for 2-connection client-server network
 * application that performs simple file transfer. See README.txt for instructions on 
 * compilation and execution.
 * Code Sources: http://stackoverflow.com/questions/12489/how-do-you-get-a-directory-listing-in-c
 * http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
 * http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 * http://cboard.cprogramming.com/c-programming/98138-fflush-stdout.html
 */

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

#define BACKLOG 10     // how many pending connections queue will hold
#define HANDLE_SIZE 11
#define MESSAGE_SIZE 500
#define CLEARBUFFER() char ch; while ((ch = getchar()) != '\n' && ch != EOF);

void signalHandler(int);
int sendAll(int,char*,int*);
char* receiveMessage(char*,int,size_t);
void sendMessage(char*,char*,int,size_t);
char* startUp(int,char**);

int main(int argc, char* argv[]){
    char *portNumber = startUp(argc,argv);

    int status, welcomeSocket, connectionSocket, yes=1;  // listen on sock_fd, new connection on connectionSocket
    struct addrinfo hints, *serverInfo, *p;
    struct sockaddr_storage clientAddr; // connector's address information
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

    char handle[] = "server> ";
    size_t handleLen = strlen(handle);
    printf("\nYour handle is %s",handle);
    fflush(stdout);

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
        sa.sa_handler = signalHandler; // reap all dead processes
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
        
        while(1) {  // main accept() loop
            sin_size = sizeof clientAddr;
            connectionSocket = accept(welcomeSocket, (struct sockaddr *)&clientAddr, &sin_size);        /*open a connection socket*/
            fcntl(connectionSocket, F_SETFL, O_NONBLOCK);
            if(connectionSocket == -1) {
                perror("accept");
                continue;
            }
            clock_t startTime;
            int child;
            if((child=fork())<0){ 
                perror("Fork Error");
                close(welcomeSocket);
                exit(-1);
            }
            if(child==0) {
                while(1){ 
                    strcpy(response,receiveMessage(response,connectionSocket,startTime));      

                    if(!strncmp(response,"-l",2)){
                        printf("-l received. DATA CONNECTION CLOSED.");
                        printf("Response was %s",response); fflush(stdout);
                        waitForConnection = 1;
                        //print the list of files and return to waiting for connections
                        fflush(stdout);
                        char *listAck = "-l acknowledged\n"; 
                        int listAckLen=strlen(listAck);
                        if (sendAll(connectionSocket, listAck, &listAckLen) == -1) {
                            perror("sendall");
                            printf("We only sent %d bytes because of the error!\n", listAckLen);
                        } else printf("-l acknowl send successfull"); fflush(stdout);
                        break;
                    }

                    if(!strncmp(response,"-g",2)){
                        printf("-g received. DATA CONNECTION CLOSED."); fflush(stdout);
                        printf("Response was %s",response); fflush(stdout);
                        waitForConnection=1;
                        //parse the filename out and check if it is in the directory
                        fflush(stdout);
                        char *gAck = "-g acknowledged\n"; 
                        int gAckLen=strlen(gAck);
                        if (sendAll(connectionSocket, gAck, &gAckLen) == -1) {
                            perror("sendall");
                            printf("We only sent %d bytes because of the error!\n", gAckLen);
                        } else {
                            printf("-g acknowl send successfull"); fflush(stdout);
                        }
                        break; 
                    }  

                    //printf("%s",response); //leaving this here as a test to ensure command received from client
                    //fflush(stdout);


                    else{
                        printf("invalid command! Sending ERROR message to client"); fflush(stdout);
                        char *errMsg = "ERROR: Invalid Command\n"; 
                        int errMsgLen=strlen(errMsg);
                        if (sendAll(connectionSocket, errMsg, &errMsgLen) == -1) {
                            perror("sendall");
                            printf("We only sent %d bytes because of the error!\n", errMsgLen);
                        } else printf("ERR acknowl send successfull"); fflush(stdout);
                        continue;
                    }
                        
                    /*char* fGetsStatus = fgets(input, MESSAGE_SIZE, stdin); //truncates string to the input length, PLACES NULL TERMINATOR FOR YOU
                    if((strncmp(input,"\\quit",5))==0){
                        printf("\n**TCP connection closed**\n");fflush(stdout);
                        waitForConnection = 1;
                        char *signalToClose = "terminate"; 
                        int terminateLen=strlen(signalToClose);
                        if (sendAll(connectionSocket, signalToClose, &terminateLen) == -1) {
                            perror("sendall");
                            printf("We only sent %d bytes because of the error!\n", terminateLen);
                        } 
                        break;
                    }*/
                    //sendMessage(input,handle,connectionSocket,handleLen);
                }
                if(waitForConnection==1){
                    printf("Ready and waiting for new client connections on port %s...\n",portNumber);
                    close(connectionSocket);
                    break;
                }
                else{
                    printf("closing connection");
                    close(connectionSocket);
                }
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
char* receiveMessage(char* resp,int cSocket,size_t startTime){
    int transferStarted = 0, charsRecv;
    char chunk[MESSAGE_SIZE];
    memset(resp,0,MESSAGE_SIZE);
    do{
        memset(chunk,0,MESSAGE_SIZE);
        charsRecv=recv(cSocket,chunk,MESSAGE_SIZE,0);
        if(charsRecv==0){
        }
        else if(charsRecv > 0){ //receive success 
            if(transferStarted == 0) {
                startTime = clock();
                transferStarted = 1;
            }
            strcat(resp,chunk);
        }
        else if(charsRecv < 0 && transferStarted == 1) {
            if((clock() - startTime) > 1000) {
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