#ifndef THREADPLAYER_H
#define THREADPLAYER_H

#include <QThread>
#include <QImage>
#include "config.h"
#define MAX_AUDIO_SIZE (1024*16*25*10)  //音频阈值
#define MAX_VIDEO_SIZE (1024*255*25*2)  //视频阈值
//当队列里面的数据超过某个大小的时候 就暂停读取
//防止一下子就把视频读完了，导致的空间分配不足

struct VideoState;

class threadPlayer : public QThread
{
    Q_OBJECT
public:
    static const threadPlayer* m_player;
    QString m_fileName = "";

    threadPlayer();
    void run() override;

    //设置发送信号的图片的大小
    void setOutPutWidthHeight(int width,int height);

    void transOneFrame(QImage img);
    void transImgWH(int width,int height);

    //播放控制
    bool play();
    bool pause();
    bool stop(bool isWait);
    bool setFileName(const QString &fileName);
    double getCurrentTime();
    PlayerState m_playerState;

    PlayerState getState() const;
    void seek(int64_t pos);
    double getTotalTime();
private:
    int m_width = 0;
    int m_height = 0;
    VideoState m_videoState;
    int onlyVideoStreamTimerID = -1;
    //信号槽----------------------------------------------------------------------------
signals:
    //每次获取一帧画面, 发送此信号
    //这样, 我们可以在外面使用槽函数连接, 得到这个图片, 然后粘贴到控件上面
    void SIG_transOneFrame(QImage);
    void SIG_sendVieoWH(int imgWidth,int imgHeight);
    void SIG_totalTime(qint64 uSec);
    void SIG_currentTime(qint64 cSec);

    //--------------------------------------------------------------------------------
public slots:



    //------------------------------------------------------------------------------------
private slots:

};



#endif // THREADPLAYER_H
