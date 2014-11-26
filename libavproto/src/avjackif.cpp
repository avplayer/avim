
#include <openssl/asn1.h>
#include <openssl/dh.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "serialization.hpp"

#include "avjackif.hpp"
#include "avproto.hpp"

#include "avim_proto/message.pb.h"
#include "avim_proto/user.pb.h"

static inline std::string i2d_X509(X509 * x509)
{
	unsigned char * out = NULL;
	int l = i2d_X509(x509, & out);
	std::string ret((char*)out, l);
	OPENSSL_free(out);
	return ret;
}

template<typename AsyncStream>
static inline google::protobuf::Message*
async_read_protobuf_message(AsyncStream &_sock, boost::asio::yield_context yield_context)
{
	std::uint32_t l;
	boost::asio::async_read(_sock, boost::asio::buffer(&l, sizeof(l)), boost::asio::transfer_exactly(4), yield_context);
	auto hostl = htonl(l);
	std::string  buf;

	buf.resize(hostl + 4);
	memcpy(&buf[0], &l, 4);
	hostl = boost::asio::async_read(_sock, boost::asio::buffer(&buf[4], hostl),
		boost::asio::transfer_exactly(hostl), yield_context);

	return av_proto::decode(buf);
}

/*
 * 这个类呢，是用来实现和 JACK 实现的那个 AVROUTER 对接的。也就是 JACK 版本的 AV NETWORK SERVICE PROVIDER
 */
void avjackif::set_pki(std::shared_ptr<RSA> _key, std::shared_ptr<X509> cert)
{
    _rsa = _key;
	_x509 = cert;

	unsigned char * CN = NULL;

	auto cert_name = X509_get_subject_name(cert.get());
	auto cert_entry = X509_NAME_get_entry(
		cert_name,
		X509_NAME_get_index_by_NID(cert_name, NID_commonName, 0)
	);
	ASN1_STRING *entryData = X509_NAME_ENTRY_get_data(cert_entry);
	auto strlengh = ASN1_STRING_to_UTF8(&CN, entryData);
	printf("%s\n",CN);
	std::string commonname((char*)CN, strlengh);
	m_local_addr.reset(new proto::av_address(av_address_from_string(commonname)));
	OPENSSL_free(CN);
}

// av地址可以从证书里获取，所以外面无需传递进来
avjackif::avjackif(std::shared_ptr<boost::asio::ip::tcp::socket> _sock)
	: m_sock(_sock)
{
	static unsigned t = 0;
	m_ifname = boost::str(boost::format("avjack%d") % t++);
}

avjackif::~avjackif()
{
}

bool avjackif::async_handshake(boost::asio::yield_context yield_context)
{
	uint32_t hostl, netl;
	std::string  buf;

	auto random_pub_key = async_client_hello(yield_context);

	// 接着私钥加密 随机数
	auto singned = RSA_private_encrypt(_rsa.get(), random_pub_key);

	proto::login login_packet;
	login_packet.set_user_cert(i2d_X509(_x509.get()));
	login_packet.set_encryped_radom_key(singned);

	boost::asio::async_write(*m_sock, boost::asio::buffer(av_proto::encode(login_packet)),
		yield_context);

	// 读取回应并解码
	std::unique_ptr<proto::login_result> login_result(
		(proto::login_result*)async_read_protobuf_message(*m_sock, yield_context));

	return login_result.get()->result() == proto::login_result::LOGIN_SUCCEED;
}

std::string avjackif::async_client_hello(boost::asio::yield_context yield_context)
{
	proto::client_hello client_hello;
	client_hello.set_client("avim");
	client_hello.set_version(0001);

	unsigned char to[512];

	auto dh = DH_new();
	DH_generate_parameters_ex(dh,64,DH_GENERATOR_5,NULL);
	DH_generate_key(dh);

	// 把 g,p, pubkey 传过去
	client_hello.set_random_g((const void*)to, BN_bn2bin(dh->g, to));
	client_hello.set_random_p((const void*)to, BN_bn2bin(dh->p, to));
	client_hello.set_random_pub_key((const void*)to, BN_bn2bin(dh->pub_key, to));

	auto tobesend = av_proto::encode(client_hello);

	boost::asio::async_write(*m_sock, boost::asio::buffer(tobesend), yield_context);

	// 解码
	std::unique_ptr<proto::server_hello> server_hello(
		(proto::server_hello*)async_read_protobuf_message(*m_sock, yield_context));

	m_remote_addr.reset(new proto::av_address(
		av_address_from_string(server_hello->server_av_address())));

	auto server_pubkey = BN_bin2bn((const unsigned char *) server_hello->random_pub_key().data(),
		server_hello->random_pub_key().length(), NULL);

	m_shared_key.resize(DH_size(dh));
	// 密钥就算出来啦！
	DH_compute_key(&m_shared_key[0], server_pubkey, dh);
	BN_free(server_pubkey);

    std::printf("key = 0x");
    for (int i=0; i<DH_size(dh); ++i)
	{
        std::printf("%x%x", (m_shared_key[i] >> 4) & 0xf, m_shared_key[i] & 0xf);
    }
    std::printf("\n");
	DH_free(dh);

	return server_hello->random_pub_key();
}

static inline int X509_NAME_add_entry_by_NID(X509_NAME *subj, int nid, std::string value)
{
	return X509_NAME_add_entry_by_NID(subj, nid, MBSTRING_UTF8, (unsigned char*) value.data(), -1, -1 , 0);
}

bool avjackif::async_register_user_check_name(std::string user_name, boost::asio::yield_context yield_context)
{
	if (m_shared_key.empty())
		async_client_hello(yield_context);

	proto::username_availability_check username_availability_check;
	username_availability_check.set_user_name(user_name);

	boost::asio::async_write(*m_sock, boost::asio::buffer(av_proto::encode(username_availability_check)), yield_context);

	std::unique_ptr<proto::username_availability_result> username_availability_result((proto::username_availability_result*)async_read_protobuf_message(*m_sock, yield_context));

	return username_availability_result->result() == proto::username_availability_result::NAME_AVAILABLE;
}

bool avjackif::async_register_new_user(std::string user_name, boost::asio::yield_context yield_context)
{
	// 先发 client_hello
	if( m_shared_key.empty())
		async_client_hello(yield_context);

	auto digest = EVP_sha1();

	// 先生成 RSA 密钥
	_rsa.reset(RSA_generate_key(2048, 65537, 0, 0), RSA_free);

	// 然后生成 CSR
	boost::shared_ptr<X509_REQ> csr(X509_REQ_new(), X509_REQ_free);

	boost::shared_ptr<EVP_PKEY> pkey(EVP_PKEY_new(), EVP_PKEY_free);
	EVP_PKEY_set1_RSA(pkey.get(), _rsa.get());

	// 添加证书申请信息

	auto subj =X509_REQ_get_subject_name(csr.get());
/*	X509_NAME_add_entry_by_NID(subj, NID_countryName, "CN");
	X509_NAME_add_entry_by_NID(subj, NID_stateOrProvinceName, "Shanghai");
	X509_NAME_add_entry_by_NID(subj, NID_localityName, "Shanghai");
	X509_NAME_add_entry_by_NID(subj, NID_organizationName, "avplayer");
	X509_NAME_add_entry_by_NID(subj, NID_organizationalUnitName, "sales");
*/	X509_NAME_add_entry_by_NID(subj, NID_commonName, user_name);
//	X509_NAME_add_entry_by_NID(subj, NID_pkcs9_emailAddress, "test-client");

	X509_REQ_set_pubkey(csr.get(), pkey.get());

	// 签出 CSR
	X509_REQ_sign(csr.get(), pkey.get(), digest);

	unsigned char * out = NULL;
	auto csr_out_len = i2d_X509_REQ(csr.get(), &out);

	std::string csrout((char*)out, csr_out_len);

	OPENSSL_free(out);
	out = NULL;
	auto rsa_key_out_len = i2d_RSA_PUBKEY(_rsa.get(), &out);

	std::string rsa_key((char*)out, rsa_key_out_len);
	OPENSSL_free(out);

	PEM_write_X509_REQ(stderr, csr.get());

	// 然后发送 注册信息
	proto::user_register user_register;

	user_register.set_user_name(user_name);
	user_register.set_rsa_pubkey(rsa_key);
	user_register.set_csr(csrout);

	boost::asio::async_write(*m_sock, boost::asio::buffer(av_proto::encode(user_register)), yield_context);

	// 读取应答
	std::unique_ptr<proto::user_register_result> user_register_result((proto::user_register_result*)async_read_protobuf_message(*m_sock, yield_context));

	return user_register_result->result() == proto::user_register_result::REGISTER_SUCCEED;
}

boost::asio::io_service& avjackif::get_io_service() const
{
	return m_sock->get_io_service();
}

std::string avjackif::get_ifname() const
{
	return m_ifname;
}

const proto::av_address* avjackif::if_address() const
{
	return m_local_addr.get();
}

const proto::av_address* avjackif::remote_address() const
{
	return m_remote_addr.get();
}

RSA* avjackif::get_rsa_key()
{
	return _rsa.get();
}

X509* avjackif::get_cert()
{
	return _x509.get();
}

void avjackif::notify_remove()
{
	signal_notify_remove();
}

boost::shared_ptr<proto::avpacket> avjackif::async_read_packet(boost::asio::yield_context yield_context)
{
	boost::system::error_code ec;
	std::string buf;
	std::uint32_t l;
	if( boost::asio::async_read(*m_sock, boost::asio::buffer(&l, sizeof(l)), yield_context[ec]) != 4)
		return boost::shared_ptr<proto::avpacket>();

	auto hostl = htonl(l);
	buf.resize(htonl(l) + 4);
	memcpy(&buf[0], &l, 4);
	boost::asio::async_read(*m_sock, boost::asio::buffer(&buf[4], htonl(l)),
		boost::asio::transfer_exactly(hostl), yield_context[ec]);
	if(ec)
		return boost::shared_ptr<proto::avpacket>();

	return boost::shared_ptr<proto::avpacket>(dynamic_cast<proto::avpacket*>(av_proto::decode(buf)));
}

bool avjackif::async_write_packet(proto::avpacket* pkt, boost::asio::yield_context yield_context)
{
	boost::system::error_code ec;
	boost::asio::async_write(*m_sock, boost::asio::buffer(av_proto::encode(*pkt)), yield_context[ec]);
	return !ec;
}

