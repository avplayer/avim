
#pragma once

#include <queue>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/atomic.hpp>
#include <boost/make_shared.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

// #include "protocol/avim-base.pb.h"

#include <openssl/rsa.h>
#include <boost/regex.hpp>

namespace boost {
	template<typename ListType> class async_coro_queue;
}

namespace proto{
	namespace base{
		class avPacket;
		class avAddress;
	}
}

namespace detail {

	struct avif_implement_interface{

		virtual ~avif_implement_interface(){};

		virtual boost::asio::io_service & get_io_service() const = 0;

		virtual std::string get_ifname() const = 0 ;

		virtual const proto::base::avAddress * if_address() const = 0;
		virtual const proto::base::avAddress * remote_address() const = 0 ;

		virtual RSA * get_rsa_key() = 0;
		virtual X509 * get_cert() = 0;

		// 读取 av数据包
		virtual boost::shared_ptr<proto::base::avPacket> async_read_packet(boost::asio::yield_context yield_context) = 0;

		// 发送 av数据包
		virtual bool async_write_packet(proto::base::avPacket*, boost::asio::yield_context yield_context) = 0;
	};

	template<class RealImpl>
	struct avif_implement_wrapper : public avif_implement_interface
	{
		boost::asio::io_service & get_io_service() const
		{
			return _impl->get_io_service();
		}

		std::string get_ifname() const
		{
			return _impl->get_ifname();
		};

		const proto::base::avAddress * if_address() const
		{
			return _impl->if_address();
		}

		const proto::base::avAddress * remote_address() const
		{
			return _impl->remote_address();
		}

		RSA * get_rsa_key()
		{
			return _impl->get_rsa_key();
		}


		X509 * get_cert()
		{
			return _impl->get_cert();
		}
		// 读取 av数据包
		boost::shared_ptr<proto::base::avPacket> async_read_packet(boost::asio::yield_context yield_context)
		{
			return _impl->async_read_packet(yield_context);
		}

		// 发送 av数据包
		bool async_write_packet(proto::base::avPacket* pkt, boost::asio::yield_context yield_context)
		{
			return _impl->async_write_packet(pkt, yield_context);
		}

		avif_implement_wrapper(boost::shared_ptr<RealImpl> other)
		{
			_impl = other;
		}

	private:
		boost::shared_ptr<RealImpl> _impl;
	};

}

// 一个接口类， av核心用这个类来对外数据沟通，类似 linux 内核里的 sbk_buf
struct avif
{
	boost::asio::io_service & get_io_service() const
	{
		return _impl->get_io_service();
	}

	std::string get_ifname() const
	{
		return _impl->get_ifname();
	};

	const proto::base::avAddress * if_address() const
	{
		return _impl->if_address();
	}

	const proto::base::avAddress * remote_address() const
	{
		return _impl->remote_address();
	}

	RSA * get_rsa_key()
	{
		return _impl->get_rsa_key();
	}

	X509 * get_cert()
	{
		return _impl->get_cert();
	}

	// 读取 av数据包
	boost::shared_ptr<proto::base::avPacket> async_read_packet(boost::asio::yield_context yield_context);

	// 发送 av数据包
	bool async_write_packet(proto::base::avPacket* pkt, boost::asio::yield_context yield_context);

	template<class AV_IF_IMPL>
	avif(boost::shared_ptr<AV_IF_IMPL> impl)
	{
		_impl.reset( new detail::avif_implement_wrapper<AV_IF_IMPL>(impl) );
		construct();
	}

	avif(const avif &other)
	{
		quitting = other.quitting;
		_impl = other._impl;
		_write_queue = other._write_queue;
	}

	avif(avif &&other)
	{
		quitting = other.quitting;
		_impl = other._impl;
		_write_queue = other._write_queue;
	}

	boost::shared_ptr< boost::atomic<bool> > quitting;

	typedef boost::shared_ptr<proto::base::avPacket> auto_avPacketPtr;

	boost::shared_ptr<
		boost::async_coro_queue<
			std::queue<
				std::pair<
					auto_avPacketPtr, boost::function<void(boost::system::error_code)>
				>
			>
		>
	> _write_queue;
private:

	void construct();

	boost::shared_ptr<detail::avif_implement_interface> _impl;
};

proto::base::avAddress av_address_from_string(std::string av_address);
std::string av_address_to_string(const proto::base::avAddress & addr);
