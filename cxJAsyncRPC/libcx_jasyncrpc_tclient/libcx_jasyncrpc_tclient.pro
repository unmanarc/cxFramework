QT       -= core gui

SOURCES += \ 
    src/jasyncrpc_client.cpp \
    src/jasyncrpc_fhandler.cpp \
    src/retinfo.cpp
#\
#    src/jasyncrpc_client_thread.cpp
HEADERS += \ 
    src/jasyncrpc_client.h \
    src/jasyncrpc_fhandler.h \
    src/retinfo.h
#\
#    src/jasyncrpc_client_thread.h
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

TARGET = cx_jasyncrpc_tclient
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
