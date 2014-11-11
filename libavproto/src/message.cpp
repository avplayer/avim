
#include "message.hpp"

#include "serialization.hpp"

proto::avim_message_packet decode_message(const std::string& payload)
{
	proto::avim_message_packet ret;
	// payload 的第一个字节表示消息是否加密, 有的话, 返回失败, 必须使用对称加密的密钥解开
	char type = payload[0];

	switch(type & 0b1000000)
	{
		throw im_decode_error(0, "encrypted message");
	}

	if(!ret.ParseFromArray(payload.data()+1, payload.length()-1))
	{
		throw im_decode_error(1, "protobuf decode error");
	}
	return ret;
}

std::string encode_message(const proto::avim_message_packet& pkt)
{
	std::string ret;
	ret.push_back((char)0b1000000);


	if(!pkt.AppendToString(&ret))
	{
		throw im_decode_error(2, "protobuf decode error");
	}
	return ret;
}


