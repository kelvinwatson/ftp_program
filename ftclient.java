/* Programmed by: Kelvin Watson
 * OSU ID: 932540242
 * ONID: watsokel
 * FileName: chatclient.java
 * Description: Chat client for 2-way communication with a TCP chat server
 * Sources: https://systembash.com/a-simple-java-tcp-server-and-tcp-client/
 * http://docs.oracle.com/javase/tutorial/networking/sockets/readingWriting.html
 */

import java.io.*; 
import java.net.*;
import java.util.Scanner; 

public class ftclient{
  public static void main(String argv[]) throws Exception  {   
    FTPClient client = new FTPClient();
    client.hostName = argv[0];
    client.portNumber = Integer.parseInt(argv[1]);
    try{
      client.initiateContact();          
      client.printTitle();
      
      do{
        String sendAction = client.sendCommand(); //returns either -l, -g, or ERR so client knows what to expect
        String receiveAction = client.receiveMessage(); //server should send an acknowledgement or ERROR
        System.out.println(receiveAction);
        if(receiveAction=="ERROR: Invalid Command"){
          System.out.println("I'm in the error block");
        }
        else{
          System.out.println("I'm in the else block");
        //receive the file or list from the user, server should signal if it's sending a list or file
          String receiveAck = client.receiveMessage();
          System.out.println(receiveAck); 
          //return; 
        }
      }while(true);
    } finally {
      System.out.print("\nThank you for using File Transfer.\n\n");
      System.exit(0);
    }
  } 
}
