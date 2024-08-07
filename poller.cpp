#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include<map>
#include<vector>
#include <signal.h>



using namespace std;
// some others that will need in signal handler
string poll_log;
string poll_stats;
map<string,int> comma_map;

fstream MyFile;
// the thinks that will be used for the threads globally
int bufferSize;
pthread_t *tid;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int counter;
vector<int> client_names;
map<string,string> names_map;
static sigset_t   signal_mask;
int WORKERS;
//end of global threads


// mask for the threads to ingore ctrl-C
void mask_sig(void)
{
	 sigset_t   mask;
	sigemptyset(&mask);
  	sigaddset(&mask, SIGINT);
        pthread_sigmask(SIG_BLOCK, &mask, NULL);

}


// what to do after you press ctrl-c
void sighandler(int signo, siginfo_t *info, void *extra){

// cout << counter ;
 pthread_mutex_lock(&lock);
  while(counter>0){
   pthread_cond_wait(&cond2,&lock);
  }
  pthread_mutex_unlock(&lock);



 ofstream MyFile2;
 MyFile2.open(poll_stats);
 MyFile2.close();
map<string,string>::iterator itr;
if(names_map.size()==0){
  cout << "\n 0 votes so i exit \n";
exit(-1);
}

cout << "\n i  close  \n";


MyFile2.open(poll_stats);



 for (itr=names_map.begin(); itr != names_map.end(); ++itr) {
  // find comma and how many votes
  map<string, int>::iterator it = comma_map.find(itr->second);
  if(comma_map.find(itr->second) == comma_map.end()) {
  comma_map.insert(pair<string,int>(itr->second,1));
}

  if (it != comma_map.end()){
    it->second = it->second+1;
  }
}

// write the commata and votes to file
map<string,int>::iterator  atr=comma_map.begin();
  MyFile2 <<  atr->first << " " << atr->second ;
  atr++;
 for (atr; atr != comma_map.end(); ++atr) {

  MyFile2 << "\n" << atr->first << " " << atr->second ;
}

    MyFile.close();
    MyFile2.close();

exit(0);
}
// connect master thread and signal_handler to ctrl-C
void set_sig_handler(void)
{
        struct sigaction action;
        action.sa_flags = SIGINT;
        action.sa_sigaction = sighandler;

        if (sigaction(SIGINT, &action, NULL) == -1) {
            perror("sigusr: sigaction");
            _exit(1);
        }

}

// the servant threads
void *connect(void * sD){


    int clientSocket;
    // max words
    int MAX=150;
    char strData[MAX]="test";

  //     cout << "sleeping \n";
  // ingore ctrl-c
  mask_sig();
while(1){

    pthread_mutex_lock(&lock);
    //connect to client
    while(client_names.size()==0){
       pthread_cond_wait(&cond1, &lock);
    }
    clientSocket=client_names.front();
       client_names.erase(client_names.begin());
    counter++;
     pthread_mutex_unlock(&lock);
     //read the file
     while (strcmp(strData,"stoptalking")!=0){
      	bzero(strData, sizeof(strData));
    // integer to hold client socket.
    strcpy(strData,"SEND NAME PLEASE");

    write(clientSocket, strData, sizeof(strData));
    bzero(strData, sizeof(strData));
		read(clientSocket, strData, sizeof(strData));
       string namefull,comma;

        if(strcmp(strData,"stoptalking")!=0){
           for (int i=0; i<strlen(strData); i++){
            namefull.append(1,strData[i]);
              }
           if (names_map.find(namefull) == names_map.end()) {
               bzero(strData, sizeof(strData));
                strcpy(strData,"SEND VOTE PLEASE");
                // cout << strData << "\n";
                write(clientSocket, strData, sizeof(strData));
                  bzero(strData, sizeof(strData));
		               read(clientSocket, strData, sizeof(strData));
                  //  cout << strData << "\n";
                     for (int i=0; i<strlen(strData); i++){
                      comma.append(1,strData[i]);
                        }
                pthread_mutex_lock(&lock);
                names_map.insert(pair<string,string>(namefull,comma));
                MyFile.open(poll_log,std::ios::app);
                MyFile << namefull << " " << comma << "\n";
                MyFile.close();
                 pthread_mutex_unlock(&lock);
                  bzero(strData, sizeof(strData));
                  string vote="VOTE FOR PARTY";
                  vote.append(comma);
                strcpy(strData,vote.c_str());
                // cout << strData << "\n";
                write(clientSocket, strData, sizeof(strData));

           }else {
              bzero(strData, sizeof(strData));
                strcpy(strData,"ALREADY VOTED");
                // cout << strData << "\n";
                write(clientSocket, strData, sizeof(strData));
           }

        }

     }
    //  sleep(10);
    //  cout << "wake up samurai \n";
     pthread_mutex_lock(&lock);
      counter--;
      pthread_cond_signal(&cond2);
      pthread_mutex_unlock(&lock);

}


return 0;
}




int main(int argc, char const* argv[])
{
  //  signal(SIGINT, sighandler);
    if(argc<5){
      perror("you need 5 arguments");
      exit(1);
    }
    // taking the basics
    int portnum=atoi(argv[1]);
    int numWorkerThreads=atoi(argv[2]);
    bufferSize=atoi(argv[3]);
    poll_log=argv[4];
    poll_stats=argv[5];
    WORKERS=numWorkerThreads;

   if(numWorkerThreads=<0 || bufferSize=<0){
      perror("you need THREADS>0 OR BUFFER >0 in arguments");
      exit(1);
    }
    //create server
    struct sockaddr_in servAddr;
    struct hostent *ptrh;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(portnum);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int servSockD = socket(AF_INET, SOCK_STREAM, 0);

    bind(servSockD, (struct sockaddr*)&servAddr,sizeof(servAddr));

// while( myfile.good()) {
// myfile >> mystring;
// comma_map.insert(pair<string,int>(mystring,0));
// }
// make lock
 if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }
counter=0;
int threads=0;
pthread_t tidwork[numWorkerThreads];
tid=tidwork;
//make all servant threads
while(threads<numWorkerThreads){
pthread_create(&tid[threads],NULL,&connect,NULL);
threads++;
}
  set_sig_handler();
  // delete previus data on file
  MyFile.open(poll_log, ios::out);
  MyFile.close();
  //starting master threads
while(1){
   pthread_mutex_lock(&lock);
   int size=client_names.size();
   pthread_mutex_unlock(&lock);

    if(size<bufferSize){
       unsigned int len;
      struct sockaddr_in  cli;
      listen(servSockD, 1);
     len = sizeof(cli);
     int clientSocket =accept(servSockD, (struct sockaddr*)&cli,&len);
      pthread_mutex_lock(&lock);
     client_names.push_back(clientSocket);
        pthread_cond_signal(&cond1);
      pthread_mutex_unlock(&lock);
    }
    // pthread_cond_wait(&cond1, &lock);
}
    return 0;
}
