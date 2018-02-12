#include "pipeCommand.h"

void printShellLine(){
  /*this funciton would print the myShell line*/
  char * pathBuff;
  pathBuff = get_current_dir_name();
  std::cout<<"myShell:"<<pathBuff<<" $ ";
  free(pathBuff);
}

/* this method would check all the CommandLine in commands 
 * if the argv[0] of all of them are exist 
 * if not an error would be print
 */
bool pipeCommand::getAllCommand(){
  for(size_t i = 0;i < commands.size();i++){
    if(!commands[i].getCommand()){
      std::cerr<<"Command "<<commands[i].returnCommand()<<" not found\n";
      return false;
    }
  }
  return true;
}


 /* run all the command using pipe if all of them exist 
  * this method would fork multi time and run all the command in vectors commands at the same time
  * and creat several pipes to link all the command 
  */
void pipeCommand::PipeAndRun(){ 
  if(getAllCommand()){
    size_t CmdNum = commands.size();
    pipe_t * pipes = new pipe_t[CmdNum-1];
    pid_t * children = new pid_t[CmdNum];
    size_t i = 0;
    for(int in = 0;i<commands.size()-1;i++){
      pipe(pipes[i].pd);
      if((children[i] = fork()) == 0){
	close(pipes[i].pd[0]);
	commands[i].child(in,pipes[i].pd[1]);   //this child method will do execve
      }
      close(pipes[i].pd[1]);
      in = pipes[i].pd[0];
    }
    if((children[i] = fork())==0){
      commands[i].child(pipes[i-1].pd[0],0);
    }   
    else{
      while(parentWait(-1) != -1);   // wait untile all the child process exit (declear in CommandLine.h)
    }
    delete[] pipes;
      delete[] children;
  }
}



 /* this constructor takes an input and a pointer to a map
  * CommandLine would be creat according to the input and varMap
  * if the argv in that CommandLine is not empty it would be appended to commands
  */
pipeCommand::pipeCommand(std::string input,
			 std::map<std::string,std::string> * varMap){
  if(varMap == NULL){                     //err handling
    std::cout<<"please give me a map\n";  //would be better if change varMap from a pointer to a reference(out of time) 
    exit(EXIT_FAILURE);
  }
  while(!input.empty()){
    CommandLine temp(input.substr(0,input.find("|")),varMap);//note:if no "|" in it substr would be (0 to end of the string)
    if(!temp.empty()){                                       //append nonempty temp to commands
      commands.push_back(temp);
    }
    if(input.find("|") == std::string::npos){
      input.erase();
    }
    else{
      input.erase(0,input.find("|") + 1);
    }
  }
}

pipeCommand::pipeCommand(const pipeCommand &rhs):commands(rhs.commands){
  /* copy constrctor*/
}

pipeCommand & pipeCommand::operator=(const pipeCommand & rhs){
    /* assignment operator*/
  if(this != &rhs){
    commands = rhs.commands;
  }
  return *this;
}

 
/* this method would run the all the command in vector commands
 * Then print the myShell line after it
 */
void pipeCommand::RunPipe(){
  if(commands.empty()){
    std::cerr<<"no command input\n";
  }    
  if(commands.size() == 1){
    commands[0].Run();    //this may run build in command like set cd export
  }
  else{
    PipeAndRun();
  }
  printShellLine();
}

bool pipeCommand::empty(){
    /*return true if commands is empty*/
  return commands.empty();
}


