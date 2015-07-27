#include "netconn.h"

using namespace std;

template <class T_CONN>
net_handle_t CNetConnManager<T_CONN>::Connect(string strIp, uint16_t nPort)
{
	net_handle_t fd = netlib_connect(strIp.c_str(), nPort, Callback, (void*)&m_conn_map);
	if (fd != NETLIB_INVALID_HANDLE) {
		m_conn_map.insert(make_pair(fd, std::shared_ptr<T_CONN>(new T_CONN(fd))));
		return fd;
	} else {
        return 0;
	}
}

template <class T_CONN>
net_handle_t CNetConnManager<T_CONN>::Listen(string strIp, uint16_t nPort)
{
    ret = netlib_listen(strIp.c_str(), nPort, Callback, NULL);
    if (fd != NETLIB_ERROR) {
        // m_conn_map.insert(make_pair(fd, std::shared_ptr<T_CONN>(new T_CONN(fd))));
        return fd;
    } else {
        return 0;
    }
}

template <class T_CONN>
shared_ptr<T_CONN> CNetConnManager::FindConn(ConnMap* connMap, net_handle_t handle)
{
	shared_ptr<T_CONN> spConn;
	auto iter = connMap->find(handle);
	if (iter != connMap->end()) {
		spConn = iter->second;
		return spConn;
	} else {
		return nullptr;
	}
}


template <class T_CONN>
void CNetConnManager::Callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
// 	NOTUSED_ARG(handle);
	NOTUSED_ARG(pParam);
	
	if (msg == NETLIB_MSG_CONNECT) {
		auto conn = shared_ptr<T_CONN>(new T_CONN(fd));
		conn->OnConnect();
		return;
	}

	shared_ptr<T_CONN> spConn = FindConn((ConnMap*)callback_data, handle);
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



//////////////////////////
CNetConn::CNetConn(net_handle_t fd) : m_handle(fd)
{
	//log("CImConn::CImConn ");

	m_busy = false;
	m_handle = NETLIB_INVALID_HANDLE;
	m_recv_bytes = 0;

	m_last_send_tick = m_last_recv_tick = get_tick_count();
}

CNetConn::~CNetConn()
{
	//log("CImConn::~CImConn, handle=%d ", m_handle);
}

int CNetConn::Send(void* data, int len)
{
	m_last_send_tick = get_tick_count();
//	++g_send_pkt_cnt;

	if (m_busy)
	{
		m_out_buf.Write(data, len);
		return len;
	}

	int offset = 0;
	int remain = len;
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

void CNetConn::OnRead()
{
	for (;;)
	{
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

template<class T_PDU>
void CNetConn::HandleRead()
{
    T_PDU* pPdu = NULL;
	try
    {
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

void CNetConn::OnWrite()
{
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