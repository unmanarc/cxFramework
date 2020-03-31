TEMPLATE = subdirs

# Sockets LIB.
SUBDIRS = libcx_net_sockets
# Project folders:
libcx_net_sockets.subdir      = libcx_net_sockets

# Multiplexer LIB.
SUBDIRS+=libcx_net_multiplexer
# Project folders:
libcx_net_multiplexer.subdir    = libcx_net_multiplexer
libcx_net_multiplexer.depends   = libcx_net_sockets


# TCP Server LIB. 2
SUBDIRS+=libcx_net_threadedacceptor
# Project folders:
libcx_net_threadedacceptor.subdir    = libcx_net_threadedacceptor
libcx_net_threadedacceptor.depends   = libcx_net_sockets

SUBDIRS+=libcx_net_poolthreadedacceptor
# Project folders:
libcx_net_poolthreadedacceptor.subdir    = libcx_net_poolthreadedacceptor
libcx_net_poolthreadedacceptor.depends   = libcx_net_sockets
libcx_net_poolthreadedacceptor.depends   = libcx_net_tls

# Network Interface configuration
SUBDIRS+=libcx_net_ifcfg
# Project folders:
libcx_net_ifcfg.subdir    = libcx_net_ifcfg

# Virtual Interfaces (TUN/TAP)
SUBDIRS+=libcx_net_virtualif
# Project folders:
libcx_net_virtualif.subdir    = libcx_net_virtualif
libcx_net_virtualif.depends   = libcx_net_ifcfg

# TLS Server LIB.
SUBDIRS+=libcx_net_tls
# Project folders:
libcx_net_tls.subdir    = libcx_net_tls
libcx_net_tls.depends   = libcx_net_sockets

# Chain Sockets LIB.
SUBDIRS+=libcx_net_chains
# Project folders:
libcx_net_chains.subdir    = libcx_net_chains
libcx_net_chains.depends   = libcx_net_sockets libcx_net_tls

# Chain TLS Sockets LIB.
SUBDIRS+=libcx_net_chains_tls
# Project folders:
libcx_net_chains_tls.subdir    = libcx_net_chains_tls
libcx_net_chains_tls.depends   = libcx_net_sockets libcx_net_tls libcx_net_chains

#END-
