#include "avbuddy.hpp"

avbuddy::avbuddy() {}

avbuddy::avbuddy(const std::string& addr)
{
    av_addr = av_address_from_string(addr);
}

avbuddy::avbuddy(const avbuddy& other)
{
    av_addr = other.av_addr;
    labels = other.labels;
}

avbuddy::avbuddy(const char addr[])
{
    av_addr = av_address_from_string(addr);
}

avbuddy& avbuddy::operator=(const avbuddy& other)
{
    av_addr = other.av_addr;
    labels = other.labels;
}

avbuddy& avbuddy::operator=(avbuddy&& other)
{
    av_addr = other.av_addr;
	labels = other.labels;
}

avbuddy::avbuddy(avbuddy&& other)
{
    av_addr = other.av_addr;
    labels = other.labels;
}
