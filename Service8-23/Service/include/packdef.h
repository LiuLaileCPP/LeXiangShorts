#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include "err_str.h"
#include <malloc.h>
#include<map>
#include<list>
#include<string>
#include<memory>
#include<iostream>
using namespace std;


//边界值
#define _DEF_SIZE           45
#define _DEF_BUFFERSIZE     1000
#define _DEF_PORT           8080
#define _DEF_SERVERIP       "0.0.0.0"
#define _DEF_LISTEN         128
#define _DEF_EPOLLSIZE      4096
#define _DEF_IPSIZE         16
#define _DEF_COUNT          10
#define _DEF_TIMEOUT        10
#define _DEF_SQLIEN         400
#define TRUE                true
#define FALSE               false



/*-------------数据库信息-----------------*/
#define _DEF_DB_NAME    "VideoServer"
#define _DEF_DB_IP      "localhost"
#define _DEF_DB_USER    "root"
#define _DEF_DB_PWD     "123"
/*--------------------------------------*/
#define _MAX_PATH           (260)
#define _DEF_BUFFER         (4096)
#define _DEF_CONTENT_SIZE	(1024)
#define _MAX_SIZE           (40)




//mine---------------------------------------------------------------------------------------------------------------------
struct STRU_VIDEO_LABEL{
    char cb_first;
    char cb_car;
    char cb_comic;
    char cb_dance;
    char cb_fashion;
    char cb_film;
    char cb_game;
    char cb_laugh;
    char cb_luxury;
    char cb_music;
    char cb_news;
    char cb_pet;
    char cb_science;
    char cb_sport;
    char cb_tourism;
    char cb_vlog;
    STRU_VIDEO_LABEL()
    {
        memset(this,0,sizeof(STRU_VIDEO_LABEL));
    }
};


typedef int PackType;
#define _DEF_NAME_LENGTH    (100)
#define _DEF_CONTENT_LENGTH (2048)
#define _DEF_PROTOCOL_COUNT (40)
#define _MAX_FILE_PATH      (512)
#define _DEF_WORK_TYPE_SIZE_ 1000 //工作函数数组最大长度
#define VIDEO_ROOT_PATH     ("/home/lll/Videos")

//定义协议头------------------------------------------------------------------------------------

#define _DEF_PROTOCOL_BASE (1000)
//上线请求
#define _DEF_UDP_ONLINE_RQ (_DEF_PROTOCOL_BASE + 1)
//上线回复
#define _DEF_UDP_ONLINE_RS (_DEF_PROTOCOL_BASE + 2)
//聊天请求
#define _DEF_UDP_CHAT_RQ (_DEF_PROTOCOL_BASE + 3)
//下线请求
#define _DEF_UDP_OFFLINE_RQ (_DEF_PROTOCOL_BASE + 4)

//TCP
//注册请求
#define _DEF_TCP_REGISTER_RQ (_DEF_PROTOCOL_BASE + 5)
//注册回复
#define _DEF_TCP_REGISTER_RS (_DEF_PROTOCOL_BASE + 6)
//登录请求
#define _DEF_TCP_LOGIN_RQ (_DEF_PROTOCOL_BASE + 7)
//登录回复
#define _DEF_TCP_LOGIN_RS (_DEF_PROTOCOL_BASE + 8)
//Service发送的好友信息
#define _DEF_TCP_FRIEND_INFO (_DEF_PROTOCOL_BASE + 9)
//聊天请求
#define _DEF_TCP_CHAT_RQ (_DEF_PROTOCOL_BASE + 10)
//聊天回复
#define _DEF_TCP_CHAT_RS (_DEF_PROTOCOL_BASE + 11)
//添加好友请求
#define _DEF_TCP_ADD_FRIEND_RQ (_DEF_PROTOCOL_BASE + 12)
//添加好友回复
#define _DEF_TCP_ADD_FRIEND_RS (_DEF_PROTOCOL_BASE + 13)
//删除好友请求 发给服务器
#define _DEF_TCP_DELETE_FRIEND_RQ (_DEF_PROTOCOL_BASE + 17)
//删除好友操作回复 来自服务器
#define _DEF_TCP_DELETE_FRIEND_RS (_DEF_PROTOCOL_BASE + 18)
//朋友删除你的消息
#define _DEF_TCP_FRIEND_DELETE_YOU_MSG (_DEF_PROTOCOL_BASE + 19)
//下线请求
#define _DEF_TCP_OFFLINE_RQ (_DEF_PROTOCOL_BASE + 14)
//服务器->客户端 的登录验证
#define _DEF_TCP_VALIDATE_LOGINED_RQ (_DEF_PROTOCOL_BASE + 15)
//客户端->服务器 的登录验证回复
#define _DEF_TCP_VALIDATE_LOGINED_RS (_DEF_PROTOCOL_BASE + 16)
//发送客户端它的群聊信息
#define _DEF_TCP_SEND_CLIENT_GROUP_INFO (_DEF_PROTOCOL_BASE + 20)
//发送客户端它的群聊的所有成员信息
#define _DEF_TCP_SEND_CLIENT_GROUP_MEMBERS_INFO (_DEF_PROTOCOL_BASE + 21)
//群聊聊天请求
// #define _DEF_TCP_GROUP_CHAT_RQ (_DEF_PROTOCOL_BASE + 99)
//群聊聊天消息转发
#define _DEF_TCP_GROUP_CHAT_MSG (_DEF_PROTOCOL_BASE + 22)
//接收服务器的文件消息
#define _DEF_TCP_RECV_FILE_MSG (_DEF_PROTOCOL_BASE + 23)

#define _DEF_TCP_SEND_FILE_RQ (_DEF_PROTOCOL_BASE + 24)
//发送文件结果
#define _DEF_TCP_SEND_FILE_RES (_DEF_PROTOCOL_BASE + 25)

//接收服务器的文件消息的回复
#define _DEF_TCP_RECV_FILE_RES (_DEF_PROTOCOL_BASE + 26)

//上传下载视频
#define _DEF_TCP_UPLOAD_RQ      (_DEF_PROTOCOL_BASE + 27)
#define _DEF_TCP_UPLOAD_RS      (_DEF_PROTOCOL_BASE + 28)
#define _DEF_TCP_FILEBLOCK_RQ   (_DEF_PROTOCOL_BASE + 29)
enum _ENUM_DEF_PROTOCOL{
    _DEF_TCP_NEED_VIDEO_RQ = (_DEF_PROTOCOL_BASE + 30),
    _DEF_TCP_NEED_VIDEO_RS,
    _DEF_TCP_USER_ACTIVITY,
    _DEF_TCP_NEED_COMMENT_RQ,
    _DEF_TCP_COMMENT_RS,
    _DEF_TCP_NEED_BARRAGE_RQ,
    _DEF_TCP_BARRAGE_RS,
};



//消息事件-----------------------------------------------------------------------------------------------
#define _MSG_EVENTS_BASE 800

#define client_error (_MSG_EVENTS_BASE + 200)
#define mysql_select_error (_MSG_EVENTS_BASE + 201)
#define mysql_insert_error (_MSG_EVENTS_BASE + 202)

//注册结果
#define register_success (_MSG_EVENTS_BASE + 1)
#define register_false_tel_used (_MSG_EVENTS_BASE + 2)
#define register_false_name_used (_MSG_EVENTS_BASE + 3)
#define register_false_sql_select_error (3_MSG_EVENTS_BASE + 4)
#define register_false_sql_update_error (_MSG_EVENTS_BASE + 5)

//登录结果
#define login_success (_MSG_EVENTS_BASE + 6)
#define login_false_no_tel (_MSG_EVENTS_BASE + 7)
#define login_false_password_wrong (_MSG_EVENTS_BASE + 8)
#define login_false_sql_select_error (_MSG_EVENTS_BASE + 9)
#define login_false_illegal (_MSG_EVENTS_BASE + 10)

//在线状态
#define status_online (_MSG_EVENTS_BASE + 11)
#define status_offline (_MSG_EVENTS_BASE + 12)

#define chat_send_success (_MSG_EVENTS_BASE + 13)
#define chat_send_false (_MSG_EVENTS_BASE + 14)

#define delete_friend_success (_MSG_EVENTS_BASE + 15)
#define delete_friend_false (_MSG_EVENTS_BASE + 16)

//添加好友的结果
#define add_friend_success (_MSG_EVENTS_BASE + 17)
#define add_friend_false_offline (_MSG_EVENTS_BASE + 18)
#define add_friend_false_no_exist (_MSG_EVENTS_BASE + 19)
#define add_friend_false_disagree (_MSG_EVENTS_BASE + 20)

#define group_offline (_MSG_EVENTS_BASE + 21)
#define group_online (_MSG_EVENTS_BASE + 22)

#define file_friend (_MSG_EVENTS_BASE + 23)
#define file_group (_MSG_EVENTS_BASE + 24)

#define send_file_succeed (_MSG_EVENTS_BASE + 25)
#define send_file_failed_ser_recv (_MSG_EVENTS_BASE + 26)
#define send_file_failed_ser_send (_MSG_EVENTS_BASE + 27)
#define send_file_failed_fri_offline (_MSG_EVENTS_BASE + 28)
#define send_file_failed_fri_refuse (_MSG_EVENTS_BASE + 29)
#define send_file_failed_gro_offline (_MSG_EVENTS_BASE + 30)

#define user_recv_file_yes (_MSG_EVENTS_BASE + 31)
#define user_recv_file_no (_MSG_EVENTS_BASE + 32)
//mode方式-----------------------------------------------------------------------
#define _DEF_MODE_BASE 700

#define reg_mode_tel (_DEF_MODE_BASE + 1)
#define reg_mode_email (_DEF_MODE_BASE + 2)

#define log_mode_tel (_DEF_MODE_BASE + 3)
#define log_mode_email (_DEF_MODE_BASE + 4)


//请求协议包----------------------------------------------------------------------

//UDP数据报结构体
//上线请求：协议头,ip,name
struct _STRU_ONLINE {
    _STRU_ONLINE() :type(_DEF_UDP_ONLINE_RQ)
        //,ip(0)
    {
        memset(name, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    //long ip;
    char name[_DEF_NAME_LENGTH];
};

//聊天请求：协议头,ip,message
struct _STRU_CHAT_RQ {
    _STRU_CHAT_RQ() :type(_DEF_UDP_CHAT_RQ)
        //,ip(0)
    {
        memset(content, 0, _DEF_CONTENT_LENGTH);
    }
    PackType type;
    //long ip;
    char content[_DEF_CONTENT_LENGTH];
};
//下线请求：协议头,ip,name

struct _STRU_OFFLINE_RQ {
    _STRU_OFFLINE_RQ() :type(_DEF_UDP_OFFLINE_RQ)//,ip(0)
    {
    }
    PackType type;
    //long ip;
};




//TCP数据报结构体

//注册请求---type 电话 昵称 密码
struct _STRU_TCP_REGISTER_RQ {
    _STRU_TCP_REGISTER_RQ
        () :type(_DEF_TCP_REGISTER_RQ),regMode(reg_mode_tel)
    {
        memset(tel, 0, _DEF_NAME_LENGTH);
        memset(email,0,_DEF_NAME_LENGTH);
        memset(name, 0, _DEF_NAME_LENGTH);
        memset(password, 0, _DEF_NAME_LENGTH);
        memset(&label,0,sizeof(STRU_VIDEO_LABEL));
    }
    PackType type;
    int regMode;
    char tel[_DEF_NAME_LENGTH];
    char email[_DEF_NAME_LENGTH];
    char name[_DEF_NAME_LENGTH];
    char password[_DEF_NAME_LENGTH];
    STRU_VIDEO_LABEL label;
};

//注册回复
struct _STRU_TCP_REGISTER_RS {
    _STRU_TCP_REGISTER_RS
        ()
        :type(_DEF_TCP_REGISTER_RS)
        , result(register_false_name_used)//默认失败
        ,userId(0)
    {}
    PackType type;
    int result; //注册结果---成功(1) 失败1电话重复(1) 失败2昵称重复(2)
    int userId;
};

//登录请求
struct _STRU_TCP_LOGIN_RQ {
    _STRU_TCP_LOGIN_RQ
        () :type(_DEF_TCP_LOGIN_RQ),logMode(log_mode_tel),userId(-1)
    {
        memset(tel, 0, _DEF_NAME_LENGTH);
        memset(email,0,_DEF_NAME_LENGTH);
        memset(password, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    int logMode;
    int userId;
    char tel[_DEF_NAME_LENGTH];
    char email[_DEF_NAME_LENGTH];
    char password[_DEF_NAME_LENGTH];
};

//登录回复
struct _STRU_TCP_LOGIN_RS {
    _STRU_TCP_LOGIN_RS
        () :type(_DEF_TCP_LOGIN_RS), result(login_false_no_tel),userId(0)//,sock(0)
      ,iconId(0)
    {
        memset(name,0,sizeof(name));
    }
    PackType type;
    int userId;
    int result;
    int iconId;
    //long sock;    //x86long 4字节；x64long在WIndows4字节 Linux上8字节
    char name[_DEF_NAME_LENGTH];
    //#define login_success (0)#define login_false_no_tel (1)
    //#define login_false_password_wrong (2)

};

//用户交互信息-点赞 评论 收藏
struct _STRU_TCP_USER_ACTIVITY{
    _STRU_TCP_USER_ACTIVITY()
    {
        type = _DEF_TCP_USER_ACTIVITY;
        userId = 0;
        videoId = 0;
        reduceFlag = false;
        upvoteChg = false;
        collectChg = false;
        userCilck = false;
        userWatched = false;
        commentChg = false;
        shareChg = false;
        barrageChg = false;
        barrageTime = 0;
        destUserId = 0;
        memset(comment,0,sizeof(comment));
        memset(comment,0,sizeof(shareBuf));
        memset(barrage,0,sizeof(shareBuf));
    }
    int type;
    int userId;
    int videoId;
    bool reduceFlag;
    bool upvoteChg;
    bool collectChg;
    bool userCilck;
    bool userWatched;
    bool commentChg;
    bool shareChg;
    bool barrageChg;
    int64_t barrageTime;
    int destUserId;
    char comment[_DEF_CONTENT_LENGTH];
    char shareBuf[_DEF_CONTENT_LENGTH];
    char barrage[_DEF_CONTENT_LENGTH];
};


//评论请求回复
struct _STRU_TCP_NEED_COMMENT_RQ
{
    _STRU_TCP_NEED_COMMENT_RQ()
    {
        videoId = 0;
    }
    int type = _DEF_TCP_NEED_COMMENT_RQ;
    int videoId;
};

struct _STRU_TCP_COMMENT_RS
{
    _STRU_TCP_COMMENT_RS()
    {
        videoId = 0;
    }
    int type = _DEF_TCP_COMMENT_RS;
    int videoId;
    int masteId_1 = 0;
    int masteId_2 = 0;
    int masteId_3 = 0;
    int masteId_4 = 0;
    int masteId_5 = 0;
    int masteId_6 = 0;
    int masteId_7 = 0;
    int masteId_8 = 0;
    int masteId_9 = 0;
    int masteId_10 = 0;
    char name_1[_DEF_NAME_LENGTH] = "";
    char name_2[_DEF_NAME_LENGTH] = "";
    char name_3[_DEF_NAME_LENGTH] = "";
    char name_4[_DEF_NAME_LENGTH] = "";
    char name_5[_DEF_NAME_LENGTH] = "";
    char name_6[_DEF_NAME_LENGTH] = "";
    char name_7[_DEF_NAME_LENGTH] = "";
    char name_8[_DEF_NAME_LENGTH] = "";
    char name_9[_DEF_NAME_LENGTH] = "";
    char name_10[_DEF_NAME_LENGTH] = "";
    char comment_1[_DEF_CONTENT_LENGTH] = "";
    char comment_2[_DEF_CONTENT_LENGTH] = "";
    char comment_3[_DEF_CONTENT_LENGTH] = "";
    char comment_4[_DEF_CONTENT_LENGTH] = "";
    char comment_5[_DEF_CONTENT_LENGTH] = "";
    char comment_6[_DEF_CONTENT_LENGTH] = "";
    char comment_7[_DEF_CONTENT_LENGTH] = "";
    char comment_8[_DEF_CONTENT_LENGTH] = "";
    char comment_9[_DEF_CONTENT_LENGTH] = "";
    char comment_10[_DEF_CONTENT_LENGTH] = "";
};

//弹幕请求和回复
struct _STRU_TCP_NEED_BARRAGE_RQ
{
    _STRU_TCP_NEED_BARRAGE_RQ()
    {
        videoId = 0;
    }
    int type = _DEF_TCP_NEED_BARRAGE_RQ;
    int videoId;
};

struct _STRU_TCP_BARRAGE_RS //弹幕要全发过来
{
    _STRU_TCP_BARRAGE_RS()
    {
        videoId = 0;
    }
    int type = _DEF_TCP_BARRAGE_RS;
    int videoId;
    int64_t time_1 = 0;
    int64_t time_2 = 0;
    int64_t time_3 = 0;
    int64_t time_4 = 0;
    int64_t time_5 = 0;
    int64_t time_6 = 0;
    int64_t time_7 = 0;
    int64_t time_8 = 0;
    int64_t time_9 = 0;
    int64_t time_10 = 0;
    char barrage_1[_DEF_CONTENT_LENGTH] = "";
    char barrage_2[_DEF_CONTENT_LENGTH] = "";
    char barrage_3[_DEF_CONTENT_LENGTH] = "";
    char barrage_4[_DEF_CONTENT_LENGTH] = "";
    char barrage_5[_DEF_CONTENT_LENGTH] = "";
    char barrage_6[_DEF_CONTENT_LENGTH] = "";
    char barrage_7[_DEF_CONTENT_LENGTH] = "";
    char barrage_8[_DEF_CONTENT_LENGTH] = "";
    char barrage_9[_DEF_CONTENT_LENGTH] = "";
    char barrage_10[_DEF_CONTENT_LENGTH] = "";
};







//上传下载视频---------------------------------------------------------------------------------------------
#define _DEF_FILE_MAX_CONTENT_LEN 65536
#define file_type_picture 0
#define file_type_video 1
struct STRU_TCP_UPLOAD_RQ
{
    STRU_TCP_UPLOAD_RQ():type(_DEF_TCP_UPLOAD_RQ),userId(0),fileId(0)
    {
        memset(fileName,0,sizeof (fileName));
        memset(fileType,0,sizeof (fileType));
        memset(coverImgName,0,sizeof (coverImgName));
        memset(description,0,_DEF_CONTENT_LENGTH);
        strcpy(description,"这是一个不错的视频，耐心看完它吧。");
        fileSize = 0;
    }
    PackType type;
    int userId;
    int fileId;
    char fileType[_DEF_NAME_LENGTH];      //区分文件是图片还是视频
    char fileName[_DEF_NAME_LENGTH];      //文件名
    char coverImgName[_DEF_NAME_LENGTH];  //方便数据库写库 视频——封面图
    char description[_DEF_CONTENT_LENGTH];
    STRU_VIDEO_LABEL label;
    int64_t fileSize;                     //文件大小 用于文件传输结束判断
};


#define file_transmit_res_success 1
#define file_transmit_res_failure 0
struct STRU_TCP_UPLOAD_RS{
    STRU_TCP_UPLOAD_RS():type(_DEF_TCP_UPLOAD_RS),result(file_transmit_res_failure)
    {

    }
    PackType type;
    int result;
};

//文件块请求
struct STRU_TCP_FILEBLOCK_RQ{
    STRU_TCP_FILEBLOCK_RQ()
    {
        type = _DEF_TCP_FILEBLOCK_RQ;
        userId = 0;
        fileId = 0;
        blockLen = 0;
        pos = 0;
        memset(fileContent,0,sizeof(fileContent));
    }
    PackType type;
    int userId;
    int fileId;
    int64_t blockLen; //文件写入大小
    char fileContent[_DEF_FILE_MAX_CONTENT_LEN];
    int64_t pos;
};

#include <QString>
#include <QFile>
//文件信息(跨函数发送时用得到)
struct STRU_FILE_INFO{
    int fileId;
    int64_t filePos;
    int64_t fileSize;
    QString filePath;
    QString fileName;
    QFile *pFile;
};

//服务器使用的数据结构
typedef struct STRU_SERVER_FILE_INFO
{
public:
    STRU_SERVER_FILE_INFO():fileID(0),videoID(0),userId(0),fileSize(0),Pos(0),pFile(0),hotdegree(0)
    {
        upvoteNum = 0;
        collectNum = 0;
        commentNum = 0;
        memset(filePath, 0 , _DEF_NAME_LENGTH);
        memset(fileName, 0 , _DEF_NAME_LENGTH);
        memset(coverImgPath , 0 , _DEF_NAME_LENGTH );
        memset(coverImgName, 0 , _DEF_NAME_LENGTH);
        memset(fileType, 0 , _DEF_NAME_LENGTH);
        memset(userName, 0 , _DEF_NAME_LENGTH);
        memset(rtmp ,0 , _DEF_NAME_LENGTH);
        memset(description,0,_DEF_CONTENT_LENGTH);
        memset(&label,0,sizeof(label));
    }
    int fileID;//下載的時候是用來做 UI 控件編號的， 上傳的時候是一個隨機數， 區分文件。
    int videoID;//真是文件 ID 與 Mysql 的一致
    int userId;
    int64_t fileSize;
    int64_t Pos;

    FILE* pFile;
    char filePath[_DEF_NAME_LENGTH];
    char fileName[_DEF_NAME_LENGTH];
    char fileType[_DEF_NAME_LENGTH];
    char coverImgPath[_DEF_NAME_LENGTH];
    char coverImgName[_DEF_NAME_LENGTH];
    char userName[_DEF_NAME_LENGTH];
    char description[_DEF_CONTENT_LENGTH];
    char rtmp[_DEF_NAME_LENGTH];
    STRU_VIDEO_LABEL label;
    int64_t hotdegree;
    int64_t upvoteNum;
    int64_t collectNum;
    int64_t commentNum;
    int fd;
}ServerFileInfo;



//请求视频请求
typedef struct STRU_NEED_VIDEO_RQ
{
    STRU_NEED_VIDEO_RQ()
    {
        type = _DEF_TCP_NEED_VIDEO_RQ;
        userId = 0;
    }
    PackType type; //包类型
    int userId; //用户 ID

}STRU_NEED_VIDEO_RQ;

//视频推送回复
typedef struct STRU_NEED_VIDEO_RS
{
    STRU_NEED_VIDEO_RS()
    {
        type = _DEF_TCP_NEED_VIDEO_RS;
        fileId = 0;
        upvoteNum = 0;
        collectNum = 0;
        commentNum = 0;
        memset(fileName , 0 ,_DEF_NAME_LENGTH);
        memset(rtmp , 0 ,_DEF_NAME_LENGTH);
        memset(description , 0 ,_DEF_CONTENT_LENGTH);
        memset(authorName,0,_DEF_NAME_LENGTH);
    }
    PackType type; //包类型
    int fileId;
    int videoId;
    char authorName[_DEF_NAME_LENGTH];
    char fileName[_DEF_NAME_LENGTH];
    char rtmp[_DEF_NAME_LENGTH];    // 播放地址 如//1/103.MP3 用户本地需要转化为 rtmp://服务器 ip/app 名/ + 这个字符串 //本项目 app 名为 vod
    char description[_DEF_CONTENT_LENGTH];
    int64_t fileSize;
    int64_t upvoteNum;
    int64_t collectNum;
    int64_t commentNum;
}STRU_NEED_VIDEO_RS;



//--------------------------------------------------------------------------------------------------------------------------------
//Service发送的好友信息---type id name iconid feeling 在线状态
struct _STRU_TCP_FRIEND_INFO {
    _STRU_TCP_FRIEND_INFO
        () :type(_DEF_TCP_FRIEND_INFO), id(0), iconId(0), status(status_offline)
    {
        memset(name, 0, _DEF_NAME_LENGTH);
        memset(feeling, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    int id;
    int iconId;
    int status;
    char name[_DEF_NAME_LENGTH];
    char feeling[_DEF_NAME_LENGTH];
    map<int,string>hisGroupMap;
};
//聊天请求---type 聊天内容content 服务器转发对象toId 服务器转发自谁fromId
struct _STRU_TCP_CHAT_RQ {
    _STRU_TCP_CHAT_RQ
        () :type(_DEF_TCP_CHAT_RQ), fromId(0), toId(0)
    {
        memset(content, 0, _DEF_CONTENT_LENGTH);
    }
    PackType type;
    int fromId;
    int toId;
    char content[_DEF_CONTENT_LENGTH];
};
//聊天回复---type 聊天结果（聊天失败时收到聊天回复）
struct _STRU_TCP_CHAT_RS {
    _STRU_TCP_CHAT_RS
        () :type(_DEF_TCP_CHAT_RS), m_nChatRes(chat_send_false)
        , fromId(-1)
    {

    }
    PackType type;
    int fromId;
    int m_nChatRes;
};
//添加好友请求---type 昵称name(只支持根据昵称添加好友) 自己的id 自己的name
struct _STRU_TCP_ADD_FRIEND_RQ {
    _STRU_TCP_ADD_FRIEND_RQ
        () :type(_DEF_TCP_ADD_FRIEND_RQ), fromId(0)
    {
        memset(fromName, 0, _DEF_NAME_LENGTH);
        memset(toName, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    int fromId;
    char fromName[_DEF_NAME_LENGTH];
    char toName[_DEF_NAME_LENGTH]; //加谁为好友
};
//添加好友回复---type 添加结果 自己id 好友f_id 好友昵称f_name
struct _STRU_TCP_ADD_FRIEND_RS {
    _STRU_TCP_ADD_FRIEND_RS
        () :type(_DEF_TCP_ADD_FRIEND_RS), m_nAddRes(add_friend_false_no_exist)
        , fromId(0), friendId(0)
    {
        memset(friendName, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    int fromId;
    int friendId;
    char friendName[_DEF_NAME_LENGTH];
    int m_nAddRes;
};
//删除好友请求
struct _STRU_TCP_DELETE_FRIEND_RQ {
    _STRU_TCP_DELETE_FRIEND_RQ()
        :type(_DEF_TCP_DELETE_FRIEND_RQ), userId(-1)
    {
        memset(friendName, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    int userId;
    char friendName[_DEF_NAME_LENGTH];
};
//删除好友回复
struct _STRU_TCP_DELETE_FRIEND_RS {
    _STRU_TCP_DELETE_FRIEND_RS()
        :type(_DEF_TCP_DELETE_FRIEND_RS), friendId(-1), userId(-1), deleteRes(delete_friend_false)
    {
        memset(friendName, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    int userId;
    int friendId;
    char friendName[_DEF_NAME_LENGTH];
    int deleteRes;
};

//朋友删除你的msg
struct _STRU_TCP_FRIEND_DELETE_YOU_MSG {
    _STRU_TCP_FRIEND_DELETE_YOU_MSG() :friendId(-1), type(_DEF_TCP_FRIEND_DELETE_YOU_MSG) {}
    PackType type;
    int friendId;
};

//发送客户端他加入的一个群信息
struct _STRU_TCP_SEND_CLIENT_GROUP_INFO {
    _STRU_TCP_SEND_CLIENT_GROUP_INFO():type(_DEF_TCP_SEND_CLIENT_GROUP_INFO)
        ,groupId(-1),groupIconid(1),groupStatus(group_offline)
    {
        memset(groupName,0,_DEF_NAME_LENGTH);
    }
    PackType type;
    int groupId;
    int groupIconid;
    char groupName[_DEF_NAME_LENGTH];
    int groupStatus;
};

//发送客户端他加入的群的一个成员的信息
struct _STRU_TCP_SEND_CLIENT_GROUP_MEMBERS_INFO {
    _STRU_TCP_SEND_CLIENT_GROUP_MEMBERS_INFO() :type(_DEF_TCP_SEND_CLIENT_GROUP_MEMBERS_INFO)
        , groupId(-1), groupIconid(1), memberId(-1), memberIconid(0),memberStatus(status_offline), groupStatus(group_offline)
    {
        memset(groupName, 0, _DEF_NAME_LENGTH);
        memset(memberName, 0, _DEF_NAME_LENGTH);
    }
    PackType type;
    int groupId;
    int groupIconid;
    int memberId;
    int memberIconid;
    int memberStatus;
    int groupStatus;
    char groupName[_DEF_NAME_LENGTH];
    char memberName[_DEF_NAME_LENGTH];
};

//客户端1发送文件的请求
struct _STRU_TCP_CLIENT_SEND_FILE_RQ
{
    _STRU_TCP_CLIENT_SEND_FILE_RQ()
        :type(_DEF_TCP_SEND_FILE_RQ), toFriOrGop(file_friend), id(-1), userId(-1)
    {
        memset(szFileId, 0, _MAX_FILE_PATH);
    }
    PackType type;
    int toFriOrGop;
    int id;
    int userId;
    char szFileId[_MAX_FILE_PATH];
};

//客户端2接收文件的消息
struct _STRU_TCP_CLIENT_RECV_FILE_MSG {
    _STRU_TCP_CLIENT_RECV_FILE_MSG()
        :type(_DEF_TCP_RECV_FILE_MSG), fromFriOrGop(file_friend), id(-1)
        ,userId(-1)
    {
        memset(szFileId, 0, _MAX_FILE_PATH);
    }
    PackType type;
    int fromFriOrGop;
    int id;
    int userId;
    char szFileId[_MAX_FILE_PATH];
};

//客户端2接收文件的回复
struct _STRU_TCP_CLIENT_RECV_FILE_RES {
    _STRU_TCP_CLIENT_RECV_FILE_RES()
        :type(_DEF_TCP_RECV_FILE_RES), fromFriOrGop(file_friend), id(-1),userId(-1)
        ,userRecvRes(user_recv_file_no)
    {
        memset(szFileId, 0, _MAX_FILE_PATH);
    }
    PackType type;
    int fromFriOrGop;
    int id;
    int userId;
    int userRecvRes;
    char szFileId[_MAX_FILE_PATH];
};

//客户端1发送文件结果 服务器已转发/转发失败
struct _STRU_TCP_SEND_FILE_RES
{
    _STRU_TCP_SEND_FILE_RES()
        :type(_DEF_TCP_SEND_FILE_RES), toFriOrGop(file_friend), id(-1), sendRes(send_file_failed_ser_recv)
    {}
    PackType type;
    int toFriOrGop;
    int id;
    int sendRes;
};



//下线请求----type 自己的id
struct _STRU_TCP_OFFLINE_RQ {
    _STRU_TCP_OFFLINE_RQ
        () :type(_DEF_TCP_OFFLINE_RQ), id(0)
    {

    }
    PackType type;
    int id;
};
//验证登录请求
struct _STRU_TCP_VALIDATE_RQ {
    _STRU_TCP_VALIDATE_RQ
        () :type(_DEF_TCP_VALIDATE_LOGINED_RQ), id(0)
    {

    }
    PackType type;
    int id;
};
//验证登录回复
struct _STRU_TCP_VALIDATE_RS {
    _STRU_TCP_VALIDATE_RS
        () :type(_DEF_TCP_VALIDATE_LOGINED_RS), id(0), status(false)
    {

    }
    PackType type;
    int id;
    bool status;
};
// //群聊聊天请求
// struct _STRU_TCP_GROUP_CHAT_RQ{

//     PackType type;
//     int memberId;
//     int groupId;
//     char content[_DEF_CONTENT_LENGTH];
// };

//群聊聊天消息
struct _STRU_TCP_GROUP_CHAT_MSG{
    _STRU_TCP_GROUP_CHAT_MSG()
        :type(_DEF_TCP_GROUP_CHAT_MSG),memberId(-1),groupId(-1)
    {
        memset(content,0,_DEF_CONTENT_LENGTH);
    }
    PackType type;
    int memberId;
    int groupId;
    char content[_DEF_CONTENT_LENGTH];
};

//用户信息表t_user: (id int(自增 主键)查找快,tel,name,password,iconid,feeling)
//好友关系表t_friend:(id1,id2,) 为了快速采用双向存储
struct STRU_pINetMediator{
    STRU_pINetMediator():pUDP(nullptr),pTCP(nullptr)
    {}
    void* pUDP;
    void* pTCP;
};

//发送文件相关的

//协议头
#define _DEF_PROTOCOL_BASE (1000)
//文件信息
#define _DEF_PROTOCOL_FILE_INFO_RQ (_DEF_PROTOCOL_BASE + 100)
//文件块
#define _DEF_PROTOCOL_FILE_BLOCK_RQ (_DEF_PROTOCOL_BASE + 101)
//接收文件的结果
#define _DEF_PROTOCOL_FILE_RECV_RES (_DEF_PROTOCOL_BASE + 102)
#define recv_file_succeed (0)
#define recv_file_failed (1)

//id name最大长度
#define  _MAX_FILE_PATH (512)
//文件传输块的最大长度
#define  _MAX_FILE_CONTENT_SIZE (8 * 1024)

//协议结构体
//文件信息请求：协议头 文件名 文件大小 文件的唯一标识id
struct STRU_FILE_INFO_RQ {
    STRU_FILE_INFO_RQ():nType(_DEF_PROTOCOL_FILE_INFO_RQ),szFileSize(0)
    {
        memset(szFileId, 0, _MAX_FILE_PATH);
        memset(szFileName, 0, _MAX_FILE_PATH);
    }
    int nType;
    char szFileId[_MAX_FILE_PATH];
    char szFileName[_MAX_FILE_PATH];
    long long szFileSize; // long long可以发送
};
//文件块
struct STRU_FILE_BLOCK_RQ {
    STRU_FILE_BLOCK_RQ() :nType(_DEF_PROTOCOL_FILE_BLOCK_RQ), nBlockSize(0)
    {
        memset(szFileId, 0, _MAX_FILE_PATH);
        memset(szFileContent, 0, _MAX_FILE_CONTENT_SIZE);
    }
    int nType;
    char szFileId[_MAX_FILE_PATH];
    char szFileContent[_MAX_FILE_CONTENT_SIZE];
    int nBlockSize;
};

//接收文件的结果
struct STRU_RECV_FILE_RES {
    STRU_RECV_FILE_RES():nType(_DEF_PROTOCOL_FILE_RECV_RES),recvRes(recv_file_failed)
    {

    }
    int nType;
    int recvRes;
};

//本地保存的文件信息:文件标识 文件名 文件路径 当前位置 总大小 文件指针
//struct STRU_FILE_INFO {
//    STRU_FILE_INFO():nPos(0),nFileSize(0),pFile(nullptr)
//    {
//        memset(szFileId, 0, _MAX_FILE_PATH);
//        memset(szFileName, 0, _MAX_FILE_PATH);
//        memset(szFilePath, 0, _MAX_FILE_PATH);
//    }
//    char szFileId[_MAX_FILE_PATH];
//    char szFileName[_MAX_FILE_PATH];
//    char szFilePath[_MAX_FILE_PATH];
//    long long nPos; //当前在什么位置
//    long long nFileSize; //文件总大小
//    FILE* pFile; //文件指针
//};

// #pragma once

// #include<memory.h>
// #define _UDP_PORT 7099
// #define _TCP_PORT 10099
// #define str_ip "192.168.14.215"
// #define _TCP_IP ("192.168.202.215")
// typedef int PackType;
// #define _DEF_NAME_LENGTH (100)
// #define _DEF_CONTENT_LENGTH (2048)
// #define _DEF_PROTOCOL_COUNT (20)

// //宏定义BASE
// #define _DEF_PROTOCOL_BASE (1000)

// //UDP
// //上线请求
// #define _DEF_UDP_ONLINE_RQ (_DEF_PROTOCOL_BASE + 1)
// //上线回复
// #define _DEF_UDP_ONLINE_RS (_DEF_PROTOCOL_BASE + 2)
// //聊天请求
// #define _DEF_UDP_CHAT_RQ (_DEF_PROTOCOL_BASE + 3)
// //下线请求
// #define _DEF_UDP_OFFLINE_RQ (_DEF_PROTOCOL_BASE + 4)

// //TCP
// //注册请求
// #define _DEF_TCP_REGISTER_RQ (_DEF_PROTOCOL_BASE + 5)
// //注册回复
// #define _DEF_TCP_REGISTER_RS (_DEF_PROTOCOL_BASE + 6)
// //注册结果
// #define register_success (0)
// #define register_false_tel_used (1)
// #define register_false_name_used (2)
// #define register_false_sql_select_error (3)
// #define register_false_sql_update_error (4)

// //登录请求
// #define _DEF_TCP_LOGIN_RQ (_DEF_PROTOCOL_BASE + 7)
// //登录回复
// #define _DEF_TCP_LOGIN_RS (_DEF_PROTOCOL_BASE + 8)
// //登录结果
// #define login_success (0)
// #define login_false_no_tel (1)
// #define login_false_password_wrong (2)
// #define login_false_sql_select_error (3)
// #define login_false_illegal (4)
// //Service发送的好友信息
// #define _DEF_TCP_FRIEND_INFO (_DEF_PROTOCOL_BASE + 9)
// //在线状态
// #define status_online (0)
// #define status_offline (1)
// //聊天请求
// #define _DEF_TCP_CHAT_RQ (_DEF_PROTOCOL_BASE + 10)
// //聊天回复
// #define _DEF_TCP_CHAT_RS (_DEF_PROTOCOL_BASE + 11)
// #define chat_send_success (0)
// #define chat_send_false (1)

// //添加好友请求
// #define _DEF_TCP_ADD_FRIEND_RQ (_DEF_PROTOCOL_BASE + 12)
// //添加好友回复
// #define _DEF_TCP_ADD_FRIEND_RS (_DEF_PROTOCOL_BASE + 13)
// //添加好友的结果
// #define add_friend_success (0)
// #define add_friend_false_offline (2)
// #define add_friend_false_no_exist (1)
// #define add_friend_false_disagree (3)
// //删除好友请求 发给服务器
// #define _DEF_TCP_DELETE_FRIEND_RQ (_DEF_PROTOCOL_BASE + 17)
// //删除好友操作回复 来自服务器
// #define _DEF_TCP_DELETE_FRIEND_RS (_DEF_PROTOCOL_BASE + 18)
// #define delete_friend_success (0)
// #define delete_friend_false (1)
// //朋友删除你的消息
// #define _DEF_TCP_FRIEND_DELETE_YOU_MSG (_DEF_PROTOCOL_BASE + 19)
// //下线请求
// #define _DEF_TCP_OFFLINE_RQ (_DEF_PROTOCOL_BASE + 14)
// //验证登录状态请求
// #define _DEF_TCP_VALIDATE_LOGINED_RQ ((_DEF_PROTOCOL_BASE + 15))
// //验证登录状态请求
// #define _DEF_TCP_VALIDATE_LOGINED_RS ((_DEF_PROTOCOL_BASE + 16))

// //请求协议包

// //上线请求：协议头,ip,name
// struct _STRU_ONLINE{
//     _STRU_ONLINE():type(_DEF_UDP_ONLINE_RQ)
//         //,ip(0)
//     {
//         memset(name,0,_DEF_NAME_LENGTH);
//     }
//     PackType type;
//     //long ip;
//     char name[_DEF_NAME_LENGTH];
// };

// //聊天请求：协议头,ip,message
// struct _STRU_CHAT_RQ{
//     _STRU_CHAT_RQ():type(_DEF_UDP_CHAT_RQ)
//         //,ip(0)
//     {
//         memset(content,0,_DEF_CONTENT_LENGTH);
//     }
//     PackType type;
//     //long ip;
//     char content[_DEF_CONTENT_LENGTH];
// };
// //下线请求：协议头,ip,name

// struct _STRU_OFFLINE_RQ{
//     _STRU_OFFLINE_RQ():type(_DEF_UDP_OFFLINE_RQ)//,ip(0)
//     {
//     }
//     PackType type;
//     //long ip;
// };




// //TCP数据报结构体

// //注册请求---type 电话 昵称 密码
// struct _STRU_TCP_REGISTER_RQ{
//     _STRU_TCP_REGISTER_RQ
//         ():type(_DEF_TCP_REGISTER_RQ)
//     {
//         memset(tel,0,_DEF_NAME_LENGTH);
//         memset(name,0,_DEF_NAME_LENGTH);
//         memset(password,0,_DEF_NAME_LENGTH);
//     }
//     PackType type;
//     char tel[_DEF_NAME_LENGTH];
//     char name[_DEF_NAME_LENGTH];
//     char password[_DEF_NAME_LENGTH];
// };

// //注册回复
// struct _STRU_TCP_REGISTER_RS{
//     _STRU_TCP_REGISTER_RS
//         ()
//         :type(_DEF_TCP_REGISTER_RS)
//         ,m_nRegister(register_false_name_used)//默认失败
//     {}
//     PackType type;
//     int m_nRegister; //注册结果---成功(1) 失败1电话重复(1) 失败2昵称重复(2)

// };
// //登录请求
// struct _STRU_TCP_LOGIN_RQ{
//     _STRU_TCP_LOGIN_RQ
//         ():type(_DEF_TCP_LOGIN_RQ),bOnlineStatus(false)
//     {
//         memset(tel,0,_DEF_NAME_LENGTH);
//         memset(password,0,_DEF_NAME_LENGTH);
//     }
//     PackType type;
//     char tel[_DEF_NAME_LENGTH];
//     char password[_DEF_NAME_LENGTH];
//     bool bOnlineStatus;
// };
// //登录回复
// struct _STRU_TCP_LOGIN_RS{
//     _STRU_TCP_LOGIN_RS
//         ():type(_DEF_TCP_LOGIN_RS),m_nLogin(login_false_no_tel),userId(0),sock(0)
//     {

//     }
//     PackType type;
//     int userId;
//     int m_nLogin;
//     long sock;
//     //#define login_success (0)#define login_false_no_tel (1)
//                   //#define login_false_password_wrong (2)

// };
// //Service发送的好友信息---type id name iconid feeling 在线状态
// struct _STRU_TCP_FRIEND_INFO{
//     _STRU_TCP_FRIEND_INFO
//         ():type(_DEF_TCP_FRIEND_INFO),id(0),iconId(0),status(status_offline)
//     {
//         memset(name,0,_DEF_NAME_LENGTH);
//         memset(feeling,0,_DEF_NAME_LENGTH);
//     }
//     PackType type;
//     int id;
//     int iconId;
//     int status;
//     char name[_DEF_NAME_LENGTH];
//     char feeling[_DEF_NAME_LENGTH];

// };
// //聊天请求---type 聊天内容content 服务器转发对象toId 服务器转发自谁fromId
// struct _STRU_TCP_CHAT_RQ{
//     _STRU_TCP_CHAT_RQ
//         ():type(_DEF_TCP_CHAT_RQ),fromId(0),toId(0)
//     {
//         memset(content,0,_DEF_CONTENT_LENGTH);
//     }
//     PackType type;
//     int fromId;
//     int toId;
//     char content[_DEF_CONTENT_LENGTH];
// };
// //聊天回复---type 聊天结果（聊天失败时收到聊天回复）
// struct _STRU_TCP_CHAT_RS{
//     _STRU_TCP_CHAT_RS
//         ():type(_DEF_TCP_CHAT_RS),m_nChatRes(chat_send_false),fromId(-1)
//     {

//     }
//     PackType type;
//     int fromId;
//     int m_nChatRes;
// };
// //添加好友请求---type 昵称name(只支持根据昵称添加好友) 自己的id 自己的name
// struct _STRU_TCP_ADD_FRIEND_RQ{
//     _STRU_TCP_ADD_FRIEND_RQ
//         ():type(_DEF_TCP_ADD_FRIEND_RQ),fromId(0)
//     {
//         memset(fromName,0,_DEF_NAME_LENGTH);
//         memset(toName,0,_DEF_NAME_LENGTH);
//     }
//     PackType type;
//     int fromId;
//     char fromName[_DEF_NAME_LENGTH];
//     char toName[_DEF_NAME_LENGTH]; //加谁为好友
// };
// //添加好友回复---type 添加结果 自己id 好友f_id 好友昵称f_name
// struct _STRU_TCP_ADD_FRIEND_RS{
//     _STRU_TCP_ADD_FRIEND_RS
//         ():type(_DEF_TCP_ADD_FRIEND_RS),m_nAddRes(add_friend_false_no_exist)
//         ,fromId(0),friendId(0)
//     {
//         memset(friendName,0,_DEF_NAME_LENGTH);
//     }
//     PackType type;
//     int fromId;
//     int friendId;
//     char friendName[_DEF_NAME_LENGTH];
//     int m_nAddRes;
// };
// //删除好友请求
// struct _STRU_TCP_DELETE_FRIEND_RQ{
//     _STRU_TCP_DELETE_FRIEND_RQ()
//         :type(_DEF_TCP_DELETE_FRIEND_RQ),userId(-1)
//     {memset(friendName,0,_DEF_NAME_LENGTH);}
//     PackType type;
//     int userId;
//     char friendName[_DEF_NAME_LENGTH];
// };
// //删除好友回复
// struct _STRU_TCP_DELETE_FRIEND_RS{
//     _STRU_TCP_DELETE_FRIEND_RS()
//         :type(_DEF_TCP_DELETE_FRIEND_RS),userId(-1),friendId(-1),deleteRes(delete_friend_false)
//     {memset(friendName,0,_DEF_NAME_LENGTH);}
//     PackType type;
//     int userId;
//     int friendId;
//     char friendName[_DEF_NAME_LENGTH];
//     int deleteRes;
// };

// //朋友删除你的msg
// struct _STRU_TCP_FRIEND_DELETE_YOU_MSG{
//     _STRU_TCP_FRIEND_DELETE_YOU_MSG():friendId(-1),type(_DEF_TCP_FRIEND_DELETE_YOU_MSG){}
//     PackType type;
//     int friendId;
// };

// //下线请求----type 自己的id
// struct _STRU_TCP_OFFLINE_RQ{
//     _STRU_TCP_OFFLINE_RQ
//         ():type(_DEF_TCP_OFFLINE_RQ),id(0)
//     {

//     }
//     PackType type;
//     int id;
// };

// //验证登录请求
// struct _STRU_TCP_VALIDATE_RQ{
//     _STRU_TCP_VALIDATE_RQ
//         ():type(_DEF_TCP_VALIDATE_LOGINED_RQ),id(0)
//     {

//     }
//     PackType type;
//     int id;

// };
// //验证登录回复
// struct _STRU_TCP_VALIDATE_RS{
//     _STRU_TCP_VALIDATE_RS
//         ():type(_DEF_TCP_VALIDATE_LOGINED_RS),id(0),status(false)
//     {

//     }
//     PackType type;
//     int id;
//     bool status;
// };
// //用户信息表t_user: (id int(自增 主键)查找快,tel,name,password,iconid,feeling)
// //好友关系表t_friend:(id1,id2,) 为了快速采用双向存储

//自定义协议   先写协议头 再写协议结构
//登录 注册 获取好友信息 添加好友 聊天 发文件 下线请求
#define _DEF_PACK_BASE	(1000)
#define _DEF_PACK_COUNT (10000)

//注册
#define _DEF_PACK_REGISTER_RQ	(_DEF_PACK_BASE + 0 )
#define _DEF_PACK_REGISTER_RS	(_DEF_PACK_BASE + 1 )
//登录
#define _DEF_PACK_LOGIN_RQ	(_DEF_PACK_BASE + 2 )
#define _DEF_PACK_LOGIN_RS	(_DEF_PACK_BASE + 3 )


//返回的结果
//注册请求的结果
//#define user_is_exist		(0)
//#define register_success	(1)
////登录请求的结果
//#define user_not_exist		(0)
//#define password_error		(1)
//#define login_success		(2)


//typedef int PackType;

////协议结构
////注册
//typedef struct STRU_REGISTER_RQ
//{
//    STRU_REGISTER_RQ():type(_DEF_PACK_REGISTER_RQ)
//    {
//        memset( tel  , 0, sizeof(tel));
//        memset( name  , 0, sizeof(name));
//        memset( password , 0, sizeof(password) );
//    }
//    //需要手机号码 , 密码, 昵称
//    PackType type;
//    char tel[_MAX_SIZE];
//    char name[_MAX_SIZE];
//    char password[_MAX_SIZE];

//}STRU_REGISTER_RQ;

//typedef struct STRU_REGISTER_RS
//{
//    //回复结果
//    STRU_REGISTER_RS(): type(_DEF_PACK_REGISTER_RS) , result(register_success)
//    {
//    }
//    PackType type;
//    int result;

//}STRU_REGISTER_RS;

////登录
//typedef struct STRU_LOGIN_RQ
//{
//    //登录需要: 手机号 密码
//    STRU_LOGIN_RQ():type(_DEF_PACK_LOGIN_RQ)
//    {
//        memset( tel , 0, sizeof(tel) );
//        memset( password , 0, sizeof(password) );
//    }
//    PackType type;
//    char tel[_MAX_SIZE];
//    char password[_MAX_SIZE];

//}STRU_LOGIN_RQ;

//typedef struct STRU_LOGIN_RS
//{
//    // 需要 结果 , 用户的id
//    STRU_LOGIN_RS(): type(_DEF_PACK_LOGIN_RS) , result(login_success),userid(0)
//    {
//    }
//    PackType type;
//    int result;
//    int userid;

//}STRU_LOGIN_RS;







