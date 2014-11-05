
#include <openssl/x509.h>
#include <openssl/rsa.h>
#include <openssl/crypto.h>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/atomic.hpp>

#include "avtcpif.hpp"
#include "avproto.hpp"
#include "protocol/avim-base.pb.h"

static  boost::atomic<uint64_t> ifcounts;// = 0ul;

std::string avtcpif::allocate_ifname()
{
	return boost::str( boost::format("avif_tcp%d") % ifcounts ++ );
}

avtcpif::~avtcpif()
{
   RSA_free(_rsa);

	if(0==CRYPTO_add(&_x509->references, -1, CRYPTO_LOCK_X509))
    {
	   X509_free(_x509);
	}
}

avtcpif::avtcpif(boost::shared_ptr<boost::asio::ip::tcp::socket> _sock, std::string local_addr, RSA * _key, X509 * cert)
	: _rsa(_key)
	, _x509(cert)
{
	RSA_up_ref(_rsa);
	CRYPTO_add(&_x509->references, 1, CRYPTO_LOCK_X509);
	m_sock = _sock;
	ifname = allocate_ifname();

	m_local_addr.reset( new proto::base::avAddress(av_address_from_string(local_addr)));
}

boost::asio::io_service& avtcpif::get_io_service() const
{
	return m_sock->get_io_service();
}

std::string avtcpif::get_ifname() const
{
    return ifname;
}

RSA* avtcpif::get_rsa_key()
{
    return _rsa;
}

const proto::base::avAddress* avtcpif::if_address() const
{
    return m_local_addr.get();
}

const proto::base::avAddress * avtcpif::remote_address() const
{
	return m_remote_addr.get();
}

bool avtcpif::async_master_handshake(bool as_master, boost::asio::yield_context yield_context)
{
	boost::uint32_t l;
	boost::system::error_code ec;
	// 接收用户的 av地址
	int len = boost::asio::async_read(*m_sock, boost::asio::buffer(&l, sizeof(l)), yield_context[ec]);

	if( len != sizeof(l))
		return false;

	len = boost::asio::async_read(*m_sock, m_recv_buf, boost::asio::transfer_exactly(ntohl(l)), yield_context[ec]);

	std::istream inputstream(&m_recv_buf);

	boost::scoped_ptr<proto::base::avTCPPacket> pkt(new proto::base::avTCPPacket);

	pkt->ParseFromIstream(&inputstream);

	if(pkt->type() != 1)
		return false;

	m_remote_addr.reset(new proto::base::avAddress(pkt->endpoint_address()));

	// TODO 检查证书

	* pkt->mutable_endpoint_address() = * m_local_addr;

	pkt->clear_endpoint_cert();

	std::ostream outstream(&m_send_buf);

	pkt->SerializeToOstream(&outstream);

	// 返回服务器地址

	l = htonl(m_send_buf.size());

	boost::asio::async_write(*m_sock, boost::asio::buffer(&l, sizeof(l)), yield_context[ec]);

	boost::asio::async_write(*m_sock, m_send_buf, boost::asio::transfer_all(), yield_context[ec]);

	return !ec;
}

bool avtcpif::slave_handshake(bool as_master)
{
	boost::uint32_t l;
	std::ostream outstream(&m_send_buf);
	boost::system::error_code ec;

	proto::base::avTCPPacket pkt;
	pkt.set_type(1);

	* pkt.mutable_endpoint_address() = * m_local_addr;

	pkt.SerializeToOstream(&outstream);

	// 返回服务器地址

	l = htonl(m_send_buf.size());

	boost::asio::write(*m_sock, boost::asio::buffer(&l, sizeof(l)));

	boost::asio::write(*m_sock, m_send_buf, boost::asio::transfer_all());

	int len = boost::asio::read(*m_sock, boost::asio::buffer(&l, sizeof(l)));

	if( len != sizeof(l))
		return false;

	len = boost::asio::read(*m_sock, m_recv_buf, boost::asio::transfer_exactly(ntohl(l)));

	std::istream inputstream(&m_recv_buf);

	pkt.Clear();
	pkt.ParseFromIstream(&inputstream);

	if(pkt.type() != 1)
		return false;

	m_remote_addr.reset( new proto::base::avAddress(pkt.endpoint_address()));

	// TODO 检查证书

	return true;
}

std::string avtcpif::remote_addr()
{
	return m_remote_addr->username() + "@" + m_remote_addr->domain();
}

bool avtcpif::async_write_packet(proto::base::avPacket* avpkt, boost::asio::yield_context yield_context)
{
	boost::system::error_code ec;
	proto::base::avTCPPacket pkt;

	std::ostream outstream(&m_send_buf);

	pkt.set_type(0);
	pkt.set_allocated_avpacket(avpkt);
	pkt.SerializeToOstream(&outstream);
	pkt.release_avpacket();

	boost::uint32_t l = htonl(m_send_buf.size());

	boost::asio::async_write(*m_sock, boost::asio::buffer(&l, sizeof(l)), yield_context[ec]);

	boost::asio::async_write(*m_sock, m_send_buf, boost::asio::transfer_all(), yield_context[ec]);
	return ! ec;
}

boost::shared_ptr<proto::base::avPacket> avtcpif::async_read_packet(boost::asio::yield_context yield_context)
{
	do {
		proto::base::avTCPPacket pkt;
		boost::uint32_t l;
		boost::system::error_code ec;
		// 接收用户的 av地址
		int len = boost::asio::async_read(*m_sock, boost::asio::buffer(&l, sizeof(l)), yield_context[ec]);

		if( len != sizeof(l))
			return nullptr;

		len = boost::asio::async_read(*m_sock, m_recv_buf, boost::asio::transfer_exactly(ntohl(l)), yield_context[ec]);

		if( ec )
			return nullptr;

		std::istream inputstream(&m_recv_buf);

		if(!pkt.ParseFromIstream(&inputstream))
		{
			return nullptr;
		}

		if(pkt.type() == 0)
		{
			boost::shared_ptr<proto::base::avPacket> avPacket;
			avPacket.reset(new proto::base::avPacket);
			avPacket->CopyFrom(pkt.avpacket());
			return avPacket;
		}

	}while(1);
}
