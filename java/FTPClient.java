/* PROGRAMMED BY: Kelvin Watson
 * OSU ID: 932540242
 * ONID: watsokel
 * FILENAME: chatclient.java
 * DESCRIPTION: FTPClient class data members and methods required by ftclient.java
 * CODE SOURCES: https://systembash.com/a-simple-java-tcp-server-and-tcp-client/
 * http://docs.oracle.com/javase/tutorial/networking/sockets/readingWriting.html
 * http://stackoverflow.com/questions/7771303/getting-the-character-returned-by-read-in-bufferedreader
 */

import java.io.*; 
import java.net.*;
import java.util.Scanner;
import java.io.Writer;

public class FTPClient {  
    public String hostName;
    public int portNumber;
    public int dataPortNumber;
    public String prompt;
    public String controlCommand;
    public String requestedFileName;
    public String message;
    public String handleMsg;
    public String lengthStr;
    public BufferedReader inFromUser;
    public DataOutputStream outToServer;
    public BufferedReader inFromServer;
    public Socket clientSocket;
    public ServerSocket welcomeSocket;
    public Socket dataSocket;
    public BufferedReader fileDataFromServer;
    public BufferedReader inFromClient;
    public DataOutputStream outToClient;
    /*Default constructor
    *Pre-conditions: none
    *Post-conditions: initializes lengthStr to empty string
    */
    public FTPClient(){
  	  lengthStr="";
      inFromUser = null;
      outToServer = null;
      outToClient = null;
      inFromServer = null;
      inFromClient = null;
      clientSocket = null;
      dataSocket = null;
      controlCommand = "";
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
        System.out.println(">>CONTROL CONNECTION ESTABLISHED AND COMMAND RECOGNIZED.");
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
    public String sendCommand(String command){
      try{ 
        outToServer.writeBytes(command); 
        if(command.startsWith("-l")){
          return "-l";
        }
        else if(command.startsWith("-g")){
          return "-g";
        }
        else{
          return "ERR";
        }
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
      String response = "";
      System.out.println("in receiveMessage");
      try{
        while ((response = inFromServer.readLine())!= null) {
          //System.out.println(response);
          
          return response;
        }
      } catch(IOException e){
        //Socket may already be closed
        //System.err.println(e); 
      }
      return response;
    }

    public String replaceDelimiter(String original){
      if(original != null && !original.isEmpty()) {
        return original.replace(',', '\n');
      } return null;
    }

    public void displayFileList(){
      String sendAction = sendCommand(controlCommand); //returns either -l, -g, or ERR so client knows what to expect
      String message = receiveMessage(); //server should send an acknowledgement or ERROR
      String fileList = replaceDelimiter(message);
      System.out.println(fileList);
     
    }

    public void getFile(){
      String sendAction = sendCommand(controlCommand + " " + requestedFileName + " " + dataPortNumber + "\n"); //returns either -l, -g, or ERR so client knows what to expect
      //check if file already exists in the current directory
      System.out.println("in GET file");
      //String message = receiveMessage(); //server should send an acknowledgement or ERROR
      //System.out.println(message);
      listenForDataConnection();
    }

    public void listenForDataConnection(){
      try{
        welcomeSocket = new ServerSocket(dataPortNumber);           
        dataSocket = welcomeSocket.accept();
        storeFileData();                           
      }catch(IOException e){

      }finally{
        try{
          System.out.print("FINALLY");
          inFromClient.close();
          outToClient.close();
          dataSocket.close();
          welcomeSocket.close();
        }catch(IOException e){

        }
      }     
      /*try{
        welcomeSocket = new ServerSocket(dataPortNumber);
        System.out.println("printing");
        System.out.println("dataPortNumber is " + dataPortNumber);
        System.out.println("waiting for server to establish connection for data transfer!");
        dataSocket = welcomeSocket.accept();
        System.out.println("CONNECTION ESTABLISHED");
        fileDataFromServer = new BufferedReader(new InputStreamReader(dataSocket.getInputStream()));
        DataOutputStream outToClient = new DataOutputStream(dataSocket.getOutputStream());             
        String resp = "";
        try{
          while ((resp = fileDataFromServer.readLine())!= null) {
          }
        } catch (IOException e){
          System.err.println(e); 
        }
        System.out.println("printing response from Server " + resp);
      } catch(IOException e){
        System.err.println(e);  
      }*/
    }

    public void storeFileData(){  //overwrite file if exists
      StringBuilder response = new StringBuilder();          
      int c;
      Writer fWriter = null;
      try{
        inFromClient = new BufferedReader(new InputStreamReader(dataSocket.getInputStream()));             
        outToClient = new DataOutputStream(dataSocket.getOutputStream());             
        while ((c = inFromClient.read()) != -1) {
          System.out.print((char)c);
          response.append((char)c); //holds string of text file data
        }
        String buffer = response.toString();
        System.out.print("DONE READING");
        fWriter = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(requestedFileName),"utf-8"));    
        fWriter.write(buffer);
        System.out.print("DONE WRITING");
      }catch(IOException e){
        System.err.println("File Read/Write IOException in storeFileData(): " + e.getMessage());
      }finally{
        try{
          fWriter.close();
        } catch(Exception e){
        }
      }
    }


}
