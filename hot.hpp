#ifndef __MY_HOT__
#define __MY_HOT__

#include "data.hpp"
#include <unistd.h>

extern cloud::DataManager* _data;//全局变量，指向DataManager对象

namespace cloud {
    class HotManager {
    private:
        int _hot_time = 30; //热点判断时长，应该是一个可配置项，当前简化，默认30s
        std::string _backup_dir = "./backup_dir/";//要检测的源文件路径
    public:
        HotManager() {
            FileUtil(_backup_dir).CreateDirectory();
        }

        bool IsHot(const std::string& filename) {
            time_t atime = FileUtil(filename).ATime();
            time_t ctime = time(NULL);
            if ((ctime - atime) > _hot_time) {
                return false;
            }
            return true;
        }
        bool RunModule() {
            while (1) {
                //1.遍历目录
                std::vector<std::string> arry;
                FileUtil(_backup_dir).ScanDirectory(&arry);
                //2.获取信息
                for (auto& file : arry) {
                    //3.获取指定文件时间属性，以及当前系统时间，进行热点判断
                    if (IsHot(file)) {
                        continue;
                    }
                    //3.5获取当前文件的历史信息
                    FileInfo info;
                    bool ret = _data->SelectOneByRealpath(file, &info);
                    if (ret == false) {
                        //当前检测到的文件，没有历史备份信息，这可能是一个异常上传的文件，删除处理
                        std::cout << "An exception file detected,delete it" << std::endl;
                        FileUtil(file).Remove();
                        continue;
                        //对于检测到没有历史信息的文件，也可以进行新增,然后进行压缩：
                        //_data->Insert(file);
                        //_data->SelectOneByRealpath(file, &info);
                    }
                    //4.非热点进行压缩存储
                    FileUtil(file).Compress(info.pack_path);

                    //5.压缩后进行备份信息修改
                    _data->UpdateStatus(file, true);
                    std::cout << info.real_path << "<--compress-->" << info.pack_path << std::endl;
                }
                usleep(1000);//避免空目录情况下，空遍历消耗cpu
            }
            return true;
        }
    };
}



#endif