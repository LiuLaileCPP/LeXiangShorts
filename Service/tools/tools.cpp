#include "tools.h"

Tools::Tools() {}

void Tools:: CodeConv_utf8ToGb2312(QString src,char*res ,int nRes)
{
    QTextCodec* dc = QTextCodec::codecForName("gb2312");
    QByteArray ba = dc->fromUnicode(src);
    if(ba.length() <= nRes)
        memcpy(res,ba.data(),ba.length());
    else
    {
        qDebug()<<"CodeConv:: utf8ToGb2312 Src is too long";
    }
}

QString Tools:: CodeConv_gb2312ToUtf8(char* src)
{
    QTextCodec* dc = QTextCodec::codecForName("gb2312");
    return dc->toUnicode(src).toStdString().c_str(); //转换为Unicode的UTF-8格式
}

//手机号 邮箱格式检查 密码格式检查
//返回值 0格式全正确 1手机号为空 2密码为空 3手机号有空格 4密码有空格
//5手机号长度不对 6密码长度不对 7手机号不规范 8密码不规范
void Tools::Check_telAndPwd(QString account,QString password,int* res,int len)
{
    QString tel = account;
    QString tel_temp = account;
    QString password_temp = password;
    // 2 校验数据的合法性
    //判断是否是空字符串 或者是全空格
    res[0] = 1;
    if(len > 2 && (password.isEmpty() || password_temp.remove(" ").isEmpty()))
    {
        res[0] = 1 - (res[2] = 1);
    }
    if(len > 4 && (password.indexOf(QString(" ")) != -1))
    {
        res[0] = 1 - (res[4] = 1);
    }
    if(len > 1 && (tel.isEmpty() || tel_temp.remove(" ").isEmpty()))
    {
        res[0] = 1 - (res[1] = 1);
    }
    if(len > 3 && (tel.indexOf(QString(" ")) != -1))
    {
        res[0] = 1 - (res[3] = 1);
    }
    //检查长度是否合法
    if(len > 5 && (tel.length() <= 2)) //todo 改成!=11
    {
        res[0] = 1 - (res[5] = 1);
    }
    if(len > 6 && (password.length() > 15 || password.length() <3))
    {
        res[0] = 1 - (res[6] = 1);
    }
    //检查内容是否合法(tel为数字 name password 3-15位 为大小写字母 数字 下划线的组合)
    if(len > 7 && 0)
    {
        res[0] = 1 - (res[7] = 1);
    }
    if(len > 8 && 0)
    {
        res[0] = 1 - (res[8] = 1);
    }
    return;
}

//返回值0格式全正确 1邮箱为空 2密码为空 3邮箱有空格 4密码有空格
//5邮箱长度不对 6密码长度不对 7邮箱不规范 8密码不规范
void Tools::Check_emailAndPwd(QString account,QString password,int* res,int len)
{
    return;
}

QByteArray Tools::GetMD5(QString val)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    QString tmp = QString("%1_%2").arg(val).arg(MD5_KEY);
    hash.addData(tmp.toStdString().c_str());
    QByteArray bt = hash.result();
    return bt.toHex(); //转16进制
}
















