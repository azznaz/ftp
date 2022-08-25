#include "Cmd_Handle.h"

Cmd_Handle::Cmd_Handle(ACE_HANDLE handle):login_state_(0),state_(noconnect),stop_(false){
    //cmd_map_["pwd"] = this->pwd();
    //  f.push_back(this->pwd);
    //handler x = std::bind(&Cmd_Handle::pwd,this);
    peer_.set_handle(handle);
    cwd_="";
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
    cmd_list_.push_back("retr");

    cmd_handler_.push_back(std::bind(&Cmd_Handle::pwd,this));
    cmd_handler_.push_back(std::bind(&Cmd_Handle::cwd,this));
    cmd_handler_.push_back(std::bind(&Cmd_Handle::user,this));
    cmd_handler_.push_back(std::bind(&Cmd_Handle::pass,this));
    cmd_handler_.push_back(std::bind(&Cmd_Handle::cdup,this));
    cmd_handler_.push_back(std::bind(&Cmd_Handle::size,this));
    cmd_handler_.push_back(std::bind(&Cmd_Handle::rmd,this));
    cmd_handler_.push_back(std::bind(&Cmd_Handle::quit,this));
    cmd_handler_.push_back(std::bind(&Cmd_Handle::mkd,this));
    cmd_handler_.push_back(std::bind(&Cmd_Handle::dele,this));
    cmd_handler_.push_back(std::bind(&Cmd_Handle::list,this));
    cmd_handler_.push_back(std::bind(&Cmd_Handle::pasv,this));
    cmd_handler_.push_back(std::bind(&Cmd_Handle::port,this));
    cmd_handler_.push_back(std::bind(&Cmd_Handle::stor,this));
    cmd_handler_.push_back(std::bind(&Cmd_Handle::retr,this));
}
ACE_HANDLE Cmd_Handle::get_handle(){
    ACE_SOCK_Stream peer;
    ACE_SOCK_Connector connector;
    ACE_INET_Addr client_addr;
    if(passive_){
        if(pasv_acceptor_.accept(peer) == -1){
            return -1;
        }
        return peer.get_handle();
    }else{
        if(client_addr.set(port_,client_ip_.c_str()) < 0){
            return -1;
        }
        if(connector.connect(peer,client_addr) < 0){
            return -1;
        }
        return peer.get_handle();
    }
}
vector<string> Cmd_Handle::parse(char *buf){
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

int Cmd_Handle::check(string &cmd){
    for(int i = 0;i < cmd_list_.size();i++){
        string& mycmd = cmd_list_[i];
        if(cmd.size() == mycmd.size() && strncasecmp(cmd.c_str(),mycmd.c_str(),cmd.size())==0)
           return i;
    }
    return -1;
}
void Cmd_Handle::reply(char *buf,int n){
    printf("send:%s\n",buf);
    peer_.send(buf,n);
}
int Cmd_Handle::handle(){
    char buf[BUFSIZ];
    ssize_t n = peer_.recv(buf,SIZE_BUF);
    buf[n] = '\0';
    printf("recv cmd:%s\n",buf);
    if(n == 0){
        return -1;
    }
    arg_ = parse(buf);
    int code = check(arg_[0]);
    if(code < 0){
        snprintf(buf,BUFSIZ,"%d Unknown command.",FTP_BADCMD);
        reply(buf,strlen(buf));
    }else{
        cmd_handler_[code]();
        if(stop_) return -1;
    }
    return 0;
}

void Cmd_Handle::user(){
    char buf[BUFSIZ];
    snprintf(buf,BUFSIZ,"%d need password.",FTP_LOGINOK);
    reply(buf,strlen(buf));
}

void Cmd_Handle::pass(){
    char buf[BUFSIZ];
    snprintf(buf,BUFSIZ,"%d login.",FTP_LOGINOK);
    reply(buf,strlen(buf));
}

void Cmd_Handle::pwd(){
    char buf[BUFSIZ];
    // //strncpy(buf,cwd_.c_str(),cwd_.size());
    // if(state_ == noconnect){
    //     snprintf(buf,BUFSIZ,"%d no connect.",FTP_BADCMD);
    //     reply(buf,strlen(buf));
    // }
    // ACE_HANDLE handle = get_handle();
    // ACE_SOCK_Stream peer;
    // if(handle == -1){
    //     snprintf(buf,BUFSIZ,"%d pwd error.",FTP_BADCMD);
    //     reply(buf,strlen(buf));
    //     return;
    // } 
    // snprintf(buf,BUFSIZ,"%d pwd successful.",FTP_PWDOK);
    // reply(buf,strlen(buf));
    if(cwd_.length() == 0){
         snprintf(buf,BUFSIZ,"/");
    }else{
        snprintf(buf,BUFSIZ,"%s",cwd_.c_str());
    }
    // peer.set_handle(handle);
    // peer.send(buf,strlen(buf));
    reply(buf,strlen(buf));
}
void Cmd_Handle::pasv(){
    ACE_INET_Addr addr;
    char buf[BUFSIZ];
    int res = addr.set("pasv_ftp",(ACE_UINT32) INADDR_ANY);
    if(res == -1){
         snprintf(buf,BUFSIZ,"%d set addr error.",FTP_BADCMD);
         reply(buf,strlen(buf));
         return;
    }
    if(pasv_acceptor_.open(addr,1) == -1){
         snprintf(buf,BUFSIZ,"%d open addr error.",FTP_BADCMD);
         reply(buf,strlen(buf));
         return;
    }
    state_ = cmd_pasv;
    snprintf(buf,BUFSIZ,"%d entering passive mode (%s.%u.%u)",FTP_PASVOK,addr.get_host_name(),addr.get_port_number());
    reply(buf,strlen(buf));
}
void Cmd_Handle::port(){
    int ip[4];
    unsigned int p1,p2;
    char client_ip[21]={0};
    char buf[BUFSIZ];
    sscanf(arg_[1].c_str(),"%d,%d,%d,%d,%u,%u",&ip[0],&ip[1],&ip[2],&ip[3],&p1,&p2);
    snprintf(client_ip,sizeof(client_ip),"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
    client_ip_ = client_ip;
    port_ = (p1<<8)|(p2 & (0xff));
    state_ = cmd_port;
    snprintf(buf,BUFSIZ,"%d port successful.",FTP_PORTOK);
    reply(buf,strlen(buf));
}
void Cmd_Handle::cwd(){
    char buf[BUFSIZ];
    string d = "./ftp"+cwd_;
    
    DIR* dir = opendir(d.c_str());
    dirent* ent;
    bool flag = false;
    while((ent = readdir(dir))!=NULL){
        if(ent->d_type == DT_DIR && strcmp(ent->d_name,arg_[1].c_str()) == 0){
            flag = true;
            cwd_.push_back('/');
            cwd_ = cwd_ + arg_[1];
            break;
        }
    }
    closedir(dir);
    if(flag){
        if(cwd_.length() == 0){
            snprintf(buf,BUFSIZ,"%d cwd successful.\ncwd is /",FTP_CWDOK);
        }else{
            snprintf(buf,BUFSIZ,"%d cwd successful.\ncwd is %s",FTP_CWDOK,cwd_.c_str());
        }
        reply(buf,strlen(buf));
    }else{
        snprintf(buf,BUFSIZ,"%d cwd error.",FTP_BADCMD);
        reply(buf,strlen(buf));
    }
}

void Cmd_Handle::cdup(){
    char buf[BUFSIZ];
    int pos = -1;
    for(int i = cwd_.length()-1;i >= 0;i--){
        if(cwd_[i] == '/'){
            pos = i;
            break;
        }
    }
    printf("pos:%d\n",pos);
    if(pos == -1){
        snprintf(buf,BUFSIZ,"%d cdup successful.\ncwd is /",FTP_CWDOK);
        reply(buf,strlen(buf));
        return;
    }
    string fa = cwd_.substr(0,pos);
    cwd_ = fa;
    if(cwd_.length() == 0){
        snprintf(buf,BUFSIZ,"%d cdup successful.\ncwd is /",FTP_CWDOK);
    }else{
        snprintf(buf,BUFSIZ,"%d cdup successful.\ncwd is %s",FTP_CWDOK,cwd_.c_str());
    }
    reply(buf,strlen(buf));
}
void Cmd_Handle::size(){
    struct stat info;
    char buf[BUFSIZ];
    string path = "./ftp" + cwd_ + "/" + arg_[1];
    if(stat(path.c_str(),&info) < 0){
        snprintf(buf,BUFSIZ,"%d size error.\nfile not found.",FTP_BADCMD);
        reply(buf,strlen(buf));
        return;
    }
    snprintf(buf,BUFSIZ,"%d size successful.\nsize is %d",FTP_SIZEOK,info.st_size);
    reply(buf,strlen(buf));
}
void Cmd_Handle::mkd(){
    char buf[BUFSIZ];
    string path = "./ftp" + cwd_ + "/" + arg_[1];
    printf("mkd: path is %s\n",path.c_str());
    if(mkdir(path.c_str(),0777) < 0){
        snprintf(buf,BUFSIZ,"%d mkdir fail.",FTP_FILEFAIL);
        reply(buf,strlen(buf));
        return;
    }
    snprintf(buf,BUFSIZ,"%d mkdir successful.",FTP_MKDIROK);
    reply(buf,strlen(buf));
}
void Cmd_Handle::rmd(){
    char buf[BUFSIZ+1];
    string path = "./ftp" + cwd_ + "/" + arg_[1];
    if(rmdir(path.c_str()) < 0){
        snprintf(buf,BUFSIZ,"%d rmdir fail.",FTP_FILEFAIL);
        reply(buf,strlen(buf));
        return;
    }
    snprintf(buf,BUFSIZ,"%d rmdir successful.",FTP_RMDIROK);
    reply(buf,strlen(buf));
}
void Cmd_Handle::quit(){
    char buf[BUFSIZ+1];
    stop_ = true;
    snprintf(buf,BUFSIZ,"%d Goodbye!",FTP_GOODBYE);
    reply(buf,strlen(buf));
}
void Cmd_Handle::dele(){
    char buf[BUFSIZ+1];
    string path = "./ftp" + cwd_ + "/" + arg_[1];
    if(remove(path.c_str()) < 0){
        int n = snprintf(buf,BUFSIZ,"%d dele fail.",FTP_FILEFAIL);
        reply(buf,n);
        return;
    }
    int n = snprintf(buf,BUFSIZ,"%d dele successful.",FTP_DELEOK);
    reply(buf,n);
}
void Cmd_Handle::list(){
    ACE_HANDLE h;
    ACE_SOCK_Stream peer;
    char buf[BUFSIZ];
    if(state_ == noconnect){
        snprintf(buf,BUFSIZ,"%d stor error.\nnot connect!",FTP_BADSENDFILE);
        reply(buf,strlen(buf));
        return;
    }
    if((h = get_handle()) < 0){
        snprintf(buf,BUFSIZ,"%d stor error.\nconnect error!",FTP_BADSENDFILE);
        reply(buf,strlen(buf));
        return;
    }
    peer.set_handle(h);
    string path = "./ftp" + cwd_;
    DIR *dir = opendir(path.c_str());
    dirent *ent = 0;
    int offset = 0;
    while ((ent = readdir(dir)) != NULL){
         if(strncmp(ent->d_name,".",1) == 0){
            continue;
         }
         if(ent->d_type == DT_DIR){
            int n = snprintf(buf + offset,BUFSIZ,"\033[1m\033[32m%-15s\033[0m ",ent->d_name);
            offset += n;
         }else{
            int n = snprintf(buf + offset,BUFSIZ,"%-15s ",ent->d_name);
            offset += n;
         }
    }
    buf[offset] = '\0';
    closedir(dir);
    peer.send(buf,strlen(buf));
    snprintf(buf,BUFSIZ,"%d list successful.",FTP_TRANSFEROK);
    reply(buf,strlen(buf));
}
void Cmd_Handle::stor(){
    char buf[BUFSIZ];
    ACE_HANDLE h = -1;
    string path = "./ftp" + cwd_ + "/" + arg_[1];
    FILE *file = NULL;
    ACE_SOCK_Stream peer;
    if(state_ == noconnect){
        snprintf(buf,BUFSIZ,"%d stor error.\nnot connect!",FTP_BADSENDFILE);
        reply(buf,strlen(buf));
        return;
    }
    if((h = get_handle()) < 0){
        snprintf(buf,BUFSIZ,"%d stor error.\nconnect error!",FTP_BADSENDFILE);
        reply(buf,strlen(buf));
        return;
    }
    file = fopen(path.c_str(),"w");
    peer.set_handle(h);
    while (1)
    {
       ssize_t n = peer.recv(buf,BUFSIZ);
       printf("n:%d\n",n);
       if(n == 0){
        break;
       }
       if(fwrite(buf,1,n,file) == 0){
        // fclose(file);
        // snprintf(buf,BUFSIZ,"%d stor error.\nwrite file error!",FTP_BADSENDFILE);
        // reply(buf,strlen(buf));
         break;
       }
    }
    fclose(file);
    peer.close();
    if(passive_){
        pasv_acceptor_.close();
    }
    snprintf(buf,BUFSIZ,"%d stor successful.",FTP_STATFILE_OK);
    reply(buf,strlen(buf));
}
void Cmd_Handle::retr(){
    char buf[BUFSIZ];
    string path = "./ftp" + cwd_ + "/" + arg_[1];
    FILE* file = NULL;
    ACE_SOCK_Stream peer;
    ACE_HANDLE h;
    if(state_ == noconnect){
        snprintf(buf,BUFSIZ,"%d retr error.\nnot connect!",FTP_BADSENDFILE);
        reply(buf,strlen(buf));
        return;
    }
    if((h = get_handle()) < 0){
        snprintf(buf,BUFSIZ,"%d retr error.\nconnect error!",FTP_BADSENDFILE);
        reply(buf,strlen(buf));
        return;
    }
    file = fopen(path.c_str(),"r");
    peer.set_handle(h);
    while (1)
    {
        size_t n = fread(buf,1,BUFSIZ,file);
        if(n == 0){
            break;
        }
        peer.send(buf,n);
    }
    fclose(file);
    peer.close();
    snprintf(buf,BUFSIZ,"%d retr successful.",FTP_TRANSFEROK);
    reply(buf,strlen(buf));
}
