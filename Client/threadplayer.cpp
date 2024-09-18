#include "threadplayer.h"
#include <QDebug>
#define FLUSH_DATA "FLUSH"

AVFrame     wanted_frame;
PacketQueue audio_queue;
int         quit = 0;
const threadPlayer* threadPlayer::m_player = nullptr;

//包含音频处理函数声明
//回调函数
void audio_callback(void *userdata, Uint8 *stream, int len);
//解码函数
int audio_decode_frame(VideoState *state, uint8_t *audio_buf, int buf_size);
//找 auto_stream
int find_stream_index(AVFormatContext *pformat_ctx, int *video_stream, int
                      *audio_stream);


threadPlayer::threadPlayer()
{
    m_player = this;
    m_playerState = Stop;
    m_width = 0;
    m_height = 0;
}

void threadPlayer::transImgWH(int width,int height)
{
    emit SIG_sendVieoWH(width,height);
}
void threadPlayer::transOneFrame(QImage img)
{
    emit SIG_transOneFrame(img);
}

//播放控制
bool threadPlayer::play()
{
    if(m_playerState == Pause)
    {
        //维护状态
        m_videoState.pause = false;
        m_playerState = Playing;
        return true;
    }
    //标志位置位
    return false;
}
bool threadPlayer::pause()
{
    if(m_playerState == Playing)
    {
        m_videoState.pause = true;
        m_playerState = Pause;
        return true;
    }
    return false;
}
bool threadPlayer::stop(bool isWait)
{
    m_videoState.quit = true;
    SDL_Delay(100);
    this->exit();
    this->wait();

    //关闭 SDL 音频设备
    if (m_videoState.audioID != 0)
    {
        SDL_LockAudio();
        SDL_PauseAudioDevice(m_videoState.audioID,1);//停止播放,即停止音频回调函数
        SDL_CloseAudioDevice( m_videoState.audioID );
        SDL_UnlockAudio();
        m_videoState.audioID = 0;
        qDebug()<<"threadPlayer:: SDL over!!!";
    }

    m_playerState = Stop;
    return true;
}
bool threadPlayer::setFileName(const QString &fileName)
{
    if(m_playerState != Stop)
    {
        stop(true);
    }
    m_fileName = fileName;
    m_playerState = Playing;
    this->start();
    return true;
}

//跳转
void threadPlayer::seek(int64_t pos)//微妙
{
    //没有在跳转状态 才可以跳转
    if(!m_videoState.seek_req) //更新跳转请求
    {
        m_videoState.seek_pos = pos;
        m_videoState.seek_req = 1;
    }
}

PlayerState threadPlayer::getState() const
{
    return m_playerState;
}

void reSetVideoState()
{

}

void threadPlayer::setOutPutWidthHeight(int width,int height)
{

}

double threadPlayer::getCurrentTime()
{
    return m_videoState.audio_clock;
}

double threadPlayer::getTotalTime()
{
    if(m_videoState.pFormatCtx) //有文件指针才能返回时间
        return m_videoState.pFormatCtx->duration;
    else
        return -1;
}


// 回调函数的参数，时间和有无流的判断
typedef struct {
    time_t lasttime;
    bool connected;
} Runner;
//添加解决打开资源阻塞
Runner input_runner = { 0 };
// 回调函数
int interrupt_callback(void *p) {
    Runner *r = (Runner *)p;
    if (r->lasttime > 0) {
        if (time(NULL) - r->lasttime > 5 && !r->connected) {
            // 等待超过 5s 则中断
            return 1;
        }
    }
    return 0;
}
//视频数据包处理函数 新线程
int thread_videopackets_work(void *arg);
//时间补偿函数--视频延时
double synchronize_video(VideoState *is, AVFrame *src_frame, double pts);

Uint32 onlyVideoStreamTimer_callback(Uint32 interval, void *param);

void threadPlayer::run()
{
    qDebug()<<"threadPlayer:: "<<__func__;
    //1. 初始化 FFMPEG 调用了这个才能正常适用编码器和解码器 注册所用函数
    avformat_network_init();
    av_register_all();
    //SDL 初始化
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        qDebug()<< "Couldn't init SDL: " << SDL_GetError() ;
        return;
    }
    memset(&m_videoState,0,sizeof(m_videoState));

    //2. 需要分配一个 AVFormatContext，FFMPEG 所有的操作都要通过这个 AVFormatContext 来进行 可以理解为视频文件指针
    AVFormatContext *pFormatCtx = avformat_alloc_context(); //new 一个文件指针

    //音频处理所需变量--------------------------------------------------------------------------------
    int audioStream = -1;                   //音频解码器需要的流的索引
    AVCodecContext *pAudioCodecCtx = NULL;  //音频解码器信息指针
    AVCodec *pAudioCodec = NULL;            //音频解码器
    //SDL
    SDL_AudioSpec wanted_spec;              //SDL 音频设置
    SDL_AudioSpec spec ;                    //SDL 音频设置


    //视频处理所需变量---------------------------------------------------------------------------------
    int videoStream = -1;
    AVCodecContext *pCodecCtx = nullptr;
    AVCodec *pCodec = nullptr;
    AVPacket *packet = (AVPacket *)malloc(sizeof(AVPacket));   //new一个 packet;



    //中文兼容
    std::string path = m_fileName.toStdString();
    const char* file_path = path.c_str();
    qDebug()<<"threadPlayer:: open: "<<m_fileName;
    pFormatCtx->interrupt_callback.callback = interrupt_callback; //打开视频文件失败进入回调函数,解决网络视频数据断流(等待5s)
    pFormatCtx->interrupt_callback.opaque = &input_runner;
    input_runner.lasttime = time(NULL);
    input_runner.connected = false;
    //3. 打开视频文件
    if( avformat_open_input(&pFormatCtx, file_path, NULL, NULL) < 0 ) //根据视频给文件指针赋值
    {
        qDebug()<<"Can't open file";
        avformat_close_input(&pFormatCtx);
        stop(true);
        m_videoState.playerFinished = true;
        m_playerState = Stop;
        return;
    }
    else
    {
        input_runner.connected = true;
    }

    //获取视频文件信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)   //获取视频里面的数据流
    {
        qDebug()<<"Could't find stream infomation.";
        return;
    }

    //4. 从数据流数组中读取音视频流
    if (find_stream_index(pFormatCtx, &videoStream, &audioStream) == -1)
    {
        qDebug()<<"Couldn't find stream index" ;
        return;
    }


    m_videoState.pFormatCtx = pFormatCtx;
    m_videoState.videoStream = videoStream;
    m_videoState.audioStream = audioStream;
    m_videoState.m_player = this;

    //视频流处理
    if (videoStream >= 0)
    {
        //5. 查找解码器
        pCodecCtx = pFormatCtx->streams[videoStream]->codec;
        emit SIG_sendVieoWH(pCodecCtx->width,pCodecCtx->height);
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if (pCodec == NULL) {
            qDebug()<< "Could not find codec." ;
            return;
        }
        //打开解码器
        if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
            qDebug()<< "Could not open codec." ;
            return;
        }

        //设置MyWidth MyHeight
        setOutPutWidthHeight(pCodecCtx->width, pCodecCtx->height);

        //视频流
        m_videoState.video_st = pFormatCtx->streams[ videoStream ];
        m_videoState.pCodecCtx = pCodecCtx;
        //视频同步队列
        m_videoState.videoq = new PacketQueue;
        packet_queue_init( m_videoState.videoq);

        //有音频就创建视频队列线程 在线程里面做音视频同步
        if(audioStream >= 0)
        {
            //创建视频线程
            m_videoState.video_tid = SDL_CreateThread( thread_videopackets_work ,"thread_videopackets_work" ,
                                                       &m_videoState );
        }
        else //没有音频流 就用定时器单独做视频同步
        {
            double fps = av_q2d(m_videoState.video_st->r_frame_rate);
            double fps_diff = 1/fps;
            onlyVideoStreamTimerID = SDL_AddTimer(fps_diff * 1000,onlyVideoStreamTimer_callback,&m_videoState);
            if(onlyVideoStreamTimerID == 0)
            {
                fprintf(stderr,"SDL_AddTimer failed ERR: %s\n",SDL_GetError());
                return;
            }
        }

    }
    else{
        qDebug()<< "This file could't find a video stream." ;
    }


    //音频流处理
    if (audioStream >= 0)
    {
        //找到对应的音频解码器
        pAudioCodecCtx = pFormatCtx->streams[audioStream]->codec;
        pAudioCodec = avcodec_find_decoder(pAudioCodecCtx ->codec_id);
        if (!pAudioCodec)
        {
            qDebug()<< "Couldn't find decoder";
            return;
        }//打卡音频解码器
        avcodec_open2(pAudioCodecCtx, pAudioCodec, NULL);

        m_videoState.audio_st = pFormatCtx->streams[audioStream];
        m_videoState.pAudioCodecCtx = pAudioCodecCtx;
        //6.设置音频信息, 用来打开音频设备。
        SDL_LockAudio();
        wanted_spec.freq = pAudioCodecCtx->sample_rate;
        switch (pFormatCtx->streams[audioStream]->codec->sample_fmt)
        {
        case AV_SAMPLE_FMT_U8:
            wanted_spec.format = AUDIO_S8;
            break;
        case AV_SAMPLE_FMT_S16:
            wanted_spec.format = AUDIO_S16SYS;
            break;
        default:
            wanted_spec.format = AUDIO_S16SYS;
            break;
        };
        //设置音频信息, 用来打开音频设备。
        wanted_spec.channels = pAudioCodecCtx->channels; //通道数
        wanted_spec.silence = 0;                        //设置静音值
        wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE; //采样点
        wanted_spec.callback = audio_callback;      //回调函数
        wanted_spec.userdata = &m_videoState;      //回调函数参数

        //打开音频设备 开启处理队列音频数据包的线程_use: audio_call_back
        m_videoState.audioID = SDL_OpenAudioDevice(NULL,0,&wanted_spec, &spec,0);
        if( m_videoState.audioID < 0 ) //第二次打开 audio 会返回-1
        {
            qDebug()<< "Couldn't open Audio: " << SDL_GetError() ;
            return;
        }

        //设置参数，供解码时候用, swr_alloc_set_opts 的 in 部分参数
        switch (pFormatCtx->streams[audioStream]->codec->sample_fmt)
        {
        case AV_SAMPLE_FMT_U8:
            m_videoState.out_frame.format = AV_SAMPLE_FMT_U8;
            break;
        case AV_SAMPLE_FMT_S16:
            m_videoState.out_frame.format = AV_SAMPLE_FMT_S16;
            break;
        default:
            m_videoState.out_frame.format = AV_SAMPLE_FMT_S16;
            break;
        };
        m_videoState.out_frame.sample_rate = pAudioCodecCtx->sample_rate;
        m_videoState.out_frame.channel_layout = av_get_default_channel_layout(pAudioCodecCtx->channels);
        m_videoState.out_frame.channels = pAudioCodecCtx->channels;
        m_videoState.audioq = new PacketQueue;

        //初始化队列
        packet_queue_init(m_videoState.audioq);
        m_videoState.audioFrame = av_frame_alloc();
        SDL_UnlockAudio();
        // SDL 播放声音 0 播放
        SDL_PauseAudioDevice(m_videoState.audioID,0);
    }
    else{
        qDebug()<< "This file could't find a audio stream." ;
    }


    emit SIG_totalTime(getTotalTime());
    //8. 循环读取视频帧, 转换为 RGB 格式, 抛出信号去控件显示
    int stopMark = 0;
    while(1)
    {
        if( m_videoState.quit ) break;

        //这里做了个限制 当队列里面的数据超过某个大小的时候 就暂停读取 (让线程抓紧去消化队列中的数据) 防止一下子就把视频读完了，导致的空间分配不足
        /* 这里 audioq.size 是指队列中的所有数据包带的音频数据的总量或者视频数据总量，并
        不是包的数量 */
        //这个值可以稍微写大一些
        if( m_videoState.audioStream != -1 && m_videoState.audioq->size > MAX_AUDIO_SIZE ) {
            SDL_Delay(100);
            continue;
        }
        if ( m_videoState.videoStream != -1 && m_videoState.videoq->size > MAX_VIDEO_SIZE) {
            SDL_Delay(100);
            continue;
        }

        //跳转处理
        if( m_videoState.seek_req ) //判断是否跳转
            // 跳转标志位 seek_req --> 1 清除队列里的缓存 3s --> 3min 3s 里面的数据 存在 队列和解码器
            // 3s 在解码器里面的数据和 3min 的会合在一起 引起花屏 --> 解决方案 清理解码器缓存 AV_flush_...
            //什么时候清理 -->要告诉它 , 所以要来标志包 FLUSH_DATA "FLUSH"
            //关键帧--比如 10 秒 --> 15 秒 跳转关键帧 只能是 10 或 15 , 如果你要跳到 13 , 做法是跳到10 然后 10-13 的包全扔掉
            {
                int stream_index = -1;
                int64_t seek_target = m_videoState.seek_pos;//微秒

                if (m_videoState.videoStream >= 0)
                    stream_index = m_videoState.videoStream;
                else if (m_videoState.audioStream >= 0)
                    stream_index = m_videoState.audioStream;

                AVRational aVRational = {1, AV_TIME_BASE}; //时基换算
                if (stream_index >= 0)
                {
                    seek_target = av_rescale_q(seek_target, aVRational,
                                               pFormatCtx->streams[stream_index]->time_base); //获取在流里面跳转到的位置(精确位置)
                }
                //ffmpeg的文件指针跳转(指定流)函数(跳到关键帧)
                if (av_seek_frame(m_videoState.pFormatCtx, stream_index, seek_target, AVSEEK_FLAG_BACKWARD
                                  /*目标位置没有关键帧 跳到上一个关键帧*/) < 0)
                {
                    fprintf(stderr, "%s: failed seeking.\n",m_videoState.pFormatCtx->filename);
                }
                else
                {
                    //清空队列 再向队列插入跳转标志数据包(用来指示清空解码器数据 防止花屏(用跳转前的帧推测跳转后的帧就会花屏))
                    if (m_videoState.audioStream >= 0)
                    {
                        AVPacket *packet = (AVPacket *) malloc(sizeof(AVPacket)); //分配一个 packet
                        av_new_packet(packet, 10);
                        strcpy((char*)packet->data,FLUSH_DATA);
                        packet_queue_flush(m_videoState.audioq);        //清除队列
                        packet_queue_put(m_videoState.audioq, packet); //往队列中存入用来清除的包
                    }
                    if (m_videoState.videoStream >= 0)
                    {
                        AVPacket *packet = (AVPacket *) malloc(sizeof(AVPacket)); //分配一个 packet
                        av_new_packet(packet, 10);
                        strcpy((char*)packet->data,FLUSH_DATA);
                        packet_queue_flush(m_videoState.videoq); //清除队列
                        packet_queue_put(m_videoState.videoq, packet); //往队列中存入用来清除的包
                        m_videoState.video_clock = 0;       //考虑到向左快退 避免卡死
                        //视频解码过快会等音频 循环 SDL_Delay 在循环过程中 音频时钟会改变 , 快退 音频时钟变小
                    }
                }
                m_videoState.seek_req = 0;

                //更新跳转位置 和 跳转标志
                //seek_time用来: 从BACK关键帧出发 一直continue到目标跳转帧 再播放和显示
                m_videoState.seek_time = m_videoState.seek_pos ; //精确到微妙 seek_time 是用来做视频音频的时钟调整 --关键帧
                m_videoState.seek_flag_audio = 1; //在视频音频循环中 , 判断, AVPacket 是 FLUSH_DATA 清空解码器缓存
                m_videoState.seek_flag_video = 1;
            }

                //可以看出 av_read_frame 读取的是一帧视频，并存入一个 AVPacket 的结构中
                if (av_read_frame(pFormatCtx, packet) < 0)
                {
                    if(stopMark >= 400)
                    {
                        m_videoState.readPacketsFinished = true;    //超时读不到数据或者队列为空就是自然退出 另一种是认为点击退出
                        stopMark = 0;
                    }
                    if( m_videoState.quit ) break; //这里认为视频读取完了
                    SDL_Delay(10);  //wait 10ms
                    stopMark++;
                    continue;
                }
        stopMark = 0;

        //判断一个数据压缩帧的类型 并加入到相应的队列
        if (m_videoState.videoStream >=0 && packet->stream_index == m_videoState.videoStream)
        {
            packet_queue_put(m_videoState.videoq, packet);
        }
        else if (m_videoState.audioStream >=0 && packet->stream_index == m_videoState.audioStream)
        {
            packet_queue_put(m_videoState.audioq, packet);
        }
        else
        {
            av_free_packet(packet);
        }
    }

    //9.回收数据
    while(!m_videoState.quit)
    {
        SDL_Delay(100);
    }
    //清空队列
    if(m_videoState.videoStream >= 0)
    {
        packet_queue_flush(m_videoState.videoq);
    }
    if(m_videoState.audioStream >= 0)
    {
        packet_queue_flush(m_videoState.audioq);
    }

    //回收空间
    //等待解码线程结束
    while( m_videoState.videoStream >= 0 && !m_videoState.threadVideoCodecImgFinished )
    {
        SDL_Delay(100);
    }

    //视频自动结束 置标志位
    if( audioStream != -1 )
        avcodec_close(pAudioCodecCtx);

    if( videoStream != -1 )
        avcodec_close(pCodecCtx);

    avformat_close_input(&pFormatCtx);

    av_free( m_videoState.audioFrame ) ;
    m_videoState.audioFrame = NULL;              //为什么[这个回收]不要了

    if(onlyVideoStreamTimerID >= 0)   //onlyVideoStreamTimerID初始值-1 创建timer成功返回>=0
        SDL_RemoveTimer(onlyVideoStreamTimerID);
    SDL_Quit();
    avformat_network_deinit();
    //回收资源之后,在最后添加读取文件线程退出标志.
    m_playerState = Stop;
    m_videoState.playerFinished = true;  //标志着run运行完了 可以再次start

    qDebug()<<"threadPlayer:: run over!!!";
}


//注意 userdata 是前面的编解码器上下文 AVCodecContext.


//这个函数的工作模式是:
//1. 解码数据放到 audio_buf, 大小放 audio_buf_size。
//2. 调用一次 callback 只能写入 len 个字节,而每次取回的解码数据可能比 len 大，一次发不完。
//3. 发不完的时候，会 len 为 0，不继续循环，退出函数，继续调用 callback，进行下一次发送。
//4. 由于上次没发完，这次不取数据，发上次的剩余的，audio_buf_size 标记发送到哪里了。
//5. 注意，callback 每次一定要发且仅发 len 个数据，否则不会退出。
//如果没发够，缓冲区又没有了，就再取。发够了，就退出，留给下一个发，以此循环。
//三个变量设置为 static 就是为了保存上次数据，也可以用全局变量，但是感觉这样更好。

//13.回调函数中将从队列中取数据, 解码后填充到播放缓冲区.
void audio_callback(void *userdata, Uint8 *stream/*传出参数,往这里面写入*/, int len)
{
    memset(stream,0,len);

    VideoState * state = (VideoState *) userdata;
    if(state->pause)
    {
        return;
    }

    //len 表示一次发送多少。
    //audio_buf_size：一直为样本缓冲区的大小，wanted_spec.samples.（一般每次解码这么多，文件不同，这个值不同)
    int len1, audio_data_size;

    /* len 是由 SDL 传入的 SDL 缓冲区的大小，如果这个缓冲未满，我们就一直往里填充数据 */

    /* audio_buf_index 和 audio_buf_size表示缓冲区数据的末尾 标示我们自己用来放置解码出来的数据的缓冲区，*/
    /* 这些数据待 copy 到 SDL 缓冲区， 当 audio_buf_index >= audio_buf_size 的时候意味着我*/
    /* 们的缓冲为空，没有数据可供 copy，这时候需要调用 audio_decode_frame 来解码出更
/* 多的桢数据 */

    //函数一次执行要拷贝len大小的数据 通过while实现
    //一次while只向目的缓冲区stream拷贝len1大小的数据,
    //但是源(解码的数据)缓冲区是audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2],(这个就像一个窗口)
    //所以每次while都要更新len1 len 和这个源缓冲区向目的缓冲区的拷贝情况(通过audio_buf_index audio_buf_size来判断)
    //考完了就解码新的数据到audio_buf中接着向stream里面拷 没拷贝完就继续拷贝
    //直到len <= 0就结束while
    while (len > 0)
    {
        if (state->audio_buf_index >= state->audio_buf_size) //此时要去解码拿到一些新数据
        {
            audio_data_size = audio_decode_frame(state,state->audio_buf,sizeof(state->audio_buf));   //为什么不能把每次解码的数据拷贝到stream而是借助len1
            //因为每次拷贝多少是和wanted_spec的采样率和缓冲区大小有关
            /* audio_data_size < 0 标示没能解码出数据，我们默认播放静音 */
            if (audio_data_size < 0) {
                /* silence */
                state->audio_buf_size = 1024;
                /* 清零，静音 */
                memset(state->audio_buf, 0, state->audio_buf_size);  //赋空就做到了静音
            } else {
                state->audio_buf_size = audio_data_size;
            }
            state->audio_buf_index = 0;

            //解码成功audio_buf_size就是解码了多少数据
            //失败audio_buf_size就是1024
            //audio_buf_index都会置0  所以它永远不会超过audio_buf的大小
        }
        /* 查看 stream 可用空间，决定一次 copy 多少数据，剩下的下次继续 copy */
        len1 = state->audio_buf_size - state->audio_buf_index; //len1是一次while循环拷贝到播放缓冲区的数据
        if (len1 > len) {
            len1 = len;     //单次while拷贝不能超过总拷贝值
        }

        //单次while拷贝len1 先赋空再拷贝 (否则有杂音 为什么 因为他要做两个缓冲区声音的叠加混合)
        memset( stream , 0 , len1);
        //混音函数 sdl 2.0 版本使用该函数 替换 SDL_MixAudio
        SDL_MixAudioFormat(stream, (uint8_t *) state->audio_buf + state->audio_buf_index,
                           AUDIO_S16SYS,len1,10/*音量参数*/);

        len -= len1;
        stream += len1;
        state->audio_buf_index += len1;
    }
}


//对于音频来说，一个 packet 里面，可能含有多帧(frame)数据。
//读一帧 写一帧 返回
int audio_decode_frame(VideoState *state, uint8_t *audio_buf/*传出参数 解码的数据写入这里*/, int buf_size) {

    static AVPacket pkt;    //原始帧(压缩帧数据)包
    static uint8_t *audio_pkt_data = NULL; //原始帧(压缩帧数据)数据
    static int audio_pkt_size = 0;         //原始帧(压缩帧数据)大小
    int len1, data_size;
    int sampleSize = 0;

    AVCodecContext *aCodecCtx = state->pAudioCodecCtx; //解码器上下文
    AVFrame *audioFrame =state->audioFrame;             //目标数据帧(原始音频采样数据帧)
    PacketQueue *audioq = state->audioq;     //原始帧数据包队列
    AVFrame wanted_frame = state->out_frame;
    if( !aCodecCtx|| !audioFrame ||!audioq) return -1;


    struct SwrContext *swr_ctx = NULL;  //转码器上下文
    int convert_len;
    int n = 0;

    //处理audio_buf大小的原压缩数据,并写入audio_buf
    for(;;)
    {
        if( state->quit ) return -1;
        if(state->pause) return -1;
        if( !audioq ) return -1;
        if(packet_queue_get(audioq, &pkt, 0/*非阻塞*/) <= 0) //一定注意
        {
            if(state->readPacketsFinished && state->videoq->nb_packets == 0)
                state->quit = true;
            return -1;
        }

        //判断跳转
        if(strcmp((char*)pkt.data,FLUSH_DATA) == 0)
        {
            avcodec_flush_buffers(state->audio_st->codec);
            av_free_packet(&pkt);
            continue;
        }
        audioFrame = av_frame_alloc();   //一帧原始音频采样数据申请空间
        audio_pkt_data = pkt.data;
        audio_pkt_size = pkt.size;

        //处理一帧原压缩数据
        while(audio_pkt_size > 0)
        {
            if( state->quit ) return -1;

            int got_picture;
            memset(audioFrame, 0, sizeof(AVFrame));

            int ret =avcodec_decode_audio4( aCodecCtx, audioFrame, &got_picture/*结果标志位*/, &pkt); //解码一帧数据出来
            if( ret < 0 ) {
                printf("Error in decoding audio frame.\n");
                exit(0);
            }
            //一帧一个声道读取数据是 nb_samples , channels 为声道数 2 表示 16 位 2 个字节
            //data_size = audioFrame->nb_samples * wanted_frame.channels * 2/*16位的一个数据---采样点*/;
            switch( state->out_frame.format )
            {
            case AV_SAMPLE_FMT_U8:
                data_size = audioFrame->nb_samples * state->out_frame.channels * 1;
                break;
            case AV_SAMPLE_FMT_S16:
                data_size = audioFrame->nb_samples * state->out_frame.channels * 2;
                break;
            default:
                data_size = audioFrame->nb_samples * state->out_frame.channels * 2;
                break;
            }

            //计算音频时钟
            if( pkt.pts != AV_NOPTS_VALUE)
            {
                state->audio_clock = pkt.pts *av_q2d( state->audio_st->time_base )*1000000 ;
                //取音频时钟
            }else
            {
                state->audio_clock = (*(uint64_t *)
                                      audioFrame->opaque)*av_q2d( state->audio_st->time_base )*1000000 ;
            }

            //跳转处理
            if(state->seek_flag_audio)//发生跳转
            {
                if(state->audio_clock < state->seek_time) //当前帧位于目标帧前面 就一直continue
                {
                    break; //在目标跳转帧之前 就进行下一次回调
                }
                else //跳转成功
                    state->seek_flag_audio = 0; //更新跳转标志
            }

            //解码一帧成功
            if( got_picture )
            {
                //回收上一次的转码器
                if (swr_ctx != NULL)
                {
                    swr_free(&swr_ctx);
                    swr_ctx = NULL;
                }

                //解码出来的数据转换为想要的格式的数据
                //设置转码器
                swr_ctx = swr_alloc_set_opts(NULL, wanted_frame.channel_layout,
                                             (AVSampleFormat)wanted_frame.format,wanted_frame.sample_rate,
                                             audioFrame->channel_layout,(AVSampleFormat)audioFrame->format,
                                             audioFrame->sample_rate, 0, NULL);

                //初始化
                if (swr_ctx == NULL || swr_init(swr_ctx) < 0)
                {
                    printf("swr_init error\n");
                    break;
                }

                //解码出来的数据转换为想要的格式的数据
                //开启转码器->存入audio_buf
                convert_len = swr_convert(swr_ctx, &audio_buf,                                  //todo 画质提升 音质提升
                                          AVCODEC_MAX_AUDIO_FRAME_SIZE,
                                          (const uint8_t **)audioFrame->data,
                                          audioFrame->nb_samples);
                swr_free( &swr_ctx );

            }

            //更新这一帧处理的进度
            audio_pkt_size -= ret;

            if (audioFrame->nb_samples <= 0)                 //这一帧还没处理完 继续处理
            {
                continue;
            }

            //处理一帧结束 回收数据包
            av_free_packet(&pkt);
            return data_size ;                              //这里可完成倍速设置 todo
        }
        if(audioFrame)
            av_free(audioFrame);    //应该有
        av_free_packet(&pkt);                               //???????????????????????能走到这里吗
    }
}

int find_stream_index(AVFormatContext *pformat_ctx, int *video_stream, int
                      *audio_stream)
{
    assert(video_stream != NULL || audio_stream != NULL); //两个都为空 就报错
    /*assert 宏定义在 <assert.h> 或 <cassert> 头文件中，并接受一个参数，通常为一个语句。
    如果这个语句的结果为 false，assert 宏就会以"Assertion failed: , file , line "的形式显示出错信息，
    然后使程序崩溃并终止运行。如果该语句的结果为 true，则 assert 宏不做任何操作。*/

    int i = 0;
    int audio_index = -1;
    int video_index = -1;

    for (i = 0; i < pformat_ctx->nb_streams; i++)
    {
        if (pformat_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)  {
            video_index = i;
        }
        if (pformat_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audio_index = i;
        }
    }

    //注意以下两个判断有可能返回-1.
    if (video_stream == NULL)
    {
        *audio_stream = audio_index;
        return *audio_stream;
    }

    if (audio_stream == NULL)
    {
        *video_stream = video_index;
        return *video_stream;
    }

    *video_stream = video_index;
    *audio_stream = audio_index;

    return 0;
}


int thread_videopackets_work(void *arg)
{
    VideoState *state = (VideoState *) arg;
    AVPacket pkt1, *packet = &pkt1;

    int ret, got_picture, numBytes;

    double video_pts = 0; //当前视频的 pts
    double audio_pts = 0; //音频 pts

    //解码视频相关
    AVFrame *pFrame, *pFrameRGB;
    uint8_t *out_buffer_rgb; //解码后的 rgb 数据
    struct SwsContext *img_convert_ctx; //用于解码后的视频格式转换

    AVCodecContext *pCodecCtx = state->pCodecCtx; //视频解码器

    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    //这里我们改成了 将解码后的 YUV 数据转换成 RGB32
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                     pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                     AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
    numBytes = avpicture_get_size(AV_PIX_FMT_RGB32,
                                  pCodecCtx->width,pCodecCtx->height);

    out_buffer_rgb = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, out_buffer_rgb, AV_PIX_FMT_RGB32,
                   pCodecCtx->width, pCodecCtx->height);

    while(1)
    {
        if(state->quit)   break;
        if(state->pause)
        {
            SDL_Delay(30); //wait 5ms
            continue;
        }
        if (packet_queue_get(state->videoq, packet, 0/*队列空不阻塞*/) <= 0) //队列里面没有数据了读取完毕了
        {
            if(state->readPacketsFinished && state->audioq->nb_packets == 0) //文件读完了 音频也播放完了
            {
                break;          //结束这个视频解码线程
            }
            else
            {
                SDL_Delay(3);
                continue;
            }
        }
        //根据数据包判断是不是要跳转 (即是不是要清空解码器)
        if(strcmp((char*)packet->data,FLUSH_DATA) == 0)
        {
            avcodec_flush_buffers(state->video_st->codec);  //为什么要清空这个 这个只会赋值一次
            av_free_packet(packet);
            state->video_clock = 0; //防止向之前跳转一直死循环同步(video_clock 恒> audio_clock)跳不出来   //直接置0????????????????
            continue;
        }


        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,packet);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Error decoding video frame\n");
            break;
        }
        //音视频同步 视频帧别太快 等一等音频
        //获取显示时间 pts
        video_pts = pFrame->pts = pFrame->best_effort_timestamp;
        video_pts *= 1000000 *av_q2d(state->video_st->time_base);
        video_pts = synchronize_video(state, pFrame, video_pts);//视频时钟补偿
        //跳转处理
        if(state->seek_flag_video)//发生跳转
        {
            if(video_pts < state->seek_time) //当前帧位于目标帧前面 就一直continue
            {
                av_free_packet(packet);
                continue;
            }
            else
                state->seek_flag_video = 0; //更新跳转标志
        }

        while(state->audioStream >= 0)
        {
            if(state->quit)
            {
                break;
            }
            if(state->audioq->nb_packets == 0)
            {
                break;
            }
            audio_pts = state->audio_clock;   //做回调就得更新
            video_pts = state->video_clock;
            if (video_pts <= audio_pts) break;
            SDL_Delay(5);
        }
        if (got_picture) {
            sws_scale(img_convert_ctx,
                      (uint8_t const * const *) pFrame->data,
                      pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                      pFrameRGB->linesize);

            //把这个 RGB 数据 用 QImage 加载
            QImage tmpImg((uchar
                           *)out_buffer_rgb,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
            QImage img = tmpImg.copy(); //把堆区out_buffer_rgb的img资源深拷贝转移到Qt内存
            state->m_player->transOneFrame(img); //调用激发信号的函数
        }
        av_free_packet(packet); //新版考虑使用 av_packet_unref() 函数来代替
    }
    av_free(pFrame);
    av_free(pFrameRGB);
    av_free(out_buffer_rgb);
    if(img_convert_ctx)
        av_free(img_convert_ctx);
    state->quit = true;
    state->threadVideoCodecImgFinished = true;

    qDebug()<<"threadPlayer:: video_frame_thread over!!!";
    return 0;
}

//时间补偿函数--视频延时
double synchronize_video(VideoState *is, AVFrame *src_frame, double pts) {

    double frame_delay; // 缓存帧和帧之间的延迟

    if (pts != 0) {
        /* if we have pts, set video clock to it */
        // 如果当前帧有 PTS 时间戳，那么使用它来更新视频时钟
        is->video_clock = pts;
    } else {
        /* if we aren't given a pts, set it to the clock */
        // 如果没有 PTS 时间戳，则采用视频时钟作为当前时间
        pts = is->video_clock;
    }
    /* update the video clock */
    // 计算当前帧和上一帧之间的时钟差
    frame_delay = av_q2d(is->video_st->codec->time_base);
    /* if we are repeating a frame, adjust clock accordingly */
    // 如果当前帧是重复帧，需要根据重复数调整帧之间的时间差
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
    // 更新视频时钟
    is->video_clock += frame_delay;
    // 返回当前帧的 PTS 时间戳
    return pts;
}

Uint32 onlyVideoStreamTimer_callback(Uint32 interval, void *param)
{
    VideoState *is = (VideoState *)param;
    AVPacket pkt1, *packet = &pkt1;
    int ret, got_picture, numBytes;
    double video_pts = 0; //当前视频的 pts
    double audio_pts = 0; //音频 pts
    //解码视频相关
    AVFrame *pFrame, *pFrameRGB;
    uint8_t *out_buffer_rgb; //解码后的 rgb 数据
    struct SwsContext *img_convert_ctx; //用于解码后的视频格式转换
    AVCodecContext *pCodecCtx = is->pCodecCtx; //视频解码器
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    //这里我们改成了 将解码后的 YUV 数据转换成 RGB32
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                     pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                     AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
    numBytes = avpicture_get_size(AV_PIX_FMT_RGB32,
                                  pCodecCtx->width,pCodecCtx->height);
    out_buffer_rgb = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, out_buffer_rgb, AV_PIX_FMT_RGB32,
                   pCodecCtx->width, pCodecCtx->height);
    do
    {
        if(is->quit)
            break;
        if (packet_queue_get(is->videoq, packet, 0) <= 0) //非阻塞
        {
            if(is->readPacketsFinished && is->audioq->nb_packets == 0)
            {
                break;
            }
            else
            {
                SDL_Delay(1);
                continue;
            }
        }

        //读取完毕了

        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,packet);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Error decoding video frame\n");
            break;
        }
        //获取显示时间 pts
        video_pts = pFrame->pts = pFrame->best_effort_timestamp;
        video_pts *= 1000000 *av_q2d(is->video_st->time_base);
        video_pts = synchronize_video(is, pFrame, video_pts);//视频时钟补偿

        if (got_picture) {
            sws_scale(img_convert_ctx,
                      (uint8_t const * const *) pFrame->data,
                      pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                      pFrameRGB->linesize);
            //把这个 RGB 数据 用 QImage 加载
            QImage tmpImg((uchar
                           *)out_buffer_rgb,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
            QImage image = tmpImg.copy(); //把图像复制一份 传递给界面显示

            is->m_player->transOneFrame(image); //调用激发信号的函数

        }
        av_free_packet(packet); //新版考虑使用 av_packet_unref() 函数来代替
    }while(0);

    av_free(pFrame);
    av_free(pFrameRGB);
    av_free(out_buffer_rgb);
    is->threadVideoCodecImgFinished = true;
    return interval;
}





























