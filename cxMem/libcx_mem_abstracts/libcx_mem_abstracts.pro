QT       -= core gui
CONFIG   += c++11

SOURCES += \
    src/abstractvar.cpp \
    src/absvar_int16.cpp \
    src/absvar_int8.cpp \
    src/absvar_bool.cpp \
    src/absvar_int32.cpp \
    src/absvar_uint8.cpp \
    src/absvar_uint16.cpp \
    src/absvar_uint32.cpp \
    src/absvar_uint64.cpp \
    src/absvar_int64.cpp \
    src/absvar_double.cpp \
    src/absvar_bin.cpp \
    src/absvar_string.cpp \
    src/absvar_stringlist.cpp \
    src/absvar_ipv4.cpp \
    src/absvar_ipv6.cpp
HEADERS += \
    src/abstractvar.h \
    src/absvar_bool.h \
    src/absvar_int8.h \
    src/absvar_int16.h \
    src/absvar_int32.h \
    src/absvar_uint8.h \
    src/absvar_uint16.h \
    src/absvar_uint32.h \
    src/absvar_uint64.h \
    src/absvar_int64.h \
    src/absvar_double.h \
    src/absvar_bin.h \
    src/absvar_string.h \
    src/absvar_stringlist.h \
    src/absvar_ipv4.h \
    src/absvar_ipv6.h

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

TARGET = cx_mem_abstracts
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
