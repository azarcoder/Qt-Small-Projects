QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# --- QWT CONFIGURATION ---

# Your Qwt root folder
QWT_ROOT = C:/qwt-6.1.4

# 1. Point to 'src' because that is where your .h files are
INCLUDEPATH += $$QWT_ROOT/src

# 2. Point to 'lib' for the library files
# (The lib folder is usually next to the src folder)
LIBS += -L$$QWT_ROOT/lib -lqwt

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
