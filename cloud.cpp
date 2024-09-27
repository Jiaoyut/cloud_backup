/*************************************************************************
        > File Name: cloud.cpp
        > Author: Jiao Yutong
        > Mail: 1624701240@qq.com
        > Created Time: Mon 09 Sep 2024 05:22:51 PM CST
 ************************************************************************/

#include "hot.hpp"
#include "server.hpp"
#include <unordered_map>
#include <thread>
void FileUtilTest() {
    //cloud::FileUtil("./testdir/adir").CreateDirectory();//在当前目录下递归创建一个目录
   // cloud::FileUtil("./testdir/a.txt").Write("Hello bit\n");//创建一个文件并写入信息
   // std::string body;
    //cloud::FileUtil("./testdir/a.txt").Read(&body);
    //std::cout<<body<<std::endl;
    //std::cout<<cloud::FileUtil("./testdir/a.txt").Size()<<std::endl;
    //std::cout<<cloud::FileUtil("./testdir/a.txt").MTime()<<std::endl;
    //std::cout<<cloud::FileUtil("./testdir/a.txt").ATime()<<std::endl;
    std::vector<std::string> arry;
    cloud::FileUtil("./testdir").ScanDirectory(&arry);
    for (auto& a : arry) {
        std::cout << a << std::endl;
    }
}

void JsonTest() {
    Json::Value val;
    val["name"] = "xiao hu";
    val["age"] = 18;
    val["sex"] = "man";
    val["grade"].append(77.5);
    val["grade"].append(81);
    val["grade"].append(90);
    std::string body;
    cloud::JsonUtil::Serialize(val, &body);
    std::cout << body << std::endl;

    Json::Value root;
    cloud::JsonUtil::UnSerialize(body, &root);
    std::cout << root["name"].asString() << std::endl;
    std::cout << root["age"].asInt() << std::endl;
    std::cout << root["sex"].asString() << std::endl;
    // for(int i=0;i<3;++i){
    for (auto& e : root["grade"]) {
        std::cout << e.asFloat() << std::endl;
    }
}

void DataTest() {
    cloud::DataManager data;
    std::vector<cloud::FileInfo> arry;
    data.SelectAll(&arry);
    for (auto& e : arry) {
        std::cout << e.filename << std::endl;
        std::cout << e.url_path << std::endl;
        std::cout << e.real_path << std::endl;
        std::cout << e.file_size << std::endl;
        std::cout << e.back_time << std::endl;
        std::cout << e.pack_flag << std::endl;
        std::cout << e.pack_path << std::endl;
    }
    /*

    data.Insert("./backup_dir/hello.txt");
    data.UpdateStatus("./backup_dir/hello.txt",true);
    std::vector<cloud::FileInfo> arry;
    data.SelectAll(&arry);
    for(auto& e:arry){
        std::cout<<e.filename<<std::endl;
        std::cout<<e.url_path<<std::endl;
        std::cout<<e.real_path<<std::endl;
        std::cout<<e.file_size<<std::endl;
        std::cout<<e.back_time<<std::endl;
        std::cout<<e.pack_flag<<std::endl;
        std::cout<<e.pack_path<<std::endl;
    }
    std::cout<<"--------------------------\n";

    data.UpdateStatus("./backup_dir/hello.txt",false);
    cloud::FileInfo info;
    data.SelectOne("/download/hello.txt",&info);
    std::cout<<info.filename<<std::endl;
    std::cout<<info.url_path<<std::endl;
    std::cout<<info.real_path<<std::endl;
    std::cout<<info.file_size<<std::endl;
    std::cout<<info.back_time<<std::endl;
    std::cout<<info.pack_flag<<std::endl;
    std::cout<<info.pack_path<<std::endl;


    std::cout<<"----------delete-----------\n";
    data.DeleteOne("/download/hello.txt");

    arry.clear();
    data.SelectAll(&arry);
    for(auto& e:arry){
        std::cout<<e.filename<<std::endl;
        std::cout<<e.url_path<<std::endl;
        std::cout<<e.real_path<<std::endl;
        std::cout<<e.file_size<<std::endl;
        std::cout<<e.back_time<<std::endl;
        std::cout<<e.pack_flag<<std::endl;
        std::cout<<e.pack_path<<std::endl;
    }*/
}

void compress_test() {
    cloud::FileUtil("./hello.txt").Compress("hello.zip");
    cloud::FileUtil("./hello.zip").UnCompress("bit.txt");
}

cloud::DataManager* _data;
void HotTest() {
    _data = new cloud::DataManager();
    cloud::HotManager cloud;
    cloud.RunModule();
}

void ServerTest() {
    cloud::Server srv;
    //std::cout << "in test\n";
    srv.RunModule();
}


int main() {
    _data = new cloud::DataManager();
    //FileUtilTest();
    //JsonTest();
    //compress_test();
    //DataTest();
    //HotTest();
    //ServerTest();
    std::thread hot_thread(HotTest);
    std::thread srv_thread(ServerTest);

    hot_thread.join();
    srv_thread.join();
    return 0;
}
