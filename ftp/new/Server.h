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
#include "ace/Event_Handler.h"
#include "ace/Reactor.h"
using namespace std;
#define SIZE_BUF 1024
#define USER 0
#define PASS 1
typedef ACE_Hash_Map_Manager<ACE_HANDLE,
                             Cmd_Handle*,
                             ACE_Null_Mutex> FTP_MAP;
class Server : public ACE_Event_Handler{
public:
  int run (int argc, char *argv[]);
  int open (ACE_Reactor *react,u_short port = 0){
    this->reactor(react);
    ACE_INET_Addr server_addr;
    int result;

    if(server_addr.set (port,(ACE_UINT32) INADDR_ANY) < 0) return -1;
    if(acceptor_.open (server_addr, 1) == -1) return -1;
    return reactor()->register_handler(this, ACCEPT_MASK);
    }
  virtual ACE_HANDLE get_handle() const{
        return acceptor_.get_handle();
  }
  virtual int handle_input(ACE_HANDLE){
     ACE_SOCK_Stream peer;
     char hostname[MAXHOSTNAMELEN];
     if(acceptor_.accept(peer) != -1){
        ACE_INET_Addr client;
        peer.get_remote_addr(client);
        client.get_host_name(hostname,MAXHOSTNAMELEN);
        ACE_DEBUG((LM_DEBUG,"accept : %s : %d\n",hostname,client.get_port_number()));
     }
     if(acceptor_.accept(peer) < 0){
        printf("accept error!\n");
     }else{
        Cmd_Handle* cmd_handle = new Cmd_Handle(peer.get_handle());
        if(cmd_handle->open(reactor()) < 0){
            peer.close();
            delete cmd_handle;
        }
     }
     return 0;
  }
private:
  ACE_SOCK_Acceptor acceptor_; // Socket acceptor endpoint.
};