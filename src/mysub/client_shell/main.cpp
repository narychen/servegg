//  main.cpp
//  client_shell
//
//  Created by luoning on 14-11-4.
//  Copyright (c) 2014年 luoning. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "client_shell.h"
#include "Timer.hpp"
#include <sys/signal.h>
#include "slog_api.h"

void writePid()
{
    uint32_t curPid;
#ifdef _WIN32
    curPid = (uint32_t) GetCurrentProcess();
#else
    curPid = (uint32_t) getpid();
#endif
    FILE* f = fopen("server.pid", "w");
    assert(f);
    char szPid[32];
    snprintf(szPid, sizeof(szPid), "%d", curPid);
    fwrite(szPid, strlen(szPid), 1, f);
    fclose(f);
}


int main() {
    // insert code here...
    logi("start push server...");
    writePid();
    signal(SIGPIPE, SIG_IGN);
    
    CEpollIOLoop* io = new CEpollIOLoop();
    io->Start();
    
    CClientShell* p = new CClientShell();
    CTCPClientAsync* tcpclient = new CTCPClientAsync(io);
    p->SetClient(tcpclient);
    p->SetClientHandler(new CClientHandler(tcpclient));
    p->Start();
    
    while (true) {
        S_Sleep(1000);
    }
    return 0;
}
