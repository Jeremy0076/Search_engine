#include <iostream>
#include <string>
#include "httplib.h"
#include "../searcher/searcher.hpp"
#include "./service.hpp"

using std::cout;
using std::endl;
using std::string;

const string g_input_path = "../data/input/html";
// const string g_output_path = "../data/tmp/raw_input.txt"; 
const string g_url_head = "https://www.search_engine/doc";
const string g_root_path = "./web";


int main(){
    using namespace httplib;
    int err = serviceInit();
    if(err != 0){
        return SERVER_ERROR;
    }
    Server server;
    server.set_base_dir(g_root_path.c_str());

    server.Get("/searcher", GetWebData);
    server.listen("0.0.0.0", 19998);
    return 0;
}
