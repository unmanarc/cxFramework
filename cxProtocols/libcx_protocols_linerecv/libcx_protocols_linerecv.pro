QT       -= core gui


SOURCES += src/linerecv.cpp \
    src/linerecv_subparser.cpp
HEADERS += src/linerecv.h \
    src/linerecv_subparser.h

isEmpty(PREFIX) {
    PREFIX = /usr/local
}

# includes dir
QMAKE_INCDIR += $$PREFIX/include
INCLUDEPATH += $$PREFIX/include

QMAKE_INCDIR += src
INCLUDEPATH += src

QMAKE_INCDIR += src/helpers
INCLUDEPATH += src/helpers

QMAKE_INCDIR += src/subparsers
INCLUDEPATH += src/subparsers

QMAKE_INCDIR += src/containers
INCLUDEPATH += src/containers

QMAKE_INCDIR += src/cookies
INCLUDEPATH += src/cookies

QMAKE_INCDIR += src/defs
INCLUDEPATH += src/defs

# C++ standard.
include(../../cflags.pri)

TARGET = cx_protocols_linerecv
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

