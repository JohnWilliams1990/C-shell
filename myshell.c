
// John Williams
// 105201054
// Date: Fri. Sep. 7, 2018
// Program: myshell.c



#include<sys/stat.h>
#include<fcntl.h> 
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>
#include<stdbool.h>


// function that counts number of spaces 
int counter(char* str){
int count = 0; 
 for (int i = 0; i < strlen(str) + 1; i++)
  {
    if( (int)str[i] == 32 && i !=  strlen(str) -1) 
    { count += 1; }
  }
return count +1; 
}


int main()
{


int status = 0;
pid_t childpid;
int child_return_value;

  const int COMMANDSIZE = 50; // 
  char str[COMMANDSIZE]; // string to hold initial input
  char * Aptr;
  int ret = 1;
  int count = 0;
  bool pipe_to = false; // flags for testing for a pipe character
  bool pipe_fr = false; //
  int pipe_index = 0;
  int fd[3];// file descriptor used for piping and reading and writing
  while(1  == 1){ // while the user hasn't exited the program
    pipe_to = false; 
    pipe_fr = false;// reset flags and counter 
    count = 0;
    printf("[myShell]> "); 
    scanf("%[^\n]%*c", str);// gather input from user
    char pipechar = '\0';
    char **commands = (char**)malloc(counter(str)*sizeof(char *));
    ret = strncmp(str, "exit", 4);// test to see if it is an exit or change directory command
    if (ret == 0){ break;}
    ret = strncmp(str, "cd", 2);  
    if (ret == 0){// if change directory then skipp forking process so as to maintain relative directory 
      char * dirPtr = str;
      dirPtr += 3;// increment to get the location of the directory to change to and then change directory 
      if (chdir(dirPtr) != 0){
        perror("Cannot change directory. No such directory");
      }
    }

    else // this isn't a change directory command
    {    //but needs to be visable for parent and child or all hell is coming out of pipe

      if(pipe(fd) == -1){ // initialize file descriptor for piping
        perror("pipe");
        exit(1);  
      }

      pid_t pid = fork(); // fork here: this allows that upon executing an exec command the exit for that command will deal with freeing all allocated memory

// parse string into individual commands and arguements
      if (str[strlen(str)-1] == ' ') {str[strlen(str)-1] = '\0';}
      Aptr = str;
        for (int i = 0; i < strlen(str) + 1; i++)
        { 
          if (str[i] == '>' || str[i] == '|')
          {
            pipechar = str[i];
    	  count +=1;
    	  pipe_index = count;
            pipe_to = true;		
            i+=1;
            Aptr = &str[i+1];
         }
    
          if (str[i] == '<')
          {
            pipechar = str[i];
    	  count +=1;
    	  pipe_index = count;
            pipe_fr = true;		
            i+=1;
            Aptr = &str[i+1];
          }
          if( (int)str[i] == 32 && str[i-1] != '|') 
          {
            str[i] = '\0';
            commands[count] = (char*)malloc(strlen(Aptr));
            strcpy(commands[count],Aptr);
            count ++;
            Aptr = &str[i];
            Aptr++;
            str[i] = ' ';	
          }
          else if (str[i] == '\0' )
          {
            commands[count] = (char*)malloc(strlen(Aptr));
            strcpy(commands[count],Aptr);
            count ++;
          }  
        }

        if (pipe_to == true  && pipechar != '>'){
          commands[count] = NULL;
        }

  //determine what case we are in here for arguements 
  //look at pipe flags and look to see if we are mains's child
      
        //pid_t pid = fork();
        if (pid < 0) {
          perror("fork failed.");
          exit(1);
        }
        else if (pid == 0){   // main's child thread 

          if (pipe_to == true && pipechar != '>')  // here we not are piping to a file 
           {
               dup2(fd[1], 1);
               close(fd[0]);
               close(fd[1]);
             if (commands [1] != NULL)
             {
               if (execvp( commands[0], commands) == -1 ) // based on number of arguements do exec
               {
                  fprintf(stderr, "Error %s command not recognized\n\n", commands[0]);
		  exit(1);
                 perror("exec");
               }
             }
             else 
             {
               if (execlp(commands[0],commands[0],(char*)NULL) == -1 )
               {
                  fprintf(stderr, "Error %s command not recognized\n\n", commands[0]);
		  exit(1);
                 perror("exec");
               }
             }
           }
          else if (pipe_to == true  && pipechar == '>') // here we are piping to a file 

          { 
            // pipe command to file if good file name and descriptor
            fd[2] = open(commands[count-1], O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
            if (fd[2] < 0)
            {
              perror("open failed\n"); 
              exit(1);    
            }
      
            fcntl(fd[2], FD_CLOEXEC); // close the file descriptor on exit
            ret = dup2(fd[2], 1); 
            close(fd[1]);
            close(fd[0]);
            if (ret < 0)
            {
              perror("dup2 failed\n");
              exit(1);
            }
            // execute command
            if (execvp( commands[0], commands) == -1 )
            {
                  fprintf(stderr, "Error %s command not recognized\n\n", commands[0]);
		  exit(1);
              perror("exec");
            }
          }
          else if (pipe_fr == true) // here we are getting input from a file
          {
               dup2(fd[0], 0);
               close(fd[1]);
               close(fd[0]); 
      
            if (execvp( commands[0], commands) == -1 )
            {
                  fprintf(stderr, "Error %s command not recognized\n\n", commands[0]);
		  exit(1);
              perror("exec");
            }
      
          }
          else 
            { // no piping involved for simple execution

		
              if (execvp( commands[0], commands) == -1 )
              {
                  fprintf(stderr, "Error %s command not recognized\n\n", commands[0]);
		  exit(1);
              }
            }
        }





        else {		// main's parent thread
          if (pipe_to == true  && pipechar != '>')// writing to a file
          {
             pid=fork(); // fork main's parent again 
             if(pid == 0 )
             {
               dup2(fd[0], 0);
               close(fd[1]);
               close(fd[0]); 
               commands = commands +  pipe_index;

	       if (commands [1] != NULL)
               {
               // many arguements 
                 if (execvp( commands[0], commands) == -1 )
                 {
                  fprintf(stderr, "Error %s command not recognized\n\n", commands[0]);
		  exit(1);
                 }
               }
               else 
               {
               // no arguements
                 if (execlp(commands[0],commands[0],(char*)NULL) == -1 )
                 {
                  fprintf(stderr, "Error %s command not recognized\n\n", commands[0]);
		  exit(1);
                 }
               }  
             }
             else // wait for children to end and then close up 
             {
               close(fd[0]);
               close(fd[1]);
               wait(NULL);

             }
          }
      
          else if (pipe_to == true &&  pipechar == '>')
          { // command is piped to file --> see child
            // no need to close file descriptor because 
            // it is done at exec's thread. via FD_CLOEXEC
               close(fd[0]);
               close(fd[1]);
//            wait(NULL);

	    status = 0;
	    childpid = wait(&status);
	    child_return_value = WEXITSTATUS(status);
	    if (child_return_value == 1){return 0;}



          }
          else if (pipe_fr == true)// pipe file to child
          {
             pid=fork();
             if(pid == 0 )
             {
               dup2(fd[1], 1);
               close(fd[0]);
               close(fd[1]);
               //#cat here  
               if (execlp("cat","cat",commands[count-1],(char*)NULL) == -1 )
               {
                  fprintf(stderr, "Error %s command not recognized\n\n", commands[0]);
		  exit(1);
                 perror("exec");
               }
             }
             else 
             {
               close(fd[0]);
               close(fd[1]);
//               wait(NULL);


	        status = 0;
	        childpid = wait(&status);
	        child_return_value = WEXITSTATUS(status);
	        if (child_return_value == 1){return 0;}
             }
          }
          else { // simple command no pipeing for parent so wait

             //wait(NULL);
             status = 0;
             childpid = wait(&status);
             child_return_value = WEXITSTATUS(status);
             if (child_return_value == 1){return 0;}

          }
          if (pipe_to == true || pipe_fr == true )// wait for main's child, not main's child's child... whew
          {
             wait(NULL);
          }
        }
     }  

  }

  

  return 0;
}





