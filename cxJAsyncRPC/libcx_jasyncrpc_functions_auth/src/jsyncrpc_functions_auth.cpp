#include "jsyncrpc_functions_auth.h"

template<typename T>
Json::Value stringListToValue(const T & value)
{
    Json::Value x;
    int i=0;
    for (const std::string & strVal : value)
    {
        x[i++] = strVal;
    }
    return x;
}

Json::Value accountChangePassword(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{   
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    sPasswordData passData;
    passData.setJson(payload["passData"]);
    payloadOut["retCode"] = auth->accountChangePassword(payload["accountName"].asString(),  passData,
                                                        payload["passIndex"].asUInt(),  payload["domainName"].asString());
    return payloadOut;
}

Json::Value accountRemove(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->accountRemove(payload["accountName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value accountDisable(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->accountDisable(payload["accountName"].asString(), payload["domainName"].asString(), payload["disabled"].asBool());
    return payloadOut;
}


Json::Value accountConfirm(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->accountDisable(payload["accountName"].asString(), payload["domainName"].asString(), payload["disabled"].asBool());
    return payloadOut;
}

Json::Value accountChangeDescription(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->accountChangeDescription(payload["accountName"].asString(), payload["description"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value accountChangeEmail(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->accountChangeEmail(payload["accountName"].asString(), payload["email"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value accountChangeExtraData(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->accountChangeExtraData(payload["accountName"].asString(), payload["extraData"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value accountChangeExpiration(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->accountChangeExpiration(payload["accountName"].asString(), payload["domainName"].asString(), payload["expiration"].asUInt64());
    return payloadOut;
}

Json::Value isAccountDisabled(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["disabled"] = auth->isAccountDisabled(payload["accountName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value isAccountConfirmed(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["confirmed"] = auth->isAccountConfirmed(payload["accountName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value isAccountSuperUser(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["superuser"] = auth->isAccountSuperUser(payload["accountName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value accountDescription(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["description"] = auth->accountDescription(payload["accountName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value accountEmail(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["email"] = auth->accountEmail(payload["accountName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value accountExtraData(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["extraData"] = auth->accountExtraData(payload["accountName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value accountExpirationDate(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["expirationDate"] = Json::Int64(auth->accountExpirationDate(payload["accountName"].asString(), payload["domainName"].asString()));
    return payloadOut;
}

Json::Value isAccountExpired(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["isAccountExpired"] = auth->isAccountExpired(payload["accountName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value accountValidateAttribute(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["isAccountExpired"] = auth->accountValidateAttribute(payload["accountName"].asString(), payload["attribName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value accountsList(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["accountsList"] = stringListToValue(auth->accountsList(payload["domainName"].asString()));
    return payloadOut;
}

Json::Value accountGroups(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["accountGroups"] = stringListToValue(auth->accountGroups(payload["accountName"].asString(),payload["domainName"].asString()));
    return payloadOut;
}

Json::Value accountDirectAttribs(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["accountDirectAttribs"] = stringListToValue(auth->accountDirectAttribs(payload["accountName"].asString(),payload["domainName"].asString()));
    return payloadOut;
}

Json::Value accountUsableAttribs(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["accountUsableAttribs"] = stringListToValue(auth->accountUsableAttribs(payload["accountName"].asString(),payload["domainName"].asString()));
    return payloadOut;
}

Json::Value attribAdd(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->attribAdd(payload["attribName"].asString(), payload["description"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value attribRemove(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->attribRemove(payload["attribName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value attribGroupAdd(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->attribGroupAdd(payload["attribName"].asString(),payload["groupName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value attribGroupRemove(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->attribGroupRemove(payload["attribName"].asString(),payload["groupName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value attribAccountAdd(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->attribAccountAdd(payload["attribName"].asString(),payload["accountName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value attribAccountRemove(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->attribAccountRemove(payload["attribName"].asString(),payload["accountName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value attribChangeDescription(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->attribAccountRemove(payload["attribName"].asString(),payload["attribDescription"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value attribsList(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["attribsList"] = stringListToValue(auth->attribsList(payload["domainName"].asString()));
    return payloadOut;
}

Json::Value attribGroups(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["attribGroups"] = stringListToValue(auth->attribGroups(payload["attribName"].asString(),payload["domainName"].asString()));
    return payloadOut;
}

Json::Value attribAccounts(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["attribAccounts"] = stringListToValue(auth->attribAccounts(payload["attribName"].asString(),payload["domainName"].asString()));
    return payloadOut;
}

Json::Value groupAdd(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->groupAdd(payload["groupName"].asString(), payload["groupDescription"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value groupRemove(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->groupRemove(payload["groupName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value groupExist(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->groupExist(payload["groupName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value groupAccountAdd(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->groupAccountAdd(payload["groupName"].asString(),payload["accountName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value groupAccountRemove(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->groupAccountRemove(payload["groupName"].asString(),payload["accountName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value groupChangeDescription(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->groupChangeDescription(payload["groupName"].asString(),payload["groupDescription"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value groupValidateAttribute(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->groupValidateAttribute(payload["groupName"].asString(),payload["attribName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value groupDescription(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["retCode"] = auth->groupDescription(payload["groupName"].asString(), payload["domainName"].asString());
    return payloadOut;
}

Json::Value groupsList(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["groupsList"] = stringListToValue(auth->groupsList(payload["domainName"].asString()));
    return payloadOut;
}

Json::Value groupAttribs(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["groupAttribs"] = stringListToValue(auth->groupAttribs(payload["groupName"].asString(),payload["domainName"].asString()));
    return payloadOut;
}

Json::Value groupAccounts(void * obj, IAuth_Session * session, const Json::Value & payload,const Json::Value &extraInfo, Json::Value *extraInfoOut)
{
    Json::Value payloadOut;
    IAuth *auth = static_cast<IAuth *>(obj?obj:session->getAuthenticator());
    payloadOut["groupAccounts"] = stringListToValue(auth->groupAccounts(payload["groupName"].asString(),payload["domainName"].asString()));
    return payloadOut;
}

void jASyncRPC_AddAuthMethods(JAsyncRPC_Server_Methods *methods, IAuth *auth)
{
    methods->addRPCMethod("accountChangePassword", {"admin"}, &accountChangePassword, auth);
    methods->addRPCMethod("accountRemove", {"admin"}, &accountRemove, auth);
    methods->addRPCMethod("accountDisable", {"admin"}, &accountDisable, auth);
    methods->addRPCMethod("accountConfirm", {"admin"}, &accountConfirm, auth);
    methods->addRPCMethod("accountChangeDescription", {"admin"}, &accountChangeDescription, auth);
    methods->addRPCMethod("accountChangeEmail", {"admin"}, &accountChangeEmail, auth);
    methods->addRPCMethod("accountChangeExtraData", {"admin"}, &accountChangeExtraData, auth);
    methods->addRPCMethod("accountChangeExpiration", {"admin"}, &accountChangeExpiration, auth);

    methods->addRPCMethod("isAccountDisabled", {"admin"}, &isAccountDisabled, auth);
    methods->addRPCMethod("isAccountConfirmed", {"admin"}, &isAccountConfirmed, auth);
    methods->addRPCMethod("isAccountSuperUser", {"admin"}, &isAccountSuperUser, auth);
    methods->addRPCMethod("accountDescription", {"admin"}, &accountDescription, auth);
    methods->addRPCMethod("accountEmail", {"admin"}, &accountEmail, auth);
    methods->addRPCMethod("accountExtraData", {"admin"}, &accountExtraData, auth);
    methods->addRPCMethod("accountExpirationDate", {"admin"}, &accountExpirationDate, auth);
    methods->addRPCMethod("isAccountExpired", {"admin"}, &isAccountExpired, auth);
    methods->addRPCMethod("accountValidateAttribute", {"admin"}, &accountValidateAttribute, auth);
    methods->addRPCMethod("accountsList", {"admin"}, &accountsList, auth);
    methods->addRPCMethod("accountGroups", {"admin"}, &accountGroups, auth);
    methods->addRPCMethod("accountDirectAttribs", {"admin"}, &accountDirectAttribs, auth);
    methods->addRPCMethod("accountUsableAttribs", {"admin"}, &accountUsableAttribs, auth);

    methods->addRPCMethod("attribAdd", {"admin"}, &attribAdd, auth);
    methods->addRPCMethod("attribRemove", {"admin"}, &attribRemove, auth);
    methods->addRPCMethod("attribGroupAdd", {"admin"}, &attribGroupAdd, auth);
    methods->addRPCMethod("attribGroupRemove", {"admin"}, &attribGroupRemove, auth);
    methods->addRPCMethod("attribAccountAdd", {"admin"}, &attribAccountAdd, auth);
    methods->addRPCMethod("attribAccountRemove", {"admin"}, &attribAccountRemove, auth);
    methods->addRPCMethod("attribChangeDescription", {"admin"}, &attribChangeDescription, auth);
    methods->addRPCMethod("attribsList", {"admin"}, &attribsList, auth);
    methods->addRPCMethod("attribGroups", {"admin"}, &attribGroups, auth);
    methods->addRPCMethod("attribAccounts", {"admin"}, &attribAccounts, auth);

    methods->addRPCMethod("groupAdd", {"admin"}, &groupAdd, auth);
    methods->addRPCMethod("groupRemove", {"admin"}, &groupRemove, auth);
    methods->addRPCMethod("groupExist", {"admin"}, &groupExist, auth);
    methods->addRPCMethod("groupAccountAdd", {"admin"}, &groupAccountAdd, auth);
    methods->addRPCMethod("groupAccountRemove", {"admin"}, &groupAccountRemove, auth);
    methods->addRPCMethod("groupChangeDescription", {"admin"}, &groupChangeDescription, auth);
    methods->addRPCMethod("groupValidateAttribute", {"admin"}, &groupValidateAttribute, auth);
    methods->addRPCMethod("groupDescription", {"admin"}, &groupDescription, auth);
    methods->addRPCMethod("groupsList", {"admin"}, &groupsList, auth);
    methods->addRPCMethod("groupAttribs", {"admin"}, &groupAttribs, auth);
    methods->addRPCMethod("groupAccounts", {"admin"}, &groupAccounts, auth);
}
