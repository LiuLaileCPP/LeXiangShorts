#include <iostream>
#include <cstdlib> // for std::system
#include <string>
bool transcode(const std::string& inputFile, const std::string& outputFile) {
    // 构建 FFmpeg 命令
    /*需要转成 H.264 和 AAC 以满足客户端的需求使用这个
std::string command = "ffmpeg -i " + inputFile + " -c:v libx264 -c:a aac -start_number 0 -hls_time 10
-hls_list_size 0 -f hls " + outputFile;*/
    //直接复制编码, 可以快速实现文件切分 ts 文件
    std::string command = "ffmpeg -i " + inputFile + " -codec: copy -start_number 0 -hls_time 10 -hls_list_size 0 -f hls " + outputFile;
            // 调用 FFmpeg 进行转码
    int result = std::system(command.c_str());
    // 检查转码是否成功
    if (result == 0) {
        std::cout << "Transcoding completed successfully." << std::endl;
        return true;
    } else {
        std::cerr << "Transcoding failed with error code: " << result << std::endl;
        return false;
    }
}int main() {
    std::string inputFile = "/home/lll/Videos/flv/atman.mp4";
    std::string filename = "output.m3u8";
    std::string outputFile = "/home/lll/tmp/hls/" + filename ; // 注意 路径需要提前创建好
    std::cout << outputFile << std::endl;
    if (transcode(inputFile, outputFile)) {
        std::cout << "Continue with the next steps." << std::endl;
        std::cout << "play this url= http://localhost:80/hls/" + filename << std::endl;
        // 在这里继续执行后续操作
    } else {
        std::cerr << "Handle the error." << std::endl;
        // 在这里处理错误
    }
    return 0;
}
