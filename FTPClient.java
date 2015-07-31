/* Programmed by: Kelvin Watson
 * OSU ID: 932540242
 * ONID: watsokel
 * FileName: TCPClient.java
 * Description: Methods required by the class chatclient.java for two-way 
 * communication with server program
 * Sources: https://systembash.com/a-simple-java-tcp-server-and-tcp-client/
 * http://docs.oracle.com/javase/tutorial/networking/sockets/readingWriting.html
 */

//http://docs.oracle.com/javase/tutorial/networking/sockets/readingWriting.html
import java.io.*; 
import java.net.*;
import java.util.Scanner; 

public class FTPClient {  
    public String hostName;
    public int portNumber;
    public String prompt;
    public String command;
    public String message;
    public String response;
    public String handleMsg;
    public String lengthStr;
    public BufferedReader inFromUser;
    public DataOutputStream outToServer;
    public BufferedReader inFromServer;
    public Socket clientSocket;

    /*Default constructor
    *Pre-conditions: none
    *Post-conditions: initializes lengthStr to empty string
    */
    public FTPClient(){
  	  lengthStr="";
      inFromUser = null;
      outToServer = null;
      inFromServer = null;
      clientSocket = null;
      command = "";
      prompt = "\nPlease enter either -l or -g <FILENAME>: ";
    }

    /*Method to print title and prompt for user handle
    *Pre-conditions: none
    *Post-conditions: no return value
    */
    public void printTitle(){
        System.out.println("\n\nFILE TRANFSER SYSTEM (CLIENT)");
        System.out.println("Programmed by Kelvin Watson, OSU ID 932540242, onid: watsokel)");
        System.out.println("**************************************************************");
        System.out.println("CONTROL CONNECTION ESTABLISHED. Welcome to Simple File Transfer.");
    }

    /*Method for creating client socket and buffers
    *Pre-conditions: member variable must be declared
    *Post-conditions: client socket and buffers allocated
    */
    public void initiateContact(){
      try{
        clientSocket = new Socket(hostName, portNumber);   
        inFromUser = new BufferedReader(new InputStreamReader(System.in));   
        outToServer = new DataOutputStream(clientSocket.getOutputStream());   
        inFromServer = new BufferedReader(new InputStreamReader(clientSocket.getInputStream())); 
      }catch(IOException e){
         System.err.println("Caught IOException in initiateContact: " + e.getMessage());
      }
    }
    
    /*public String validateCommand(String cmd){
      if(cmd.)
    }*/


    /*Method for sending messages to the server
    *Pre-conditions: client must have established TCP connection with server
    *Post-conditions: client socket and buffers allocated
    */
    public String sendCommand(){
      try{ 
        System.out.print(prompt);
        command = inFromUser.readLine();
        outToServer.writeBytes(command); 
        if(command.startsWith("-l")){
          return "-l";
        }
        else if(command.startsWith("-g")){
          return "-g";
        }
        else{
          //invalid command
          return "ERR";
        }
        //parse and validate command
      } catch(IOException e){
        System.err.println("Caught IOException in sendCommand(): " + e.getMessage());
      } 
      return null;
    }

    /*Method for receiving messages from the server
    *Pre-conditions: client must have established TCP connection with server
    *Post-conditions: client socket and buffers allocated
    */
    public String receiveMessage(){
      System.out.println("in receiveMessage");
      try{
        response = inFromServer.readLine();
      }catch(IOException e){
        System.err.println("Caught IOException in inFromServer.readLine(): " + e.getMessage());
      }
      System.out.println(response);
      return "RECEIVED";
    }
}
