#ifndef __MY_UTIL__
#define __MY_UTIL__
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <time.h>
#include <experimental/filesystem>
#include <sys/stat.h>
#include <jsoncpp/json/json.h>
#include "bundle.h"
namespace fs = std::experimental::filesystem;

using namespace std;

namespace cloud {

    class FileUtil {
    private:
        std::string _name;

    public:
        FileUtil(const std::string& name) :_name(name) {}
        std::string Name() {
            return fs::path(_name).filename().string();
        }
        bool Exists() {
            return fs::exists(_name);
        }//文件是否存在
        size_t Size() {
            if (this->Exists() == false) { return 0; }
            return fs::file_size(_name);
        }//文件大小
        time_t MTime() {
            if (this->Exists() == false) { return 0; }
            auto ftime = fs::last_write_time(_name);
            std::time_t cftime = decltype(ftime)::clock::to_time_t(ftime);
            return cftime;
        }//最后一次修改时间
        time_t ATime() {
            if (this->Exists() == false) { return 0; }
            struct stat st;
            stat(_name.c_str(), &st);
            return st.st_atime;
        }//对后一次访问时间


        bool Read(std::string* body) {
            if (this->Exists() == false) { return false; }
            std::ifstream ifs;
            ifs.open(_name, std::ios::binary);//以二进制方式打开文件
            if (ifs.is_open() == false) {
                cout << "Read open filed\n";
                return false;
            }
            size_t fsize = this->Size();

            body->resize(fsize);
            ifs.read(&(*body)[0], fsize);//从body起始位置写入fsize长度
            if (ifs.good() == false) {
                std::cout << "read file filed!\n";
                ifs.close();
                return false;
            }
            ifs.close();
            return true;
        }//读取文件所有数据到body中
        bool Write(const std::string& body) {
            std::ofstream ofs;
            ofs.open(_name, std::ios::binary);
            //std::cout << "_name="<<_name << std::endl;
            if (ofs.is_open() == false) {
                std::cout << "open filed!\n";
                return false;
            }
            ofs.write(body.c_str(), body.size());
            if (ofs.good() == false) {
                std::cout << "write file filed!\n";
                ofs.close();
                return false;
            }
            ofs.close();
            return true;
        }//将body数据写入文件

        bool CreateDirectory()//创建目录
        {
            if (this->Exists()) { return true; }
            fs::create_directories(_name);
            return true;
        }
        bool ScanDirectory(std::vector<std::string>* arry)//遍历目录，获取目录下所有文件路径名
        {
            if (this->Exists() == false) { return false; }

            for (auto& a : fs::directory_iterator(_name)) {
                if (fs::is_directory(a) == true) {
                    continue;//如果当前文件是一个文件夹，则不处理，遍历下一个
                }
                //std::string pathname = fs::path(a).filename().string();//只有文件名
                std::string pathname = fs::path(a).relative_path().string();
                arry->push_back(pathname);
            }
            return true;
        }

        bool Remove()//删除文件
        {
            if (this->Exists() == false) {
                return true;
            }
            fs::remove_all(_name);
            return true;
        }

        bool Compress(const std::string& packname) {
            if (this->Exists() == false) { return false; }

            std::string body;
            if (this->Read(&body) == false) {
                std::cout << "Compress read file error!\n";
                return false;
            }
            //std::cout << body << std::endl;
            std::string packed = bundle::pack(bundle::LZIP, body);
           
            if (FileUtil(packname).Write(packed) == false) {
                std::cout << "Compress write pack data failed!\n";
                return false;
            }
            fs::remove_all(_name);
            return true;
        }
        bool UnCompress(const std::string& filename) {
            if (this->Exists() == false) {
                return false;
            }
            std::string body;
            if (this->Read(&body) == false) {
                std::cout << "Uncompress read pack data failed\n";
                return false;
            }
            std::string unpack_data = bundle::unpack(body);
            if (FileUtil(filename).Write(unpack_data) == false) {
                std::cout << "uncompress write file data failed!\n";
                return false;
            }
            fs::remove_all(_name);
            return true;
        }
    };

    class JsonUtil {
    public:
        //定义成静态成员函数，没有this指针，外界可以通过JsonUtil::Serialize()进行调用，不需要实例化对象
        static bool Serialize(Json::Value& val, std::string* body)//序列化
        {
            Json::StreamWriterBuilder swb;
            Json::StreamWriter* sw = swb.newStreamWriter();
            std::stringstream ss;
            int ret = sw->write(val, &ss);
            if (ret != 0) {
                std::cout << "Serial error" << endl;
                delete sw;
                return false;
            }
            *body = ss.str();
            delete sw;
            return true;

        }
        static bool UnSerialize(const std::string& body, Json::Value* val)//反序列化
        {
            Json::CharReaderBuilder crb;
            Json::CharReader* cr = crb.newCharReader();
            std::string err;
            //prase(strbegin,strend,Json::Value,错误信息string)
            bool ret = cr->parse(body.c_str(), body.c_str() + body.size(), val, &err);
            if (ret == false) {
                cout << "unserialize error" << endl;
                delete cr;
                return false;
            }
            delete cr;
            return true;
        }

    };

}

#endif
