QT     += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TEMPLATE = app
CONFIG += console c++11
#CONFIG -= app_bundle
#CONFIG -= qt

INCLUDEPATH += ./include
INCLUDEPATH += ./tools

SOURCES += \
        src/Mysql.cpp \
        src/TCPKernel.cpp \
        src/Thread_pool.cpp \
        src/block_epoll_net.cpp \
        src/clogic.cpp \
        src/err_str.cpp \
        src/main.cpp \
        tools/tools.cpp

DISTFILES += \
    src/makefile

#(include) += ./netapi/netapi.pri

LIBS += -lpthread -lmysqlclient

HEADERS += \
    include/Mysql.h \
    include/TCPKernel.h \
    include/Thread_pool.h \
    include/block_epoll_net.h \
    include/clogic.h \
    include/err_str.h \
    include/packdef.h \
    tools/tools.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target






