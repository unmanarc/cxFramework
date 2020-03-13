TEMPLATE = subdirs

# Base lib for authenticate/validate account-password
SUBDIRS += libcx_auth
# Project folders:
libcx_auth.subdir = libcx_auth

# DB based authenticator (sqlite3 based)
SUBDIRS += libcx_auth_sqlite3
# Project folders:
libcx_auth_sqlite3.subdir    = libcx_auth_sqlite3
libcx_auth_sqlite3.depends   = libcx_auth

# Internal based authenticator (filesystem based)
SUBDIRS += libcx_auth_fs
# Project folders:
libcx_auth_fs.subdir    = libcx_auth_fs
libcx_auth_fs.depends   = libcx_auth

#END-
