//#various include directives
#include <unistd.h>
#include <limits.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <signal.h>
bool mycomp (char c1, char c2)
{ return std::tolower(c1)<std::tolower(c2); }
struct split
{
};

template <typename Container>
Container& split(
  Container& result,
  const typename Container::value_type& s,
  const typename Container::value_type& delimiters)
{
  result.clear();
  size_t current;
  size_t next = -1;
  do
  {
    {
      next = s.find_first_not_of( delimiters, next + 1 );
      if (next == Container::value_type::npos) break;
      next -= 1;
    }
    current = next + 1;
    next = s.find_first_of( delimiters, current );
    result.push_back( s.substr( current, next - current ) );
  }
  while (next != Container::value_type::npos);
  return result;
}

void ls( const std::string &dirName, std::string opt = "" )
{
DIR *dirp;

if(dirName.empty())
    dirp = opendir( "." );
  else
  dirp = opendir( dirName.c_str() );

  if ( dirp )
  {
    struct dirent *dp = NULL;

    while ( (dp = readdir( dirp )) != NULL )
    {
        std::string file( dp->d_name );

        if ( file == "." || file == ".." )    // skip these
          continue;

        if ( (dp->d_type & DT_DIR) && strcmp(opt.c_str(),std::string("-R").c_str()) == 0 )
        {
          // found a directory; recurse into it.
          std::string filePath = dirName + "/" + file;
          std::cout << filePath << std::endl;

          ls( filePath, "-R" );
        }
        else
        {
          // regular file found
          std::cout << "filename is: " << file << std::endl;
        }
    }

    closedir( dirp );
  }
}

using namespace std;



extern int errno;		// you may want these.
extern char **environ;

// Possible function definitions omitted here

//***************************************************************
// Print a prompt, get user input, then depending on what
// they typed, either use a built-in with a syscall, or
// call exec.
//***************************************************************
int main(void) {

  char cmdline[100];		// An input line.
  char *token;			// Used for parsing input lines
				// using strtok etc.
  char outbuf[100];		// Generic print buffer
  char *argv[256];		// The argument vector.  User-typed.
  int argc;			// The argument count.
  pid_t pid;			// PID from fork.
  int stat;			// Used by waidpid.
  bool done = false;
  int counter = 0, ret;
  vector<string> cmd;
  string cmdstr;



// You may wish to have some sort of "initialize argv" loop here.

  while (!done) {			// Until the user types exit...

// Similarly, you may wish to have some sort of "clean up argv"
// loop through here.  Reset pointers to null.  Call delete[] on any
// that are not null.

    cout << "% ";			// Print a prompt to the user

    cin.getline(cmdline,100);
    cmdstr = cmdline;

      if (strcmp(cmdline,"exit") == 0)	// They typed exit, so do that.
        done = true;
      else {				// They didn't type exit.
        // Start using strtok to parse the string into component
	// parts.  These get "placed into" (pointed-at by) argv[i].
    split(cmd,cmdstr," ");

	if(strcmp(cmd[0].c_str(),string("cd").c_str()) == 0){
	auto it = find_if_not(cmd.begin(),cmd.end(),[](string i){ char pch[]="cd"; return lexicographical_compare(i.begin(),i.end(),pch,pch+1,mycomp);});
	if(it == cmd.end() )
	getenv("HOME");
    else{
    ret = chdir(cmd[1].c_str());
    getcwd(outbuf,100);
    printf("%s",outbuf);
    }
    }

    if(strcmp(cmd[0].c_str(),string("mkdir").c_str()) == 0)
    {
    if(find(cmd.begin(),cmd.end(),"-m") != cmd.end())
    {
    mkdir(cmd[3].c_str(), umask(stoi(cmd[2])));
    }else
    mkdir(cmd[1].c_str(), S_IRWXU);

    }


    if(strcmp(cmd[0].c_str(),string("pwd").c_str()) == 0)
        {
        getcwd(outbuf,100);
        printf("%s",outbuf);
        }

    if(strcmp(cmd[0].c_str(),string("date").c_str()) == 0)
    {
    time_t rawtime;
  struct tm * timeinfo;

  time (&rawtime);
  timeinfo = localtime (&rawtime);
  printf ("%s", asctime(timeinfo));
     }


  // This is where we check to see what it is they typed.
  // If it's one of the built-ins, then fork a child to do
  // it (unless it was cd).

//          stuff the parent does, including fork.

          if (fork() == 0) { // I am the child

            /// do various child stuff involving implementing the
            /// built-ins, OR using exec to execute a command I do
            /// not have built-in.
    if(strcmp(cmd[0].c_str(),string("ls").c_str()) == 0)
    {
        if(cmd.size() == 1)
        {
        getcwd(outbuf,100);
        //printf("%s",outbuf);
        ls(string(outbuf));
        }
        else if(find(cmd.begin(),cmd.end(),"-R") != cmd.end() && cmd.size() > 2)
        ls(cmd[2], cmd[1]);
        else if(find(cmd.begin(),cmd.end(),"-R") != cmd.end() && cmd.size() == 2)
        {
        getcwd(outbuf,100);
        ls(outbuf,cmd[1]);
        }
    }

            if(strcmp(cmd[0].c_str(),string("hostname").c_str()) == 0)
     {
     char hostname[HOST_NAME_MAX];
     gethostname(hostname, HOST_NAME_MAX);
    printf("%s\n", hostname);
     }

     if(strcmp(cmd[0].c_str(),string("rmdir").c_str()) == 0)
    {
    rmdir(cmd[1].c_str());

    }

    if(strcmp(cmd[0].c_str(),string("kill").c_str()) == 0){
     if(cmd.size() > 2)
     kill(stoi(cmd[1].c_str()), stoi(cmd[2].c_str()));
     else if(cmd.size() == 2)
     kill(stoi(cmd[1].c_str()),9);
    }
	if(strcmp(cmd[0].c_str(),string("echo").c_str()) == 0)
     {
     cout << cmd[1] << endl;
     }



          }		// end if (I am child)
          else {
          // Wait for child to terminate
            // (call wait or waitpid here)
          }
        }
        counter = 0;				// End of kill processing
  }					// End while (!done)
  return 0;
}
