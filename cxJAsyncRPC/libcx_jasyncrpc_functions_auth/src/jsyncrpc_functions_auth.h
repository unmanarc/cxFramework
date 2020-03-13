#ifndef JSYNCRPC_FUNCTIONS_AUTH_H
#define JSYNCRPC_FUNCTIONS_AUTH_H

#include <cx_jasyncrpc_tserver/jasyncrpc_server.h>
#include <cx_auth/iauth.h>

void jASyncRPC_AddAuthMethods(JAsyncRPC_Server_Methods *methods, IAuth * auth = nullptr);

#endif // JSYNCRPC_FUNCTIONS_AUTH_H
