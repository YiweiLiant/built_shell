#include "CommandLine.h"

/* this function takes the reference of a string and a reference to its iterator
 * this function would delete all the space right after it in the str
 */
void eraseSpace(std::string::iterator & it,std::string & str){
  while(*it == ' '){
    str.erase(it);
  }
}

pid_t parentWait(pid_t pid){
    /*this method is used to wait process to end and print its status when the process terminate*/
    //this code from man waitpid
  pid_t w;
  int status;
  do{
    w = waitpid(pid,&status,WUNTRACED|WCONTINUED);
    if(w == -1){
      return -1;
    }
    if(WIFEXITED(status)){
      std::cout<<"Program exited with status "<<WEXITSTATUS(status)<<"\n";
    }
    else if(WIFSIGNALED(status)){
      std::cout<<"Program was killed by signal "<<WTERMSIG(status)<<"\n";
    }
  }while(!WIFEXITED(status) && !WIFSIGNALED(status));
  return w;
}

/* this method take a string and find it in varMap and env
 * if there is a var exist return the value of it accordingly
 * if not, return a empty string
 */
std::string CommandLine::findValue(std::string var){
  std::string value;
  if((*varMap).find(var) == (*varMap).end()){
    if(getenv(var.c_str()) != NULL){
      value = std::string (getenv(var.c_str()));
    }
    else{
      std::cerr<<"variable "<<var<<" have not been initialized\n";
	return value;
    }
  }
  else{
    value = (*varMap).find(var)->second;
  }
  return value;
}

/* this method take a string and find if it protentially contains a variable
 * if yes find the value of all the variable and 
 * return a string that change all the variable part of it
 * if variable do not exsit then skip its part and print error
 * if no return original str
 */     
std::string CommandLine::getValue(std::string str){
  if(str.find("$") == std::string::npos){
    return str;
  }
  std::string::iterator varStart, varEnd;
  varStart = str.begin() + str.find("$")+1;
  varEnd = varStart;
  while(((*varEnd>='a'&&*varEnd<='z')
	 ||(*varEnd>='A'&&*varEnd<='Z')
	 ||(*varEnd>='0'&&*varEnd<='9')
	 ||(*varEnd == '_'))&&(varEnd != str.end())){
    varEnd ++;
  }
  std::string before,var,follow;   
  before.assign(str.begin(),varStart-1);      //assign[begin,end)
  follow.assign(varEnd,str.end());
  var.assign(varStart,varEnd);
  if(var.empty()){
    std::cerr<<"no valid variable followed by $\n";
      str = before + follow;
  }
  else{
    str = before + findValue(var) + follow;
  }
  return getValue(str);                      //do recursion until no '$' exist
}


/* this method take two iterator of a string 
 * and add the string between those two argv
 * if there is "\ "in that part, the '\' would be deleted
 */
void CommandLine::addargv(std::string::iterator itStart,std::string::iterator itEnd){
  std::string tempArg;
  tempArg.assign(itStart,itEnd);
  for(size_t found = tempArg.find("\\ ",0);
      found!= std::string::npos;
      found=tempArg.find("\\ ",found)){
    tempArg.erase(found,1);
  }
  argv.push_back(tempArg);
}
    
/* this method separate command to parts by ' ' and add to argv
 * while '\ ' would be count as ' ' and literally included in argv
 */
void CommandLine::getArgv(){
  std::string::iterator itEnd = command.begin();
  eraseSpace(itEnd,command);
  std::string::iterator itStart = itEnd;
  while(itEnd != command.end()){
    if((*itEnd == ' ') && (*(itEnd - 1)!='\\')){
      addargv(itStart,itEnd);
      itEnd ++;
      eraseSpace(itEnd,command);
      itStart = itEnd;
    }
    if(itEnd != command.end()){
      itEnd ++;
    }
  }
  if(itStart != itEnd){
    addargv(itStart,itEnd);
  }
}
    
/* this method takas a string(could be empty) and get files from it
 * if there is unexpected part, an error would be print and skip the rest part
 * note that only string after the last '>''<'or'2>'would count as filename
 */   
void CommandLine::getfiles(std::string files){
  std::string::iterator itStart = files.begin();
  eraseSpace(itStart,files);
  if(itStart == files.end()){
    return;                    //base case of recursion
  }
  std::string::iterator itEnd = itStart + 1;
  if(*itStart == '2'&& *itEnd == '>'){
    itEnd ++;
  }
  eraseSpace(itEnd,files);
  while((*itEnd != '<') && !(*itEnd == '>' && *(itEnd-1)!='2')&&
	!(*itEnd == '2' && *(itEnd+1)== '>')&&
	(*itEnd!=' ') && (itEnd!=files.end())){
      itEnd ++;
  }
  switch(*itStart){
  case '<':inFile.assign(itStart+1,itEnd);          break;
  case '>':outFile.assign(itStart+1,itEnd);         break;
  case '2':
    if(*(itStart+1) == '>'){
      errFile.assign(itStart+2,itEnd);
      break;
    }
  default:
    std::cerr<<"cannot getfiles from "<<files<<"\n";
    return;
  }
  std::string rest;
  rest.assign(itEnd,files.end());
  getfiles(rest);                  //do recursion to get all the files
}

/* this function take a directory name as input and find if argv[0] is inside it
 * if yes the argv[0] would be change to full path and return ture, else return false
 */     
bool CommandLine::findInDir(std::string dir){
  if(dir.empty() || dir[0] != '/'){
    std::cerr<<"cannot find root dir "<<dir<<" \n";
    return false;
  }
  DIR * dp = opendir(dir.c_str());
  if(dp == NULL){
    std::cerr<<"cannot find dir "<<dir<<"\n";
      return false;
  }
  struct dirent * dirp;
  while((dirp = readdir(dp))){
    if(strcmp(dirp->d_name,argv[0].c_str()) == 0){
      argv[0] = dir + "/" +argv[0];
      closedir(dp);
      return true;
    }
    }
  closedir(dp);   
  return false;
}

/* this method would search all the paths in PATH environment variable to find argv[0]
 * if be found, argv[0] would be change to full path of the command and return true
 * else return false   
 */  
bool CommandLine::findCommand(){
  std::string paths(getenv("PATH"));
  while(!paths.empty()){
    if(findInDir(paths.substr(0,paths.find(":")))){
      return true;
    }
    if(paths.find(":") == std::string::npos){
      paths.erase();
    }
    else{
      paths.erase(0,paths.find(":")+1);
    }
  }
  return false;
}
  
/*run the cd command to change current directory*/
void CommandLine::RunCd(){
  if(argv[0].compare("cd") != 0){           //if this occur must be something really bad happen
    std::cerr<<"unknown error in RunCd\n";
    exit(EXIT_FAILURE);
  }
  if(chdir(argv[1].c_str()) == -1){
      std::cerr<<argv[1]<<" : no such directory\n";
  }
}

/* add a pair of var and value into varMap 
 * if var already exist, update its value
 */
void CommandLine::addToMap(std::string var,std::string value){
  std::map<std::string,std::string>::iterator it = (*varMap).find(var);
  if(it != (*varMap).end()){
    it->second  = value;
    }
  else{
    (*varMap).insert(std::pair<std::string,std::string>(var,value));
  }
}


/* this method check whether var is a valid variables
 * which only have letters underscores and numbers
 * if so return true else return false
 */
bool CommandLine::validVar(std::string var){
  std::string::iterator it;
  for(it = var.begin(); it != var.end();++it){
    if(!((*it>='a'&&*it<='z')||(*it>='A'&&*it<='Z')||(*it>'0'&&*it<'9')||(*it == '_'))){
      return false;
    }
  }
  return true;
}


/* run the built in command set to set map
 * if the variable name is invalid an error would be printed
 * NOTE: if the value or var have "$" inside it,
 * that part will be treat as variable and try to find its value
 */
void CommandLine::RunSet(){
  if(argv[0].compare("set") != 0){
    std::cerr<<"unknown error in RunSet\n";
    exit(EXIT_FAILURE);                     //if this occur must be something really bad happen 
  }
  if(argv.size() < 2){
      std::cerr<<"invalid use of set: set var (value)\n";
      return;
    }
  if(!validVar(argv[1])){
    std::cerr<<"invalid variable: only letters,underscores and numbers\n";
    return;
  }
  if(argv.size() >= 2){
    std::string value;
    value = line.substr(line.find(argv[0]) + argv[0].length());
    value = value.substr(value.find(argv[1]) + argv[1].length());
    std::string::iterator it = value.begin();
    eraseSpace(it,value);
    addToMap(argv[1],value);
  }
}

/* run the built in command export to edit envirnment
 * if the variable have not been set an error would be print
 */
void CommandLine::RunExport(){
  if(argv[0].compare("export") != 0){
    std::cerr<<"unknown error in RunExport\n";
    exit(EXIT_FAILURE);                //if this occur must be something really bad happen
  }
  if(argv.size() != 2){
    std::cerr<<"invalid use of export: export var\n";
    return;
  }
  std::map<std::string,std::string>::iterator find;
  std::string var(argv[1]);
  find = (*varMap).find(var);
  if(find == (*varMap).end()){
    std::cerr<<"variable "<<var<<" have not been initialized\n";
  }
  else{
    std::string value(find->second);
    if(setenv(var.c_str(),value.c_str(),1) != 0){
      std::cerr<<"fail to update environment\n";
    }
  }
}


/* this function turns std::vector<std::string> argv into std::vector<char *> 
 * so it can be used in execv()
 */  
std::vector<char *> CommandLine::getArgChar(){
  std::vector<char *> argChar;
  for(size_t i = 0;i < argv.size();i++){
    char * temp = new char[argv[i].length() + 1];
    strcpy(temp,argv[i].c_str());
    argChar.push_back(temp);
  }
  argChar.push_back(NULL);
    return argChar;
}


/* this function free the spase created by getArgChar*/ 
void CommandLine::freeArgChar(std::vector<char *> argChar){
  for(size_t i = 0;i < argChar.size();i++){
    delete[] argChar[i];
  }
}



/* this function take a file name and the type(a int number)
 * and use dup2 to duplicate a file descriptor accordingly if file is not empty
 */
void CommandLine::getfile(std::string file,int n){
  if(!file.empty()){
    int fd;
    switch(n){
    case 0: fd = open(inFile.c_str(),O_RDONLY);
      break;
    case 1: fd = open(outFile.c_str(),O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU);
      break;
    case 2: fd = open(errFile.c_str(),O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU);
    }
    if(fd > 0){
      dup2(fd,n);
	close(fd);
    }
    else{
      std::cerr<<"cannot open "<<file<<"\n";
    }
  }
}

/* run the command in argv[0] using all the argument in argv */
void CommandLine::RunCommand(){  
  pid_t pid;
  pid = fork();
  if(pid == -1){
    std::cerr<<"fail to fork"<<"\n";
    exit(EXIT_FAILURE);
  }
  else if(pid == 0){
    child();
    _exit(EXE_FAILURE);
  }
  else{
    while(parentWait(-1)!= -1);
  }
}



/* this constructor take the input string 
 * and a pointer to a map which contians the variable set by users
 * and if the input contains "<" or ">" or "2>" it would set 
 * command and filenames accordingly then initialize argc and argv
 */  
CommandLine::CommandLine(std::string input,
			 std::map<std::string,std::string> * _varMap
			 ):varMap(_varMap){
  if(varMap == NULL){
    std::cout<<"the map cannot be NULL\n"; //err handling
    exit(EXIT_FAILURE);                    //would be better if change varMap from a pointer to a reference (out of time)
  }
  input = getValue(input);
  command = input;
  line = input;
  size_t in, out, err;
  in = input.find("<");
  err = input.find("2>");
  out = input.find(">");
  if(!(in == std::string::npos && err == std::string::npos && out == std::string::npos)){
    size_t reDirect = std::min(std::min(in,err),out);
    command.assign(input.begin(),input.begin() + reDirect);
    std::string FilePart;
    FilePart.assign(input.begin() + reDirect,input.end());
    getfiles(FilePart);
  }
  getArgv();
}


/* this method would run the command*/
void CommandLine::Run(){
  if(argv.empty() || argv[0].empty()){
    std::cerr<<"cannot find argv\n";
  }
  else if(argv[0].compare("cd") == 0){
    RunCd();
  }
  else if(argv[0].compare("set") == 0){
    RunSet();
  }
  else if(argv[0].compare("export") == 0){
    RunExport();
  }
  else{
    if(!getCommand()){
      std::cerr<<"Command "<<argv[0]<<" not found\n";
    }
    else{
      RunCommand();
    }
  }
}

/* this method would check whether command exist or not
 *  if exist return true and change argv[0] to path name
 */  
bool CommandLine::getCommand(){ 
  if(argv.empty()||argv[0].empty()){
    return false;
  }
  if(argv[0].find("/")!=std::string::npos){
    if(access(argv[0].c_str(),F_OK)){
      return false;
    }
    else{
      return true;
    }
  }
  else{
    return findCommand();
  }
}


/* this function will execute a program according to argv
 * and the input output of it can be redirected accoring to the parameter
 * if parameter is 0 the program would be redirected according to the files 
 */
void CommandLine::child(int in, int out){
  std::vector<char *> argChar = getArgChar();
  if(in == 0){
    getfile(inFile,0);
  }
  else{
    dup2(in,0);
    close(in);
  }
  if(out == 0){
    getfile(outFile,1);
  }
  else{
    dup2(out,1);
      close(out);
  }
  getfile(errFile,2);
  execv(argv[0].c_str(),&argChar[0]);   //or execve(argChar[0],&argChar[0],environ)
  freeArgChar(argChar);    
}

/* return true if argv is empty*/ 
bool CommandLine::empty(){
    return (argv.empty());
}
  
std::string CommandLine::returnCommand(){
  /*return argv[0]*/
  return argv[0];
}


  
  
CommandLine::CommandLine(const CommandLine & rhs):argv(rhs.argv),
						  varMap(rhs.varMap),
						  command(rhs.command),
						  line(rhs.line),
						  inFile(rhs.inFile),
						  outFile(rhs.outFile),
						  errFile(rhs.errFile){
  /* copy constrctor*/
  }

CommandLine & CommandLine::operator= (const CommandLine & rhs){
    /* assignment operator (reuse copy constructor) */
  if(this != &rhs){
      CommandLine temp(rhs);
      std::swap(argv,temp.argv);
      std::swap(varMap,temp.varMap);
      std::swap(command,temp.command);
      std::swap(line,temp.line);
      std::swap(inFile,temp.inFile);
      std::swap(outFile,temp.outFile);
      std::swap(errFile,temp.errFile);
  }
    return *this;
}


  
