#include "clogic.h"
#include <QThread>

//mysql tables: t_users t_userslabel t_videolabel t_usergot t_userseen
void CLogic::setNetPackMap()
{
    NetPackMap(_DEF_TCP_REGISTER_RQ)    = &CLogic::RegisterRq;
    NetPackMap(_DEF_TCP_LOGIN_RQ)       = &CLogic::LoginRq;
    NetPackMap(_DEF_TCP_UPLOAD_RQ)      = &CLogic::UploadRq;
}

#define _DEF_COUT_FUNC_    cout << "clientfd:"<< clientfd <<" " <<__func__ << " thread: "<<QThread::currentThread()<<endl;

//注册
void CLogic::RegisterRq(sock_fd clientfd,char* szbuf,int nlen)
{
    //cout << "clientfd:"<< clientfd << __func__ << endl;
    _DEF_COUT_FUNC_

            // 1 拆包
            _STRU_TCP_REGISTER_RQ* rq = (_STRU_TCP_REGISTER_RQ*)szbuf;

    //msg.data1 0是手机号注册，1是邮箱注册
    if (rq->regMode == reg_mode_email)
    {
        //检测邮箱格式

        //读数据库查看是否注册

        //写数据库

        //回复注册结果

        return;
    }

    //手机号注册

    // 2 校验手机号的合法性
    string tel = rq->tel;
    string tel_temp = rq->tel;
    string password = rq->password;
    string password_temp = rq->password;
    _STRU_TCP_REGISTER_RS msgrs;

    //判断是否是空字符串 或者是全空格
    int ite = 0;

    while (ite != -1)
    {
        ite = tel_temp.find(" ");
        if (ite != -1) tel_temp.erase(ite, ite + 1);
    }
    while (ite != -1)
    {
        ite = password_temp.find(" ");
        if (ite != -1) password_temp.erase(ite, ite + 1);
    }
    if (password.empty() || tel.empty()
            || password_temp.empty()
            || tel_temp.empty())
    {
        cout << "kernel:: dealRegRq 注册信息输入为空白 name:"<<rq->name << endl;
        msgrs.result = client_error;
        m_tcp->SendData(clientfd,(char*)&msgrs,sizeof(msgrs));
        return;
    }
    //检查长度是否合法（tel = 11,password 3-15）
    if (tel.length() != 11 || password.length() > 15 || password.length() < 3)
    {
        cout << "kernel:: dealRegRq 注册信息输入长度有误" << endl;
        msgrs.result = client_error;
        m_tcp->SendData(clientfd,(char*)&msgrs,sizeof(msgrs));
        return;
    }
    //检查内容是否合法(tel为数字 name password为大小写字母 数字 下划线的组合)
    if (0)
    {
        cout << "kernel:: dealRegRq 注册信息输入格式有误，请重试" << endl;
        msgrs.result = client_error;
        m_tcp->SendData(clientfd,(char*)&msgrs,sizeof(msgrs));
        return;
    }

    _STRU_TCP_REGISTER_RS msg;
    //读数据库查看是否注册
    list<string> listRes;
    char sqlBuf[1024];
    sprintf(sqlBuf,"select tel,password from t_users where tel = '%s';",rq->tel);
    if (!m_sql->SelectMysql(sqlBuf,2,listRes))
    {
        //查询失败
        cout << "kernel:: dealRegRq 数据库查询失败" << endl;

        //发一个注册回复包
        msg.result = mysql_select_error;
        m_tcp->SendData(clientfd,(char*)&msg,sizeof(msg));
        return;
    }
    if (listRes.size() > 0)
    {
        //被注册过了

        cout << "kernel:: dealRegRq 手机号已被注册" << endl;
        msg.result = register_false_tel_used;
        m_tcp->SendData(clientfd,(char*)&msg,sizeof(msg));
        return;
    }

    //写入数据库
    sprintf(sqlBuf, "insert into t_users (tel,name,password) values ('%s','%s','%s');"
            ,rq->tel,rq->name,rq->password);
    if (!m_sql->UpdataMysql(sqlBuf))
    {
        cout << "kernel:: dealRegRq 新用户写入数据库t_users失败" << endl;
        msg.result = mysql_insert_error;
        m_tcp->SendData(clientfd,(char*)&msg,sizeof(msg));
        return;
    }
    //查找这个用户的id 给他发过去
    listRes.clear();
    sprintf(sqlBuf, "select id from t_users where tel = '%s';", rq->tel);
    if (!m_sql->SelectMysql(sqlBuf, 1, listRes))
    {
        //查询失败
        cout << "kernel:: dealRegRq 写入后查询id失败" << endl;

        //发一个注册回复包
        msg.result = mysql_select_error;
        m_tcp->SendData(clientfd,(char*)&msg,sizeof(msg));
        return;
    }
    if (listRes.size() != 1)
    {
        cout << "kernel:: dealRegRq 查询id成功 listRes为空" << endl;
        return;
    }
    msg.userId = stoi(listRes.front());

    //写入数据库
    sprintf(sqlBuf, "insert into t_userslabel (id, car,comic,dance,fashion,film "
                    ",lovered,game ,laugh ,luxury ,music,news,pet ,science,sport,tourism,vlog) "
                    "values (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d);",msg.userId
            ,rq->label.cb_car,rq->label.cb_comic,rq->label.cb_dance,rq->label.cb_fashion,rq->label.cb_film
            ,rq->label.cb_first,rq->label.cb_game,rq->label.cb_laugh,rq->label.cb_luxury,rq->label.cb_music
            ,rq->label.cb_news,rq->label.cb_pet,rq->label.cb_science,rq->label.cb_sport,rq->label.cb_tourism,rq->label.cb_vlog);
    if (!m_sql->UpdataMysql(sqlBuf))
    {
        cout << "kernel:: dealRegRq 新用户写入数据库t_userslabel失败" << endl;
        msg.result = mysql_insert_error;
        m_tcp->SendData(clientfd,(char*)&msg,sizeof(msg));
        return;
    }

    //new user  make a dir for him
    char path[_DEF_NAME_LENGTH];
    sprintf(path,"%s/flv/%s/",VIDEO_ROOT_PATH,rq->name); //"/home/lll/Videos/flv/user_name/"
    mkdir(path,S_IRWXU); //S_IRWXU can write and read

    //写入和注册成功
    msg.result = register_success;
    int num = m_tcp->SendData(clientfd,(char*)&msg,sizeof(msg));
    if(num > 0) cout << "kernel:: dealRegRq 新用户注册成功" << endl;
    return;

}

//登录
void CLogic::LoginRq(sock_fd clientfd ,char* szbuf,int nlen)
{
    //    cout << "clientfd:"<< clientfd << __func__ << endl;
    _DEF_COUT_FUNC_

    //    STRU_LOGIN_RS rs;
    //    rs.m_lResult = password_error;
    //    SendData( clientfd , (char*)&rs , sizeof rs );

    // 1 拆包
    _STRU_TCP_LOGIN_RQ* rq = (_STRU_TCP_LOGIN_RQ*)szbuf;

    //msg.data1 0是手机号登录，1是邮箱登录
    if (rq->logMode == log_mode_email)
    {
        cout << "[ kernel:: dealLogRq 邮箱账号登录中... ]" << endl;
        //检测邮箱格式

        //读数据库查看是否登录

        //写数据库

        //回复登录结果

        return;
    }

    //手机号登录
    cout << "[ kernel:: dealLogRq 手机号登录中... ]" << endl;

//    // 2 校验手机号的合法性
    string tel = rq->tel;
    string tel_temp = rq->tel;
    string password = rq->password;
    string password_temp = rq->password;
//    _STRU_TCP_LOGIN_RS msgrs;
//    msgrs.result = client_error;
//    //判断是否是空字符串 或者是全空格
//    int iter = 0;

//    while (iter != -1)
//    {
//        iter = tel_temp.find(" ");
//        if (iter != -1) tel_temp.erase(iter, iter + 1);
//    }
//    while (iter != -1)
//    {
//        iter = password_temp.find(" ");
//        if (iter != -1) password_temp.erase(iter, iter + 1);
//    }
//    if (password.empty() || tel.empty()
//            || password_temp.empty()
//            || tel_temp.empty())
//    {
//        cout << "kernel:: dealLOGRq 登录信息输入为空白，请重试" << endl;
//        m_tcp->SendData(clientfd,(char*)&msgrs,sizeof(msgrs));
//        return;
//    }
//    //检查长度是否合法（tel = 11,password 3-15）
//    if (tel.length() != 11 || password.length() > 15 || password.length() < 3)
//    {
//        cout << "kernel:: dealLOGRq 登录信息输入长度有误，请重试" << endl;
//        m_tcp->SendData(clientfd,(char*)&msgrs,sizeof(msgrs));
//        return;
//    }
//    //检查内容是否合法(tel为数字 name password为大小写字母 数字 下划线的组合)
//    if (0)
//    {
//        cout << "kernel:: dealLOGRq 登录信息输入格式有误，请重试" << endl;
//        m_tcp->SendData(clientfd,(char*)&msgrs,sizeof(msgrs));
//        return;
//    }

    _STRU_TCP_LOGIN_RS msg;
    list<string> listRes;
    char sqlBuf[1024];
    bzero(sqlBuf,sizeof(sqlBuf));

    //查询用户password,iconId
    sprintf(sqlBuf, "select id,password,iconId,name from t_users where tel = '%s';", rq->tel);
    if (!m_sql->SelectMysql(sqlBuf, 4, listRes))
    {
        //查询失败
        cout << "kernel:: dealLogRq 数据库查询失败" << endl;

        //发一个登录回复包
        msg.result = mysql_select_error;
        m_tcp->SendData(clientfd,(char*)&msg,sizeof(msg));
        return;
    }
    cout << "kernel:: dealLogRq 数据库t_users查询succeed" << endl;

    //查看用户是否注册
    if (listRes.size() != 4)
    {
        cout << "kernel:: dealLOGRq 手机号未注册" << endl;
        msg.result = login_false_no_tel;
        m_tcp->SendData(clientfd,(char*)&msg,sizeof(msg));
        return;
    }
    cout<<"kernel:: dealLOGRq Has registered, search login illage."<<endl;

    //根据id查看服务器是否登录
    int id = stoi(listRes.front()); listRes.pop_front();
    if (listRes.size() == 3 && m_mapIdToFd.count(id) > 0)
    {
        //被登录过了

        cout << "kernel:: dealLogRq 手机号已登录" << endl;
        msg.result = login_false_illegal;
        m_tcp->SendData(clientfd,(char*)&msg,sizeof(msg));
        return;
    }

//    //摘取信息
//    string  email, pwd, name, iconId, sign;
//    email = pwd = name = iconId = sign = "";
//    list<string>::iterator ite = listRes.begin();
//    for(int i = 0; i < 5 && ite != listRes.end() ;i++)
//    {
//        if (i == 0)	email = *(++ite);
//        else if (i == 1) pwd = *(++ite);
//        else if (i == 2) name = *(++ite);
//        else if (i == 3) iconId = *(++ite);
//        else if (i == 4) sign = *(++ite);
//    }
    if(listRes.size() != 3)
    {
        cout<<"pwd icon select error. listRes.size() != 2"<<endl;
        return;
    }

    string pwd = listRes.front();
    listRes.pop_front();
    int iconId = 0;
    if(listRes.front() != "")
    {
        iconId = stoi(listRes.front());
    }
    listRes.pop_front();
    string name = listRes.front();
    listRes.pop_front();

    //验证密码
    if (pwd != password)
    {
        cout << "kernel:: dealLOGRq 密码错误 登录失败" << endl;
        msg.result = login_false_password_wrong;
        m_tcp->SendData(clientfd,(char*)&msg,sizeof(msg));
        return;
    }
    cout<<"kernel:: dealLOGRq key right."<<endl;

    //登录成功
    msg.userId = id;
    msg.result = login_success;
    msg.iconId = iconId;
    strcpy(msg.name,name.c_str());
    int num = m_tcp->SendData(clientfd,(char*)&msg,sizeof(msg));

    //写入服务器记录
    if(num > 0)
    {
        m_mapIdToFd[id] = clientfd;
        cout << "kernel:: dealLOGRq 用户登录成功" << endl;
    }

    ///登录成功后发送给他好友信息和群聊信息，并告诉所有好友所有群成员他上线了
    //登录成功后发送给他de xihuan shoucang videos和 guanzhude bozhu信息，并告诉所有好友所有群成员他上线了
    //todo

    return;
}

void CLogic::UploadRq(sock_fd clientfd ,char* szbuf,int nlen)
{
    //_DEF_COUT_FUNC_

    //see file header
    STRU_TCP_UPLOAD_RQ* rq = (STRU_TCP_UPLOAD_RQ*)szbuf;
    ServerFileInfo *info = new ServerFileInfo;
    info->fileID = rq->fileId;//下載的時候是用來做
    info->videoID;//真是文件 ID
    info->userId = rq->userId;
    info->fileSize = rq->fileSize;
    info->Pos = 0;

    info->pFile;
    info->filePath;
    strcpy(info->fileName,rq->fileName);
    strcpy(info->fileType,rq->fileType);
    info->coverImgPath;
    strcpy(info->coverImgName,rq->coverImgName);
    info->userName; //sql
    info->rtmp;
    info->label = rq->label;

    //select sql userName
    char sqlBuf[1024];
    list<string>listRes;
    sprintf(sqlBuf,"select userName from t_users where id = %d;",rq->userId);
    if(!m_sql->SelectMysql(sqlBuf,1,listRes))
    {

    }
}










