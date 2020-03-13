QT       -= core gui

SOURCES += \ 
    src/socket_multiplexer.cpp \
    src/socket_multiplexer_plugin.cpp \
    src/socket_multiplexer_lines.cpp \
    src/socket_multiplexer_callbacks.cpp \
    src/socket_multiplexer_a_connect.cpp \
    src/socket_multiplexer_a_data.cpp \
    src/socket_multiplexer_a_plugins.cpp \
    src/socket_multiplexer_a_server.cpp \
    src/socket_multiplexed_line.cpp
HEADERS += \ 
    src/vars.h \
    src/socket_multiplexer_plugin.h \
    src/socket_multiplexer.h \
    src/socket_multiplexer_callbacks.h \
    src/socket_multiplexer_lines.h \
    src/socket_multiplexer_a_struct_threadparams.h \
    src/socket_multiplexer_a_enum_mplx_msgs.h \
    src/socket_multiplexer_a_enum_lineaccept_msgs.h \
    src/socket_multiplexed_line.h \
    src/socket_multiplexer_a_struct_databuffer.h \
    src/socket_multiplexer_a_struct_lineid.h

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

TARGET = cx_net_multiplexer
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
