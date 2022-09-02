#include <iostream>
#include <string>
#include <map>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <iostream>
#include <map>
#include <algorithm>
#include <sys/types.h>
#include <grp.h>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
const int bufsize = 10000;
int main(){
   char *path = "/home/scutech/azznaz/ftp/sss.txt";
   if(remove(path)<0){
      cout<<"no"<<endl;
   }else{
      cout<<"yes"<<endl;
   }
      
}