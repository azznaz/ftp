#include<iostream>
#include<string>
using namespace std;
int main(){
    string s("12345/");
    cout<<s.back()<<endl;
    cout<<(s.back() == '/')<<endl;
    s.pop_back();
    cout<<s<<"  "<<s.length()<<endl;
}