QT       -= core gui

SOURCES += \  
    src/urlvarcontent_subparser.cpp \
    src/url_vars.cpp
HEADERS += \  
    src/urlvarcontent_subparser.h \
    src/url_vars.h

isEmpty(PREFIX) {
    PREFIX = /usr/local
}

# includes dir
QMAKE_INCDIR += $$PREFIX/include
INCLUDEPATH += $$PREFIX/include

QMAKE_INCDIR += src
INCLUDEPATH += src
# C++ standard.
include(../../cflags.pri)

TARGET = cx_protocols_urlvars
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
