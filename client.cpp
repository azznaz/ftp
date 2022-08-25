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
vector<string> cmd_list_;
vector<string> arg_;
void init(){
    cmd_list_.push_back("pwd");
    cmd_list_.push_back("cwd");
    cmd_list_.push_back("user");
    cmd_list_.push_back("pass");
    cmd_list_.push_back("cdup");
    cmd_list_.push_back("size");
    cmd_list_.push_back("rmd");
    cmd_list_.push_back("quit");
    cmd_list_.push_back("mkd");
    cmd_list_.push_back("dele");
    cmd_list_.push_back("list");
    cmd_list_.push_back("pasv");
    cmd_list_.push_back("port");
    cmd_list_.push_back("stor");
}
vector<string> parse(char *buf){
    string line(buf);
    vector<string> cmdAndpara;
    string tmp;
    int i = 0;
    while (i < line.size())
    {
        if(line[i] == ' '){
            i++;
            continue;
        }
        int pos = line.find(' ',i);
        tmp = line.substr(i,pos-i);
        i = pos;
        cmdAndpara.push_back(tmp);
    }
    return cmdAndpara;
}
void port(){
    ACE_SOCK_Stream stream;
    ACE_INET_Addr addr;
    ACE_SOCK_Acceptor acceptor;
    char buf[BUFSIZ];
    u_int32_t x = 12345;
    u_int32_t p1 = x >> 8;
    u_int32_t p2 = x & (0xff);
    printf("%d %d\n",p1,p2);
    addr.set(12345,(ACE_UINT32) INADDR_ANY);
    if(acceptor.open(addr,1) == -1){
        exit(0);
    }
    if(acceptor.accept(stream) ==-1){
        exit(0);
    }
    string s = "./" + arg_[1];
    FILE* file = fopen(s.c_str(),"r");
    printf("%s\n",s.c_str());
    while(1){
        ssize_t n = fread(buf,1,BUFSIZ,file);
        buf[n]='\0';
        if(n==0){
            break;
        }
        stream.send(buf,strlen(buf));
    }
    fclose(file);
    stream.close();
    acceptor.close();
}
void list(char* cmd,ACE_SOCK_Stream &client_stream_){

    ACE_SOCK_Stream stream;
    ACE_INET_Addr addr;
    ACE_SOCK_Acceptor acceptor;
    char buf[BUFSIZ];
    u_int32_t x = 12345;
    u_int32_t p1 = x >> 8;
    u_int32_t p2 = x & (0xff);
    printf("%d %d\n",p1,p2);
    addr.set(12345,(ACE_UINT32) INADDR_ANY);
    if(acceptor.open(addr,1) == -1){
        exit(0);
    }
    client_stream_.send(cmd,strlen(cmd));
    if(acceptor.accept(stream) ==-1){
        exit(0);
    }
    int n = stream.recv(buf,BUFSIZ);
    buf[n]='\0';
    printf("%s\n",buf);
     stream.close();
    acceptor.close();
}
void retr(char* cmd,ACE_SOCK_Stream &client_stream_){

    ACE_SOCK_Stream stream;
    ACE_INET_Addr addr;
    ACE_SOCK_Acceptor acceptor;
    char buf[BUFSIZ+1];
    u_int32_t x = 12345;
    u_int32_t p1 = x >> 8;
    u_int32_t p2 = x & (0xff);
    addr.set(12345,(ACE_UINT32) INADDR_ANY);
    if(acceptor.open(addr,1) == -1){
        exit(0);
    }
    client_stream_.send(cmd,strlen(cmd));
    if(acceptor.accept(stream) ==-1){
        exit(0);
    }
    string path = "./" + arg_[1]+"(1)";
    FILE *file = fopen(path.c_str(),"w");
    while (1){
        size_t n = stream.recv(buf,BUFSIZ);
        buf[n] = '\0';
        printf("n:%d\n",n);
        if(n == 0){
            break;
        }
        fwrite(buf,1,n,file);
    }
    fclose(file);
    stream.close();
    acceptor.close();
}
void port(char* cmd,ACE_SOCK_Stream &client_stream_){
    string s(cmd);
    char buf[BUFSIZ];
    s = s + " 127,0,0,1,48,57";
    client_stream_.send(s.c_str(),s.length());
     ssize_t n = client_stream_.recv(buf,BUFSIZ);
       buf[n] = '\0';
       printf("recv bytes:%d\n",n);
       string t(buf);
       printf("%s\n",t.c_str());
}
int main(){
    init();
    ACE_SOCK_Stream client_stream_;
    ACE_INET_Addr remote_addr_;
    ACE_SOCK_Connector connector_;
    char buf[BUFSIZ];
    remote_addr_.set(1234,"127.0.0.1");
    if (connector_.connect (client_stream_, remote_addr_) == -1)
        ACE_ERROR_RETURN ((LM_ERROR,"(%P|%t) %p\n","connection failed"),-1);
    else
        ACE_DEBUG ((LM_DEBUG,"(%P|%t) connected to %s\n",remote_addr_.get_host_name ()));
    client_stream_.recv(buf,BUFSIZ);
    printf("recv:%s\n",buf);
    char cmd[BUFSIZ];
    while (1){
       printf("ftp>");
       cin.getline(cmd,BUFSIZ);
       printf("send:%s\n",cmd);
       arg_= parse(cmd);
       if(arg_[0] == "stor"){
            client_stream_.send(cmd,strlen(cmd));
            port();
            ssize_t n = client_stream_.recv(buf,BUFSIZ);
            buf[n] = '\0';
            printf("recv bytes:%d\n",n);
            string t(buf);
            printf("%s\n",t.c_str());

            continue;
       }
       if(arg_[0] == "list"){
        list(cmd,client_stream_);
         ssize_t n = client_stream_.recv(buf,BUFSIZ);
            buf[n] = '\0';
            printf("recv bytes:%d\n",n);
            string t(buf);
            printf("%s\n",t.c_str());

            continue;
       }
       if(arg_[0] == "retr"){
            retr(cmd,client_stream_);
            ssize_t n = client_stream_.recv(buf,BUFSIZ);
            buf[n] = '\0';
            printf("recv bytes:%d\n",n);
            string t(buf);
            printf("%s\n",t.c_str());

            continue;
       }
       if(arg_[0] == "port"){
         port(cmd,client_stream_);
         continue;
       }
       client_stream_.send(cmd,strlen(cmd));
       ssize_t n = client_stream_.recv(buf,BUFSIZ);
       buf[n] = '\0';
       printf("recv bytes:%d\n",n);
       string t(buf);
       printf("%s\n",t.c_str());
    }
    
}