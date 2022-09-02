#include <iostream>
#include <map>
#include <functional>
#include <algorithm>
#include <sys/types.h>
#include <grp.h>
#include <string>
#include <fstream>
#include "ace/ACE.h"
#include "ace/FILE_IO.h"
#include "ace/Handle_Set.h"
#include "ace/Hash_Map_Manager.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Record.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Stream.h"
#include "ace/Null_Mutex.h"
#include "ace/SOCK_Connector.h"
#include "ace/Truncate.h"
#include "ace/os_include/os_fcntl.h"
#include "ace/Event_Handler.h"
#include "ace/Reactor.h"
using namespace std;
#define SIZE_BUF 1024
class Cmd_Handle : public ACE_Event_Handler{
public:
  Cmd_Handle(ACE_HANDLE handle);
  int open();
  int handle();
private:
  typedef  _Bind<void (Cmd_Handle::*(Cmd_Handle *))()> handler;

  vector<string> parse(char *buf);
  //int check(string &cmd);
  void reply(char *buf,int n);
  ACE_HANDLE get_handle();
  int get_file_info(struct stat &sbuf,char *buf,int size);
  int send_list(string &path,ACE_SOCK_Stream &peer);
  void parse_path(const string &path,string &res)const;
  void get_father_dir(string &res)const;
  void pwd();
  void cwd();
  void user();
  void pass();
  void cdup();
  void size();
  void rmd();
  void quit();
  void mkd();
  void dele();
  void list();
  void pasv();
  void port();
  void stor();
  void retr();
  void type();
  void syst();
  void rnfr();
  void rnto();

  typedef enum{
    st_no,
    st_pasv,
    st_port
  }State;
  map<string,handler> cmd_handler_; 
  map<string,string> user_;
  vector<string> arg_;
  ACE_SOCK_Stream peer_;
  ACE_SOCK_Acceptor pasv_acceptor_;
  ACE_HANDLE handle_;
  string cwd_;
  string client_ip_;
  string server_addr_;
  string pre_;
  string username_;
  string rename_;
  u_short port_;
  bool stop_;
  bool login_;
  bool binary_;
  State state_;
};