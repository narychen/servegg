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
    
private:
    ConnMap m_conn_map;
    
public:
    CNetConnManager() { netlib_init(); }
    virtual ~CNetConnManager() {}
    
    virtual std::shared_ptr<T_CONN> Connect(string strIp, uint16_t nPort) {
		net_handle_t fd = netlib_connect(strIp.c_str(), nPort, Callback, (void*)this);
		if (fd != NETLIB_INVALID_HANDLE) {
		    auto sp = std::shared_ptr<T_CONN>(new T_CONN(fd));
			m_conn_map.insert(make_pair(fd, sp));
			return sp;
		} else {
			throw netex("connect to %s:%d failed", strIp.c_str(), nPort);
		}
	}

    virtual void Listen(string strIp, uint16_t nPort) {
        auto ret = netlib_listen(strIp.c_str(), nPort, Callback, NULL);
		if (ret == NETLIB_ERROR)
			throw netex("listen on %s:%d failed", strIp.c_str(), nPort);
	}

    virtual void Start() { netlib_eventloop(); }

    void SetTimer(uint64_t interval) {
        netlib_register_timer(Callback, (void*)this, interval);
    }
    
    virtual shared_ptr<T_CONN> FindConn(net_handle_t handle) {
		shared_ptr<T_CONN> spConn;
		auto iter = m_conn_map.find(handle);
		if (iter != m_conn_map.end()) {
			spConn = iter->second;
			return spConn;
		} else {
			return nullptr;
		}
	}

    static void Callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
// 	NOTUSED_ARG(handle);
		NOTUSED_ARG(pParam);
		auto self = (CNetConnManager<T_CONN>*)callback_data;

        if (msg == NETLIB_MSG_TIMER) {
            for (auto& e : self->m_conn_map){
                auto conn = e.second;
                conn->OnTimer(get_tick_count());
            }
        }

		if (msg == NETLIB_MSG_CONNECT) {
			auto conn = shared_ptr<T_CONN>(new T_CONN(handle));
			conn->OnConnect();
			return;
		}

		auto spConn = self->FindConn(handle);
		if (!spConn)
			return;

		log("msg=%d, handle=%d ", msg, handle);

		switch (msg)
		{
			case NETLIB_MSG_CONFIRM:
				spConn->OnConfirm();
				break;
			case NETLIB_MSG_READ:
				spConn->OnRead();
				break;
			case NETLIB_MSG_WRITE:
				spConn->OnWrite();
				break;
			case NETLIB_MSG_CLOSE:
				spConn->OnClose();
				break;
			default:
				log("!!!imconn_callback error msg: %d ", msg);
				break;
		}
	}


    
};

template<class T_PDU>
class CNetConn : public CRefObject, public enable_shared_from_this<CNetConn<T_PDU>>
{
public:
	CNetConn(net_handle_t fd) : m_handle(fd){
		//log("CImConn::CImConn ");

		m_busy = false;
		m_handle = NETLIB_INVALID_HANDLE;
		m_recv_bytes = 0;

		m_last_send_tick = m_last_recv_tick = get_tick_count();
	}

	virtual ~CNetConn() {}
	
	int Send(void* data, uint32_t len) {
        m_last_send_tick = get_tick_count();
//	++g_send_pkt_cnt;

        if (m_busy)
        {
            m_out_buf.Write(data, len);
            return len;
        }

        int offset = 0;
        uint32_t remain = len;
        while (remain > 0) {
            int send_size = remain;
            if (send_size > NETLIB_MAX_SOCKET_BUF_SIZE) {
                send_size = NETLIB_MAX_SOCKET_BUF_SIZE;
            }

            int ret = netlib_send(m_handle, (char*)data + offset , send_size);
            if (ret <= 0) {
                ret = 0;
                break;
            }

            offset += ret;
            remain -= ret;
        }

        if (remain > 0)
        {
            m_out_buf.Write((char*)data + offset, remain);
            m_busy = true;
            log("send busy, remain=%d ", m_out_buf.GetWriteOffset());
        }
        else
        {
            OnWriteCompelete();
        }

        return len;
    }

	virtual void OnRead() {
        for (;;) {
            uint32_t free_buf_len = m_in_buf.GetAllocSize() - m_in_buf.GetWriteOffset();
            if (free_buf_len < READ_BUF_SIZE)
                m_in_buf.Extend(READ_BUF_SIZE);

            int ret = netlib_recv(m_handle, m_in_buf.GetBuffer() + m_in_buf.GetWriteOffset(), READ_BUF_SIZE);
            if (ret <= 0)
                break;

            m_recv_bytes += ret;
            m_in_buf.IncWriteOffset(ret);

            m_last_recv_tick = get_tick_count();
        }

        HandleRead();

    }

	virtual void OnWrite() {
        if (!m_busy)
            return;

        while (m_out_buf.GetWriteOffset() > 0) {
            int send_size = m_out_buf.GetWriteOffset();
            if (send_size > NETLIB_MAX_SOCKET_BUF_SIZE) {
                send_size = NETLIB_MAX_SOCKET_BUF_SIZE;
            }

            int ret = netlib_send(m_handle, m_out_buf.GetBuffer(), send_size);
            if (ret <= 0) {
                ret = 0;
                break;
            }

            m_out_buf.Read(NULL, ret);
        }

        if (m_out_buf.GetWriteOffset() == 0) {
            m_busy = false;
        }

        log("onWrite, remain=%d ", m_out_buf.GetWriteOffset());
    }
	
	bool IsBusy() { return m_busy; }
	int SendPdu(T_PDU* pPdu) { return Send(pPdu->GetBuffer(), pPdu->GetLength()); }

	virtual void OnConnect(){}
	virtual void OnConfirm(){}
	virtual void OnClose(){}
	virtual void OnTimer(uint64_t){}
    virtual void OnWriteCompelete(){}
	virtual void HandlePdu(T_PDU*){}

	void HandleRead() {
        T_PDU* pPdu = NULL;
        try {
            while ( ( pPdu = T_PDU::ReadPdu(m_in_buf.GetBuffer(), m_in_buf.GetWriteOffset()) ) )
            {
                uint32_t pdu_len = pPdu->GetLength();

                HandlePdu(pPdu);

                m_in_buf.Read(NULL, pdu_len);
                delete pPdu;
                pPdu = NULL;
//			++g_recv_pkt_cnt;
            }
        } catch (const exception& ex) {
            log("!!!catch exception, %s, close the connection ", ex.what());
            if (pPdu) {
                delete pPdu;
                pPdu = NULL;
            }
            OnClose();
        }
    }
	
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