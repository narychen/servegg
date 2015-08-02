
#include <ctime>
#include <cstdlib>
#include <string>
#include "../DBPool.h"
#include "Register.h"
#include "IM.Server.pb.h"
#include "ImLog.h"
#include "../ProxyConn.h"

#include "EncDec.h"

namespace DB_PROXY {
    
struct UserRegInfo {
    string name;
    string passwd;
};
    
bool checkForExist(CDBConn* conn, string name)
{
    string sqlcmd = "select * from IMUser where name='" + name + "'";
    CResultSet* pResultSet = conn->ExecuteQuery(sqlcmd.c_str());
    return pResultSet->Next() ? true : false;
}

bool addUser(CDBConn* conn, UserRegInfo& userinfo)
{
    string salt;
    std::srand(std::time(0));
    salt = std::to_string(std::rand()%10000);
    string inPasswd = userinfo.passwd + salt;
    char md5passwd[33];
    CMd5::MD5_Calculate(inPasswd.c_str(), inPasswd.length(), md5passwd);
    string outPasswd(md5passwd);
    string sqlcmd = "insert into IMUser set name='" + 
        userinfo.name + "',password='" + outPasswd + "',salt='" + salt +"'";
    return conn->ExecuteUpdate(sqlcmd.c_str()) ? true : false;
}

void doRegister(CImPdu* pPdu, uint32_t conn_uuid)
{
    RUNTIME_TRACE;
    CImPdu* pPduRes = new CImPdu;
    IM::Server::IMDbRegReq msg;
    IM::Server::IMDbRegRes msgRes;
    string name, passwd;

    CDBManager* pDBManger = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManger->GetDBConn("teamtalk_slave");
    if (!pDBConn) {
        loge("Get db teamtalk_slave connection failed");
        msgRes.set_result_code(2);
        msgRes.set_result_string("Get db teamtalk_slave connection failed");
        pDBManger->RelDBConn(pDBConn);
    }

do{
    if(!msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength())) {
        loge("msg.ParseFromArray failed");
        msgRes.set_result_code(2);
        msgRes.set_result_string("msg.ParseFromArray failed");
        break;
    }

    name = msg.user_name();
    passwd = msg.password();
    msgRes.set_user_name(name);
    msgRes.set_attach_data(msg.attach_data());
    
    if (checkForExist(pDBConn, name)) {
        loge("username already exist");
        msgRes.set_result_code(2);
        msgRes.set_result_string("username already exist");
        break;
    }

    UserRegInfo userinfo;
    userinfo.name = name;
    userinfo.passwd = passwd;
    if (!addUser(pDBConn, userinfo)) {
        loge("add user %s failed", name.c_str());
        msgRes.set_result_code(2);
        msgRes.set_result_string("execute sql failed");
    }

}while(0);
    
    pPduRes->SetPBMsg(&msgRes);
    pPduRes->SetSeqNum(pPdu->GetSeqNum());
    pPduRes->SetServiceId(IM::BaseDefine::SID_OTHER);
    pPduRes->SetCommandId(IM::BaseDefine::CID_OTHER_DB_REGISTER_RES);
    CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
    pDBManger->RelDBConn(pDBConn);
}
    
}