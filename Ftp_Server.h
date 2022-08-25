#include <iostream>
#include "Cmd_Handle.h"
#include "ace/ACE.h"
#include "ace/FILE_IO.h"
#include "ace/Handle_Set.h"
#include "ace/Hash_Map_Manager.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Record.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Stream.h"
#include "ace/Null_Mutex.h"
#include "ace/Truncate.h"
#include "ace/os_include/os_fcntl.h"
using namespace std;
#define SIZE_BUF 1024
#define USER 0
#define PASS 1
typedef ACE_Hash_Map_Manager<ACE_HANDLE,
                             Cmd_Handle*,
                             ACE_Null_Mutex> FTP_MAP;
class Ftp_Server{
public:
  int run (int argc, char *argv[]);
  int open (u_short port = 0);
  int wait_for_multiple_events ();
  int handle_connections ();
  int handle_data (ACE_SOCK_Stream * = 0);
  int handle_command(ACE_HANDLE handle);
  vector<string> parse(char *buf);
  int check(string &cmd);
private:
  ACE_Handle_Set master_handle_set_;
  ACE_Handle_Set active_read_handles_;
  ACE_SOCK_Acceptor acceptor_; // Socket acceptor endpoint.
  FTP_MAP htoc;
};