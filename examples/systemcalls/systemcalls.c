#include "systemcalls.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{
  bool b_result = false;
  /*
   * TODO  add your code here
   * Call the system() function with the command set in the cmd
   *   and return a boolean true if the system() call completed with success
   *   or false() if it returned a failure
   */

  int ret = system(cmd);

  if(ret == -1)
  {
    perror("do_system, ");   
  } 
  else
  {
    if(WEXITSTATUS(ret) == EXIT_SUCCESS)
    {
      b_result = true;
    }
  }
  return b_result;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    char *p_path = NULL;
    int i;
    pid_t pid;
    bool b_result = false;
    char *p_char = NULL;
      
    for (i = 0; i < count; i++)
    {
      if(i==0)
      {
        p_path = va_arg(args, char *);

        p_char = strrchr(p_path,'/');
        if(NULL != p_char)
        {
          command[i] = (p_char + 1);
        }
        else
        {
          command[i] = p_path;
        }
      }
      else
      {
        command[i] = va_arg(args, char *);
      }
    }
    command[count] = NULL;
    
/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

    va_end(args);
       
    pid = fork();
    
    if(pid == -1)
    {
      perror("do_exec, fork()");
      return false;
    }
    else if(pid == 0)
    {
      /* The child process */
      int status = execv(p_path,command);

      if(status == -1)
      {
        perror("do_exec, execv(), ");
      }
      exit(status);
    }
    else
    {
      int child_status;
      /* Parent process */
      pid_t this_pid = waitpid(pid,&child_status,0);
      if(this_pid == -1)
      {
        perror("do_exec, wait(), ");
      }
      else if(child_status == 0)
      {
        b_result = true;
      }
      
    }
    return b_result;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will b-e closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    char *p_path = NULL;
    char *p_char = NULL;
    bool b_result = false;
    char *p_redirect_msg = NULL;
    
    pid_t pid;

    for (i = 0; i < count; i++)
    {
      if(i == 0)
      {
          p_path = va_arg(args, char *);

          p_char = strrchr(p_path, '/');
          if (NULL != p_char)
          {
            command[i] = (p_char + 1);
          }
          else
          {
            command[i] = p_path;
          }
      }
      else
      {
        command[i] = va_arg(args, char *);
      }
    }
    va_end(args);
  
    command[count] = NULL;
    

/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

    pid = fork();
    
    if (pid == -1)
    {
      perror("do_exec, fork()");
    }
    else if (pid == 0)
    {
      /* The child process */
      int status = 0;
      int outfile_fd = open(outputfile, O_WRONLY | O_TRUNC | O_CREAT,
                            S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH);

      if(outfile_fd < 0)
      {
        perror("Failed to open file, ");
        return false;
      }

      if(dup2(outfile_fd, 1) <= 0)
      {
          perror("do_exec_redirect, failed redirection, ");
          return false;
      }
      close(outfile_fd);

      status = execv(p_path, command);

      if (status == -1)
      {
        perror("do_exec_redirect, execvp(), ");
      }
      exit(status);
    }
    else
    {
        int child_status;
        /* Parent process */
        
        pid_t this_pid = waitpid(pid, &child_status, 0);

        if (this_pid == -1)
        {
            perror("do_exec, wait(), ");
        }
        else if (WIFEXITED(child_status))
        {
          printf("child_status:%d\n",child_status);
          /* command terminated normally, check exit status */
           if (WEXITSTATUS(child_status) == EXIT_SUCCESS)
           {
             b_result = true;
           }
        }
        free(p_redirect_msg);
    }
    return b_result;
}
