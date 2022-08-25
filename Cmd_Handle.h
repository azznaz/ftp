#include <iostream>
#include <map>
#include <functional>
#include <algorithm>
#include <string>
#include "Ftp_Code.h"
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
using namespace std;
#define SIZE_BUF 1024
class Cmd_Handle{
public:
  Cmd_Handle(ACE_HANDLE handle);
  int handle();
private:
  typedef  _Bind<void (Cmd_Handle::*(Cmd_Handle *))()> handler;

  vector<string> parse(char *buf);
  int check(string &cmd);
  void reply(char *buf,int n);
  ACE_HANDLE get_handle();
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

  enum State{
    noconnect,
    cmd_pasv,
    cmd_port,
  };
  State state_;
  vector<string> cmd_list_;
  vector<handler> cmd_handler_;
  ACE_SOCK_Stream peer_;
  int login_state_;
  vector<string> arg_;
  string cwd_;
  bool passive_;
  string client_ip_;
  u_short port_;
  ACE_SOCK_Acceptor pasv_acceptor_;
  bool stop_;
};