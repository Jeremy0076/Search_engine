#include <iostream>
#include <string>
#include "httplib.h"
#include "../searcher/searcher.hpp"
#include "./service.hpp"
#include "./config.hpp"
#include "../parser/parser.hpp"

using std::cout;
using std::endl;
using std::string;

// const string g_input_path = "../data/input/"
// const string g_output_path = "../data/tmp/raw_input.txt"; 
// const string g_url_head = "https://www.boost.org/doc/libs/1_53_0/doc/"
extern string g_root_path;


int main(){
    using namespace httplib;
    int err;

    // 加载基本配置
    err = loadConfig();
    if(err != 0){
        cout<<" load config error "<<endl;
        return SERVER_ERROR;
    }

    // 加载解析路径配置
    ParseInit();
    outputConfig();

    // // 解析html数据
    // err = ParseHTMLData(g_input_path);
    // if(err != 0){
    //     cout<< " parse html file error "<<endl;
    //     return SERVER_ERROR;
    // } 

    // 初始化索引
    err = serviceInit(indexModel);
    if(err != 0){
        cout<< " service init error "<<endl;
        return SERVER_ERROR;
    }

    // 设置服务根目录web界面数据
    Server server;
    server.set_base_dir(g_root_path.c_str());

    // server api
    server.Get("/searcher", GetWebData);
    server.listen("0.0.0.0",19998);
    return 0;
}
