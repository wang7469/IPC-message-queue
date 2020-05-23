// test machine : Vole
// date : 04/29/2020
// name : Daisy Wang, Franklin Tan, ShouKang Mao
// x500 : wang7469, tan00081, maoxx243#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>

#define MSG_SIZE 512

//global word count array
int count = 0;
int count_arr[26];
int end;
int command_input;
pthread_mutex_t array_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t end_lock = PTHREAD_MUTEX_INITIALIZER;

struct msg_buffer {
    long msg_type;
    char msg_text[MSG_SIZE];
};

struct parameters {
  // pthread_cond_t* cond;
  // pthread_mutex_t* mutex;
  long type;
  int msgid;
};

void word_count(char file_path[MSG_SIZE]){
  //remove all white spaces at the end
  char path[MSG_SIZE];
  strcpy(path, file_path);
  int length = strlen(path);
  while(length > 0 && isspace (path[length -1])){
    length--;
  }
  path[length] = '\0';
  //start word count
  FILE* f = fopen(path, "r");
  if (f == NULL) {
      perror("Failed to open .txt file for word count\n");
      exit(1);
  }
  char c;
  char temp[MSG_SIZE];
  int index;
  while (fgets(temp, MSG_SIZE, f) != NULL){
    c = temp[0];
    if(isspace(c) == 0){

      // for (char ch = 'A'; ch <= 'Z'; ++ch) {
    	// 	printf("%d\t", ch - 'A');
    	// }
      //
    	// for (char ch = 'a'; ch <= 'z'; ++ch) {
    	// 	printf("%d\t", ch - 'a');
    	// }

      if(c >= 'A' && c <= 'Z'){
        index = (int)c;
        //printf("%d\n", index);
        index -= 65;
        // lock
        pthread_mutex_lock(&array_lock);
        count_arr[index]++;
        // unlock
        pthread_mutex_unlock(&array_lock);
      }
      else if(c >= 'a' && c <= 'z'){
        index = (int)c;
        //printf("%d\n", index);
        index -= 97;
        // lock
        pthread_mutex_lock(&array_lock);
        count_arr[index]++;
        // unlock ()
        pthread_mutex_unlock(&array_lock);
      }
    }
  }
  fclose(f);
}

void *server_response(void *arg){
  struct msg_buffer msg;
  struct parameters para = *((struct parameters *)arg);
  long type = para.type;
  int msgid = para.msgid;
  free(arg);
  while(1){
    //printf("Thread: %ld\n", type);
    if(msgrcv(msgid, &msg, sizeof(msg.msg_text), type, 0) == -1){
      perror("Receive message from client FAILED\n");
      exit(1);
    }else{
      count++;
    }
    printf("Thread %ldreceived: %s from client process %ld\n", type, msg.msg_text, msg.msg_type);

    if(strcmp(msg.msg_text, "END") == 0){
        printf("Thread %ld received END from client process %ld", type, msg.msg_type);
      // lock here (different lock)
        pthread_mutex_lock(&end_lock);
        end++;
      // unlock here
        pthread_mutex_unlock(&end_lock);

      while (end < command_input);
      if(end == command_input){
        char word_count[MSG_SIZE];
        char temp[10];
        memset(word_count, '\0', MSG_SIZE);
        // for(int i = 0; i < 26; i++){
        //   //count_arr[i] = 0;
        //   printf("%d ", count_arr[i]);
        // }
        printf("\n");
        snprintf(temp, 10 ,"%d", count_arr[0]);
        strcpy(word_count, temp);
        strcat(word_count, "#");
        memset(temp, '\0', 10);

        for(int i = 1; i < 26; i++){
          snprintf(temp, 10 ,"%d", count_arr[i]);
          strcat(word_count, temp);
          strcat(word_count, "#");
          memset(temp, '\0', 10);
        }

        //for(int temp = 0; temp < command_input; temp++){
        strcpy(msg.msg_text, word_count);
        msg.msg_type = (5000+type);
        if(msgsnd(msgid, &msg, sizeof(msg.msg_text), 0) == -1){
          perror("msgsnd to client failed\n");
          exit(1);
        }
        printf("Thread %ld sending final letter count to client process %ld\n", type, msg.msg_type);
        //}

      }
      break;
    }

    //received file path from client, start word counter
    word_count(msg.msg_text);
    printf("Word count done\n");


    strcpy(msg.msg_text, "ACK");
    msg.msg_type = (1000+type);
    if(msgsnd(msgid, &msg, sizeof(msg.msg_text), 0) == -1){
      perror("msgsnd to client failed\n");
      exit(1);
    }
    printf("Thread %ldsending ACK to client %ld for %s\n", type, msg.msg_type, msg.msg_text);
  }
  printf("%s %ld\n", "Message received and word count done for thread: ", type);
  return NULL;
}


int main(int argc, char *argv[]) {
  if(argc < 2){ // less than one command line input
		printf("Not enough arguments, enter one arguments\n");
		exit(-1);
	}
  printf(">>>>>>>>>> Server Starts <<<<<<<<<<\n");
  int num_clients = atoi(argv[1]);
  command_input = num_clients;
  end = 0;

  key_t key;
  int msgid;

  //note: ftok path should be SAME for client and server
  key = ftok("../README.md", 56);
  // if(key == -1){ //error checking
  //   perror("ftok failed in server\n");
  //   exit(1);
  // }

  if((msgid = msgget(key, 0666 | IPC_CREAT)) < 0){
    printf("error");
    exit(1);
  }else{
    msgctl(msgid, IPC_RMID, NULL);
    if((msgid = msgget(key, 0666 | IPC_CREAT)) < 0){
      printf("error");
      exit(1);
    }
  }

  for (int i = 0; i < 26; i++){
    count_arr[i] = 0;
  }
  // struct parameters *para = malloc(sizeof(struct parameters));
  // para->cond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
  // para->mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  // para->type = (long) malloc(sizeof(long));
  // para->msgid = (int) malloc(sizeof(int));
  // pthread_cond_init(para->cond, NULL);
  // pthread_mutex_init(para->mutex, NULL);
  // para->msgid = msgid;

  //int i = 0;
  //create num_clients number of threads
  pthread_t threads[num_clients];
  for (int i = 0; i < num_clients; i++){
      //struct parameters to pass into server_response
      struct parameters *para = malloc(sizeof(struct parameters));
      para->type = i+1;
      para->msgid = msgid;

      if (pthread_create(&threads[i], NULL, server_response, para) != 0)
        {
          perror("Failed to create thread\n");
          exit(1);
        }
    }
  for (int k = 0; k < num_clients; k++){
      if (pthread_join(threads[k], NULL) != 0)
        {
          perror("Failed to join thread\n");
          exit(1);
        }
    }

  // free(para->type);
  // free(para->msgid);
  // free(para);
  printf(">>>>>>>>>> Server Ends <<<<<<<<<<\n");
  return 0;
}
