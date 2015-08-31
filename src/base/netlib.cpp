#include "netlib.h"
#include "BaseSocket.h"
#include "EventDispatch.h"

// #define __LIBEVENT__

#ifndef __LIBEVENT__

int netlib_init()
{
	int ret = NETLIB_OK;
#ifdef _WIN32
	WSADATA wsaData;
	WORD wReqest = MAKEWORD(1, 1);
	if (WSAStartup(wReqest, &wsaData) != 0)
	{
		ret = NETLIB_ERROR;
	}
#endif

	return ret;
}

int netlib_destroy()
{
	int ret = NETLIB_OK;
#ifdef _WIN32
	if (WSACleanup() != 0)
	{
		ret = NETLIB_ERROR;
	}
#endif

	return ret;
}

int netlib_listen(
		const char*	server_ip, 
		uint16_t	port,
		callback_t	callback,
		void*		callback_data)
{
	auto spSocket = sp_CBaseSocket(new CBaseSocket());
	if (!spSocket)
		return NETLIB_ERROR;

	int ret =  spSocket->Listen(server_ip, port, callback, callback_data);
	// if (ret == NETLIB_ERROR)
	// 	delete pSocket;
	return ret;
}

net_handle_t netlib_connect(
		const char* server_ip, 
		uint16_t	port, 
		callback_t	callback, 
		void*		callback_data)
{
	auto spSocket = sp_CBaseSocket(new CBaseSocket());
	if (!spSocket)
		return NETLIB_INVALID_HANDLE;

	net_handle_t handle = spSocket->Connect(server_ip, port, callback, callback_data);
	// if (handle == NETLIB_INVALID_HANDLE)
	// 	delete pSocket;
	return handle;
}

int netlib_send(net_handle_t handle, void* buf, int len)
{
	auto spSocket = FindBaseSocket(handle);
	if (!spSocket)
	{
		return NETLIB_ERROR;
	}
	int ret = spSocket->Send(buf, len);
	// pSocket->ReleaseRef();
	return ret;
}

int netlib_recv(net_handle_t handle, void* buf, int len)
{
	auto spSocket = FindBaseSocket(handle);
	if (!spSocket)
		return NETLIB_ERROR;

	int ret = spSocket->Recv(buf, len);
	// pSocket->ReleaseRef();
	return ret;
}

int netlib_close(net_handle_t handle)
{
	auto spSocket = FindBaseSocket(handle);
	if (!spSocket)
		return NETLIB_ERROR;

	int ret = spSocket->Close();
	// pSocket->ReleaseRef();
	return ret;
}

int netlib_option(net_handle_t handle, int opt, void* optval)
{
	auto spSocket = FindBaseSocket(handle);
	if (!spSocket)
		return NETLIB_ERROR;

	if ((opt >= NETLIB_OPT_GET_REMOTE_IP) && !optval)
		return NETLIB_ERROR;

	switch (opt)
	{
	case NETLIB_OPT_SET_CALLBACK:
		spSocket->SetCallback((callback_t)optval);
		break;
	case NETLIB_OPT_SET_CALLBACK_DATA:
		spSocket->SetCallbackData(optval);
		break;
	case NETLIB_OPT_GET_REMOTE_IP:
		*(string*)optval = spSocket->GetRemoteIP();
		break;
	case NETLIB_OPT_GET_REMOTE_PORT:
		*(uint16_t*)optval = spSocket->GetRemotePort();
		break;
	case NETLIB_OPT_GET_LOCAL_IP:
		*(string*)optval = spSocket->GetLocalIP();
		break;
	case NETLIB_OPT_GET_LOCAL_PORT:
		*(uint16_t*)optval = spSocket->GetLocalPort();
		break;
	case NETLIB_OPT_SET_SEND_BUF_SIZE:
		spSocket->SetSendBufSize(*(uint32_t*)optval);
		break;
	case NETLIB_OPT_SET_RECV_BUF_SIZE:
		spSocket->SetRecvBufSize(*(uint32_t*)optval);
		break;
	}

	// pSocket->ReleaseRef();
	return NETLIB_OK;
}

int netlib_register_timer(callback_t callback, void* user_data, uint64_t interval)
{
	CEventDispatch::Instance()->AddTimer(callback, user_data, interval);
	return 0;
}

int netlib_delete_timer(callback_t callback, void* user_data)
{
	CEventDispatch::Instance()->RemoveTimer(callback, user_data);
	return 0;
}

int netlib_add_loop(callback_t callback, void* user_data)
{
	CEventDispatch::Instance()->AddLoop(callback, user_data);
	return 0;
}

void netlib_eventloop(uint32_t wait_timeout)
{
	CEventDispatch::Instance()->StartDispatch(wait_timeout);
}

void netlib_stop_event()
{
    CEventDispatch::Instance()->StopDispatch();
}

bool netlib_is_running()
{
    return CEventDispatch::Instance()->isRunning();
}

#else

#include <unordered_map>
#include <event2/event.h>

static unordered_map<net_handle_t, SOCKET_STATE> g_socket_state;
static unordered_map<net_handle_t, struct event*> g_event_map;
struct event_base* g_libevent_base;

static int _GetErrorCode()
{
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}

static void _SetNonblock(SOCKET fd)
{
#ifdef _WIN32
	u_long nonblock = 1;
	int ret = ioctlsocket(fd, FIONBIO, &nonblock);
#else
	int ret = fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL));
#endif
	if (ret == SOCKET_ERROR)
	{
		log("_SetNonblock failed, err_code=%d", _GetErrorCode());
	}
}

static bool _IsBlock(int error_code)
{
#ifdef _WIN32
	return ( (error_code == WSAEINPROGRESS) || (error_code == WSAEWOULDBLOCK) );
#else
	return ( (error_code == EINPROGRESS) || (error_code == EWOULDBLOCK) );
#endif
}

static void _SetReuseAddr(SOCKET fd)
{
	int reuse = 1;
	int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
	if (ret == SOCKET_ERROR)
	{
		log("_SetReuseAddr failed, err_code=%d", _GetErrorCode());
	}
}

static void _SetNoDelay(SOCKET fd)
{
	int nodelay = 1;
	int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
	if (ret == SOCKET_ERROR)
	{
		log("_SetNoDelay failed, err_code=%d", _GetErrorCode());
	}
}

static void _SetAddr(const char* ip, const uint16_t port, sockaddr_in* pAddr)
{
	memset(pAddr, 0, sizeof(sockaddr_in));
	pAddr->sin_family = AF_INET;
	pAddr->sin_port = htons(port);
	pAddr->sin_addr.s_addr = inet_addr(ip);
	if (pAddr->sin_addr.s_addr == INADDR_NONE)
	{
		hostent* host = gethostbyname(ip);
		if (host == NULL)
		{
			log("gethostbyname failed, ip=%s", ip);
			return;
		}

		pAddr->sin_addr.s_addr = *(uint32_t*)host->h_addr;
	}
}

int netlib_init()
{
	int ret = NETLIB_OK;
#ifdef _WIN32
	WSADATA wsaData;
	WORD wReqest = MAKEWORD(1, 1);
	if (WSAStartup(wReqest, &wsaData) != 0)
	{
		ret = NETLIB_ERROR;
	}
#endif
	g_libevent_base = event_base_new();
	return ret;
}

net_handle_t netlib_connect(
		const char* server_ip, 
		uint16_t	port, 
		callback_t	callback, 
		void*		callback_data)
{
	net_handle_t sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		loge("socket failed, err_code=%d", _GetErrorCode());
		return NETLIB_INVALID_HANDLE;
	}
	
	_SetNonblock(sock);
	_SetNoDelay(sock);
	
	sockaddr_in serv_addr;
	_SetAddr(server_ip, port, &serv_addr);
	int ret = connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr));
	if ( (ret == SOCKET_ERROR) && (!_IsBlock(_GetErrorCode())) {
		loge("connect failed, err_code=%d", _GetErrorCode());
		closesocket(m_socket);
		return NETLIB_INVALID_HANDLE;
	}
	
	g_socket_state[sock] = SOCKET_STATE_CONNECTING;
	struct event* ev = event_new(g_libevent_base, sock, EV_WRITE|EV_READ|EV_PERSIST, 
		netlib_libevent_on_write, callback_data);
	event_add(ev, NULL);
	g_event_map[fd] = ev;
	return sock;

}

void netlib_libevent_on_write(evutil_socket_t fd, short what, void *arg)
{
	auto it = g_socket_state.find((net_handle_t)fd);
	if (it != g_socket_state.end()) {
		auto state = it->second;
		if (state == SOCKET_STATE_CONNECTING) {
			
			int error = 0;
			socklen_t len = sizeof(error);
#ifdef _WIN32
			getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
#else
			getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (void*)&error, &len);
#endif
			if (error) {
				callback(arg, NETLIB_MSG_CLOSE, (net_handle_t)fd, NULL);
			} else {
				g_socket_state[fd] = SOCKET_STATE_CONNECTED;
				callback(arg, NETLIB_MSG_CONFIRM, (net_handle_t)fd, NULL);
			}
			
		} else if (state == SOCKET_STATE_CONNECTED) {
			callback(arg, NETLIB_MSG_WRITE, (net_handle_t)fd, NULL);
		}
	}
}




#endif
