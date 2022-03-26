#pragma once

#include <stdint.h> // int16_t 、 uint32_t 、 int64_t
#include <iostream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <cppjieba/Jieba.hpp>

using std::cout;
using std::endl;
using std::string;
using std::unordered_map;
using std::vector;

namespace searcher { 
    /*
     *  正排索引的存储结构体
     *  根据文档 id 定位到文档的内容 
     *  防止文档过多，直接使用64位的 int 来存储
    */
    struct frontIdx{
        int64_t _docId;
        string _title;
        string _url;
        string _content;
    };

    /*
     *  倒排索引存储的结构体
     *  根据文本的关键字 定位到 所属的文档Id
     *  为了后面根据权值排序，再加一个关键字的权值
    */
    struct backwardIdx{
        int64_t _docId;
        int     _weight;
        string  _word;
    };

    /*
     *  后缀自动机 endpos状态
     *  len endpos状态的最大字符串长度，link后缀链接，siz出现次数
     *  to 状态转移函数
    */
    struct state{
        int len, link;
        int siz;
        unordered_map<char, int> to;
        vector<backwardIdx> inverted_table;

        /*
        state 构造函数
        */
       state(){
           len = 0; link = 0; siz = 0;
       }

        /*
         添加正排索引到当前state的倒排表
        */
        void addDocInfoToInvert(const frontIdx& doc_info, int score){
            vector<backwardIdx>::iterator it;
            for(it = inverted_table.begin(); it != inverted_table.end(); it++){
                // 已经存在，在原有基础上加权重
                if ((*it)._docId == doc_info._docId) {
                    (*it)._weight += score;
                    return;
                }
            }

            // 未存在，添加新的倒排表节点
            if (it == inverted_table.end()){
                backwardIdx backIdx;
                backIdx._docId = doc_info._docId;
                backIdx._weight = score;
                // word字段不需要了
                inverted_table.push_back(std::move(backIdx));
            }
        }
    };

    // 索引模块
    class SAM{
    private:    
        vector<state*> stPos;   // endpos状态集合
        int _size;              // 节点个数
        int _last;              // 上一个更新到节点位置
    public:
        SAM();
        void Extend(char c, int score, const frontIdx& doc_info);       // 添加一个字符到sam, 权重
        const vector<backwardIdx>* Search(string pattern);              // 搜索一个字符串的倒排表
        ~SAM(){
            for(int i =  0; i<=_size; i++){
                delete stPos[i];
            }
        }
    };

    class Index{
    private:
        // 正排索引
        vector<frontIdx> forward_index;
        // 倒排索引  哈希表
        unordered_map<string, vector<backwardIdx> > inverted_index; 
        // jieba分词
        cppjieba::Jieba jieba;
        // 索引类型
        string _indexModel;
        // sam
        SAM* sam;
    private:
        // 根据一行 预处理 解析的文件，得到一个正排索引的节点
        frontIdx* BuildForward(const string& line);
        // 根据正排索引节点，构造倒排索引节点
        void BuildInverted(const frontIdx& doc_info);
    public:
        Index();
        ~Index(){
            delete sam;
        }
        // 查找正排索引
        const frontIdx* GetFrontIdx(const int64_t doc_id);
        // 查倒排索引
        const vector<backwardIdx>* GetBackwardIdx(const string& key);
        // 建立倒排索引 与 正排索引
        bool Build(const string& input_path, const string& indexModel);
        // jieba分词 对语句进行分词
        void CutWord(const string& input, vector<string>* output);
        // 添加一个字符串string到sam
        void ExtendString(string str, const frontIdx& doc_info, int score);  
        // 获取索引模式
        string getIndexModel();                      
    };

    // 搜索模块
    class Searcher{
    private:
        // 全文索引
        Index* index;
    private:
        //得到关键字前后的数据，在前端页面显示的文本
        string GetShowContent(const string& content, const string& word);
    public:
        Searcher() : index(new Index()) {};
        // 初始化 构建指定文档的索引
        bool Init(const string& input_path, const string& indexModel);
        // 指定文本进行搜索
        bool Search(const string& query, string* output);
    };

}
