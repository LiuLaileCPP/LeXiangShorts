#include "maindlg.h"
#include "ui_maindlg.h"
#include <QCoreApplication>
#include <QPixmap>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
enum PlayerState;

MainDlg::MainDlg(QWidget *parent):
    QDialog(parent),
    ui(new Ui::MainDlg)
{
    ui->setupUi(this);
    m_threadPlayer = new threadPlayer;
    //connect(&m_timer,SIGNAL(timeout()),this,SLOT());
    m_threadPlayer->setOutPutWidthHeight(ui->wdg_showImg->width(),ui->wdg_showImg->height()); //设置图片输出的大小

    connect(&m_timer,SIGNAL(timeout()),this,SLOT(slot_timeOut()));
    m_timer.setInterval(50);
    connect(m_threadPlayer,SIGNAL(SIG_transOneFrame(QImage)),this,SLOT(slot_setOneFrame(QImage)));
    connect(m_threadPlayer,SIGNAL(SIG_sendVieoWH(int,int)),this,SLOT(slot_getVieoWH(int,int)));
    connect(m_threadPlayer,&threadPlayer::SIG_totalTime,this,[this](qint64 uSec)
    {
        qint64 time = uSec / 1000000;
        int h = time/3600;
        int m = (time - h*3600)/60;
        int s = (time - h*3600)%60;
        ui->slider_progress->setRange(0,uSec/1000);
        char tmp[100] = "";
        sprintf(tmp,"%02d:%02d:%02d",h,m,s);
        ui->lb_alltime->setText(QString(tmp));
    });
//    connect(m_threadPlayer,&threadPlayer::SIG_currentTime,this,[this](qint64 uSec)
//    {
//        //更新lb_cur 和 进度条
//    });
    //用定时器做的  更新lb_cur 和 进度条

    connect(this,&QObject::destroyed,[this]()
    {
        //关闭主窗口杀死threadPlayer线程
        m_threadPlayer->terminate();
        delete m_threadPlayer;
    });

    //安装事件过滤器(this去管理部件的事件)
    ui->slider_progress->installEventFilter(this);
    slot_UIStateChanged(Stop);

    // 设置窗体最大化和最小化
    Qt::WindowFlags windowFlag  = Qt::Dialog;
    windowFlag                  |= Qt::WindowMinimizeButtonHint;
    windowFlag                  |= Qt::WindowMaximizeButtonHint;
    windowFlag                  |= Qt::WindowCloseButtonHint;

    setWindowFlags(windowFlag);
}

MainDlg::~MainDlg()
{
    m_threadPlayer->stop(true);
    m_threadPlayer->quit();
    m_threadPlayer->wait();
    delete ui;
}

void MainDlg::slot_timeOut()
{
    if (QObject::sender() == &m_timer)
    {
        qint64 msec = m_threadPlayer->getCurrentTime()/1000;
        qint64 time = msec/1000;

        ui->slider_progress->setValue(msec);

        int h = time/3600;
        int m = (time - h*3600)/60;
        int s = (time - h*3600)%60;
        char tmp[100] = "";
        sprintf(tmp,"%02d:%02d:%02d",h,m,s);
        ui->lb_curtime->setText(QString(tmp));

        if(ui->slider_progress->value() == ui->slider_progress->maximum()
                && m_threadPlayer->m_playerState == PlayerState::Stop)
        {
            slot_UIStateChanged( PlayerState::Stop );
        }
        else if(ui->slider_progress->value() + 1000 ==
                 ui->slider_progress->maximum()
                 && m_threadPlayer->m_playerState == PlayerState::Stop)
        {
            slot_UIStateChanged( PlayerState::Stop );
        }
    }
}

void MainDlg::slot_setOneFrame(QImage img)
{

    ui->wdg_showImg->slot_setImage(img);

    //QPixmap pmap;
//    if(img.isNull())
//    {
//        pmap = QPixmap::fromImage(img);
//    }
//    else
//        pmap = QPixmap::fromImage(img.scaled(ui->lb_show->size()/*缩放*/,Qt::KeepAspectRatio/*保持原比例*/));
//    ui->lb_show->setPixmap(pmap);
}

void MainDlg::slot_getVieoWH(int imgWidth, int imgHeight)
{
    ui->lb_mw->setText(QString("lb_show w: %1").arg(ui->wdg_showImg->width()));
    ui->lb_mh->setText(QString("lb_show h: %1").arg(ui->wdg_showImg->height()));
    ui->lb_imgw->setText(QString("img_width: %1").arg(imgWidth));
    ui->lb_imgh->setText(QString("img_height: %1").arg(imgHeight));
}

void MainDlg::on_pb_play_clicked()
{
    if(m_threadPlayer->play())
        slot_UIStateChanged(Playing);
}
void MainDlg::on_pb_pause_clicked()
{
    if(m_threadPlayer->pause())
        slot_UIStateChanged(Pause);
}

void MainDlg::on_pb_stop_clicked()
{
    if(m_threadPlayer->stop(true))
        slot_UIStateChanged(Stop);
}


void MainDlg::on_pb_open_clicked()
{

    //打开浏览文件
    QString path = QFileDialog::getOpenFileName(this,"打开媒体文件","D:/Learn/Qt/ProjectClass/build-VideoDemo-Desktop_Qt_5_12_11_MinGW_32_bit-Debug/temp/","视频文件 (*.flv *.rmvb *.avi *.mp4 *.mp3 *.mkv);; 所有文件(*.*);;");
    if(path.isEmpty())
        return;
    if(m_threadPlayer->stop(true))
        slot_UIStateChanged(Stop);
    if(m_threadPlayer->setFileName(path))
    {

        slot_UIStateChanged(Playing);
    }
}



//播放状态切换槽函数
void MainDlg::slot_UIStateChanged(int state)
{
    switch( state )
    {
    case PlayerState::Stop:
        qDebug()<< "MainDlg:: player Stop";
        m_timer.stop();
        ui->slider_progress->setValue(0);
        ui->lb_alltime->setText("00:00:00");
        ui->lb_curtime->setText("00:00:00");
        ui->lb_videoName->setText("");
        ui->pb_pause->hide();
        ui->pb_play->show();
    {
        QImage img;
        img.fill( Qt::black);
        img.save("./blackGround.png");
        slot_setOneFrame( img );
    }
        this->update();
        break;
    case PlayerState::Playing:
        qDebug()<< "MainDlg:: palyer Playing";
        ui->pb_play->hide();
        ui->pb_pause->show();
        m_timer.start();
        this->update();
        break;
    case PlayerState::Pause:
        qDebug()<< "MainDlg:: palyer Pause";
        ui->pb_play->show();
        ui->pb_pause->hide();
        m_timer.stop();
        this->update();
        break;
    default:
        break;
    }
}

#include <QStyle>
#include <QMouseEvent>
//事件过滤器
bool MainDlg::eventFilter(QObject* obj,QEvent* event)
{
    if(obj == ui->slider_progress)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {

            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);//转换事件类型
            int min = ui->slider_progress -> minimum();
            int max = ui->slider_progress -> maximum();
            int value = QStyle::sliderValueFromPosition(
                        min, max, mouseEvent->pos().x(), ui->slider_progress->width());

            m_timer.stop();
            ui->slider_progress->setValue(value);        //进度条设置位置
            m_threadPlayer->seek((qint64)value*1000); //视频跳转 value 秒
            m_timer.start();
            return true;
        }
        else
            return false;
    }
    else
    {
        return QDialog::eventFilter(obj,event);
    }
}


//打开URL并播放
void MainDlg::on_pb_url_clicked()
{
    if(m_threadPlayer->getState() != PlayerState::Stop)
    {
        m_threadPlayer->stop(true);
    }
    bool bOK;
    QString path = QInputDialog::getText(this,
                                          "打开link",
                                          "请输入播放链接URL：",
                                          QLineEdit::Normal,
                                          "http://192.168.",
                                          &bOK
                                          );
    QString sName = path.remove(" ");
    if (bOK && !sName.isEmpty())
    {
        //设置player fileName
        m_threadPlayer->setFileName(path);
        //切换状态
        slot_UIStateChanged(PlayerState::Playing);
        //播放
    }
    else if(bOK && sName.isEmpty())
    {
        QMessageBox::about(this,"打开失败","未输入或者格式不对");
    }
}




void MainDlg::on_pb_again_clicked()
{
    m_threadPlayer->stop(true);
    m_threadPlayer->start();
}

