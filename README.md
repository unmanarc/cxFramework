# cxFramework

C++11 Based Libraries  
  
Author: Aaron Mizrachi (unmanarc) <aaron@unmanarc.com>   
License: LGPL v3   

## Functionality

This framework provides C++11 based enhancing libraries  

NOTICE: only tested in Linux.

Useful for:

* Network Programming (UDP/TCP/TLS/HTTP/MIME)
* Chained Network Programming
* RPC
* Authentication
* Fast memory transactions
* Thread's
* Scripting

## Libraries


## Pre-requisites

* pthread
* openssl (1.0.x)
* sqlite3
* jsoncpp
* c++0x

## Build Guide

```
qmake-qt5 . PREFIX=/usr
make -j8 install
```
