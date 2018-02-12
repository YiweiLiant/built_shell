#ifndef  __COMMANDLINE_H__
#define  __COMMANDLINE_H__
#include<unistd.h>
#include<functional>
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
#define EXE_FAILURE -1

void eraseSpace(std::string::iterator & it,std::string & str);
  /* this function takes the reference of a string and a reference to its iterator
   * this function would delete all the space right after it in the str
   */
pid_t parentWait(pid_t pid);
    /*this method is used to wait process to end and print its status when the process terminate*/
class CommandLine{
  /* this class has a vector of string to hole arguments
   * a pointers to a map which hole all the variable and value
   * five string: command(hold all the argument) 
   * the entire input line(used in set command) and three filename
   */
private:
  std::vector<std::string> argv;
  std::map<std::string,std::string> * varMap;
  std::string command;
  std::string line;
  std::string inFile;
  std::string outFile;
  std::string errFile;
  
  std::string findValue(std::string var);
    /* this method take a string and find it in varMap and env
     * if there is a var exist return the value of it accordingly
     * if not, return a empty string
     */
  std::string getValue(std::string str);
    /* this method take a string and find if it protentially contains a variable
     * if yes find the value of all the variable and 
     * return a string that change all the variable part of it
     * if variable do not exsit then skip its part and print error
     * if no return original str
     */   
  void addargv(std::string::iterator itStart,std::string::iterator itEnd);
    /* this method take two iterator of a string 
     * and add the string between those two argv
     * if there is "\ "in that part, the '\' would be deleted
     */
  void getArgv();
    /* this method separate command to parts by ' ' and add to argv
     * while '\ ' would be count as ' ' and literally included in argv
     */
  void getfiles(std::string files);
    /* this method takas a string(could be empty) and get files from it
     * if there is unexpected part, an error would be print and skip the rest part
     * note that only string after the last '>''<'or'2>'would count as filename
     */
  bool findInDir(std::string dir);
    /* this function take a directory name as input and find if argv[0] is inside it
     * if yes the argv[0] would be change to full path and return ture, else return false
     */   
  bool findCommand();
     /* this method would search all the paths in PATH environment variable to find argv[0]
     * if be found, argv[0] would be change to full path of the command and return true
     * else return false   
     */
  void RunCd();
    /*run the cd command to change current directory*/
  void addToMap(std::string var,std::string value);
    /* add a pair of var and value into varMap 
     * if var already exist, update its value
     */
  bool validVar(std::string var);
    /* this method check whether var is a valid variables
     * which only have letters underscores and numbers
     * if so return true else return false
     */
  void RunSet();
    /* run the built in command set to set map
     * if the variable name is invalid an error would be printed
     * NOTE: if the value or var have "$" inside it,
     * that part will be treat as variable and try to find its value
     */
  void RunExport();
    /* run the built in command export to edit envirnment
     * if the variable have not been set an error would be print
     */
  std::vector<char *> getArgChar();
    /* this function turns std::vector<std::string> argv into std::vector<char *> 
     * so it can be used in execv()
     */  

  void freeArgChar(std::vector<char *> argChar);
    /* this function free the spase created by getArgChar*/
   

  void getfile(std::string file,int n);
    /* this function take a file name and the type(a int number)
     * and use dup2 to duplicate a file descriptor accordingly if file is not empty
     */
  void RunCommand();
    /* run the command in argv[0] using all the argument in argv */
public:
  CommandLine(std::string input,
	      std::map<std::string,std::string> * _varMap
	      );
    /* this constructor take the input string 
     * and a pointer to a map which contians the variable set by users
     * and if the input contains "<" or ">" or "2>" it would set 
     * command and filenames accordingly then initialize argc and argv
     */
  void Run();
    /* this method would run the command*/
  bool getCommand();
    /* this method would check whether command exist or not
     *  if exist return true and change argv[0] to path name
     */
  void child(int in=0, int out=0);
    /* this function will execute a program according to argv
     * and the input output of it can be redirected accoring to the parameter
     * if parameter is 0 the program would be redirected according to the files 
     */
  bool empty();
    /* return true if argv is empty*/
  std::string returnCommand();
    /*return argv[0]*/
  CommandLine(const CommandLine & rhs);
  CommandLine & operator= (const CommandLine & rhs);
    /* assignment operator (reuse copy constructor) */
  ~CommandLine(){}    
	        
};

  





#endif
