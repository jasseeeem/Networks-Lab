- Open two terminal windows. First one for server and the second one for client.

- In the first terminal window, cd into the server directory using the command:
  cd /home/user/Muhammed_B190703CS/FTAPServer

- Compile the .c file using the command:
  gcc FTAPServer.c -o FTAPServer

- Run the server program using the command:
  ./FTAPServer

- In the second terminal window, cd into the client directory using the command:
  cd /home/user/Muhammed_B190703CS/FTAPClient

- Compile the .c file using the command:
  gcc FTAPClient.c -o FTAPClient

- Run the client program using the command:
  ./FTAPClient

- Start typing these FTAP commands on the client terminal to execute various functionalities:
  - START: Starts a connection between the FTAPClient and the FTAPServer
  - USERN <username>: Send the username to the server to verify the user
  - PASSW <password>: Send the password to the server to verify the user
  - CreateFile <filename>: Create an empty .txt file in the FTAPServer directory
  - ListDir: List all the files that are present in the FTAPServer directory
  - StoreFile <filename>: Move a file from the FTAPClient directory to the FTAPServer directory
  - GetFile <filename>: Get a file from the FTAPServer directory to the FTAPClient directory
  - QUIT: Quit the user session and exit the program if no files are being transferred