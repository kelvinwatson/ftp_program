import java.io.*; 
import java.net.*; 

class testServer {    
  public static void main(String argv[]) throws Exception {          
    String clientSentence;          
    String capitalizedSentence;          
    ServerSocket welcomeSocket = new ServerSocket(30026);          
    while(true){             
      Socket connectionSocket = welcomeSocket.accept();             
      BufferedReader inFromClient = new BufferedReader(new InputStreamReader(connectionSocket.getInputStream()));             
      DataOutputStream outToClient = new DataOutputStream(connectionSocket.getOutputStream());             
      while(inFromClient.readLine()){
        clientSentence = inFromClient.readLine();
        System.out.println("Received: " + clientSentence);             
      }             
      capitalizedSentence = clientSentence.toUpperCase() + '\n';             
      outToClient.writeBytes(capitalizedSentence);          
    }       
  } 
} 