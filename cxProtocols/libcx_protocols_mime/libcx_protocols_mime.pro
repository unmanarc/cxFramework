QT       -= core gui

SOURCES += \ 
    src/subparsers/mime_sub_header.cpp \
    src/subparsers/mime_sub_content.cpp \
    src/mime_partmessage.cpp \
    src/subparsers/mime_sub_firstboundary.cpp \
    src/subparsers/mime_sub_endpboundary.cpp \
    src/multipart_vars.cpp
HEADERS += \ 
    src/subparsers/mime_sub_header.h \
    src/subparsers/mime_sub_content.h \
    src/mime_partmessage.h \
    src/subparsers/mime_sub_firstboundary.h \
    src/subparsers/mime_sub_endpboundary.h \
    src/multipart_vars.h

isEmpty(PREFIX) {
    PREFIX = /usr/local
}

# includes dir
QMAKE_INCDIR += $$PREFIX/include
INCLUDEPATH += $$PREFIX/include

QMAKE_INCDIR += src
INCLUDEPATH += src

QMAKE_INCDIR += src/subparsers
INCLUDEPATH += src/subparsers
# C++ standard.
include(../../cflags.pri)

TARGET = cx_protocols_mime
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
