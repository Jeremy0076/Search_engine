#include <iostream>
#include <string>
#include "httplib.h"
#include "../searcher/searcher.hpp"

const int SERVER_ERROR = 1;

searcher::Searcher search;

// const string g_input_path = "../data/input/html";
const string g_output_path = "../data/tmp/raw_input.txt"; 

int serviceInit(){
    bool ret = search.Init(g_output_path);
    if(!ret){
        cout<<"searcher init error"<<endl;
        return SERVER_ERROR;
    }
    return 0;
}

void GetWebData(const httplib::Request& req, httplib::Response& resp){
    if(!req.has_param("query")){
        resp.set_content("请求参数错误","text/plain;charset=utf-8");
        return ;
    }
    
    string query = req.get_param_value("query");
    string results;
    search.Search(query,&results);
    resp.set_content(results,"application/json;charset=utf-8");
    cout<<results<<endl;
}