TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += cxThreads
SUBDIRS += cxMem
!win32:SUBDIRS += cxServices
!win32:SUBDIRS += cxLogs
SUBDIRS += cxNetwork
SUBDIRS += cxProtocols
SUBDIRS += cxAuth
SUBDIRS += cxJAsyncRPC
SUBDIRS += cxScripts


