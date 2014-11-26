
#pragma once

#include <queue>
#include <atomic>

#include <boost/format.hpp>
#include <boost/function.hpp>
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
	class avpacket;
	class av_address;
}

namespace detail {

	struct avif_implement_interface{

		virtual ~avif_implement_interface(){};

		virtual boost::asio::io_service & get_io_service() const = 0;

		virtual std::string get_ifname() const = 0 ;

		virtual const proto::av_address * if_address() const = 0;
		virtual const proto::av_address * remote_address() const = 0 ;

		virtual RSA * get_rsa_key() = 0;
		virtual X509 * get_cert() = 0;
		virtual void notify_remove() = 0;

		// 读取 av数据包
		virtual boost::shared_ptr<proto::avpacket> async_read_packet(boost::asio::yield_context yield_context) = 0;
		// 发送 av数据包
		virtual bool async_write_packet(proto::avpacket*, boost::asio::yield_context yield_context) = 0;
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

		const proto::av_address * if_address() const
		{
			return _impl->if_address();
		}

		const proto::av_address * remote_address() const
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

		void notify_remove()
		{
			return _impl->notify_remove();
		}

		// 读取 av数据包
		boost::shared_ptr<proto::avpacket> async_read_packet(boost::asio::yield_context yield_context)
		{
			return _impl->async_read_packet(yield_context);
		}

		// 发送 av数据包
		bool async_write_packet(proto::avpacket* pkt, boost::asio::yield_context yield_context)
		{
			return _impl->async_write_packet(pkt, yield_context);
		}

		avif_implement_wrapper(std::shared_ptr<RealImpl> other)
		{
			_impl = other;
		}

	private:
		std::shared_ptr<RealImpl> _impl;
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

	const proto::av_address * if_address() const
	{
		return _impl->if_address();
	}

	const proto::av_address * remote_address() const
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

	void notify_remove()
	{
		return _impl->notify_remove();
	}

	// 读取 av数据包
	boost::shared_ptr<proto::avpacket> async_read_packet(boost::asio::yield_context yield_context);

	// 发送 av数据包
	bool async_write_packet(proto::avpacket* pkt, boost::asio::yield_context yield_context);

	template<class AV_IF_IMPL>
	avif(std::shared_ptr<AV_IF_IMPL> impl)
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

	boost::shared_ptr< std::atomic<bool> > quitting;

	typedef boost::shared_ptr<proto::avpacket> auto_avPacketPtr;

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

	std::shared_ptr<detail::avif_implement_interface> _impl;
};

proto::av_address av_address_from_string(std::string av_address);
std::string av_address_to_string(const proto::av_address & addr);
