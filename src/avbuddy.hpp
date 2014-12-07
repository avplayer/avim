#pragma once

#include <address.pb.h>
#include <avproto.hpp>

struct avbuddy
{
	avbuddy();

	avbuddy(const char addr[]);
	avbuddy(const std::string& addr);
	avbuddy(const avbuddy& other);
	avbuddy(avbuddy&& other);

	avbuddy& operator=(const avbuddy&);
	avbuddy& operator=(avbuddy&&);

	proto::av_address av_addr;
	std::vector<std::string> labels;
};
