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
    const char* const DICT_PATH         = "../jieba_dict/jieba.dict.utf8";
    const char* const HMM_PATH          = "../jieba_dict/hmm_model.utf8";
    const char* const USER_DICT_PATH    = "../jieba_dict/user.dict.utf8";
    const char* const IDF_PATH          = "../jieba_dict/idf.utf8";
    const char* const STOP_WORD_PATH    = "../jieba_dict/stop_words.utf8";

    const string defaultIndexModel      = "hash_invert";
    const string samInvertedIndexModel  = "sam_invert";

    /*
    建立索引
    */
    bool Index::Build(const string& input_path, const string& indexModel){
        // 按行读取 存放于处理解析出来的数据文件
        cout<<input_path<<" build index begin "<<endl;
        std::ifstream file(input_path.c_str());
        if(!file.is_open()){
            cout<<input_path<<" file open error "<<endl;
            return false;
        }
        
        // 索引类型
        _indexModel = indexModel;
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
            if(doc_info->_docId % 100 == 0){
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
        // sam_invert模型
        if(_indexModel == samInvertedIndexModel) {
            // 标题：10权值 内容：5权值 
            ExtendString(doc_info._title, doc_info, 10);
            ExtendString(doc_info._content, doc_info, 1);

            return;
        }
        // hash_invert模型

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

    Index::Index() :jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH), _indexModel("hash_invert")
    {
        forward_index.clear();
        // todo; 清空所有索引类型
        inverted_index.clear();
        sam = new SAM();
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
        // 根据索引类型查找
        if(_indexModel == samInvertedIndexModel)
            return sam->Search(key);

        auto it = inverted_index.find(key);
        if(it == inverted_index.end())
            return nullptr;
        
        return &(it->second);
    }

    /*
    获取索引模式
    */
    string Index::getIndexModel(){
        return _indexModel;
    } 



    // 搜索模块
    const int lengthText = 160;

    /*
    初始化 构建指定文档索引
    */
    bool Searcher::Init(const string& input_path, const string& indexModel){
       return index->Build(input_path, indexModel);
   }

    /*
    指定文本进行搜索
    */
    bool Searcher::Search(const string& query, string* output){
        vector<backwardIdx> wordsResult;

        // 根据索引模式处理
        if(index->getIndexModel() == defaultIndexModel){
            // 分词
            vector<string> words;
            index->CutWord(query, &words);

            // 触发 根据分词的结果 进行倒排索引 得到相关文档
            for(string word : words){
                boost::to_lower(word);
                auto* backList = index->GetBackwardIdx(word);
                if(backList == nullptr){
                    // 没有这个关键词
                    continue;
                }
                // 插入多个数据
                wordsResult.insert(wordsResult.end(), backList->begin(), backList->end());
            }

        }else if(index->getIndexModel() == samInvertedIndexModel){
            // 后缀自动机查找query，返回倒排表
            auto* backList = index->GetBackwardIdx(query);
            if(backList == nullptr){
                cout<<"backlist is null"<<endl;
                return false;
            }
            wordsResult.insert(wordsResult.end(), backList->begin(), backList->end());
        }

        if(wordsResult.size() == 0){
            // 没有与内容相关数据 404 Not Found
            return false;
        }

            // 排序
        std::sort(wordsResult.begin(), wordsResult.end(),
                [](const backwardIdx& le, const backwardIdx& ri){
                    return le._weight > ri._weight;
                });
        
        

        // 包装
        Json::Value value;
        for(const auto& backidx : wordsResult){
            // 根据 id 查找正排索引
            const frontIdx* doc_info = index->GetFrontIdx(backidx._docId);
            
            Json::Value tmp;
            tmp["title"]    = doc_info->_title;
            tmp["url"]      = doc_info->_url;
            tmp["desc"]     = GetShowContent(doc_info->_content, backidx._word);
            value.append(tmp);
        }

        Json::FastWriter writer;
        *output = writer.write(value);
        return true;
    }

    /*
    得到关键字前后的数据 在前端页面显示的文本
    */
    string Searcher::GetShowContent(const string& content,const string& word){
        size_t idx = content.find(word);
        string ans("");
        int pos = 0;    // 显示文本开始的位置
        int len = lengthText;   // 截取显示文本的长度
        if(idx == string::npos){
            //关键字不存在
            len = std::min(len, (int)content.size());
        }else{
            pos = std::max(0, (int)((int)idx-lengthText/2));
            len = std::min((int)lengthText/2, (int)(content.size() - idx));
        }
        ans = content.substr(pos, len);
        ans += "...";
        return ans;
    }

    /*
    sam构造函数
    */
   SAM::SAM()
    : _size(1), _last(1)
   {
       state* s0 = new state();
       state* s1 = new state();
       stPos.push_back(s0);
       stPos.push_back(s1);
   }

    /*
    添加一个字符到sam, 权重
    */
    void SAM::Extend(char c, int score, const frontIdx& doc_info){

        int p = _last, cur = ++_size;
        // 当前新增字符状态
        state *curSt = new state();

        // 添加倒排表节点
        curSt->addDocInfoToInvert(doc_info, score);
        stPos.push_back(curSt);
        stPos[cur]->len = stPos[_last]->len + 1;
        stPos[cur]->siz = 1;
        for(p = _last; p && !stPos[p]->to[c] ; p = stPos[p]->link) {
            stPos[p]->to[c] = cur;
        }
        
        if(!p)  stPos[cur]->link = 1; // link指向开始节点
        else{
            int q = stPos[p]->to[c];

            // solution2 - A类
            if(stPos[q]->len == stPos[p]->len + 1){
                stPos[cur]->link = q;
                int fa = stPos[cur]->link;
       
                // 后缀路径依次添加倒排节点和添加权重
                while(fa != 1){
                    stPos[fa]->addDocInfoToInvert(doc_info, score);
                    fa = stPos[fa]->link;
                }
            }else{
                // solution2 - B类
                int cl = ++_size;
                state* clone = new state();
                stPos.push_back(clone);
                stPos.push_back(clone);
                stPos[cl]->to               = stPos[q]->to;
                stPos[cl]->link             = stPos[q]->link;
                stPos[cl]->inverted_table   = stPos[q]->inverted_table;
                while(p && stPos[p]->to[c] == q){
                    stPos[p]->to[c] = cl;
                    p = stPos[p]->link;
                }  

                stPos[cur]->link = stPos[q]->link = cl;
                // 添加权重
                stPos[cl]->addDocInfoToInvert(doc_info, score);
            }
        }
       
        _last = cur;
    }

    /*
    搜索一个字符串的倒排表
    */
    const vector<backwardIdx>* SAM::Search(string pattern){
        int p = 1;
        int lenp = pattern.size();
        int i = 0;
        // 状态转移
        for(; i<lenp; i++){
            char c = pattern[i];
            if(stPos[p]->to.find(c) == stPos[p]->to.end()) break;
            else p = stPos[p]->to[c];
        }

        if(i<lenp) {
            cout<<"sam search i<lenp"<<endl;
            return nullptr;
        }
        return &(stPos[p]->inverted_table);
    }

    /*
    添加一个字符串string到sam
    */
    void Index::ExtendString(string str, const frontIdx& doc_info, int score){
        int lens = str.size();

        for(int i = 0; i<lens; i++){
            sam->Extend(str[i], score, doc_info);
        }
    }

}
