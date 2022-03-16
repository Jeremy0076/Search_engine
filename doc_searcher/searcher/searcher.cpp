#include <fstream>
#include <string>
#include <algorithm>

#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/json.h>
#include <boost/algorithm/string/case_conv.hpp>

#include "searcher.hpp"
#include "../common/util.hpp"

#include "searcher.hpp"
#include "../common/util.hpp"

#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/json.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <fstream>
#include <string>
#include <algorithm>

namespace searcher{
    // 索引模块

    // jieba分词词典路径
    const char* const DICT_BASE_PATH = "/Users/jeremy/School/毕业设计/thridparty/cppjieba-master/dict";
    const char* const DICT_PATH = "/Users/jeremy/School/毕业设计/thridparty/cppjieba-master/dict/jieba.dict.utf8";
    const char* const HMM_PATH = "/Users/jeremy/School/毕业设计/thridparty/cppjieba-master/dict/hmm_model.utf8";
    const char* const USER_DICT_PATH = "/Users/jeremy/School/毕业设计/thridparty/cppjieba-master/dict/user.dict.utf8";
    const char* const IDF_PATH = "/Users/jeremy/School/毕业设计/thridparty/cppjieba-master/dict/idf.utf8";
    const char* const STOP_WORD_PATH = "/Users/jeremy/School/毕业设计/thridparty/cppjieba-master/dict/stop_words.utf8";

    /*
    建立索引
    */
    bool Index::Build(const string& input_path){
        // 按行读取 存放于处理解析出来的数据文件
        cout<<input_path<<" build index begin "<<endl;
        std::ifstream file(input_path.c_str());
        if(!file.is_open()){
            cout<<input_path<<" file open error "<<endl;
            return false;
        }
        
        string line;
        int idx = 0;
        static string progress("|/-\\");

        while(std::getline(file, line)){
            // 针对当前行数据，正排索引
            frontIdx* doc_info = BuildForward(line);
            if(doc_info == nullptr){
                cout<<" forward build error "<<endl;
                continue;
            }

            // 根据正排索引，构建倒排索引
            BuildInverted(*doc_info);

            // 打印部分构建结果 防止过多cout影响时间复杂度
            if(doc_info->_docId % 100 == 0) {
                //进度条
                cout<<"\r"<<progress[idx % 4]<< doc_info->_docId << " sucessed " <<std::flush;
                idx++;
            }
        }

        cout<<"index build successed "<<endl;
        file.close();
        return true;
    }

    /*
    根据一行 预处理 解析的文件，得到一个正排索引的节点，并插入到正排数组中
    */
    frontIdx* Index::BuildForward (const string&line) {
        // 对一行数据进行拆分 \3 为分割点，依次为 title url content
        vector<string> nums;
        common::Util::Split(line, "\3", &nums);
        if(nums.size() != 3){
            cout<<" file num error "<<nums.size()<<endl;
            return nullptr;
        }
        frontIdx doc_info;
        doc_info._docId = forward_index.size();
        doc_info._title     = nums[0];
        doc_info._url       = nums[1];
        doc_info._content   = nums[2];
        forward_index.push_back(std::move(doc_info));

        return &forward_index.back();
    }

    /* 
    根据正排索引节点，构造倒排索引节点 
    */
    void Index::BuildInverted (const frontIdx& doc_info){
        // 统计关键词作为 标题和 正文的出现次数
        struct WordCnt{
            int _titleCnt;
            int _contentCnt;
            WordCnt()
                :_titleCnt(0), _contentCnt(0) {}
        };

        unordered_map<string, WordCnt> wordMap;
        
        // 针对标题进行分词
        vector<string> titleWord;
        CutWord(doc_info._title, &titleWord);
        for(string word : titleWord){
            // 全部转为小写
            boost::to_lower(word);
            wordMap[word]._titleCnt++;
        }

        // 针对正文进行分词
        vector<string> contentWord;
        CutWord(doc_info._content, &contentWord);
        for(string word : contentWord){
            boost::to_lower(word);
            wordMap[word]._contentCnt++;
        }

        // 统计结果 插入到倒排索引中
        for(const auto& word_pair : wordMap){
            backwardIdx backIdx;
            backIdx._docId = doc_info._docId;

            // 自定义 权值 = 10 * titleCnt + contentCnt
            backIdx._weight = 10 * word_pair.second._titleCnt + word_pair.second._contentCnt;
            backIdx._word = word_pair.first;

            vector<backwardIdx>& back_vector = inverted_index[word_pair.first];
            back_vector.push_back(std::move(backIdx));
        }
    }

    Index::Index() :jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH)
    {
        forward_index.clear();
        inverted_index.clear();
    }

    void Index::CutWord(const string& input, vector<string>* output){
        jieba.CutForSearch(input, *output);
    }

    /*
    查找正排索引
    */
   const frontIdx* Index::GetFrontIdx(const int64_t doc_id){
       if(doc_id < 0 || doc_id >= forward_index.size())
            return nullptr;
        return &forward_index[doc_id];
   }

   /*
   查询倒排索引
   */
    const vector<backwardIdx>* Index::GetBackwardIdx(const string& key){
        auto it = inverted_index.find(key);
        if(it == inverted_index.end())
            return nullptr;
        
        return &(it->second);
    }
}
