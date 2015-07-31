/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：test_client.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月30日
 *   描    述：
 *
 ================================================================*/

#include <vector>
#include <iostream>


#include "TokenValidator.h"
#include "Thread.h"
#include "IM.BaseDefine.pb.h"
#include "IM.Buddy.pb.h"

#include "ClientConn.h"

#include "HttpClient.h"
#include "json/json.h"

#include "util.h"
using namespace std;


class CClientShell : public CThread
{
private:
    std::shared_ptr<CClientConn> m_conn;
public:
	void OnThreadRun() {
	    string line;
	    vector<string> cmds;
		while (true)
		{
		    cout << "client> " << flush;
            getline(cin, line);
            if (line.size() == 0) {
                continue;
            }
            str_split(line, cmds);
			ExecCmd(cmds);
			cmds.clear();
		}
	}
	
	void ExecCmd(vector<string>& cmds) {
    	if (cmds.size() == 3) {
    	    if (cmds[0] == "login") {
    	        Login(cmds[1], cmds[2]);
    	    } else if (cmds[0] == "reg") {
    	        Register(cmds[1], cmds[2]);
    	    }
    	} else {
    	    cout << "command error" << endl;
    	}
	    
	}
	
	void Login(string username, string passwd) {
	    cout << "login " << username << " " << passwd << endl;
	    on_confirm_data_t data;
	    data.username = username;
	    data.passwd = passwd;
	    data.state = ON_CONFIRM_LOGIN;
	    ConnectMsgServer(data);
	}
	
	void Register(string username, string passwd) {
	    cout << "register " << username << " " << passwd << endl;
	    on_confirm_data_t data;
	    data.username = username;
	    data.passwd = passwd;
	    data.state = ON_CONFIRM_REGISTER;
	    ConnectMsgServer(data);
	    
	}
	
    void GetMsgServerAddr(string login_url, string& ip, uint16_t& port) {
        CHttpClient httpClient;
        string strResp;
        CURLcode nRet = httpClient.Get(login_url, strResp);
        if(nRet != CURLE_OK)
            throw netex("Get msgserver addr falied. access url:%s error\n", login_url.c_str());
        Json::Reader reader;
        Json::Value value;
        if (!reader.parse(strResp, value))
            throw netex("Get msgserver addr falied. parse response error:%s\n", strResp.c_str());

        uint32_t retCode = value["code"].asUInt();
        if(retCode != 0) {
            string strMsg = value["msg"].asString();
            loge("Get msgserver addr falied. errorMsg:%s\n", strMsg.c_str());
            throw netex("Get msgserver addr falied. errorMsg:%s\n", strMsg.c_str());
        }
        ip = value["priorIP"].asString();
        port = value["port"].asUInt();
    }	
	
    void ConnectMsgServer(on_confirm_data_t& data) {
        string ip;
        uint16_t port;
        GetMsgServerAddr("http://127.0.0.1:8080/msg_server", ip, port);
        log("Connect to %s:%d", ip.c_str(), port);
        init_client_conn(ip, port, data);
    }
    
    void Run() {
        signal(SIGPIPE, SIG_IGN);
        StartThread();
        netlib_init();
        cout << "Start event loop..." << endl;
        netlib_eventloop();
    }
};

int main(int argc, char* argv[])
{
//    play("message.wav");
    printf("pid is %d\n", getpid());
	signal(SIGPIPE, SIG_IGN);
	
	CClientShell cs;
	cs.Run();

	return 0;
}
