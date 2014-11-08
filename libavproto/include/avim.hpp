#pragma once

#include <boost/asio.hpp>

// 呐, 这个才是真正真正的 avim 协议实现. 这类主要处理 IM 层协议
class avim
{
	boost::asio::io_service & io_service;

public:
	avim(boost::asio::io_service &);
};

