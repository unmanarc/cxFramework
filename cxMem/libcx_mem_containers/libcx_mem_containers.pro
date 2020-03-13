QT       -= core gui
CONFIG += c++11

SOURCES += \
    src/binarycontainer_base.cpp \
    src/binarycontainer_mmap.cpp \
    src/binarycontainer_chunks.cpp \
    src/binarycontainer_mem.cpp \
    src/binarycontainer_ref.cpp \
    src/common.cpp \ #\
    src/nullcontainer.cpp
#    src/binarycontainer_file.cpp
HEADERS +=\
    src/common.h \
    src/binarycontainer_chunk.h \
    src/binarycontainer_base.h \
    src/binarycontainer_mmap.h \
    src/binarycontainer_chunks.h \
    src/binarycontainer_mem.h \
    src/binarycontainer_ref.h \
    src/binarycontainer_filereference.h \ #\
    src/nullcontainer.h
#    src/binarycontainer_file.h
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

# LIB DEFS:
TARGET = cx_mem_containers
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
