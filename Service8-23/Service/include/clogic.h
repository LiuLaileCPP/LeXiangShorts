#ifndef CLOGIC_H
#define CLOGIC_H
#include <QThread>
#include"TCPKernel.h"
#include "threadfileworker.h"
#include <QApplication>

class CLogic //:public QThread
{
    //Q_OBJECT
public:
    CLogic( TcpKernel* pkernel )
    {
        m_pKernel = pkernel;
        m_sql = pkernel->m_sql;
        m_tcp = pkernel->m_tcp;
        //m_threadr = new QThread;
        //m_worker = new threadFileWorker;
        //m_worker->moveToThread(m_threadr);
        //m_threadr->start();
//        connect(this,SIGNAL(SIG_writeFile(QString,qint64,char* ,qint64)),m_worker,SLOT(slot_writeFile(QString,qint64,char* ,qint64))
//                ,Qt::BlockingQueuedConnection);
    }

    ~CLogic();

//signals:
//    void SIG_writeFile(QString path,qint64 pos,char*content ,qint64 len);

public:
    //设置协议映射
    void setNetPackMap();
    /************** 发送数据*********************/
    void SendData( sock_fd clientfd, char*szbuf, int nlen )
    {
        m_pKernel->SendData( clientfd ,szbuf , nlen );
    }
    /************** 网络处理 *********************/
    //注册
    void RegisterRq(sock_fd clientfd, char*szbuf, int nlen);
    //登录
    void LoginRq(sock_fd clientfd, char*szbuf, int nlen);

    void UploadRq(sock_fd clientfd, char *szbuf, int nlen);
    void UploadBlockRq(sock_fd clientfd, char *szbuf, int nlen); //file block

    void SendVideoStream(sock_fd clientfd, char *szbuf, int nlen);
    void getFileList(list<ServerFileInfo *> &fileList, int userId);

    void updateUserActivity(sock_fd clientfd, char *szbuf, int nlen);

    void dealNeedComment(sock_fd clientfd, char *szbuf, int nlen);
    void dealNeedBarrage(sock_fd clientfd, char *szbuf, int nlen);
private:
    TcpKernel* m_pKernel;
    CMysql * m_sql;
    Block_Epoll_Net * m_tcp;
    map<volatile int,volatile unsigned int>m_mapIdToFd;
    map<volatile int,STRU_SERVER_FILE_INFO*> m_mapFileIdToFileInfo;
    //QThread* m_threadr;
    //threadFileWorker* m_worker;
};

#endif // CLOGIC_H
