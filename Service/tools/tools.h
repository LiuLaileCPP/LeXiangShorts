#ifndef TOOLS_H
#define TOOLS_H
#include <QString>
#include <QTextCodec>
#include <QDebug>
#include <QCryptographicHash>
#define MD5_KEY 12345


class Tools
{
public:
    Tools();

    //编码转换(QT---utf-8 VS数据库---gb2312)

    //UTF-8转GB2312
    static void CodeConv_utf8ToGb2312(QString src,char*res ,int nRes);
    //GB2312转UTF-8
    static QString CodeConv_gb2312ToUtf8(char* src);


    //手机号 邮箱格式检查 密码格式检查
    //返回值0格式全正确 1手机号为空 2密码为空 3手机号有空格 4密码有空格
    //5手机号长度不对 6密码长度不对 7手机号不规范 8密码不规范
    static void Check_telAndPwd(QString account,QString password,int* res,int resLen);

    //返回值0格式全正确 1邮箱为空 2密码为空 3邮箱有空格 4密码有空格
    //5邮箱长度不对 6密码长度不对 7邮箱不规范 8密码不规范
    static void Check_emailAndPwd(QString account,QString password,int* res,int resLen);

    //字符串加密
    static QByteArray GetMD5(QString val);
};

#endif // TOOLS_H
