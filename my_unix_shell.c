#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>


int detecta (int argc, char **argv) {
    int i, j;
    for(i=0; i<argc; i++) {
    	for(j=0; j<2; j++){
            if(argv[i][j] == '|' && argv[i][j+1] != '|') return 2;
            else if(argv[i][j] == '|' && argv[i][j+1] == '|') return 3;
            else if(argv[i][j] == '&' && argv[i][j+1] == '&') return 4;
            else if(argv[i][j] == '&' && argv[i][j+1] != '&') return 5;
        }
    }

    return 1;
}

int achaPos (int argc, char ** comandos, char * alvo){
    int ret = -1;
    int i;
    char *token = NULL;
    for (i=0; i<argc; i++) {
        token = strtok(comandos[i], alvo);
        if (token == NULL) {
            return (i);
        }
    }
    return (ret);
}

int pipe_count (char **argv) {
	int count = 0;
		for (int i = 0; argv[i]; i++) {
			if (argv[i][0] == '|') {
				count++;
			}
		}
	return count;
}

int cond_count (char **argv) {
  int count = 0;
  for (int i = 0; argv[i]; i++) {
    if ((argv[i][0] == '|' && argv[i][1] == '|') || (argv[i][0] == '&' && argv[i][1] == '&')) {
      count++;
    }
  }
  return count;
}

int exec_pipe (int argc, char **argv) {
  int pipe_num = pipe_count(argv);
  int pipe_file_descriptors[pipe_num][2];
  pid_t children[pipe_num + 1];
  char **commands[pipe_num + 1];
  char ops[pipe_num];
  
  commands[0] = &argv[1];
  

  int j = 1;
  for (int i = 0; argv[i]; i++) {
    if (argv[i][0] == '|') {
      ops[j - 1] = argv[i][0];
      commands[j++] = &argv[i + 1];
      argv[i] = NULL;
    }
  }
  
   // Executamos um pipe
  if (pipe_num == 0) {
    children[0] = fork();
    
    if (children[0] < 0) {
      perror("Single fork");
      return -1;
    }
    else if (children[0] == 0) {
      if (execvp(commands[0][0], commands[0])) {
        perror("Single execution");
        return -1;
      }
    }
    else {
      waitpid(children[0], NULL, 0);
    }
  
  }
  
  // Mais pipes
  if (pipe_num > 0) {
    if (pipe(pipe_file_descriptors[0]) < 0) {
      perror("Two argument Pipe");
      return -1;
    }
    
    children[0] = fork();
    
    if (children[0] < 0) {
      close(pipe_file_descriptors[0][0]);
      close(pipe_file_descriptors[0][1]);
      perror("Fork child 1");
      return -1;
    }
    else if (children[0] == 0) {
      close(pipe_file_descriptors[0][0]);
      dup2(pipe_file_descriptors[0][1], STDOUT_FILENO);
      if (execvp(commands[0][0], commands[0])) {
        close(pipe_file_descriptors[0][1]);
        perror("Execution child 1");
        return -1;
      }
    }
    else {
      close(pipe_file_descriptors[0][1]);
      waitpid(children[0], NULL, 0);
    }
    
    for (int i = 0; i < pipe_num; i++) {
      if (i + 1 < pipe_num) {
        if (pipe(pipe_file_descriptors[i + 1]) < 0) {
          char *error_string = NULL;
          sprintf(error_string, "Pipe %d", i + 1);
          perror(error_string);
        }
      }
      
      if (ops[i] == '|') {
        children[i + 1] = fork();
        
        if (children[i + 1] < 0) {
          char *error_string = NULL;
          sprintf(error_string, "Fork child %d", i + 1);
          perror(error_string);
          close(pipe_file_descriptors[i][0]);
          if (i + 1 < pipe_num) {
            close(pipe_file_descriptors[i + 1][0]);
            close(pipe_file_descriptors[i + 1][1]);
          }
          return -1;
        }

        else if (children[i + 1] == 0) {
          close(pipe_file_descriptors[i][1]);
          if (i + 1 < pipe_num) {
            close(pipe_file_descriptors[i + 1][0]);
          }
          dup2(pipe_file_descriptors[i][0], STDIN_FILENO);
          if (i + 1 < pipe_num) {
            dup2(pipe_file_descriptors[i + 1][1], STDOUT_FILENO);
          }
          if (execvp(commands[i + 1][0], commands[i + 1])) {
            char *error_string = NULL;
            sprintf(error_string, "Fork child %d", i + 1);
            perror(error_string);
            close(pipe_file_descriptors[i][0]);
            if (i + 1 < pipe_num) {
              close(pipe_file_descriptors[i + 1][0]);
              close(pipe_file_descriptors[i + 1][1]);
            }
            return -1;
          }
        }
        else {
          close(pipe_file_descriptors[i][0]);
          if (i + 1 < pipe_num) {
            close(pipe_file_descriptors[i + 1][1]);
          }
          waitpid(children[i + 1], NULL, 0);
        }
      }
      
    }
    
  }
  
  return 0;
}

int exec_bg (int argc, char **argv) {
  	int pipe_num = pipe_count(argv);
  	char answer[1000];
  	pid_t children[pipe_num + 1];
  	char **commands[pipe_num + 1];
  	
  	int pid;
  	int fd[2];
  	int i;
  	
	for (i=0; argv[i]; i++) 
		if (argv[i][0] == '&') 
			argv[i] = NULL;

  	
  	commands[0] = &argv[1];
  	
  	pipe(fd);
  	
  	pid = fork();
  	
  	if (pid < 0) {
      perror("Single fork");
      return -1;
    }
    else if (pid == 0) {
    	dup2(fd[1], STDOUT_FILENO);
      	if (execvp(commands[0][0], commands[0])) {
        	perror("Single execution");
        	return -1;
      	}
    }
    else {
      while (1) {
	      waitpid(0, NULL, WNOHANG);
	      read(fd[0], answer, 1000*sizeof(char*));
	      printf("\n\nResposta do BG:\n%s", answer);
    	}
    }
  
  return 0;
}

int manda_pra_exec_bg (int argc, char **argv) {
  	int pid;
  	int fd[2];

  	pipe(fd);
  	
  	pid = fork();
  	
  	if (pid < 0) {
      perror("Single fork");
      return -1;
    }
    else if (pid == 0) {
    	exec_bg(argc, argv);
    }

	return 0;
}

int exec_cond (int argc, char **argv) {
  	int cond_num = cond_count(argv);
  	pid_t children[cond_num + 1];
  	char **commands[cond_num + 1];
  	char ops[cond_num];
  	int pos[cond_num];
  	int pid, pid2;
  	int fd[2];
	char answer[1000];
  	int j = 1, i, k;
  	
	for (i = 0; argv[i]; i++) {
		if ((argv[i][0] == '&' && argv[i][1] == '&') || 
		(argv[i][0] == '|' && argv[i][1] == '|')) {
			ops[j - 1] = argv[i][1];
			commands[j++] = &argv[i + 1];
			argv[i] = NULL;
		}
	}


  	
  	commands[0] = &argv[1];
  	
  	for (i=0; i<cond_num; i++) {
  	
  		pipe(fd);
	  	pid = fork();
	  	
	  	if (pid < 0) {
		  perror("Fork cond");
		  return -1;
		}
		else if (pid == 0) {
			close(fd[0]);
			dup2(fd[1], STDOUT_FILENO);
		  	if (execvp(commands[i][0], commands[i])) {
		    	perror("Cond execution check");
		    	return -1;
		  	}
		}
		else {
			close(fd[1]);
			int status;
		  	waitpid(0, &status, 0);

		  	if (ops[i] == '&'){
		  		pid2 = fork();
		  		if (pid2 < 0) {
				  perror("Fork cond");
				  return -1;
				}
				else if (pid2 == 0) {
					if (status == 0){
					  	if (read(fd[0], answer, 1000*sizeof(char*))<0) {
					  		return -1;
						}
						close(fd[0]);
						printf("%s", answer);
						exit(0);
					}
					exit(1);
				}
				else {
					wait(NULL);
					if (status == 0 && i+1 == cond_num) {
						if (execvp(commands[i+1][0], commands[i+1])) {
							perror("And execution left");
							return -1;
					  	}
					}
					if (status != 0){
						exit(1);
					}
				}

		  	}
			else if (ops[i] == '|'){
				  if (status != 0 && i+1 == cond_num) {
				  	if (execvp(commands[i+1][0], commands[i+1])) {
				  		perror("And execution left");
				  		return -1;
				    	}
					exit(1);
				  }	
				  if (status == 0){
				    	if (read(fd[0], answer, 1000*sizeof(char*))<0) {
				    		return -1;
				  	}
				  	close(fd[0]);
				  	printf("%s", answer);
				  	exit(0);
				  }
		  	}
		  	
		  	////////////////////
		  	

		  	
		}
    }

  
  
  return 0;
}



int main (int argc, char **argv) {
	int tipo;
  
	if (argc == 1) {
		printf("Use os comandos diretamente na chamada\n");
	}
  
  	tipo = detecta(argc, argv);

	//printf("detec %d\n", tipo);

  	switch (tipo){
  		case 1: // Comando unitario
  			execvp(argv[1], &argv[1]);
  			break;
  		case 2: // Pipes
  			exec_pipe(argc, argv);
  			break;
  		case 3: // Comandos condicionais
  		case 4: 
  			exec_cond(argc, argv);
  			break;
  		case 5:	// Background
  			manda_pra_exec_bg(argc, argv);
  			break;
  		default:
  			break;
  	}
  
  
  
  return 0;
}



