#include "Ftp_Server.h"

int Ftp_Server::run(int argc, char *argv[]){
   printf("run\n");
  if (this->open (argc > 1 ? ACE_OS::atoi (argv[1]) : 0) == -1)
    return -1;
  ACE_DEBUG((LM_DEBUG,"listen in port: %s\n",argv[1]));
  for (;;) {
    if (wait_for_multiple_events () == -1)
      return -1;
    if (handle_connections () == -1)
      return -1;
    if (handle_data () == -1)
      return -1;
  }
  ACE_NOTREACHED (return 0;)
}
int Ftp_Server::open (u_short port)
{
  ACE_INET_Addr server_addr;
  int result;

  if (port != 0)
    result = server_addr.set (port,
                              (ACE_UINT32) INADDR_ANY);
  else
    result = server_addr.set ("ace_ftp",
                              (ACE_UINT32) INADDR_ANY);
  printf("11\n");
  if (result == -1) return -1;
  printf("%s\n",server_addr.get_host_name());
  // Start listening, enable reuse of listen address for quick restarts.
  if(acceptor_.open (server_addr, 1) == -1) return -1;
  master_handle_set_.set_bit (acceptor_.get_handle ());
  acceptor_.enable (ACE_NONBLOCK);
  return 0;
}

int Ftp_Server::wait_for_multiple_events () {
    ACE_DEBUG((LM_DEBUG,"wait_for_multiple_events\n"));
    active_read_handles_ = master_handle_set_;
    int width = ACE_Utils::truncate_cast<int> ((intptr_t)active_read_handles_.max_set ()) + 1;
    return ACE::select (width, active_read_handles_);
}

int Ftp_Server::handle_connections () {
    ACE_SOCK_Stream ftp_peer;
    ACE_DEBUG((LM_DEBUG,"handle_connections\n"));
    char hostname[MAXHOSTNAMELEN];
    char buf[BUFSIZ]="220 welcome!\r\n";
    while (acceptor_.accept (ftp_peer) != -1) {
      ACE_FILE_IO *log_file = new ACE_FILE_IO;
      ACE_INET_Addr client;
      ftp_peer.get_remote_addr(client);

      client.get_host_name(hostname,MAXHOSTNAMELEN);
      ACE_DEBUG((LM_DEBUG,"accept : %s : %d\n",hostname,client.get_port_number()));
      // Use the client's hostname as the logfile name.
   //   make_log_file (*log_file, &logging_peer);
      ftp_peer.send(buf,strlen(buf));
      // Add the new <logging_peer>'s handle to the map and
      // to the set of handles we <select> for input.
    //  log_map_.bind (logging_peer.get_handle (), log_file);
      master_handle_set_.set_bit (ftp_peer.get_handle ());
      htoc.bind(ftp_peer.get_handle(),new Cmd_Handle(ftp_peer.get_handle()));
    }
    active_read_handles_.clr_bit (acceptor_.get_handle ());
    return 0;
  }

int Ftp_Server::handle_data (ACE_SOCK_Stream *) {
    ACE_Handle_Set_Iterator peer_iterator (active_read_handles_);
    peer_iterator.reset_state();
    ACE_DEBUG((LM_DEBUG,"handle_data\n"));
    ACE_SOCK_Stream stream;
    for (ACE_HANDLE handle;
         (handle = peer_iterator ()) != ACE_INVALID_HANDLE;
         ) {
         Cmd_Handle *cmd_handle = 0;
         htoc.find(handle,cmd_handle);
         if(cmd_handle->handle()<0){
            stream.set_handle(handle);
            master_handle_set_.clr_bit(handle);
            htoc.unbind(handle);
            stream.close();
            delete cmd_handle;
         }
    //   ACE_FILE_IO *log_file = 0;
    //   log_map_.find (handle, log_file);
    //   Logging_Handler logging_handler (*log_file, handle);

    //   if (logging_handler.log_record () == -1) {
    //     logging_handler.close ();
    //     master_handle_set_.clr_bit (handle);
    //     log_map_.unbind (handle);
    //     log_file->close ();
    //     delete log_file;
    //   }
    }
    return 0;
}
int Ftp_Server::handle_command(ACE_HANDLE handle){
    
    //  char buf[SIZE_BUF];
    //  ssize_t n = peer_.recv(buf,SIZE_BUF);
    //  buf[n] = '\0';
    //  vector<string> cmdAndpara = parse(buf);
    //  int code = check(cmdAndpara[0]);
    //  if(code == USER){
        
    //  }
}