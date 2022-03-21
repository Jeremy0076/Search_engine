#pragma once

#include <fstream>
#include <list>
#include <iostream>

using namespace std;

#define FILENAME "env.conf"

static string g_raw_input_path;
static string g_root_path; 
static string listenIP;
static int listenPort;
static string indexModel;

struct node {
    string _key;
    string _value;
    node(const string& key, const string& value) : _key(key), _value(value){}
};

size_t config_size = 0;
list<node*> config_info = list<node*> ();

const string GetString(const string key);           // 得到字符串类型的 value
int GetInt(const string key, const int defaultKey); // 得到int 类型的数据，默认值为 del
bool Read(const string fileName);                   // 读取配置文件中的信息
void leftTrim(string &str);                         // 清除字符串左边空格
void rightTrim(string &str);                        // 清除字符串右边空格
bool isChar(char ch);                               // 判断是否为字母    
int loadConfig();                                   // 加载配置

bool Read(const string fileName = "env.conf"){
    cout<< " loading "<< fileName <<endl;
    if(fileName.size() == 0) return false;

    ifstream fp;
    fp.open(fileName, std::ios::in); //只读
    if(!fp.is_open()) return false;  //文件打开失败

    while(!fp.eof()){
        string buf("");
        getline(fp, buf);
        if(buf.size() == 0) continue;

        // 排除注释，[]标题, 不合法的key(不是_， 字母开头的数据)
        if(buf[0] != '_' && !isChar(buf[0])) continue;

        // 截取key value =号分割
        size_t p = buf.find("=");
        string key = buf.substr(0, p);
        string value = buf.substr(p+1);

        leftTrim(key), rightTrim(key);
        leftTrim(value), rightTrim(value);

        node* info = new node(key, value);
        config_info.push_back(info);
        config_size++;
    }

    cout<< " load success! "<<endl;
    fp.close();
    return true;
}

const string GetString(const string key){
    for(auto& info : config_info){
        if(info->_key == key) return info->_value;
    }
    return "";
}

int GetInt(const string key, const int defaultKey){
    for(auto& info : config_info)
        if(info->_key == key) return std::atoi(info->_value.c_str());
    return defaultKey;
}

void leftTrim(string &str){
    int p = 0;
    while(p < str.size() && (str[p] == 9 || str[p] == 10 || str[p] == 13 || str[p] == 32)) p++;
    string tmp = str.substr(p);
    str = tmp;
}

void rightTrim(string &str){
    int p = str.size() - 1;
    while(p >= 0 && (str[p] == 9 || str[p] == 10 || str[p] == 13 || str[p] == 32))
        str.erase(p--);
}

bool isChar(char ch){
    if(ch >= 'a' && ch <= 'z') return true;
    if(ch >= 'A' && ch <= 'Z') return true;
    return false;
}

int loadConfig(){
    bool ret = Read(FILENAME);
    g_raw_input_path    = GetString("g_raw_input_path");
    g_root_path = GetString("wwwroot");
    listenIP    = GetString("listenIP");
    listenPort  = GetInt("listenPort", 19998);
    indexModel = GetString("index");
    return 0;
}