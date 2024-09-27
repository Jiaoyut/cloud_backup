#ifndef __MY__SERVER__
#define __MY__SERVER__

#include "data.hpp"
#include "httplib.h"


extern cloud::DataManager* _data;
namespace cloud {
    class Server {
    private:
        int _srv_port = 6666;//服务器的绑定监听端口
        std::string _url_prefix = "/download/";
        std::string _backup_dir = "./backup_dir/";//上传文件的备份存储路径    
        httplib::Server _srv;
    private:
        static void Upload(const httplib::Request& req, httplib::Response& rsp) {
            //std::cout << "in Upload\n";
            std::string _backup_dir = "./backup_dir/";//上传文件的备份存储路径
            //判断有没有对应标识的文件上传区域数据
            if (req.has_file("file") == false) {//判断有没有name的字段值是file标识的区域                                                                                                       
                std::cout << "Upload file data format error!\n";
                rsp.status = 400;
                return;
            }
            //获取解析后的区域数据
            httplib::MultipartFormData data = req.get_file_value("file");
            //组织文件实际存储路径
            std::string realpath = _backup_dir + data.filename;
            //像文件中写入数据，实际就是吧文件备份起来
            if (FileUtil(realpath).Write(data.content) == false) {
                std::cout << "back file failed!\n";
                rsp.status = 500;
                return;
            }

            //新增备份信息
            if (_data->Insert(realpath) == false) {
                std::cout << "Insert back info failed!\n";
                rsp.status = 500;
                return;
            }
            // std::cout << data.name << std::endl;//区域字段标识名
            // std::cout << data.filename << std::endl;//文件上传则保存文件名称
            // std::cout << data.content << std::endl;//区域正文数据，即上传文件内容数据
            rsp.status = 200;
            std::cout << "new backup:" << realpath << std::endl;
            return;
        }
        static std::string StrTime(time_t t) {
            return std::asctime(std::localtime(&t));
        }
        static void List(const httplib::Request& req, httplib::Response& rsp) {
            //获取所有历史备份信息，并且根据这些信息组织出来一个html页面，作为响应正文
            std::vector<FileInfo> arry;
            if (_data->SelectAll(&arry) == false) {
                std::cout << "select all back info filed\n";
                rsp.status = 500;
                return;
            }
            std::stringstream ss;
            ss << "<html>";
            ss << "<head>";
            ss << "<meta http-quiv='Content-Type' content='text/html;charset=utf-8'>";
            ss << "<title>Download</title>";
            ss << "</head>";
            ss << "<body>";
            ss << "<h1>Download</h1>";
            ss << "<table>";
            for (auto& e : arry) {
                //组织每一行的页面标签
                ss << "<tr>";
                //<td><a href='"实际路径"'>" << 文件名称 << "</a></td>
                ss << "<td><a href='" << e.url_path << "'>" << e.filename << "</a></td>";

                ss << "<td align='right'>" << StrTime(e.back_time) << "</td>";

                ss << "<td align='right'>" << e.file_size / 1024 << "KB </td>";

                ss << "</tr>";
            }
            ss << "</table>";
            ss << "</body>";
            ss << "</html>";

            rsp.set_content(ss.str(), "text/html");
            rsp.status = 200;
            return;
        }
        static std::string StrETag(const std::string& filename) {
            //etag是一个文件的唯一标识，当文件被修改后会发生变化
            //这里的使用文件大小——最后修改时间
            time_t mtime = FileUtil(filename).MTime();
            size_t fsize = FileUtil(filename).Size();
            std::stringstream ss;
            ss << fsize << "-" << mtime;
            return ss.str();
        }
        static void Download(const httplib::Request& req, httplib::Response& rsp) {
            std::cout << "in download\n";
            FileInfo info;
            if (_data->SelectOne(req.path, &info) == false) {
                std::cout << "select all back info failed!\n";
                rsp.status = 404;
                return;
            }
            //如果文件已经被压缩了，则要先解压缩，然后再去源文件读取数据
            if (info.pack_flag) {
                FileUtil(info.pack_path).UnCompress(info.real_path);
            }
            if (req.has_header("If_Range")) {
                std::string old_etag = req.get_header_value("If-Range");//If-Range:ETag信息
                //size_t start = req.ranges[0].first;
                //size_t end = req.ranges[0].second;//如果没end数字，则表示到文件末尾，httplib会将second设置为-1
                //这时候从文件start位置开始读取end-start+1长度的数据，如果end是-1，则是文件长度-start长度
                //因为假设1000长度的文件，请求900-999，则返回包含900在内共100查长度的数据
                //而如果请求900-，1000长度末尾位置其实就是999，则直接长度减去900即可
                std::string cur_etag = StrETag(info.real_path);
                if (old_etag == cur_etag) {
                    //size_t start = req.ranges[0].first;
                    //size_t end = req.ranges[0].second;//如果没end数字，则表示到文件末尾，httplib会将second设置为-1
                    //这时候从文件start位置开始读取end-start+1长度的数据，如果end是-1，则是文件长度-start长度
                    //因为假设1000长度的文件，请求900-999，则返回包含900在内共100查长度的数据
                    //而如果请求900-，1000长度末尾位置其实就是999，则直接长度减去900即可  

                    //httplib已经替我们完成了断电续传功能，我们只需要将文件数据放到body就行
                    //然后设置响应状态码为206，httplib检测到206，就会从body中截取指定区间数据进行响应
                    FileUtil(info.real_path).Read(&rsp.body);
                    rsp.set_header("Content-Type", "application/octet-stream");//设置正文类型为二进制流
                    rsp.set_header("Accept-Ranges", "bytes");//告诉客户端我支持断点续传
                    rsp.set_header("Content-Range", "b");
                    rsp.set_header("ETag", cur_etag);
                    rsp.status = 206;
                    return;
                }

            }
            FileUtil(info.real_path).Read(&rsp.body);
            rsp.set_header("Content-Type", "application/octet-stream");//设置正文类型为二进制流
            rsp.set_header("Accept-Ranges", "bytes");//告诉客户端我支持断点续传
            rsp.set_header("ETag", StrETag(info.real_path));
            rsp.status = 200;
            return;
        }
    public:
        Server() {
            FileUtil(_backup_dir).CreateDirectory();//创建目录
        }
        bool RunModule() {
            //搭建http服务器
            //简历请求-处理函数映射关系
            //Post(请求的资源路径，对应的业务处理回调函数)；
            _srv.Post("/upload", Upload);
            _srv.Get("/list", List);//展示页面的请求
            std::string regex_download_path = _url_prefix + "(.*)";
            _srv.Get(regex_download_path, Download);
            //启动服务器
            //std::cout << "signed over!\n";
            _srv.listen("0.0.0.0", _srv_port);
            // std::cout << "listen out\n";
            return true;
        }

    };
}

#endif