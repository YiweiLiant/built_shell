#ifndef  __PIPECOMMAND_H__
#define  __PIPECOMMAND_H__
#include<unistd.h>
#include<functional>
#include<stdarg.h>
#include<sys/stat.h>
#include<dirent.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<errno.h>
#include<stdio.h>
#include<iostream>
#include<string>
#include<cstring>
#include<stdlib.h>
#include<vector>
#include<map>
#include<fcntl.h>
#include<algorithm>
#include"CommandLine.h"

void printShellLine();

struct _pipe_t{
  /*this struct hold a int[2] array which will be used as pipe*/
  int pd[2];
};
typedef _pipe_t pipe_t;

class pipeCommand{
  /*this class hold a vector of commands*/
private:
  std::vector<CommandLine> commands;

  bool getAllCommand();
    /* this method would check all the CommandLine in commands 
     * if the argv[0] of all of them are exist 
     * if not an error would be print
     */

  void PipeAndRun();
    /* run all the command using pipe if all of them exist 
     * this method would fork multi time and run all the command in vectors commands at the same time
     * and creat several pipes to link all the command 
     */

public:
  pipeCommand(std::string input,
	      std::map<std::string,std::string> * varMap);
    /* this constructor takes an input and a pointer to a map
     * CommandLine would be creat according to the input and varMap
     * if the argv in that CommandLine is not empty it would be appended to commands
     */

  pipeCommand(const pipeCommand &rhs);
    /* copy constrctor*/
 

  pipeCommand & operator=(const pipeCommand & rhs);
 
  void RunPipe();
    /* this method would run the all the command in vector commands
     * Then print the myShell line after it
     */

  bool empty();
    /*return true if commands is empty*/
  
  ~pipeCommand(){}
  //destrctor


  
};

#endif
