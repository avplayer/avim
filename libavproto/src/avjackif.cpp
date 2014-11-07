
#include "serialization.hpp"

#include "avjackif.hpp"


/*
 * 这个类呢，是用来实现和 JACK 实现的那个 AVROUTER 对接的。也就是 JACK 版本的 AV NETWORK SERVICE PROVIDER
 */

// av地址可以从证书里获取，所以外面无需传递进来
avjackif::avjackif(boost::asio::io_service & _io_service,
	std::string login_username, std::string login_password, RSA * _key, X509 *)
{

}
