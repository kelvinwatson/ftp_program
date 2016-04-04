#FTP PROGRAM (C Server, Java Client)

####CS 372 PROJECT #2 (PROGRAMMING ASSIGNMENT #2)
#####Programmed by KELVIN WATSON; OSU ID 932540242; ONID: watsokel

CONTENTS
---------------------
 * Required Files
 * How to Compile
 * How to Execute/Run the Program
 * Code Sources, Citations, and References


REQUIRED FILES
--------------
Before proceeding, please ensure that you have the 4 required files:
  ftserver.c
  makefile
  ftclient.java 
  FTPClient.java


HOW TO COMPILE
--------------
1. Open two separate terminals e.g. PuTTY, one for the server, and one for the client.
2. On the server terminal, enter:   `make` (alternatively, you can also enter: `makeall`)
3. On the client terminal, enter:   `javac ftclient.java FTPClient.java`


HOW TO RUN THE PROGRAM
----------------------
1. On the server terminal, enter  `ifconfig` to obtain the OSU flip IP address that ftserver will be running on. 
  * `flip1 = 128.193.54.226`
  * `flip2 = 128.193.54.7`
  * `flip3 = 128.193.54.10`
  
    (As an example execution, for the following steps, we will assume that ftserver is running on flip2, and will be using port number 30023, but you will enter the correct port number of your choosing)

2. On the server terminal, execute the ftserver program by entering:  `./ftserver <SERVER_PORT>` 
(where `<SERVER_PORT>` is the desired port number for the FTP server program). For example, if ftserver will be using port 30023, you would enter: `./ftserver 30020`

3. On the client terminal, there are two possible commands to execute the ftclient program:  
    i)    To list files in ftserve's directory, enter the command in the following format:
          `java ftclient <FLIP_SERVER> <SERVER_PORT> -l <DATA_PORT>`
          Using the example scenario above:  `java ftclient flip2 30023 -l 30026`
          because ftserver is running on flip2 and port 30023. The 30026 is the data port which we will be used to receive the file names on ftclient. Alternatively,

    ii)   To get (retrieve) a file from ftserve, enter the command in the following format:
          `java ftclient <FLIP_SERVER> <SERVER_PORT> -g <DATA_PORT>`
          Using the example scenario above: `java ftclient flip2 30023 -g hello.txt 30026`
          because ftserver is running on flip2 and port 30023. hello.txt is ftclient's desired file, and 30026 is the data port which we will be used to receive the file on ftclient.
 
4. Repeat step 3 on ftclient to continue getting lists or retrieving files from ftserver.

5. To terminate ftserver, on the server terminal, enter Ctrl-C to perform a SIGINT.


CODE SOURCES, CITATIONS AND REFERENCES 
--------------------------------------
http://stackoverflow.com/questions/12489/how-do-you-get-a-directory-listing-in-c
https://www.cs.bu.edu/teaching/c/file-io/intro/
http://www.cplusplus.com/reference/cstdio/fread/
http://www.programminglogic.com/example-of-client-server-program-in-c-using-sockets-and-tcp/
http://stackoverflow.com/questions/3060950/how-to-get-ip-address-from-sock-structure-in-c
http://stackoverflow.com/questions/12722904/how-to-use-struct-timeval-to-get-the-execution-time
http://pubs.opengroup.org/onlinepubs/007908775/xsh/dirent.h.html
http://stackoverflow.com/questions/5297248/how-to-compare-last-n-characters-of-a-string-to-another-string-in-c
http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
http://cboard.cprogramming.com/c-programming/98138-fflush-stdout.html
http://stackoverflow.com/questions/10202515/reading-from-a-bufferedreader-readline-returns-null
https://systembash.com/a-simple-java-tcp-server-and-tcp-client/
http://docs.oracle.com/javase/tutorial/networking/sockets/readingWriting.htm
https://systembash.com/a-simple-java-tcp-server-and-tcp-client/
http://docs.oracle.com/javase/tutorial/networking/sockets/readingWriting.html
http://stackoverflow.com/questions/7771303/getting-the-character-returned-by-read-in-bufferedreader
