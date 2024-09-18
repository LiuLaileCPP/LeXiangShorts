#include"mymd5.h"
#include <fstream>
#include <cryptopp/cryptlib.h>
#include <cryptopp/files.h>
#include <string.h>
#include <cryptopp/md5.h>
#include <cryptopp/hex.h>
#include <iostream>
using namespace std;
using namespace CryptoPP;
string getMD5( byte* buf , int len )
{
    // 创建 MD5 对象
    CryptoPP::Weak1::MD5 md5;
    // 计算消息的 MD5 哈希值
    byte digest[CryptoPP::Weak1::MD5::DIGESTSIZE];
    md5.CalculateDigest(digest, (const byte*)buf , len );
    // 将哈希值转换为十六进制字符串
    CryptoPP::HexEncoder encoder;
    std::string encoded;
    encoder.Attach(new CryptoPP::StringSink(encoded));
    encoder.Put(digest, sizeof(digest));
    encoder.MessageEnd();
    std::cout << "MD5 Hash: " << encoded << std::endl;
    return encoded;
}

string getFileMD5( string path )
{
    std::string filename = path; std::string digest;
    // Read file
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return string();
    }
    // Create MD5 hash
    CryptoPP::Weak1::MD5 md5;
    CryptoPP::FileSource(file, true, new HashFilter(md5, new CryptoPP::HexEncoder(new StringSink(digest))));
    file.close();
    // Print MD5 hash
    std::cout << "MD5 hash of file " << filename << ": " << digest << std::endl;
    return digest;
}
//因为是使用 MD5 工具 , 可以认为是在网上下载的, 实现流程就不解释了
//通过 string getFileMD5( string path ); 可以获取到文件 MD5
