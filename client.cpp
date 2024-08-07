#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include<netdb.h>
#include<string.h>
  #include <cstring>
  #include <arpa/inet.h> // inet_addr()
using namespace std;



int main(int argc, char *argv[])
{

   if(argc<3){
      perror("you need 3 arguments");
      exit(1);
    }

// check if they are right
    string serverName=argv[1];
    int portnum=atoi(argv[2]);
    string Voters_file=argv[3];


  struct sockaddr_in servAddr;// making connection to server
  struct hostent *ptrh;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port= htons(portnum); // use some unused port number
    // servAddr.sin_addr.s_addr =inet_addr("127.0.0.1");

     ptrh=gethostbyname(serverName.c_str()); //taking hostname
    memcpy(&servAddr.sin_addr,ptrh->h_addr,ptrh->h_length);

    ifstream myfile;
    string myText;
    myfile.open(Voters_file);
     char serMsg[80];
    int sockD = socket(AF_INET, SOCK_STREAM, 0); //open the socket
    int connectStatus= connect(sockD, (struct sockaddr*)&servAddr,sizeof(servAddr)); //connect to server
     if (connectStatus == -1) {
        printf("Error...\n");
    }
    else {
   while (getline (myfile, myText)) {//we open the txt file
    int j=0;
    string namefull ,comma;
     if(myText.c_str()!=" "){
         for (int i=0; i<strlen(myText.c_str()) && j<3; i++){//making strings of name and comma to vote
        if(j<2){
            namefull.append(1,myText[i]);
        }else{
            comma.append(1,myText[i]);
        }
            if(isspace(myText[i])){
            j++;
        }
        }
        //here we check and give the name and comma how its ask to do first name then return if it exist already ...
      bzero(serMsg, sizeof(serMsg));
      read(sockD,serMsg, sizeof(serMsg));
      // cout << serMsg << "\n";
      if(strcmp(serMsg,"SEND NAME PLEASE")==0){
        bzero(serMsg, sizeof(serMsg));
         strcpy(serMsg,namefull.c_str());
          // cout << serMsg << "\n";
         write(sockD,serMsg, sizeof(serMsg));
      }else{
        break;
      }
       bzero(serMsg, sizeof(serMsg));
      read(sockD,serMsg, sizeof(serMsg));
      //  cout << serMsg << "\n";
       if(strcmp(serMsg,"ALREADY VOTED")==0){
        cout << "the voter " << namefull << "has already vote \n";

      }else{
          bzero(serMsg, sizeof(serMsg));
         strcpy(serMsg,comma.c_str());
          // cout << serMsg << "\n";
         write(sockD,serMsg, sizeof(serMsg));
          bzero(serMsg, sizeof(serMsg));
        read(sockD,serMsg, sizeof(serMsg));
        //  cout << serMsg << "\n";
      }

      //  cout << serMsg << "\n";
		  bzero(serMsg, sizeof(serMsg));
      //  send(sockD,serMsg, sizeof(serMsg), 0);
    }

    }// i have made it so the server to end when it takes the  message stoptalking
      char serMsg[600]="stoptalking";
      write(sockD,serMsg, sizeof(serMsg));
		  bzero(serMsg, sizeof(serMsg));
    }
      //  send(sockD,serMsg, sizeof(serMsg), 0);


    return 0;
}
