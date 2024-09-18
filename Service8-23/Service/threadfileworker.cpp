//#include "threadfileworker.h"

//threadFileWorker::threadFileWorker(QObject *parent) : QObject(parent)
//{

//}

//bool threadFileWorker::slot_writeFile(QString path,qint64 pos,char* content,qint64 len)
//{
//    if(path == "" || pos < 0 || !content || len <= 0)
//        return false;
//    FILE* p = fopen(path.toStdString().c_str(),"w");
//    if(!p)
//        return false;
//    int res = fwrite(content,1,len,p);
//    if(res < len)
//        return false;
//    return true;
//}
