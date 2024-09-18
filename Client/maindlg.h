#ifndef MAINDLG_H
#define MAINDLG_H

#include <QDialog>
#include "threadplayer.h"
#include <QTimer>

namespace Ui {
class MainDlg;
}

class MainDlg : public QDialog
{
    Q_OBJECT

public:
    explicit MainDlg(QWidget *parent = nullptr);
    ~MainDlg();

    void slot_UIStateChanged(int state);

    bool eventFilter(QObject* obj,QEvent* event);
private:
    Ui::MainDlg *ui;
    threadPlayer* m_threadPlayer;
    QTimer m_timer;

    //信号槽------------------------------------------------------------------
signals:


    //------------------------------------------------------------------------
public slots:
     void slot_setOneFrame(QImage);
     void slot_getVieoWH(int imgWidth,int imgHeight);

     //-------------------------------------------------------------------------
     void slot_timeOut();
private slots:

     void on_pb_play_clicked();
     void on_pb_open_clicked();
     void on_pb_pause_clicked();
     void on_pb_stop_clicked();
     void on_pb_url_clicked();
     void on_pb_again_clicked();
};

#endif // MAINDLG_H
