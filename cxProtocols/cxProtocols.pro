TEMPLATE = subdirs

SUBDIRS+=libcx_protocols_urlvars
# Project folders:
libcx_protocols_urlvars.subdir    = libcx_protocols_urlvars
#libcx_protocols_urlvars.depends   =

SUBDIRS+=libcx_protocols_mime
# Project folders:
libcx_protocols_mime.subdir    = libcx_protocols_mime
#libcx_protocols_mime.depends   =

SUBDIRS+=libcx_protocols_http
# Project folders:
libcx_protocols_http.subdir    = libcx_protocols_http
libcx_protocols_http.depends   = libcx_protocols_mime libcx_protocols_urlvars

SUBDIRS+=libcx_protocols_linerecv
# Project folders:
libcx_protocols_linerecv.subdir    = libcx_protocols_linerecv
libcx_protocols_linerecv.depends   = 


#END-
