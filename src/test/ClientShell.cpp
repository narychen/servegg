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
#include "util.h"
using namespace std;

class CClientShell : public CThread
{
private:
    std::shared_ptr<CClientConn> m_conn;
public:
	void OnThreadRun() {
	    string line;
		while (true)
		{
		    cout << "client> " << flush;
            getline(cin, line);
            if (line.size() == 0) {
                continue;
            }
            client_shell_cmds_add(line);
		}
	}
    
    void Run() {
        signal(SIGPIPE, SIG_IGN);
        StartThread();
        netlib_init();
        cout << "Start event loop..." << endl;
        netlib_add_loop(client_conn_loop_callback, nullptr);
        netlib_register_timer(client_conn_timer_callback, NULL, 1000);
        netlib_eventloop();
    }
};

int main(int argc, char* argv[])
{
//    play("message.wav");
    // setlocale(LC_CTYPE, "UTF-8");
    printf("pid is %d\n", getpid());
	signal(SIGPIPE, SIG_IGN);
	
	CClientShell cs;
	cs.Run();

	return 0;
}
