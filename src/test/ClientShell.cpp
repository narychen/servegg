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

#include "netconn.hpp"
#include "HttpClient.h"
#include "json/json.h"

#include "util.h"
using namespace std;


class CClientShell : public CThread, public CNetConnManager<CClientConn>
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
    	        cout << "login " << cmds[1] << " " << cmds[2] << endl;
    	       // do_login(cmds[1], cmds[2]);
    	    } else if (cmds[0] == "reg") {
    	        cout << "register " << cmds[1] << " " << cmds[2] << endl;
    	       // do_register(cmds[1], cmds[2]);
    	    }
    	} else {
    	    cout << "command error" << endl;
    	}
	    
	}
	
    void GetMsgServerAddr(string login_url, string& ip, uint16_t& port) {
        CHttpClient httpClient;
        string strResp;
        CURLcode nRet = httpClient.Get(login_url, strResp);
        if(nRet != CURLE_OK) {
            loge("login falied. access url:%s error\n", login_url.c_str());
            return;
        }
        Json::Reader reader;
        Json::Value value;
        if(!reader.parse(strResp, value)) {
            loge("login falied. parse response error:%s\n", strResp.c_str());
            return;
        }
    
        try {
            uint32_t nRet = value["code"].asUInt();
            if(nRet != 0) {
                string strMsg = value["msg"].asString();
                loge("login falied. errorMsg:%s\n", strMsg.c_str());
                return;
            }
            ip = value["priorIP"].asString();
            port = value["port"].asUInt();
            
        } catch (std::exception ex) {
            loge("login falied. get json error:%s\n", ex.what());
            return;
        }
    }	
	
    void ConnectMsgServer() {
        string ip;
        uint16_t port;
        GetMsgServerAddr("http://127.0.0.1:8080/msg_server", ip, port);
        log("Connect to %s:%s", ip.c_str(), port);
        m_conn = Connect(ip, port);
        SetTimer(1000);
    }
    
    void Start() override {
        ConnectMsgServer();
        CNetConnManager<CClientConn>::Start();
    }
};

int main(int argc, char* argv[])
{
//    play("message.wav");
    printf("pid is %d\n", getpid());
	signal(SIGPIPE, SIG_IGN);
	
	CClientShell cs;
	cs.Start();

	return 0;
}
