#ifndef __NET_CONN_H__
#define __NET_CONN_H__


#include <unordered_map>
#include <memory>

#include "netlib.h"
#include "util.h"
#include "ImPduBase.h"

#define SERVER_HEARTBEAT_INTERVAL	5000
#define SERVER_TIMEOUT				30000
#define CLIENT_HEARTBEAT_INTERVAL	30000
#define CLIENT_TIMEOUT				120000
#define MOBILE_CLIENT_TIMEOUT       60000 * 5
#define READ_BUF_SIZE	2048

template <class T_CONN>
class CNetConnManager
{
public:
    using ConnMap = unordered_map<net_handle_t, std::shared_ptr<T_CONN>>;

public:
    CNetConnManager() {}
    virtual ~CNetConnManager() {}
    
    net_handle_t Connect(string strIp, uint16_t nPort);
    net_handle_t Listen(string strIp, uint16_t nPort);
    
    shared_ptr<T_CONN> FindConn(ConnMap* connMap, net_handle_t handle);
    void Callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam);
    
    
private:
    ConnMap m_conn_map;
    
}

template<class T_PDU>
class CNetConn : public CRefObject, public enable_shared_from_this<CNetConn>
{
public:
	CNetConn();
	virtual ~CNetConn();
	
	int Send(void* data, int len);
	virtual void OnRead();
	virtual void OnWrite();
	
	bool IsBusy() { return m_busy; }
	int SendPdu(T_PDU* pPdu) { return Send(pPdu->GetBuffer(), pPdu->GetLength()); }

	virtual void OnConnect(){}
	virtual void OnConfirm(){}
	virtual void OnClose(){}
	virtual void OnTimer(uint64_t){}
    virtual void OnWriteCompelete(){}
	virtual void HandlePdu(T_PDU*){}
	
	virtual void SetConnName(std::string name) { m_conn_name = name; }
	virtual string& GetConnName() { return m_conn_name; }

protected:
	std::string     m_conn_name;
	net_handle_t	m_handle;
	bool			m_busy;

	string			m_peer_ip;
	uint16_t		m_peer_port;
	CSimpleBuffer	m_in_buf;
	CSimpleBuffer	m_out_buf;

	bool			m_policy_conn;
	uint32_t		m_recv_bytes;
	uint64_t		m_last_send_tick;
	uint64_t		m_last_recv_tick;
    uint64_t        m_last_all_user_tick;
};


#endif