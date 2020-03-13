QT       -= core gui

SOURCES += \ 
    src/helpers/base64std.cpp \
    src/helpers/http_hlp_chunked_retriever.cpp \
    src/containers/http_version.cpp \
    src/subparsers/http_content.cpp \
    src/subparsers/http_request.cpp \
    src/subparsers/http_status.cpp \
    src/httpv1_base.cpp \
    src/httpv1_server.cpp \
    src/httpv1_client.cpp \
#    src/subparsers/http_headers.cpp \
    src/helpers/http_date.cpp \
    src/cookies/http_cookies_clientside.cpp \
    src/cookies/http_cookies_serverside.cpp \
    src/cookies/http_cookie_value.cpp

HEADERS += \
    src/helpers/base64std.h \
    src/helpers/http_hlp_chunked_retriever.h \
    src/defs/http_retcodes.h \
    src/containers/http_version.h \
    src/subparsers/http_content.h \
    src/subparsers/http_request.h \
#    src/subparsers/http_headers.h \
    src/subparsers/http_status.h \
    src/httpv1_base.h \
    src/httpv1_server.h \
    src/httpv1_client.h \
    src/helpers/http_date.h \
    src/cookies/http_cookies_clientside.h \
    src/cookies/http_cookies_serverside.h \
    src/cookies/http_cookie_value.h \
    src/helpers/fullrequest.h \
    src/helpers/fullresponse.h

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

TARGET = cx_protocols_http
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
