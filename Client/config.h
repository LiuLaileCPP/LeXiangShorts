#ifndef CONFIG_H
#define CONFIG_H
#include "packetqueue.h"
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavdevice/avdevice.h"
#include "libavutil/time.h"
#include "libavutil/pixfmt.h"
#include "SDL.h"
}
#define SDL_AUDIO_BUFFER_SIZE 1024
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000
// 1 second of 48khz 32bit audio
//192000 的由来: 播放的音乐是 48khz 32 位的音频 这里主要计算采样率 就是 1s 采样多少字节
//32bit-> 4 字节 48khz 是 1s 采样 48000 次 每次 4 字节 , 也就是 1s 采样 48000*4 = 192000 字节

enum PlayerState{
    Playing = 0,
    Pause,
    Stop
};

class threadPlayer;
typedef struct VideoState {
    AVFormatContext *pFormatCtx;    //相当于视频”文件指针”
    ///////////////音频///////////////////////////////////
    AVStream *audio_st;             //音频流
    PacketQueue *audioq;            //音频缓冲队列
    AVCodecContext *pAudioCodecCtx ;//音频解码器信息指针
    int audioStream;                //视频音频流索引
    double audio_clock;             //<pts of last decoded frame 音频时钟
    SDL_AudioDeviceID audioID = 0;      //音频 ID
    AVFrame out_frame;              //设置参数，供音频解码后的 swr_alloc_set_opts 使用。
                                    // 音频回调函数使用的量
    uint8_t audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
    unsigned int audio_buf_size = 0;
    unsigned int audio_buf_index = 0;
    AVFrame *audioFrame;

    ///////////////视频///////////////////////////////////
    AVStream *video_st;             //视频流
    PacketQueue *videoq;            //视频队列
    AVCodecContext *pCodecCtx ;     //音频解码器信息指针
    int videoStream;                //视频音频流索引
    double video_clock; ///<pts of last decoded frame 视频时钟
    SDL_Thread *video_tid;          //视频线程 id

    //////////////////////////////////////////////////////
    /// 播放控制的变量
    bool pause = false;//暂停标志
    bool quit = false; //停止
    bool playerFinished = true; //threadPlayer结束标志
    bool readPacketsFinished = true; //读取文件压缩数据包是否结束
    bool threadVideoCodecImgFinished = true; // 视频队列解码发信号是否结束
    /////////////////////////////////////////////////////
    //// 跳转相关的变量
    int seek_req; //跳转标志 -- 读线程
    int64_t seek_pos; //跳转的位置 -- 微秒
    int seek_flag_audio;//跳转标志 -- 用于音频线程中
    int seek_flag_video;//跳转标志 -- 用于视频线程中
    double seek_time; //跳转的时间(秒) 值和 seek_pos 是一样的
    //////////////////////////////////////////////////////
    int64_t start_time; //单位 微秒
    VideoState()
    {
        audio_clock = video_clock = start_time = 0;
    }
    threadPlayer* m_player;     //用于调用发送信号函数
} VideoState;

#endif // CONFIG_H
