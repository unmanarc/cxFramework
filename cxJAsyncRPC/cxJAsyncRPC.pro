TEMPLATE = subdirs

# JSON Async RPC stream (TCP/Unix Sockets) client
SUBDIRS+=libcx_jasyncrpc_common
# Project folders:
libcx_jasyncrpc_common.subdir    = libcx_jasyncrpc_common

# JSON Async RPC stream (TCP/Unix Sockets) server
SUBDIRS+=libcx_jasyncrpc_tserver
# Project folders:
libcx_jasyncrpc_tserver.subdir    = libcx_jasyncrpc_tserver
libcx_jasyncrpc_tserver.depends   = libcx_jasyncrpc_common

# JSON Async RPC stream (TCP/Unix Sockets) client
SUBDIRS+=libcx_jasyncrpc_tclient
# Project folders:
libcx_jasyncrpc_tclient.subdir    = libcx_jasyncrpc_tclient
libcx_jasyncrpc_tclient.depends   = libcx_jasyncrpc_common


# JSON Async RPC stream (TCP/Unix Sockets) functions for authentication
SUBDIRS+=libcx_jasyncrpc_functions_auth
# Project folders:
libcx_jasyncrpc_functions_auth.subdir    = libcx_jasyncrpc_functions_auth
libcx_jasyncrpc_functions_auth.depends   = libcx_jasyncrpc_tserver

# TODO:
# - qt client lib
# - user/auth manipulation functions
# - common user management gui
# - common local user management gui (2fa, pass, etc)

