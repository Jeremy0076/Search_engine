#include "parser.hpp"

using std::cout;
using std::endl;
using std::string;
using std::unordered_map;
using std::vector;

void ParseInit() {
    g_input_path    = GetString("g_input_path");
    g_output_path   = GetString("g_output_path");
    g_url_head      = GetString("g_url_head");
}

bool GetFilePath(const string& input_path,vector<string>* file_list) {
    namespace fs = boost::filesystem;
    fs::path root_path(input_path);
    if(fs::exists(root_path) == false) {
        cout<<input_path<<"no exists"<<endl;
        return false;
    }

    fs::recursive_directory_iterator end_iter;
    for(fs::recursive_directory_iterator iter(root_path);
        iter != end_iter; iter++) {
            //当前路径为目录时，直接跳过
            if(fs::is_regular_file(*iter) == false) {
                continue;
            }

            //当前文件不是 .html 文件，直接跳过
            if(iter->path().extension() != ".html") {
                continue;
            }

            //得到的路径加入到 vector 数组中
            file_list->push_back(iter->path().string());
        }
    
    return true;
}

bool ParseFile(const string& file_path,DocInfo* doc_info) {
    string html;
    bool ret = common::Util::Read(file_path, &html);
    if(!ret){
        cout<<file_path<<" file read error"<<endl;
        return false;
    }

    ret = ParseTitle(html, &doc_info->_title);
    if(!ret){
        cout<<"title analysis error "<<endl;
        return false;
    }

    ret = ParseUrl(file_path, &doc_info->_url);
    if(!ret){
        cout<<"Url analysis error "<<endl;
        return false;
    }

    ret = ParseContent(html, &doc_info->_content);
    if(!ret){
        cout<<"content analysis error "<<endl;
        return false;
    }
    return true;   
}

/*
    找到标题  <title> </title>
*/
bool ParseTitle(const string& html,string* title) {
    size_t begin = html.find("<title>");
    if(begin == string::npos){
        cout<<"title not found"<<endl;
        return false;
    }

    size_t end = html.find("</title>",begin);
    if(end == string::npos){
        cout<<"title not find"<<endl;
        return false;
    }

    begin += string("<title>").size();
    if(begin >= end){
        cout<<"title pos info error"<<endl;
        return false;
    }

    *title = html.substr(begin,end - begin);
    return true;
}

/*
    本地路径形如:
    ../data/input/html/thread.html
    在线路径形如:
    https://www.boost.org/doc/libs/1_53_0/doc/html/thread.html
*/
bool ParseUrl(const string& file_path,string* url) {
    string url_tail = file_path.substr(g_input_path.size());
    *url = g_url_head + url_tail;

    return true;
}


bool ParseContent(const string& html,string* content) {
    bool is_content = true;
    for(auto c : html){
        if(is_content){
            if(c == '<')
                //对<>中的内容忽略
                is_content = false;
            else{
                if(c == '\n')
                    c = ' ';
                content->push_back(c);
            }
        }else{
            if(c == '>')
                is_content = true;
            //忽略标签中的内容 <a>
        }
    }
    return true;
}

void WriteOutput(const DocInfo& doc_info,std::ofstream& ofstream) {
    ofstream<<doc_info._title<<"\3"<<doc_info._url
            <<"\3"<<doc_info._content<<endl;
}

/*
    解析所有html文件，生成raw_input.txt
*/
int ParseHTMLData(const string& input_path) {
    // 1. 得到html文件路径
    vector<string> file_list;
    bool ret = GetFilePath(g_input_path, &file_list);
    if(ret == false){
        cout<<"get html file path error"<<endl;
        return 1;
    }

    // for(auto& str : file_list){
    //     cout<<str<<endl;
    // }
    // cout<<file_list.size()<<endl;

    // 2. 遍历枚举的路径，针对每个文件进行单独处理
    std::ofstream output_file(g_output_path.c_str());
    if(output_file.is_open() == false){
        cout<<g_output_path<<" file open error"<<endl;
        return 1;
    }
    for(const auto& file_path : file_list){
        DocInfo doc_info;
        ret = ParseFile(file_path, &doc_info);
        if(ret == false){
            cout<<file_path<<" file analysis error"<<endl;
            continue;
        }
        //cout<<doc_info._title<<' '<<doc_info._url<<endl;
        // 3. 解析的文件写入到 指定的输出文件中
        WriteOutput(doc_info, output_file);
    }

    output_file.close();
    return 0;

}