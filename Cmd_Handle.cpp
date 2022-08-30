#include "Cmd_Handle.h"

Cmd_Handle::Cmd_Handle(ACE_HANDLE handle):login_state_(0),state_(noconnect),stop_(false),pre("./ftp"){
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
    cmd_list_.push_back("type");

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
    cmd_handler_.push_back(std::bind(&Cmd_Handle::type,this));
}
int Cmd_Handle::get_file_info(struct stat &sbuf,char *buf,int size){
    char mode[11];
    strcpy(mode,"----------");
    mode[10]='\0';
    if(S_ISDIR(sbuf.st_mode)) mode[0]='d';
    if(S_ISCHR(sbuf.st_mode)) mode[0]='c';
    if(S_ISBLK(sbuf.st_mode)) mode[0]='b';
    if(sbuf.st_mode&S_IRUSR)  mode[1]='r';
    if(sbuf.st_mode&S_IWUSR)  mode[2]='w';
    if(sbuf.st_mode&S_IXUSR)  mode[3]='x'; 
    if(sbuf.st_mode&S_IRGRP)  mode[4]='r';
    if(sbuf.st_mode&S_IWGRP)  mode[5]='w';
    if(sbuf.st_mode&S_IXGRP)  mode[6]='x';
    if(sbuf.st_mode&S_IROTH)  mode[7]='r';
    if(sbuf.st_mode&S_IWOTH)  mode[8]='w';
    if(sbuf.st_mode&S_IXOTH)  mode[9]='x';
    
    passwd* pw = NULL;
    char uname[20];
    pw = getpwuid(sbuf.st_uid);
    if(pw == NULL) return -1;
    strcpy(uname,pw->pw_name);
    group* gr = NULL;
    char gname[20];
    gr = getgrgid(sbuf.st_gid);
    strcpy(gname,gr->gr_name);
    int n = snprintf(buf,size,"%s %4d %-8s %-8s %8d %.12s ",mode,(int)sbuf.st_nlink,uname,gname,sbuf.st_size,4+ctime(&sbuf.st_mtime));
    return n;
}
int Cmd_Handle::send_list(string &path,ACE_SOCK_Stream &peer){
    DIR *dir = opendir(path.c_str());
    if(dir == NULL) {
        return -1;
    }
    char buf[BUFSIZ];
    ACE_HANDLE h = 0;
    if((h = get_handle()) == -1){

    }
    dirent *ent = NULL;
    while((ent = readdir(dir)) != NULL){
        string fname(ent->d_name);
        string cur_path = path + "/" + fname;
        struct stat st;
        if(lstat(cur_path.c_str(),&st) < 0){
            continue;
        }
        if(strncmp(ent->d_name,".",1) == 0){
            continue;
        }
        int n = get_file_info(st,buf,BUFSIZ);
        sprintf(buf+n,"%s\r\n",ent->d_name);
        printf("%s\n",buf);
        peer.send(buf,strlen(buf));
    }
    closedir(dir);
    peer.close();
    sprintf(buf,"%d list successful.\r\n",FTP_TRANSFEROK);
    reply(buf,strlen(buf));
}
void Cmd_Handle::parse_path(string &path,string &res){
    int pos = path.find_first_of('/');

    if(pos == string::npos){
        if(path == "."){// .
            res = cwd_;
        }else if(path == ".."){// ..
            string fa;
            get_father_dir(fa);
            res = fa;
        }else{// test
            res = cwd_ + "/" + path;
        }
    }else if(pos == 0){//  /test/dir1
        res = path;
    }else if(pos == 1 && path[0] == '.'){ // ./test/dir1
        string suf(path.c_str()+1);
        res = cwd_ + suf;
    }else if(pos == 2 && path[0] == '.' && path[1] == '.'){// ../test/dir1
        string suf(path.c_str()+2);
        string fa;
        get_father_dir(fa);
        res = fa + suf;
    }else{// test/dir1
        res = cwd_ + "/" + path;
    }
    if(res.length()>0 && res.back() == '/'){
        res.pop_back();
    }
}
void Cmd_Handle::get_father_dir(string &res){
    if(cwd_.length() == 0){
        res = "";
    }
    int pos = cwd_.find_last_of('/');
    res = cwd_.substr(0,pos);
}
ACE_HANDLE Cmd_Handle::get_handle(){
    ACE_SOCK_Stream peer;
    ACE_SOCK_Connector connector;
    ACE_INET_Addr client_addr;
    if(state_ == cmd_pasv){
        if(pasv_acceptor_.accept(peer) == -1){
            printf("%d\n",errno);
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
    if(n == 0){
        return -1;
    }
    buf[n-2] = '\0';
    printf("recv cmd:%s\n",buf);
    arg_ = parse(buf);
    // for(int i = 0;i<arg_.size();i++){
    //     printf("%s\n",arg_[i].c_str());
    // }
    int idx = check(arg_[0]);
    if(idx < 0){
        snprintf(buf,BUFSIZ,"502 Command not implemented.\r\n");
        reply(buf,strlen(buf));
    }else{
        cmd_handler_[idx]();
        if(stop_) return -1;
    }
    return 0;
}

void Cmd_Handle::user(){
    char buf[BUFSIZ];
    snprintf(buf,BUFSIZ,"%d need password.\r\n",FTP_GIVEPWORD);
    reply(buf,strlen(buf));
}

void Cmd_Handle::pass(){
    char buf[BUFSIZ];
    snprintf(buf,BUFSIZ,"%d login.\r\n",FTP_LOGINOK);
    reply(buf,strlen(buf));
}

void Cmd_Handle::pwd(){
    char buf[BUFSIZ];
    if(cwd_.length() == 0){
         snprintf(buf,BUFSIZ,"%d current directory is \"/\"\r\n",FTP_PWDOK);
    }else{
        snprintf(buf,BUFSIZ,"%d current directory is \"%s\"\r\n",FTP_PWDOK,cwd_.c_str());
    }
    reply(buf,strlen(buf));
}
void Cmd_Handle::pasv(){
    ACE_INET_Addr addr;
    char buf[BUFSIZ];
    if(state_ == cmd_pasv){
        sprintf(buf,"%d entering passive mode. %s\r\n",FTP_PASVOK,server_addr_.c_str());
        reply(buf,strlen(buf));
        return;
    }
    int res = addr.set(12345,(ACE_UINT32) INADDR_ANY);
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
    u_int32_t x = addr.get_port_number();
    u_int32_t h1 = x >> 8;
    u_int32_t h2 = x & (0xff);
    sprintf(buf,"127,0,0,1,%d,%d",h1,h2);
    server_addr_ = buf;
    sprintf(buf,"%d entering passive mode. %s\r\n",FTP_PASVOK,server_addr_.c_str());
    reply(buf,strlen(buf));
}
void Cmd_Handle::port(){
    int ip[4];
    unsigned int p1,p2;
    char client_ip[21]={0};
    char buf[BUFSIZ];
    if(state_ == cmd_pasv){
        pasv_acceptor_.close();
    }
    sscanf(arg_[1].c_str(),"%d,%d,%d,%d,%u,%u",&ip[0],&ip[1],&ip[2],&ip[3],&p1,&p2);
    snprintf(client_ip,sizeof(client_ip),"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
    client_ip_ = client_ip;
    port_ = (p1<<8)|(p2 & (0xff));
    state_ = cmd_port;
    snprintf(buf,BUFSIZ,"%d port successful.\r\n",FTP_PORTOK);
    reply(buf,strlen(buf));
}
void Cmd_Handle::cwd(){
    char buf[BUFSIZ];
    string path = "./ftp";
    string suf;
    parse_path(arg_[1],suf);
    printf("path:%s\n",suf.c_str());
    path = path + suf;
    DIR* dir = opendir(path.c_str());
    if(dir == NULL){
        sprintf(buf,"%d directory error!\r\n",FTP_BADCMD);
        reply(buf,strlen(buf));
        return;
    }
    cwd_ = suf;
    closedir(dir);
    if(cwd_.length() == 0){
        snprintf(buf,BUFSIZ,"%d directory changed to /\r\n",FTP_CWDOK);
    }else{
        snprintf(buf,BUFSIZ,"%d directory changed to %s\r\n",FTP_CWDOK,cwd_.c_str());
    }
    reply(buf,strlen(buf));
    // dirent* ent;
    // bool flag = false;
    // printf("%s %d\n",arg_[1].c_str(),arg_[1].length());
    // if(arg_[1].length() == 1 && strcmp(".",arg_[1].c_str()) == 0){
    //     flag = true;
    // }
    // if(arg_[1].length() == 2 && strcmp("..",arg_[1].c_str()) == 0){
    //     flag = true;
    //     int pos = -1;
    //     for(int i = cwd_.length()-1;i >= 0;i--){
    //         if(cwd_[i] == '/'){
    //             pos = i;
    //             break;
    //         }
    //     }
    //     if(pos != -1){
    //         string fa = cwd_.substr(0,pos);
    //         cwd_ = fa;
    //     }
    // }
    // while((ent = readdir(dir))!=NULL && !flag){
    //     if(ent->d_type == DT_DIR && strcmp(ent->d_name,arg_[1].c_str()) == 0){
    //         flag = true;
    //         cwd_.push_back('/');
    //         cwd_ = cwd_ + arg_[1];
    //         break;
    //     }
        
    // }
    // closedir(dir);
    // if(flag){
    //     if(cwd_.length() == 0){
    //          snprintf(buf,BUFSIZ,"%d directory changed to /\r\n",FTP_CWDOK);
    //     }else{
    //          snprintf(buf,BUFSIZ,"%d directory changed to %s\r\n",FTP_CWDOK,cwd_.c_str());
    //     }
    //     // if(cwd_.length() == 0){
    //     //     snprintf(buf,BUFSIZ,"%d cwd successful.\r\n",FTP_CWDOK);
    //     // }else{
    //     //     snprintf(buf,BUFSIZ,"%d cwd successful.\ncwd is %s",FTP_CWDOK,cwd_.c_str());
    //     // }
    //     reply(buf,strlen(buf));
    // }else{
    //     snprintf(buf,BUFSIZ,"%d cwd error.",FTP_NOPERM);
    //     reply(buf,strlen(buf));
    // }
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
    if(pos != -1){
        string fa = cwd_.substr(0,pos);
        cwd_ = fa;
    }
    // if(cwd_.length() == 0){
    //     snprintf(buf,BUFSIZ,"%d cdup successful.\r\n",FTP_CWDOK);
    // }else{
    //     snprintf(buf,BUFSIZ,"%d cdup successful.\r\n",FTP_CWDOK,cwd_.c_str());
    // }
    if(cwd_.length() == 0){
        snprintf(buf,BUFSIZ,"%d directory changed to /\r\n",FTP_CWDOK);
    }else{
        snprintf(buf,BUFSIZ,"%d directory changed to %s\r\n",FTP_CWDOK,cwd_.c_str());
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
    string path;
    string suf;
    parse_path(arg_[1],suf);
    printf("mkd: path is %s\n",suf.c_str());
    // DIR* dir = opendir(path.c_str());
    // dirent* ent;
    // while((ent = readdir(dir)) != NULL){
    //     if(strcmp(ent->d_name,arg_[1].c_str()) == 0){
    //         string s = cwd_ + "/" + arg_[1];
    //         sprintf(buf,"521 \"%s\" directory already exists\r\n",s.c_str());
    //         closedir(dir);
    //         reply(buf,strlen(buf));
    //         return;
    //     }
    // }
    path = pre + suf;
    if(mkdir(path.c_str(),0777) < 0){
        snprintf(buf,BUFSIZ,"521 mkdir fail.\r\n");
        reply(buf,strlen(buf));
        return;
    }
    snprintf(buf,BUFSIZ,"%d \"%s\" directory created\r\n",FTP_MKDIROK,suf.c_str());
    reply(buf,strlen(buf));
}
void Cmd_Handle::rmd(){
    char buf[BUFSIZ+1];
    string path = "./ftp" + cwd_ + "/" + arg_[1];
    if(rmdir(path.c_str()) < 0){
        snprintf(buf,BUFSIZ,"%d rmdir fail.\r\n",FTP_FILEFAIL);
        reply(buf,strlen(buf));
        return;
    }
    snprintf(buf,BUFSIZ,"%d rmdir successful.\r\n",FTP_RMDIROK);
    reply(buf,strlen(buf));
}
void Cmd_Handle::quit(){
    char buf[BUFSIZ];
    stop_ = true;
    snprintf(buf,BUFSIZ,"%d Goodbye!\r\n",FTP_GOODBYE);
    reply(buf,strlen(buf));
}
void Cmd_Handle::dele(){
    char buf[BUFSIZ];
    string path = "./ftp" + cwd_ + "/" + arg_[1];
    if(remove(path.c_str()) < 0){
        int n = snprintf(buf,BUFSIZ,"%d dele fail.\r\n",FTP_FILEFAIL);
        reply(buf,n);
        return;
    }
    int n = snprintf(buf,BUFSIZ,"%d dele successful.\r\n",FTP_DELEOK);
    reply(buf,n);
}
void Cmd_Handle::list(){
    ACE_HANDLE h;
    ACE_SOCK_Stream peer;
    char buf[BUFSIZ];
    struct stat st;
    if(state_ == noconnect){
        snprintf(buf,BUFSIZ,"%d list error.not connect!\r\n",FTP_BADSENDFILE);
        reply(buf,strlen(buf));
        return;
    }
    snprintf(buf,BUFSIZ,"%d Here comes the directory listing.\r\n", FTP_DATACONN);
    reply(buf,strlen(buf));
    if((h = get_handle()) < 0){
        snprintf(buf,BUFSIZ,"%d list error.connect error!\r\n",FTP_BADSENDFILE);
        reply(buf,strlen(buf));
        return;
    }
    peer.set_handle(h);
    string path = "./ftp";
    if(arg_.size() > 1){
        path = path + arg_[1];
    }else{
        path = path + cwd_;
    }
    printf("%s\n",path.c_str());
    DIR *dir = opendir(path.c_str());
    if(dir == NULL) {
        peer.close();
        snprintf(buf,BUFSIZ,"%d directory error!\r\n",FTP_BADCMD);
        reply(buf,strlen(buf));
        return;
    }
    dirent *ent = NULL;
    while((ent = readdir(dir)) != NULL){
        string fname(ent->d_name);
        string cur_path = path + "/" + fname;
        struct stat st;
        if(lstat(cur_path.c_str(),&st) < 0){
            continue;
        }
        if(strncmp(ent->d_name,".",1) == 0){
            continue;
        }
        int n = get_file_info(st,buf,BUFSIZ);
        sprintf(buf+n,"%s\r\n",ent->d_name);
        printf("%s\n",buf);
        peer.send(buf,strlen(buf));
    }
    closedir(dir);
    peer.close();
    sprintf(buf,"%d list successful.\r\n",FTP_TRANSFEROK);
    reply(buf,strlen(buf));
}
void Cmd_Handle::stor(){
    char buf[BUFSIZ];
    ACE_HANDLE h = -1;
    string path = "./ftp" + cwd_ + "/" + arg_[1];
    FILE *file = NULL;
    ACE_SOCK_Stream peer;
    if(state_ == noconnect){
        snprintf(buf,BUFSIZ,"%d stor error.not connect!\r\n",FTP_BADSENDFILE);
        reply(buf,strlen(buf));
        return;
    }
    snprintf(buf,BUFSIZ,"%d ready!\r\n", FTP_DATACONN);
    reply(buf,strlen(buf));
    if((h = get_handle()) < 0){
        snprintf(buf,BUFSIZ,"%d stor error.connect error!\r\n",FTP_BADSENDFILE);
        reply(buf,strlen(buf));
        return;
    }
    file = fopen(path.c_str(),"wb");
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
    // if(state_ == cmd_pasv){
    //     pasv_acceptor_.close();
    // }
    snprintf(buf,BUFSIZ,"%d stor successful.\r\n",FTP_STATFILE_OK);
    reply(buf,strlen(buf));
}
void Cmd_Handle::retr(){
    char buf[BUFSIZ];
    string path = "./ftp" + cwd_ + "/" + arg_[1];
    FILE* file = NULL;
    ACE_SOCK_Stream peer;
    ACE_HANDLE h;
    if(state_ == noconnect){
        snprintf(buf,BUFSIZ,"%d retr error.not connect!\r\n",FTP_BADSENDFILE);
        reply(buf,strlen(buf));
        return;
    }
    snprintf(buf,BUFSIZ,"%d ready!\r\n", FTP_DATACONN);
    reply(buf,strlen(buf));
    if((h = get_handle()) < 0){
        snprintf(buf,BUFSIZ,"%d retr error.connect error!\r\n",FTP_BADSENDFILE);
        reply(buf,strlen(buf));
        return;
    }
    file = fopen(path.c_str(),"rb");
    peer.set_handle(h);
    while (1)
    {
        size_t n = fread(buf,1,BUFSIZ,file);
        if(n == 0){
            break;
        }
        printf("n=%d\n",n);
        peer.send(buf,n);
    }
    fclose(file);
    peer.close();
    snprintf(buf,BUFSIZ,"%d retr successful.\r\n",FTP_TRANSFEROK);
    reply(buf,strlen(buf));
}
void Cmd_Handle::type(){
    char buf[BUFSIZ];
    snprintf(buf,BUFSIZ,"%d set image.\r\n",FTP_TYPEOK);
    reply(buf,strlen(buf));
}