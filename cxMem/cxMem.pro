TEMPLATE = subdirs

# Streams
SUBDIRS += libcx_mem_streams
# Project folders:
libcx_mem_streams.subdir      = libcx_mem_streams

# Stream Encoders:
SUBDIRS += libcx_mem_streamencoders
# Project folders:
libcx_mem_streamencoders.subdir    = libcx_mem_streamencoders
libcx_mem_streamencoders.depends   = libcx_mem_streams

# Containers:
SUBDIRS += libcx_mem_containers
# Project folders:
libcx_mem_containers.subdir    = libcx_mem_containers
libcx_mem_containers.depends   = libcx_mem_streams

# Vars:
SUBDIRS += libcx_mem_vars
# Project folders:
libcx_mem_vars.subdir    = libcx_mem_vars
libcx_mem_vars.depends   = libcx_mem_containers

# Parser:
SUBDIRS += libcx_mem_streamparser
# Project folders:
libcx_mem_streamparser.subdir    = libcx_mem_streamparser
libcx_mem_streamparser.depends   = libcx_mem_streams libcx_mem_containers

SUBDIRS += libcx_mem_abstracts
# Project folders:
libcx_mem_abstracts.subdir      = libcx_mem_abstracts

#END-
