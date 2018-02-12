#include<stdio.h>
#include<iostream>
#include<functional>
#include<string>
#include<cstring>
#include<fstream>
#include<vector>
#include"CommandLine.h"
#include"pipeCommand.h"

int main(void){
  printShellLine();
  std::map<std::string,std::string> varMap;               // the map hold variables
  do{
    std::string commands;
    std::getline(std::cin,commands);                      //get command line
    if(commands.compare("exit") == 0 || std::cin.eof()){  //exit conditions
      return EXIT_SUCCESS;
    }
    else{
      pipeCommand tempCom(commands,&varMap);              // set commands
      if(tempCom.empty()){                                //if empty print line and continue
	printShellLine();
	continue;
      }
      tempCom.RunPipe();                                  //run commands
    }
  }while(!std::cin.eof());
}
