QT       -= core gui
CONFIG += c++11

SOURCES += \  
    src/mutex_rw.cpp \
    src/mutex.cpp \
    src/lock_mutex_rw.cpp \
    src/lock_mutex_rd.cpp
HEADERS += \ 
    src/mutex_rw.h \
    src/mutex.h \
    src/lock_mutex_rw.h \
    src/lock_mutex_rd.h \
    src/lock_mutex_shared.h
isEmpty(PREFIX) {
    PREFIX = /usr/local
}

# includes dir
QMAKE_INCDIR += $$PREFIX/include
QMAKE_INCDIR += src

INCLUDEPATH += $$PREFIX/include
INCLUDEPATH += src

# C++ standard.
include(../../cflags.pri)

TARGET = cx_thr_mutex
TEMPLATE = lib
# INSTALLATION:
target.path = $$PREFIX/lib
header_files.files = $$HEADERS
header_files.path = $$PREFIX/include/$$TARGET
INSTALLS += target
INSTALLS += header_files
# PKGCONFIG
CONFIG += create_pc create_prl no_install_prl
QMAKE_PKGCONFIG_LIBDIR = $$PREFIX/lib/
QMAKE_PKGCONFIG_INCDIR = $$PREFIX/include/$$TARGET
QMAKE_PKGCONFIG_CFLAGS = -I$$PREFIX/include/
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

include(../../version.pri)
