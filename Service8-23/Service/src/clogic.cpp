#include "clogic.h"
#include <QThread>
#include <QTime>
#include <thread>
#include <chrono>
#include <mutex>
#include "packdef.h"
std::mutex g_mtx;
bool myFileFlag = false;
volatile int64_t filePos = 0;  //volatile ->write befor read


//mysql tables: t_users t_userslabel t_videolabel t_usergot t_userseen
void CLogic::setNetPackMap()
{
    qsrand(QTime(0,0,0).msecsTo(QTime::currentTime())); //随机数时间种子
    NetPackMap(_DEF_TCP_REGISTER_RQ)    = &CLogic::RegisterRq;
    NetPackMap(_DEF_TCP_LOGIN_RQ)       = &CLogic::LoginRq;
    NetPackMap(_DEF_TCP_UPLOAD_RQ)      = &CLogic::UploadRq;
    NetPackMap(_DEF_TCP_FILEBLOCK_RQ)   = &CLogic::UploadBlockRq;
    NetPackMap(_DEF_TCP_NEED_VIDEO_RQ)  = &CLogic::SendVideoStream;
    NetPackMap(_DEF_TCP_USER_ACTIVITY)  = &CLogic::updateUserActivity;
    NetPackMap(_DEF_TCP_NEED_COMMENT_RQ)  = &CLogic::dealNeedComment;
    NetPackMap(_DEF_TCP_NEED_BARRAGE_RQ)  = &CLogic::dealNeedBarrage;
}

#define _DEF_COUT_FUNC_    cout << "[** clientfd:"<< clientfd <<" " <<__func__ << " thread: "<<QThread::currentThread()<<" **]"<<endl;

CLogic:: ~CLogic()
{
    for(auto ite = m_mapFileIdToFileInfo.begin(); ite != m_mapFileIdToFileInfo.end(); ite++)
    {
        delete ite->second;
        ite->second = nullptr;
    }
    m_mapIdToFd.clear();
    m_mapFileIdToFileInfo.clear();
//    if(m_threadr->isRunning())
//    {
//        m_threadr->quit();
//        m_threadr->wait(10);
//        delete m_worker;
//    }
    //todo
//    m_sql->DisConnect();
//    m_tcp->
}

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
    if(listRes.front() == "")
    {
        cout<<__func__<<" stoi() failed"<<endl;
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
    umask(0);
    mkdir(path,S_IRWXU | S_IRWXG | S_IRWXO); //S_IRWXU can write and read | S_IRWXG this usersgroup can use| S_IRWXO others can use

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
    if(listRes.front() == "")
    {
        cout<<__func__<<" stoi() failed"<<endl;
        return;
    }
    int id = stoi(listRes.front()); listRes.pop_front();
    if (listRes.size() == 3 && m_mapIdToFd.count(id) > 0)
    {
        //被登录过了

//        cout << "kernel:: dealLogRq 手机号已登录" << endl;
//        msg.result = login_false_illegal;
//        m_tcp->SendData(clientfd,(char*)&msg,sizeof(msg));
//        return;
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
        //new user  make a dir for him
        char path[_DEF_NAME_LENGTH];
        sprintf(path,"%s/flv/%s/",VIDEO_ROOT_PATH,name.c_str()); //"/home/lll/Videos/flv/user_name/"
        umask(0);
        mkdir(path,S_IRWXU | S_IRWXG | S_IRWXO);

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

    strcpy(info->fileName,rq->fileName);
    strcpy(info->fileType,rq->fileType);
    strcpy(info->description,rq->description);
    strcpy(info->coverImgName,rq->coverImgName);
    info->label = rq->label;

    //select sql userName
    char sqlBuf[1024];
    list<string>listRes;
    sprintf(sqlBuf,"select name from t_users where id = %d;",rq->userId);
    if(!m_sql->SelectMysql(sqlBuf,1,listRes))
    {
        cout<<"     SelectMySql error: "<<sqlBuf<<endl;
        delete info;
        return;
    }
    if(listRes.size() <= 0)
    {
        cout<<"     SelectMySql res <= 0: "<<sqlBuf<<endl;
        delete info;
        return;
    }
    strcpy(info->userName,listRes.front().c_str()); //sql
    listRes.pop_front();

    //VIDEO_ROOT_PATH "/home/lll/Videos"
    sprintf(info->filePath,"%s/flv/%s/%s",VIDEO_ROOT_PATH,info->userName,info->fileName);
    sprintf(info->rtmp,"//%s/%s",info->userName,info->fileName);
    if(string(info->fileType) != "png" || string(info->fileType) != "jpg" || string(info->fileType) != "jepg")
    {
        strcpy(info->coverImgName,rq->coverImgName);
        sprintf(info->coverImgPath,"%s/flv/%s/%s",VIDEO_ROOT_PATH,info->userName,info->coverImgName);
    }
    info->pFile = fopen(info->filePath,"w");
    mode_t mode = S_IRUSR | S_IWUSR; // 设置文件权限为用户可读可写
    info->fd = open(info->filePath,O_WRONLY | O_CREAT,mode);
    if(info->fd <= 0)
    {
        delete info;
        return;
    }
    m_mapFileIdToFileInfo[info->fileID] = info;
    cout<<"New file! ready to receive it, name: "<<info->fileName<<" size: "<<info->fileSize
       <<" fileId: "<<info->fileID<<endl<<endl;
    myFileFlag = true;
}

void CLogic::UploadBlockRq(sock_fd clientfd ,char* szbuf,int nlen)
{
    std::lock_guard<std::mutex> lock(g_mtx);
    //cout<<"block: "<<QThread::currentThread()<<endl;
//    while(!myFileFlag)
//    {
//        //dead loop
//        QThread::usleep(10);
//    }
    STRU_TCP_UPLOAD_RS rs;
    STRU_TCP_FILEBLOCK_RQ* rq = (STRU_TCP_FILEBLOCK_RQ*)szbuf;
//    while(rq->pos != filePos)
//    {
//        //dead loop
//        QThread::usleep(10);
//    }
    int finder = -1;
    int i = 0;
    while(finder <= 0)
    {
        finder = m_mapFileIdToFileInfo.count(rq->fileId);
        //rs.result = file_transmit_res_failure;
        //m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
        i++;
        if(i >= 20)
        {
            cout<<"Don't find fileInfo in map,fileID: "<<rq->fileId<<" "<<QThread::currentThread()<<endl;
            return;
        }
        QThread::usleep(500);
    }

    int64_t res;
    ServerFileInfo *info = m_mapFileIdToFileInfo[rq->fileId];
    off_t new_offset;
    new_offset = lseek(info->fd,(long)rq->pos,SEEK_SET);
    if(new_offset < 0)
        new_offset = lseek(info->fd,(long)rq->pos,SEEK_SET);
    if(new_offset < 0)
        new_offset = lseek(info->fd,(long)rq->pos,SEEK_SET);
    if(new_offset < 0)
        new_offset = lseek(info->fd,(long)rq->pos,SEEK_SET);
    if(new_offset < 0)
        new_offset = lseek(info->fd,(long)rq->pos,SEEK_SET);
    if(new_offset < 0)
        new_offset = lseek(info->fd,(long)rq->pos,SEEK_SET);
    if (new_offset < 0) {
        printf("Error seeking in file.\n");
        close(info->fd);
        fclose(info->pFile);
        m_mapFileIdToFileInfo.erase(rq->fileId);
        delete info;
        return;
    }
    res = write(info->fd,rq->fileContent,rq->blockLen);

    info->Pos += res;
    filePos += res;
    //printf("once pos: %lld. id: %lld\n",res,info->fileID);
    if(info->Pos >= info->fileSize)
    {
        fclose(info->pFile);
        lseek(info->fd,0L,SEEK_SET);
        close(info->fd);
        //check filetype not a img ,we should write table and send msg to user
        if(strcmp(info->fileType,"mp4") == 0 || strcmp(info->fileType,"flv") == 0)
        {
            char sqlBuf[4096] = "";
            sprintf(sqlBuf,"insert into t_videolabel (userid,videoname,picname,videopath,picpath,rtmp,"
                           "car,comic,dance,fashion,film,lovered,game,laugh,luxury,music,news,pet,science"
                           ",sport,tourism,vlog,"
                           "paise,username,description) values"
                           "(%d,'%s','%s','%s','%s','%s', %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d, %d,'%s','%s');"
                    ,info->userId,info->fileName,info->coverImgName,info->filePath,info->coverImgPath,info->rtmp
                    ,info->label.cb_car,info->label.cb_comic,info->label.cb_dance,info->label.cb_fashion,info->label.cb_film
                    ,info->label.cb_first,info->label.cb_game,info->label.cb_laugh,info->label.cb_luxury,info->label.cb_music
                    ,info->label.cb_news,info->label.cb_pet,info->label.cb_science,info->label.cb_sport,info->label.cb_tourism
                    ,info->label.cb_vlog, 9999,info->userName,info->description);

            if(!m_sql->UpdataMysql(sqlBuf))
            {
                cout<<"UploadBlockRq:: insert into table failed."<<endl;
                m_mapFileIdToFileInfo.erase(rq->fileId);
                delete info;
                rs.result = file_transmit_res_failure;
                //m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
                return;
            }

            rs.result = file_transmit_res_success;
            m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
        }

        cout<<"UploadBlockRq:: write a file succeed. size: "<<info->fileSize
           <<" pos: "<<info->Pos<<" fileId: "<<info->fileID<<endl;
        m_mapFileIdToFileInfo.erase(rq->fileId);
        delete info;
        myFileFlag = false;
        filePos = 0;
    }
    //std::this_thread::sleep_for(std::chrono::nanoseconds(5));
}

void CLogic::SendVideoStream(sock_fd clientfd, char *szbuf, int nlen)
{
    cout<<__func__<<endl;
    STRU_NEED_VIDEO_RQ* rq = (STRU_NEED_VIDEO_RQ*)szbuf;

    //search
    list<ServerFileInfo*> listRes;
    getFileList(listRes,rq->userId);

    //send 18 img&rtmp
    while(listRes.size() > 0)
    {
        ServerFileInfo* info = listRes.front();
        listRes.pop_front();

        STRU_NEED_VIDEO_RS rs;
        strcpy(rs.rtmp,info->rtmp);
        rs.fileId = info->fileID;
        rs.videoId = info->videoID;
        cout<<"listRes: "<<info->videoID<<" "<<info->coverImgPath<<endl;
        strcpy(rs.fileName,info->coverImgName);
        strcpy(rs.description,info->description);
        strcpy(rs.authorName,info->userName);
        rs.fileSize = info->fileSize;
        rs.upvoteNum = info->upvoteNum;
        rs.collectNum = info->collectNum;
        rs.commentNum = info->commentNum;

        //send file_header
        m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
        cout<<"SendStream:: header! file size: "<<rs.description<<"; "<<rs.fileSize<<"Bytes"<<endl;

        //send file_img
        info->pFile = fopen(info->coverImgPath,"r");
        if(info->pFile)
        {
            while(1)
            {
                STRU_TCP_FILEBLOCK_RQ block;

                int64_t res = fread(block.fileContent,1,sizeof(block.fileContent),info->pFile); //once read a byte
                block.blockLen = res;
                info->Pos += res;
                block.fileId = info->fileID;
                block.userId = rq->userId;

                m_tcp->SendData(clientfd,(char*)&block,sizeof(block));
                if(info->Pos >= info->fileSize)
                {
                    fclose(info->pFile);
                    delete info; info = nullptr;
                    cout<<"SendStream:: ACC IMG size: "<<rs.fileSize<<"Bytes"<<endl;
                    break;
                }
            }
        }
        else
            delete info;
    }
}


void CLogic::getFileList(list<ServerFileInfo*>&fileList,int userId)
{
    cout<<__func__<<endl;
    //search user nosee video nums;
    int num;
    char sqlBuf[1024];
    list<string>listNum;
    sprintf(sqlBuf,"select count(videoid) from t_videolabel where t_videolabel.videoid not in "
                   "(select t_usergot.videoid from t_usergot where userid = %d);",userId);
    if(!m_sql->SelectMysql(sqlBuf,1,listNum) || listNum.size() != 1)
    {
        cout<<"getFileList:: select noseen nums failed."<<endl;
        return;
    }
    if(listNum.front() == "")
    {
        cout<<__func__<<" select count(videoid) res=NULL"<<endl;
        return;
    }
    num = stoi(listNum.front());

    if(num == 0)//clear this user in t_usergot
    {
        memset(sqlBuf,0,sizeof(sqlBuf));
        sprintf(sqlBuf,"delete from t_usergot where userid = %d;",userId);
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"getFileList:: noseen nums==0, delete data failed."<<endl;
            return;
        }
    }

    //write top18 video(user no seen) to fileList
    memset(sqlBuf,0,sizeof(sqlBuf));
    sprintf(sqlBuf, "SELECT videoid,picname,picpath,rtmp,hotdegree,description,username,paise,collectNum,commentNum FROM t_videolabel WHERE t_videolabel.videoid NOT IN "
             "(SELECT t_usergot.videoid FROM t_usergot WHERE userid = %d) ORDER BY hotdegree DESC LIMIT 18 OFFSET 0;"
            ,userId);
    listNum.clear();
    if(!m_sql->SelectMysql(sqlBuf,10,listNum))
    {
        cout<<"getFileList:: select TOP_18 videos failed."<<endl;
        return;
    }

    num = 0;
    int len = listNum.size();
    STRU_SERVER_FILE_INFO* info = nullptr;
    for(int i = 0; i < len / 10; i++)
    {
        info = new STRU_SERVER_FILE_INFO;
        //fuzhi
        if(listNum.front() == "")
        {
            cout<<__func__<<" select videoId res=NULL"<<endl;
            delete info;
            return;
        }
        info->videoID = stoi(listNum.front());                  listNum.pop_front();
        strcpy(info->coverImgName,listNum.front().c_str());     listNum.pop_front();
        strcpy(info->coverImgPath,listNum.front().c_str());     listNum.pop_front();
        strcpy(info->rtmp,listNum.front().c_str());             listNum.pop_front();
        if(listNum.front() == "")
        {
            cout<<__func__<<" select hotdegree res=NULL; "<<info->rtmp<<endl;
            info->hotdegree = 0;
        }
        else
            info->hotdegree = stoi(listNum.front());
                                                                listNum.pop_front();
        strcpy(info->description,listNum.front().c_str());      listNum.pop_front();
        strcpy(info->userName,listNum.front().c_str());         listNum.pop_front();
        if(listNum.front() == "")
        {
            cout<<__func__<<" select upvoteNum res=NULL; "<<info->rtmp<<endl;
            info->upvoteNum = 0;
        }
        else
            info->upvoteNum = stoi(listNum.front());
        listNum.pop_front();

        if(listNum.front() == "")
        {
            cout<<__func__<<" select collectNum res=NULL; "<<info->rtmp<<endl;
            info->collectNum = 0;
        }
        else
            info->collectNum = stoi(listNum.front());
        listNum.pop_front();


        if(listNum.front() == "")
        {
            cout<<__func__<<" select commentNum res=NULL; "<<info->rtmp<<endl;
            info->commentNum = 0;
        }
        else
            info->commentNum = stoi(listNum.front());
        listNum.pop_front();

//        info->Pos;
//        info->label;
//        info->pFile;
//        info->userId;
//        info->fileName;
//        info->filePath;        
//        info->fileType;
//        info->userName;
        info->fileID = qrand()%10000 + info->videoID;
        FILE* p = fopen(info->coverImgPath,"r");
        info->pFile = p;
        cout<<"imgpath: "<<info->coverImgPath<<" p:"<<p<<endl;
        if(!p)
        {
            delete info;
            continue;
        }
        fseek(p,0,SEEK_END);
        info->fileSize = ftell(p);  //need <= 4G
        fseek(p,0,SEEK_SET);
        fclose(p);

        //add to list
        fileList.push_back(info);

        //t_usergot add data
//        memset(sqlBuf,0,sizeof(sqlBuf));
//        sprintf(sqlBuf,"insert into t_usergot values (%d,%d);",userId,info->videoID);
//        if(!m_sql->UpdataMysql(sqlBuf))
//        {
//            cout<<"getFileList:: update t_usergot failed."<<endl;
//            delete info;
//            return;
//        }
    }
}


//click 1; upvote 3; barrage 2; share 5; collect 7; watchdone 10; comment 4;
void CLogic::updateUserActivity(sock_fd clientfd, char *szbuf, int nlen)
{
    _STRU_TCP_USER_ACTIVITY* act = (_STRU_TCP_USER_ACTIVITY*)szbuf;
    int userId = act->userId;
    int destId = act->destUserId;
    int videoId = act->videoId;
    int v_hotdegree = 0;
    int v_upvote = 0;
    int v_collect = 0;
    int v_comment = 0;
    int v_share = 0;
    int v_authorId = 0;
    string v_rtmp = "";
    string userName= "";
    list<string> listRes;
    char sqlBuf[1024] = "";
    sprintf(sqlBuf,"select name from t_users where id = %d;",userId);
    if(!m_sql->SelectMysql(sqlBuf,1,listRes))
    {
        cout<<"Update video failed: select username"<<endl;
    }else{
        userName = listRes.front();
    }
    listRes.clear();
    memset(sqlBuf,0,sizeof(sqlBuf));
    sprintf(sqlBuf,"select hotdegree,paise,collectNum,commentNum,shareNum,userId,rtmp from t_videolabel"
                   " where videoid = %d;",videoId);
    if(!m_sql->SelectMysql(sqlBuf,7,listRes))
    {
        cout<<"Update video failed: select video info"<<endl;
        return;
    }
    if(listRes.size() < 5)  return;
    v_hotdegree = listRes.front()!="" ? stoi(listRes.front()) : 0;
    listRes.pop_front();

    v_upvote = listRes.front()!="" ? stoi(listRes.front()) : 0;
    listRes.pop_front();

    v_collect = listRes.front()!="" ? stoi(listRes.front()) : 0;
    listRes.pop_front();

    v_comment = listRes.front()!="" ? stoi(listRes.front()) : 0;
    listRes.pop_front();

    v_share = listRes.front()!="" ? stoi(listRes.front()) : 0;
    listRes.pop_front();

    v_authorId = listRes.front()!="" ? stoi(listRes.front()) : 0;
    listRes.pop_front();

    v_rtmp = listRes.front();
    listRes.pop_front();
    if(act->userCilck)
    {
        v_hotdegree += 2;
        memset(sqlBuf,0,sizeof(sqlBuf));
        sprintf(sqlBuf,"update t_videolabel set hotdegree = %d where videoId = %d;"
                ,v_hotdegree,videoId);
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: click"<<endl;
        }
        return;
    }
    if(act->userWatched)
    {
        memset(sqlBuf,0,sizeof(sqlBuf));
        v_hotdegree += 10;
        sprintf(sqlBuf,"update t_videolabel set hotdegree = %d where videoId = %d;"
                ,v_hotdegree,videoId);
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: watched"<<endl;
        }
        memset(sqlBuf,0,sizeof(sqlBuf));
        sprintf(sqlBuf,"INSERT INTO `t_userseen` (`videoId`, `userId`)"
                " VALUES (%d,%d);",videoId,userId);
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: userseen"<<endl;
        }
        return;
    }
    if(act->upvoteChg && !act->reduceFlag)
    {
        memset(sqlBuf,0,sizeof(sqlBuf));
        v_hotdegree += 2;
        v_upvote += 1;
        cout<<"like: "<<v_upvote<<endl;
        sprintf(sqlBuf,"update t_videolabel set hotdegree = %d , paise = %d where videoId = %d;"
                ,v_hotdegree,v_upvote,videoId);
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: like"<<endl;
        }
        memset(sqlBuf,0,sizeof(sqlBuf));
        sprintf(sqlBuf,"INSERT INTO `t_userlike` (`videoId`, `userId`,`authorId`,`rtmp`)"
                " VALUES (%d,%d,%d,'%s');",videoId,userId,v_authorId,v_rtmp.c_str());
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: userlike"<<endl;
        }
        return;
    }
    if(act->upvoteChg && act->reduceFlag)
    {
        memset(sqlBuf,0,sizeof(sqlBuf));
        v_hotdegree -= 2;
        v_upvote -= 1;
        sprintf(sqlBuf,"update t_videolabel set hotdegree = %d , paise = %d where videoid = %d;"
                ,v_hotdegree,v_upvote,videoId);
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: --like"<<endl;
        }
        memset(sqlBuf,0,sizeof(sqlBuf));
        sprintf(sqlBuf,"delete from `t_userlike` where `videoId` = %d and `userId` = %d;"
                ,videoId,userId);
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: --userlike"<<endl;
        }
        return;
    }
    if(act->collectChg && !act->reduceFlag)
    {
        memset(sqlBuf,0,sizeof(sqlBuf));
        v_hotdegree += 7;
        v_collect += 1;
        sprintf(sqlBuf,"update t_videolabel set hotdegree = %d , collectNum = %d where videoId = %d;"
                ,v_hotdegree,v_collect,videoId);
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: collect"<<endl;
        }
        memset(sqlBuf,0,sizeof(sqlBuf));
        sprintf(sqlBuf,"INSERT INTO `t_usercollect` (`videoId`, `userId`,`authorId`,`rtmp`)"
                " VALUES (%d,%d,%d,'%s');",videoId,userId,v_authorId,v_rtmp.c_str());
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: usercollect"<<endl;
        }
        return;
    }
    if(act->collectChg && act->reduceFlag)
    {
        memset(sqlBuf,0,sizeof(sqlBuf));
        v_hotdegree -= 7;
        v_collect -= 1;
        sprintf(sqlBuf,"update t_videolabel set hotdegree = %d , collectNum = %d where videoId = %d;"
                ,v_hotdegree,v_collect,videoId);
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: --collect"<<endl;
        }
        memset(sqlBuf,0,sizeof(sqlBuf));
        sprintf(sqlBuf,"delete from `t_usercollect` where `videoId` = %d and `userId` = %d;"
                ,videoId,userId);
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: --usercollect"<<endl;
        }
        return;
    }
    if(act->shareChg)
    {
        memset(sqlBuf,0,sizeof(sqlBuf));
        v_hotdegree += 5;
        sprintf(sqlBuf,"update t_videolabel set hotdegree = %d where videoId = %d;"
                ,v_hotdegree,videoId);
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: share"<<endl;
        }
        //send rtmp to friend; todo
        return;
    }
    if(act->commentChg)
    {
        memset(sqlBuf,0,sizeof(sqlBuf));
        v_hotdegree += 4;
        v_comment += 1;
        sprintf(sqlBuf,"update t_videolabel set hotdegree = %d , commentNum = %d where videoId = %d;"
                ,v_hotdegree,v_comment,videoId);
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: comment"<<endl;
        }
        memset(sqlBuf,0,sizeof(sqlBuf));
        sprintf(sqlBuf,"INSERT INTO `t_videocomment` (`videoId`, `userId`,`destUserId`,`comment`,`username`) "
                " VALUES (%d,%d,%d,'%s','%s');",videoId,userId,destId,act->comment,userName.c_str());
        cout<<sqlBuf<<endl;
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: videocomment"<<endl;
        }
        return;
    }
    if(act->barrageChg)
    {
        memset(sqlBuf,0,sizeof(sqlBuf));
        v_hotdegree += 1;
        sprintf(sqlBuf,"update t_videolabel set hotdegree = %d where videoId = %d;"
                ,v_hotdegree,videoId);
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: barrage_hotdegree"<<endl;
        }
        memset(sqlBuf,0,sizeof(sqlBuf));
        sprintf(sqlBuf,"INSERT INTO `t_videobarrage` (`videoId`,`userId`,`time`,`barrage`) "
                " VALUES (%d,%d,%llu,'%s');",videoId,userId,act->barrageTime,act->barrage);
        if(!m_sql->UpdataMysql(sqlBuf))
        {
            cout<<"Update failed: videobarrage"<<endl;
        }
        return;
    }
}


void CLogic::dealNeedComment(sock_fd clientfd, char *szbuf, int nlen)
{
    //find 10 comments and send
    _STRU_TCP_NEED_COMMENT_RQ* rq = (_STRU_TCP_NEED_COMMENT_RQ*)szbuf;
    char sqlBuf[1024];
    list<string>listRes;
    sprintf(sqlBuf,"select comment,username,userId from t_videocomment where videoid = %d;",rq->videoId);

    if(!m_sql->SelectMysql(sqlBuf,3,listRes))
    {
        cout<<"Send comments failed: select"<<endl;
    }
    _STRU_TCP_COMMENT_RS rs;
    if(listRes.size() < 30)
        return;
    strcpy(rs.comment_1,listRes.front().c_str());                       listRes.pop_front();
    strcpy(rs.name_1,listRes.front().c_str());                          listRes.pop_front();
    rs.masteId_1 = listRes.front()!="" ? stoi(listRes.front()) : 0;     listRes.pop_front();

    strcpy(rs.comment_2,listRes.front().c_str());                       listRes.pop_front();
    strcpy(rs.name_2,listRes.front().c_str());                          listRes.pop_front();
    rs.masteId_2 = listRes.front()!="" ? stoi(listRes.front()) : 0;     listRes.pop_front();

    strcpy(rs.comment_3,listRes.front().c_str());                       listRes.pop_front();
    strcpy(rs.name_3,listRes.front().c_str());                          listRes.pop_front();
    rs.masteId_3 = listRes.front()!="" ? stoi(listRes.front()) : 0;     listRes.pop_front();

    strcpy(rs.comment_4,listRes.front().c_str());                       listRes.pop_front();
    strcpy(rs.name_4,listRes.front().c_str());                          listRes.pop_front();
    rs.masteId_4 = listRes.front()!="" ? stoi(listRes.front()) : 0;     listRes.pop_front();

    strcpy(rs.comment_5,listRes.front().c_str());                       listRes.pop_front();
    strcpy(rs.name_5,listRes.front().c_str());                          listRes.pop_front();
    rs.masteId_5 = listRes.front()!="" ? stoi(listRes.front()) : 0;     listRes.pop_front();

    strcpy(rs.comment_6,listRes.front().c_str());                       listRes.pop_front();
    strcpy(rs.name_6,listRes.front().c_str());                          listRes.pop_front();
    rs.masteId_6 = listRes.front()!="" ? stoi(listRes.front()) : 0;     listRes.pop_front();

    strcpy(rs.comment_7,listRes.front().c_str());                       listRes.pop_front();
    strcpy(rs.name_7,listRes.front().c_str());                          listRes.pop_front();
    rs.masteId_7 = listRes.front()!="" ? stoi(listRes.front()) : 0;     listRes.pop_front();

    strcpy(rs.comment_8,listRes.front().c_str());                       listRes.pop_front();
    strcpy(rs.name_8,listRes.front().c_str());                          listRes.pop_front();
    rs.masteId_8 = listRes.front()!="" ? stoi(listRes.front()) : 0;     listRes.pop_front();

    strcpy(rs.comment_9,listRes.front().c_str());                       listRes.pop_front();
    strcpy(rs.name_9,listRes.front().c_str());                          listRes.pop_front();
    rs.masteId_9 = listRes.front()!="" ? stoi(listRes.front()) : 0;     listRes.pop_front();

    strcpy(rs.comment_10,listRes.front().c_str());                       listRes.pop_front();
    strcpy(rs.name_10,listRes.front().c_str());                          listRes.pop_front();
    rs.masteId_10 = listRes.front()!="" ? stoi(listRes.front()) : 0;     listRes.pop_front();

    m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
}


void CLogic::dealNeedBarrage(sock_fd clientfd, char *szbuf, int nlen)
{
    //find all comments and send
    _STRU_TCP_NEED_BARRAGE_RQ* rq = (_STRU_TCP_NEED_BARRAGE_RQ*)szbuf;
    char sqlBuf[1024];
    list<string>listRes;
    sprintf(sqlBuf,"select barrage,time from t_videobarrage where videoid = %d;",rq->videoId);

    if(!m_sql->SelectMysql(sqlBuf,2,listRes))
    {
        cout<<"Send barrages failed: select"<<endl;
    }
    _STRU_TCP_BARRAGE_RS rs;
    if(listRes.size() < 20)
        return;
    int64_t randTime = 10;
    while(listRes.size() >= 20)
    {
        rs.type = _DEF_TCP_BARRAGE_RS;
        rs.videoId = rq->videoId;

        strcpy(rs.barrage_1,listRes.front().c_str());                            listRes.pop_front();
        rs.time_1 = listRes.front()!="" ? stoi(listRes.front()) : randTime++;    listRes.pop_front();

        strcpy(rs.barrage_2,listRes.front().c_str());                            listRes.pop_front();
        rs.time_2 = listRes.front()!="" ? stoi(listRes.front()) : randTime++;    listRes.pop_front();

        strcpy(rs.barrage_3,listRes.front().c_str());                            listRes.pop_front();
        rs.time_3 = listRes.front()!="" ? stoi(listRes.front()) : randTime++;    listRes.pop_front();

        strcpy(rs.barrage_4,listRes.front().c_str());                            listRes.pop_front();
        rs.time_4 = listRes.front()!="" ? stoi(listRes.front()) : randTime++;    listRes.pop_front();

        strcpy(rs.barrage_5,listRes.front().c_str());                            listRes.pop_front();
        rs.time_5 = listRes.front()!="" ? stoi(listRes.front()) : randTime++;    listRes.pop_front();

        strcpy(rs.barrage_6,listRes.front().c_str());                            listRes.pop_front();
        rs.time_6 = listRes.front()!="" ? stoi(listRes.front()) : randTime++;    listRes.pop_front();

        strcpy(rs.barrage_7,listRes.front().c_str());                            listRes.pop_front();
        rs.time_7 = listRes.front()!="" ? stoi(listRes.front()) : randTime++;    listRes.pop_front();

        strcpy(rs.barrage_8,listRes.front().c_str());                            listRes.pop_front();
        rs.time_8 = listRes.front()!="" ? stoi(listRes.front()) : randTime++;    listRes.pop_front();

        strcpy(rs.barrage_9,listRes.front().c_str());                            listRes.pop_front();
        rs.time_9 = listRes.front()!="" ? stoi(listRes.front()) : randTime++;    listRes.pop_front();

        strcpy(rs.barrage_10,listRes.front().c_str());                            listRes.pop_front();
        rs.time_10 = listRes.front()!="" ? stoi(listRes.front()) : randTime++;    listRes.pop_front();

        m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));
        cout<<"Barrage: "<<rs.barrage_1<<" "<<rs.barrage_2<<" "<<rs.barrage_3<<" "<<rs.barrage_4<<" "<<rs.barrage_5
         <<" "<<rs.barrage_6<<" "<<rs.barrage_7<<" "<<rs.barrage_8<<" "<<rs.barrage_9<<" "<<rs.barrage_10<<endl;
        memset(&rs,0,sizeof(rs));
    }
}






























