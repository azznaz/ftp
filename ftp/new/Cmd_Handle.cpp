#include "Cmd_Handle.h"

Cmd_Handle::Cmd_Handle(ACE_HANDLE handle):stop_(false),state_(st_no),login_(false),pre_(""),cwd_(""){
    fstream  file("../user.txt");
    string  line;
    while(getline(file,line)){
        int pos = line.find_first_of(' ');
        string username = line.substr(0,pos);
        string password = line.substr(pos+1,line.length() - pos - 1);
        user_.insert({username,password});
    }
    peer_.set_handle(handle);
    cmd_handler_.insert({"PWD",std::bind(&Cmd_Handle::pwd,this)});
    cmd_handler_.insert({"CWD",std::bind(&Cmd_Handle::cwd,this)});
    cmd_handler_.insert({"USER",std::bind(&Cmd_Handle::user,this)});
    cmd_handler_.insert({"PASS",std::bind(&Cmd_Handle::pass,this)});
    cmd_handler_.insert({"CDUP",std::bind(&Cmd_Handle::cdup,this)});
    cmd_handler_.insert({"SIZE",std::bind(&Cmd_Handle::size,this)});
    cmd_handler_.insert({"RMD",std::bind(&Cmd_Handle::rmd,this)});
    cmd_handler_.insert({"QUIT",std::bind(&Cmd_Handle::quit,this)});
    cmd_handler_.insert({"MKD",std::bind(&Cmd_Handle::mkd,this)});
    cmd_handler_.insert({"DELE",std::bind(&Cmd_Handle::dele,this)});
    cmd_handler_.insert({"LIST",std::bind(&Cmd_Handle::list,this)});
    cmd_handler_.insert({"PASV",std::bind(&Cmd_Handle::pasv,this)});
    cmd_handler_.insert({"PORT",std::bind(&Cmd_Handle::port,this)});
    cmd_handler_.insert({"STOR",std::bind(&Cmd_Handle::stor,this)});
    cmd_handler_.insert({"RETR",std::bind(&Cmd_Handle::retr,this)});
    cmd_handler_.insert({"TYPE",std::bind(&Cmd_Handle::type,this)});
    cmd_handler_.insert({"SYST",std::bind(&Cmd_Handle::syst,this)});
    cmd_handler_.insert({"RNFR",std::bind(&Cmd_Handle::rnfr,this)});
    cmd_handler_.insert({"RNTO",std::bind(&Cmd_Handle::rnto,this)});
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
    int n = snprintf(buf,size,"%s %4d %-8s %-8s %8ld %.12s ",mode,(int)sbuf.st_nlink,uname,gname,sbuf.st_size,4+ctime(&sbuf.st_mtime));
    return n;
}
void Cmd_Handle::parse_path(const string &path,string &res)const{
    int pos = path.find_first_of('/');
    if(pos == string::npos){
        if(path == "."){// .
            if(cwd_.length() == 0){
                res = "/";
            }else{
                res = cwd_;
            }
        }else if(path == ".."){// ..
            string fa;
            get_father_dir(fa);
            if(fa.length() == 0){
                res = "/";
            }else{
                res = fa;
            }
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
    if(res.length()>1 && res.back() == '/'){
        res.pop_back();
    }
    printf("parse path:%s ==> %s\n",path.c_str(),res.c_str());
}
void Cmd_Handle::get_father_dir(string &res) const{
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
    if(state_ == st_pasv){
        if(pasv_acceptor_.accept(peer) == -1){
            printf("accept fail\n");
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
void Cmd_Handle::reply(char *buf,int n){
    printf("send:%s\n",buf);
    peer_.send(buf,n);
}
int Cmd_Handle::handle(){
    char buf[BUFSIZ];
    int n = peer_.recv(buf,SIZE_BUF);
    if(n == 0){
        if(state_ == st_pasv){
            pasv_acceptor_.close();
        }
        return -1;
    }
    buf[n-2] = '\0';
    printf("recv cmd:%s\n",buf);
    arg_ = parse(buf);
    auto cmd_handler = cmd_handler_.find(arg_[0]);
    if(login_){
        if(cmd_handler == cmd_handler_.end()){
            n = sprintf(buf,"502 Command not implemented.\r\n");
            reply(buf,n);
        }else{
            cmd_handler->second();
            if(stop_) return -1;
        }
    }else{
      if(cmd_handler == cmd_handler_.end()){
            n = sprintf(buf,"530 Not logged in.\r\n");
            reply(buf,n);
        }else{
           if(cmd_handler->first == "PASS" && username_.length() == 0){
                n = sprintf(buf,"503 Bad sequence of commands.\r\n");
                reply(buf,n);
           }else if(cmd_handler->first == "PASS" || cmd_handler->first == "USER"){
                cmd_handler->second();
                if(stop_) return -1;
           }else{
                n = sprintf(buf,"530 Not logged in.\r\n");
                reply(buf,n);
           }
        }
    }
    return 0;
}

void Cmd_Handle::user(){
    char buf[BUFSIZ];
    int n;
    auto user = user_.find(arg_[1]);
    if(user == user_.end()){
        n = sprintf(buf,"430 Invalid username or password.\r\n");
        reply(buf,n);
    }else{
        username_ = user->first;
        n = sprintf(buf,"331 User name okay, need password.\r\n");
        reply(buf,n);
    }
}

void Cmd_Handle::pass(){
    char buf[BUFSIZ];
    int n;
    string password = user_[username_];
    if(password == arg_[1]){
        n = sprintf(buf,"230 User logged in, proceed.\r\n");
        login_ = true;
        reply(buf,n);
    }else{
        n = sprintf(buf,"430 Invalid username or password.\r\n");
        reply(buf,n);
    } 
}

void Cmd_Handle::pwd(){
    char buf[BUFSIZ];
    int n;
    if(cwd_.length() == 0){
         n = sprintf(buf,"257 current directory is \"/\"\r\n");
    }else{
        snprintf(buf,BUFSIZ,"257 current directory is \"%s\"\r\n",cwd_.c_str());
    }
    reply(buf,strlen(buf));
}
void Cmd_Handle::pasv(){
    ACE_INET_Addr addr;
    char buf[BUFSIZ];
    int n;
    if(state_ == st_pasv){
        n = sprintf(buf,"227 Entering Passive Mode. %s\r\n",server_addr_.c_str());
        reply(buf,n);
        return;
    }
    int res = addr.set((u_short)0,(ACE_UINT32) INADDR_ANY);
    if(res == -1){
         n = sprintf(buf,"500 set addr error.");
         reply(buf,n);
         return;
    }
    if(pasv_acceptor_.open(addr) == -1){
         n = sprintf(buf,"500 open addr error.");
         reply(buf,n);
         return;
    }
    state_ = st_pasv;
    pasv_acceptor_.get_local_addr(addr);
    u_int32_t x = addr.get_port_number();
    u_int32_t h1 = x >> 8;
    u_int32_t h2 = x & (0xff);
    sprintf(buf,"127,0,0,1,%d,%d",h1,h2);
    server_addr_ = buf;
    n = sprintf(buf,"227 Entering Passive Mode. %s\r\n",server_addr_.c_str());
    reply(buf,n);
}
void Cmd_Handle::port(){
    int ip[4];
    unsigned int p1,p2;
    char client_ip[21]={0};
    char buf[BUFSIZ];
    int n;
    sscanf(arg_[1].c_str(),"%d,%d,%d,%d,%u,%u",&ip[0],&ip[1],&ip[2],&ip[3],&p1,&p2);
    snprintf(client_ip,sizeof(client_ip),"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
    client_ip_ = client_ip;
    port_ = (p1<<8)|(p2 & (0xff));
    if(state_ == st_pasv){
        pasv_acceptor_.close();
    }
    state_ = st_port;
    n = sprintf(buf,"200 port successful.\r\n");
    reply(buf,strlen(buf));
}
void Cmd_Handle::cwd(){
    char buf[BUFSIZ];
    string path;
    string suf;
    int n;
    parse_path(arg_[1],suf);
    path = pre_ + suf;
    DIR* dir = opendir(path.c_str());
    if(dir == NULL){
        n = sprintf(buf,"553 directory error!\r\n");
        reply(buf,n);
        return;
    }
    if(suf.length() == 1){
       suf = "";
    }
    cwd_ = suf;
    closedir(dir);
    if(cwd_.length() == 0){
        n = sprintf(buf,"250 directory changed to /\r\n");
    }else{
        n = sprintf(buf,"250 directory changed to %s\r\n",cwd_.c_str());
    }
    reply(buf,n);
}

void Cmd_Handle::cdup(){
    char buf[BUFSIZ];
    string fa;
    int n;
    get_father_dir(fa);
    cwd_ = fa;
    if(cwd_.length() == 0){
        n = sprintf(buf,"250 directory changed to /\r\n");
    }else{
        n = sprintf(buf,"250 directory changed to %s\r\n",cwd_.c_str());
    }
    reply(buf,n);
}
void Cmd_Handle::size(){
    struct stat info;
    char buf[BUFSIZ];
    string path;
    string suf;
    parse_path(arg_[1],suf);
    path = pre_ + suf;
    int n;
    if(stat(path.c_str(),&info) < 0){
        n = sprintf(buf,"500 size error.file not found.\r\n");
        reply(buf,n);
        return;
    }
    n = sprintf(buf,"213 size successful.size is %ld\r\n",info.st_size);
    reply(buf,strlen(buf));
}
void Cmd_Handle::mkd(){
    char buf[BUFSIZ];
    string path;
    string suf;
    int n;
    parse_path(arg_[1],suf);
    printf("mkd: path is %s\n",suf.c_str());
    path = pre_ + suf;
    if(mkdir(path.c_str(),0777) < 0){
        n = sprintf(buf,"521 mkdir fail.\r\n");
        reply(buf,n);
        return;
    }
    n = sprintf(buf,"257 \"%s\" directory created\r\n",suf.c_str());
    reply(buf,strlen(buf));
}
void Cmd_Handle::rmd(){
    char buf[BUFSIZ];
    string path;
    string suf;
    int n;
    parse_path(arg_[1],suf);
    path = pre_ + suf;
    if(rmdir(path.c_str()) < 0){
        n = sprintf(buf,"550 rmdir fail.\r\n");
        reply(buf,n);
        return;
    }
    n = sprintf(buf,"250 rmdir successful.\r\n");
    reply(buf,n);
}
void Cmd_Handle::quit(){
    char buf[BUFSIZ];
    int n;
    stop_ = true;
    if(state_ == st_pasv){
        pasv_acceptor_.close();
    }
    n = sprintf(buf,"221 Goodbye!\r\n");
    reply(buf,n);
}
void Cmd_Handle::dele(){
    char buf[BUFSIZ];
    string path;
    string suf;
    int n;
    parse_path(arg_[1],suf);
    path = pre_ + suf; 
    printf("path:%s\n",path.c_str());
    if(remove(path.c_str()) < 0){
        n = sprintf(buf,"550 dele fail.\r\n");
        reply(buf,n);
        return;
    }
    n = sprintf(buf,"250 dele successful.\r\n");
    reply(buf,n);
}
void Cmd_Handle::list(){
    ACE_HANDLE h;
    ACE_SOCK_Stream peer;
    string path,suf;
    char buf[BUFSIZ];
    struct stat st;
    int n;
    if(state_ == st_no){
        n = sprintf(buf,"503 Bad sequence of commands.\r\n");
        reply(buf,n);
        return;
    }
    if((h = get_handle()) < 0){
        n = sprintf(buf,"425 list error.connect error!\r\n");
        reply(buf,n);
        return;
    }
    n = sprintf(buf,"125 Data connection already open; transfer starting.\r\n");
    reply(buf,n);
    peer.set_handle(h);
    if(arg_.size() > 1){
        parse_path(arg_[1],suf);
        path = pre_ + suf;
    }else{
        parse_path(cwd_,suf);
        path = pre_ + suf;
    }
    printf("%s\n",path.c_str());
    DIR *dir = opendir(path.c_str());
    if(dir == NULL) {
        peer.close();
        n = sprintf(buf,"553 directory error!\r\n");
        reply(buf,n);
        return;
    }
    dirent *ent = NULL;
    while((ent = readdir(dir)) != NULL){
        string fname(ent->d_name);
        string cur_path = path + "/" + fname;
        if(lstat(cur_path.c_str(),&st) < 0 || strncmp(ent->d_name,".",1) == 0){
            continue;
        }
        int offset = get_file_info(st,buf,BUFSIZ);
        n = sprintf(buf+offset,"%s\r\n",ent->d_name);
        n += offset;
        printf("%s\n",buf);
        peer.send(buf,n);
    }
    closedir(dir);
    peer.close();
    n = sprintf(buf,"226 list successful.\r\n");
    reply(buf,n);
}
void Cmd_Handle::stor(){
    char buf[BUFSIZ];
    ACE_HANDLE h = -1;
    string path;
    string suf;
    int n,pos;
    if(state_ == st_no){
        n = sprintf(buf,"503 Bad sequence of commands.\r\n");
        reply(buf,n);
        return;
    }
    pos = arg_[1].find_last_of('/');
    if(pos == string::npos){
        path = pre_ + cwd_ + "/" + arg_[1];
    }else{
        suf = arg_[1].substr(pos,arg_[1].length() - pos);
        path = pre_ + suf;
    }
    FILE *file = NULL;
    ACE_SOCK_Stream peer;
    if((h = get_handle()) < 0){
        n = sprintf(buf,"425 stor error.connect error!\r\n");
        reply(buf,n);
        return;
    }
    n = sprintf(buf,"125 Data connection already open; transfer starting.\r\n");
    reply(buf,n);
    file = fopen(path.c_str(),"wb");
    if(file == NULL){
        n = sprintf(buf,"553 Requested action not taken.File name not allowed.\r\n");
        reply(buf,n);
        return;
    }
    peer.set_handle(h);
    while (1)
    {
       ssize_t n = peer.recv(buf,BUFSIZ);
       printf("n:%ld\n",n);
       if(n == 0){
        break;
       }
       fwrite(buf,1,n,file);
    }
    fclose(file);
    peer.close();
    n = sprintf(buf,"226 stor successful.\r\n");
    reply(buf,strlen(buf));
}
void Cmd_Handle::retr(){
    char buf[BUFSIZ];
    string path,suf;
    FILE* file = NULL;
    ACE_SOCK_Stream peer;
    ACE_HANDLE h;
    int n;
    if(state_ == st_no){
        n = sprintf(buf,"503 Bad sequence of commands.\r\n");
        reply(buf,n);
        return;
    }
    parse_path(arg_[1],suf);
    path = pre_ + suf;
    n = sprintf(buf,"125 Data connection already open; transfer starting.\r\n");
    reply(buf,n);
    if((h = get_handle()) < 0){
        n = sprintf(buf,"451 retr error.connect error!\r\n");
        reply(buf,n);
        return;
    }
    file = fopen(path.c_str(),"rb");
    peer.set_handle(h);
    while (1){
        size_t count = fread(buf,1,BUFSIZ,file);
        if(count == 0){
            break;
        }
        peer.send(buf,count);
    }
    fclose(file);
    peer.close();
    n = sprintf(buf,"226 retr successful.\r\n");
    reply(buf,n);
}
void Cmd_Handle::type(){
    char buf[BUFSIZ];
    int n;
    if(arg_[1] == "I"){
        n = sprintf(buf,"200 Switching to binary mode\r\n");
    }else {
        n = sprintf(buf,"504 Unknown or unsupported type\r\n");
    }
    reply(buf,n);
}
void Cmd_Handle::syst(){
    char buf[BUFSIZ];
    int n;
    n = sprintf(buf,"215 UNIX\r\n");
    reply(buf,n);
}
void Cmd_Handle::rnfr(){
    char buf[BUFSIZ];
    int n;
    string path;
    string suf;
    parse_path(arg_[1],suf);
    path = pre_ + suf;
    rename_ = path;
    n = sprintf(buf,"350 Requested file action pending further information.\r\n");
    reply(buf,n);
}
void Cmd_Handle::rnto(){
    char buf[BUFSIZ];
    int n;
    string path;
    string suf;
    if(rename_.length() == 0){
        n = sprintf(buf,"503 Bad sequence of commands.\r\n");
        reply(buf,n);
        return;
    }
    parse_path(arg_[1],suf);
    path = pre_ + suf;
    if(rename(rename_.c_str(),path.c_str()) < 0){
        n = sprintf(buf,"451 Requested action aborted. Local error in processing.error: %s\r\n",strerror(errno));
        reply(buf,n);
        return;
    }
    n = sprintf(buf,"250 Requested file action okay, completed.\r\n");
    reply(buf,n);
}