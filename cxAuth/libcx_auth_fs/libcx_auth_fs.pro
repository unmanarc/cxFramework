QT       -= core gui

SOURCES += \ 
    src/iauth_fs.cpp \
    src/iauth_fs_accounts.cpp \
    src/iauth_fs_attributes.cpp \
    src/iauth_fs_groups.cpp \
    src/iauth_fs_helper_jsonfileaccess.cpp \
    src/iauth_fs_domains.cpp
HEADERS += \ 
    src/iauth_fs.h \
    src/iauth_fs_helper_jsonfileaccess.h

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

TARGET = cx_auth_fs
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
