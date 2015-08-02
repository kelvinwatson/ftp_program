/* PROGRAMMED BY: Kelvin Watson
 * OSU ID: 932540242
 * ONID: watsokel
 * FILENAME: chatclient.java
 * DESCRIPTION: FTP client main function for 2-connection client-server file transfer application
 * CODE SOURCES: http://stackoverflow.com/questions/10202515/reading-from-a-bufferedreader-readline-returns-null
 * https://systembash.com/a-simple-java-tcp-server-and-tcp-client/
 * http://docs.oracle.com/javase/tutorial/networking/sockets/readingWriting.html
 */

import java.io.*; 
import java.net.*;
import java.util.Scanner; 

public class ftclient{
  public static void main(String argv[]) throws Exception  {   
    FTPClient client = new FTPClient();

    if(argv[0].equals("flip1") || argv[0].equals("128.193.54.226")){     /*validate arg[0]*/
      argv[0] = "flip1.engr.oregonstate.edu";
    } else if(argv[0].equals("flip2") || argv[0].equals("128.193.54.7")){
      argv[0] = "flip2.engr.oregonstate.edu";
    } else if(argv[0].equals("flip3") || argv[0].equals("128.193.54.10")){
      argv[0] = "flip3.engr.oregonstate.edu";
    } 
    client.hostName = argv[0];
    
    client.portNumber = Integer.parseInt(argv[1]);  /*save arg[1]*/

    if(argv[2].equals("-l")){                       /*validate arg[2]*/
      client.controlCommand = argv[2];
      //System.out.println("CONTROL COMMAND = " + client.controlCommand);
      client.dataPortNumber = Integer.parseInt(argv[3]);
    } else if(argv[2].equals("-g")){
      client.controlCommand = argv[2];
      //System.out.println("CONTROL COMMAND = " + client.controlCommand);
      client.requestedFileName = argv[3];
      //System.out.println("REQUESTED FILENAME = " + client.requestedFileName);
      client.dataPortNumber = Integer.parseInt(argv[4]);  /*save arg[1]*/
      //System.out.println("DATA PORT = " + client.dataPortNumber);
    } else{
      System.out.println("ERROR: Invalid Control Command. Please try again.");
      System.exit(1);
    }
    
    try{
      client.initiateContact();          
      client.printTitle();
      if(client.controlCommand.equals("-l")){
        client.displayFileList();
      }
      else if(client.controlCommand.equals("-g")){
        client.getFile();
      }
      //System.out.println("BACK IN MAIN!"); 
      try { 
        client.clientSocket.close();
      }catch(IOException e){
        //System.err.println(e); 
      } 
    } finally {
      System.out.print("\nThank you for using File Transfer.\n\n");
      System.exit(0);
    }
  } 
}
