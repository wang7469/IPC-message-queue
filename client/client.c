// test machine : Vole
// date : 04/29/2020
// name : Daisy Wang, Franklin Tan, ShouKang Mao
// x500 : wang7469, tan00081, maoxx243

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <limits.h>

#define MAX_CHAR 256
#define MSG_SIZE 512

struct msg_buffer {
    long msg_type;
    char msg_text[MSG_SIZE];
} msg;

//initilize global variables

int i;
int input_num;


int traverse_folder(char path[MAX_CHAR], char Clienti_path[][MAX_CHAR]){
	//open directory
	DIR *dir = opendir(path);
	//error checking if failed to open directory
	if (!dir){
			perror("Error, cannot open directory");
			return -1;
	}
	struct dirent *dir_path;

  // if(((dir_path = readdir(dir)) == NULL)){
  //   printf("The %s folder is empty\n", path);
  //   return 1;
  // };
  int count = 0;

	while(((dir_path = readdir(dir)) != NULL)){
		if (strcmp(dir_path->d_name, ".") != 0 && strcmp(dir_path->d_name, "..") != 0) {
			if (dir_path->d_type == DT_DIR) { //if subdirectory exist
				char subdir[MAX_CHAR];
				subdir[0] = '\0';
				strcpy(subdir, path);
				strcat(subdir, "/");
				strcat(subdir, dir_path->d_name);
        count++;
				traverse_folder(subdir, Clienti_path);//recursive call
			}else {
				//ignore .DS_Store files
				if(strcmp(dir_path->d_name, ".DS_Store") != 0){
					char txt_path[MAX_CHAR];
					strcpy(txt_path, path);
					strcat(txt_path, "/");
					strcat(txt_path, dir_path->d_name);
          //write to Clienti.txt
          if(i == input_num){i = 0;}
          FILE* fptr = fopen(Clienti_path[i], "a");
      	  if (fptr == NULL) {
      			  perror("Failed to create Clienti.txt file\n");
      			  return -1;
      	  }
          fprintf(fptr, "%s\n", txt_path);
          //printf("%s\n", txt_path);
          fclose(fptr);
					i+=1; //counter increment
				}
			}
		}
	}
 closedir(dir);
 if (count == 0){
   return 1;
 }
 return 0;
}


//number of clients as first argument, path to folder as second
int main(int argc, char *argv[]){
	if(argc < 3){ // less than two command line input
		printf("Not enough arguments, enter two arguments\n");
		exit(-1);
	}
  printf(">>>>>>>>>> Client Start <<<<<<<<<<\n\n");
  //command line input
  input_num = atoi(argv[1]);

  int create_dir = mkdir("ClientInput", 0777); //create directory
  if(create_dir == -1){
      perror("Failed to create ClientInput: \n");
      //exit(1);
  }

  int create_out_dir = mkdir("Output", 0777); //create directory
  if(create_out_dir == -1){
      perror("Failed to create Output: \n");
      //exit(1);
  }

  char Clienti_path[input_num][MAX_CHAR];//array storing paths to Clienti.txt files for later use

  //get path to ClientInput
	char ClientInput_path[MAX_CHAR];
	getcwd(ClientInput_path, 256);
	strcat(ClientInput_path, "/ClientInput");

  char Clienti_out[input_num][MAX_CHAR];

  //get path to ClientInput
	char ClientOutput[MAX_CHAR];
	getcwd(ClientOutput, 256);
	strcat(ClientOutput, "/Output");

	//create argv[1] of client.txt files
	for(int k = 0; k < input_num; k++){
		char clientfile[MAX_CHAR] = "/Client";
		char fullpath[MAX_CHAR];
		char num[MAX_CHAR];

		sprintf(num, "%d", k);
		strcat(clientfile, num);
		strcat(clientfile, ".txt");
		strcpy(fullpath, ClientInput_path);
		strcat(fullpath, clientfile);

    strcpy(Clienti_path[k], fullpath);
		//create Clienti.txt files to store partitioned paths
		FILE* fptr = fopen(fullpath, "w");
	  if (fptr == NULL) {
			  perror("Failed to create Clienti.txt file\n");
			  exit(1);
	  }
    fclose(fptr);

    char Clientout[MAX_CHAR] = "/Client";
    char out_fullpath[MAX_CHAR];

		strcat(Clientout, num);
		strcat(Clientout, "_out.txt");
		strcpy(out_fullpath, ClientOutput);
		strcat(out_fullpath, Clientout);

    strcpy(Clienti_out[k], out_fullpath);
		//create Clienti.txt files to store partitioned paths
		FILE* fp = fopen(out_fullpath, "w");
	  if (fp == NULL) {
			  perror("Failed to create Clienti.txt file\n");
			  exit(1);
    }
    fclose(fp);
  }

  printf(">>>>>>>>>> File traversal and partitioning start <<<<<<<<<<<\n\n");
	i = 0; //initilize counter to 0
	int check = traverse_folder(argv[2], Clienti_path); //recursively trasver folder and write files to Clienti.txt
  if (check == -1){
    return -1; // error in traverse_folder
  }
  else if (check == 1){
    printf("%s folder is empty\n", argv[2]);
    return -1;
  }

  //start sending file paths through message queue
  key_t key;
  int msgid;

  //note: ftok path should be SAME for client and server
  key = ftok("../README.md", 56);
  // if(key == -1){ //error checking
  //   perror("ftok failed\n");
  //   exit(1);
  // }
  msgid = msgget(key, IPC_CREAT|0666);
  if(msgid == -1){ //error checking
    perror("msgget failed\n");
    exit(1);
  }

  //creating c child processes
  for(int c = 0; c < input_num; c++){
    int process = fork();
    //printf("_________________________Process: %d\n", process);
    if(process < 0){
      perror("Failed to create process");
      exit(1);
    }
    if(process == 0){//child process created
      //open corresponding client file
      FILE* f = fopen(Clienti_path[c], "r");
    	if (f == NULL) {
    			perror("Failed to open Clienti.txt file\n");
    			exit(1);
    	}

    	while(fgets(msg.msg_text, sizeof(msg.msg_text), f) != NULL) { //read line by line
        msg.msg_type = c+1;
        printf("Process: %d\n", c+1);
        printf("Sending %s from client process %ld\n", msg.msg_text, msg.msg_type);
        //fflush(stdout);
        char print_path[256];
        strcpy(print_path, msg.msg_text);
        if(msgsnd(msgid, &msg, sizeof(msg.msg_text), 0) == -1){
          perror("msgsnd to server failed\n");
          exit(1);
        }
        //fflush(stdout);

    		if(msgrcv(msgid, &msg, sizeof(msg.msg_text), 1000+c+1, 0) == -1){
          perror("Receive acknowledgement from server FAILED\n");
          exit(1);
        }
        printf("Client process %ld received %s from server for\n", msg.msg_type, print_path);
    	 }
       fclose(f);

       //finish sending all paths within a Clienti.txt file
    	 strcpy(msg.msg_text, "END");
       msg.msg_type = c+1;
       if(msgsnd(msgid, &msg, sizeof(msg.msg_text), 0) == -1){
         perror("msgsnd 'END' to server failed\n");
         exit(1);
       }
       printf("Sending END from client process %d\n", (c+1));

       if(msgrcv(msgid, &msg, sizeof(msg.msg_text), (5000+c+1), 0) == -1){
         perror("Receive word count from server FAILED\n");
         exit(1);
       }
       printf("Client process %d received |||%s||| from server\n", (c+1), msg.msg_text);

       FILE* fout = fopen(Clienti_out[c], "w");
     	 if (fout == NULL) {
     			perror("Failed to open Clienti.txt file\n");
     			exit(1);
     	 }
       fprintf(fout, "%s\n", msg.msg_text);
       //printf("%s\n", txt_path);
       fclose(fout);
       exit(1);
    }
  }

   for(int i = 0; i < input_num; i++){
     wait(NULL);
   }


   if(msgctl(msgid, IPC_RMID, NULL) == -1){
     perror("msgctl failed\n");
     exit(1);
   }


  printf(">>>>>>>>>> Client Ends <<<<<<<<<<\n");
	return 0;
}
