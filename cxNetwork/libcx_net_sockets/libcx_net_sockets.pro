QT       -= core gui

SOURCES += \
    src/socket_tcp.cpp \
    src/socket_udp.cpp \
    src/socket.cpp \
    src/streams/stream_reader.cpp \
    src/streams/stream_writer.cpp \
    src/socket_base_datagram.cpp \
    src/socket_base_stream.cpp \
    src/bridge/bridge_thread.cpp \
    src/bridge/socket_stream_bridge.cpp

HEADERS += \
    src/socket_tcp.h \
    src/socket_udp.h \
    src/socket.h \
    src/streams/stream_reader.h \
    src/streams/stream_writer.h \
    src/socket_base_datagram.h \
    src/socket_base_stream.h \
    src/bridge/bridge_thread.h \
    src/bridge/socket_stream_bridge.h

isEmpty(PREFIX) {
    PREFIX = /usr/local
}

!win32 {
    SOURCES += src/socket_unix.cpp
    HEADERS += src/socket_unix.h
}
win32 {
    SOURCES += src/win32/w32compat.cpp
    HEADERS += src/win32/w32compat.h
}

# includes dir
QMAKE_INCDIR += $$PREFIX/include
QMAKE_INCDIR += src
INCLUDEPATH += $$PREFIX/include
INCLUDEPATH += src

# C++ standard.
include(../../cflags.pri)

INCLUDEPATH += src/streams
QMAKE_INCDIR += src/streams

INCLUDEPATH += src/win32
QMAKE_INCDIR += src/win32

TARGET = cx_net_sockets
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
