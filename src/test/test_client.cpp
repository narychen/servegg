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
#include "ClientConn.h"
#include "netlib.h"
#include "TokenValidator.h"
#include "Thread.h"
#include "IM.BaseDefine.pb.h"
#include "IM.Buddy.pb.h"
#include "playsound.h"
#include "Common.h"
#include "Client.h"

#include "HttpClient.h"
#include "json/json.h"

#include "util.h"
using namespace std;

#define MAX_LINE_LEN	1024
static string g_login_domain = "http://127.0.0.1:8080";
static string g_cmd_string[10];
static int g_cmd_num;
static CClient* g_pClient = NULL;

static shared_ptr<CClientConn> g_conn = shared_ptr<CClientConn>(new CClientConn());

static void print_help()
{
	printf("Usage:\n");
    printf("login user_name user_pass\n");
    /*
	printf("connect serv_ip serv_port user_name user_pass\n");
    printf("getuserinfo\n");
    printf("send toId msg\n");
    printf("unreadcnt\n");
     */
	printf("close\n");
	printf("quit\n");
}

static void do_login(const string& strName, const string& strPass)
{
    try{
        g_pClient = new CClient(strName, strPass, g_login_domain);
        g_pClient->connect([](CClientConn* conn) {

        });
    }catch(...){
        printf("get error while alloc memory\n");
        PROMPTION;
        return;
    }
}

static void do_register(const string& name, const string& passwd)
{
    try{
        logt("do register");
        g_pClient = new CClient(name, passwd, g_login_domain);
        g_pClient->connect([](CClientConn* conn) {
            logt("do reg on connect");
        });
    }catch(const std::exception& exp){
        cout << "Exception encountered: " << exp.what() << endl;
    }

}

static void exec_cmd(std::vector<string>& cmds)
{
	if (cmds.size() == 3) {
	    if (cmds[0] == "login") {
	        do_login(cmds[1], cmds[2]);
	    } else if (cmds[0] == "reg") {
	        do_register(cmds[1], cmds[2]);
	    }
	} else if (cmds.size() == 1) {
	    if (cmds[0] == "close") {
	        g_pClient->close();
	    } else if (cmds[0] == "quit") {
	        exit(0);
	    } else if (cmds[0] == "list") {
    	    printf("+---------------------+\n");
            printf("|        用户名        |\n");
            printf("+---------------------+\n");
            CMapNick2User_t mapUser = g_pClient->getNick2UserMap();
            auto it = mapUser.begin();
            for(;it!=mapUser.end();++it){
                uint32_t nLen = 21 - it->first.length();
                printf("|");
                for(uint32_t i=0; i<nLen/2; ++it) {
                    printf(" ");
                }
                printf("%s", it->first.c_str());
                for(uint32_t i=0; i<nLen/2; ++it) {
                    printf(" ");
                }
                printf("|\n");
                printf("+---------------------+\n");
            }
	    }
	} else {
	    print_help();
	}

}


class CCmdThread : public CThread
{
public:
	void OnThreadRun()
	{
	    string line;
	    vector<string> cmds;
		while (true)
		{
		    cout << PROMPT << flush;
            getline(cin, line);
            if (line.size() == 0) {
                continue;
            }
            str_split(line, cmds);
			exec_cmd(cmds);
			cmds.clear();
		}
	}
private:
	char	m_buf[MAX_LINE_LEN];
};


void get_msg_server_addr(string login_url, string& ip, uint16_t& port)
{
    CHttpClient httpClient;
    string strResp;
    CURLcode nRet = httpClient.Get(login_url, strResp);
    if(nRet != CURLE_OK)
    {
        loge("login falied. access url:%s error\n", login_url.c_str());
        return;
    }
    Json::Reader reader;
    Json::Value value;
    if(!reader.parse(strResp, value))
    {
        loge("login falied. parse response error:%s\n", strResp.c_str());
        return;
    }

    try {
        uint32_t nRet = value["code"].asUInt();
        if(nRet != 0)
        {
            string strMsg = value["msg"].asString();
            loge("login falied. errorMsg:%s\n", strMsg.c_str());
            return;
        }
        ip = value["priorIP"].asString();
        port = value["port"].asUInt();
        
    } catch (std::runtime_error msg) {
        loge("login falied. get json error:%s\n", msg.what());
        return;
    }
}

void connect_msg_server(string ip, uint16_t port)
{
    log("Connect to %s:%s", ip.c_str(), port);
    net_handle_t fd = g_conn->connect(ip.c_str(), port);
    if(fd != INVALID_SOCKET) {
        netlib_register_timer(CClient::TimerCallback, NULL, 1000);
    } else {
        printf("invalid socket handle\n");
    }
}

int main(int argc, char* argv[])
{
//    play("message.wav");
    printf("pid is %d\n", getpid());
	CCmdThread cmdThread;
	cmdThread.StartThread();

	signal(SIGPIPE, SIG_IGN);

	int ret = netlib_init();
	
	string msgIp;
	uint16_t msgPort;
	get_msg_server_addr("http://127.0.0.1:8080/msg_server", msgIp, msgPort);
	connect_msg_server(msgIp, msgPort);

	if (ret == NETLIB_ERROR)
		return ret;
    
	netlib_eventloop();

	return 0;
}
