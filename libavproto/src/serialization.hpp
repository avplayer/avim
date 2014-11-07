//
// Copyright (C) 2013 Jack.
//
// Author: jack
// Email:  jack.wgm at gmail dot com
//

#pragma once

#include <string>

#include <boost/assert.hpp>
#include <boost/cstdint.hpp>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#if defined (WIN32) || defined (_WIN32)
#	include <winsock2.h>	// htonl, ntohl
#else
#	include <arpa/inet.h>	// htonl, ntohl
#endif

#include "logging.hpp"

namespace av_router {

	// 创建type_name指定的名字的Message对象.
	inline google::protobuf::Message* create_message(const std::string& type_name)
	{
		google::protobuf::Message* message = NULL;
		const google::protobuf::Descriptor* descriptor =
			google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
		if (descriptor)
		{
			const google::protobuf::Message* prototype =
				google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
			if (prototype)
				message = prototype->New();
		}
		return message;
	}

	// 序列化消息.
	inline std::string encode(const google::protobuf::Message& message)
	{
		std::string result;
		const std::string& type_name = message.GetTypeName();
		boost::int32_t len = static_cast<const int32_t>(type_name.size());
		boost::int32_t be_len = htonl(len);
		static const int header_len = sizeof(boost::int32_t);
		result.resize(header_len);										// 1. resize space for packet length.
		result.append(reinterpret_cast<char*>(&be_len), sizeof be_len);	// 2. append type name length.
		result.append(type_name.c_str(), len);							// 3. append type name.
		bool succeed = message.AppendToString(&result);					// 4. append message.
		if (succeed)
		{
			len = htonl(static_cast<int>(result.size()) - header_len);
			std::copy(reinterpret_cast<char*>(&len),
				reinterpret_cast<char*>(&len) + sizeof(len), result.begin());	// 5. back to fill packet length.
		}
		else
		{
			BOOST_ASSERT("serialization protobuf failed!" && false);
			LOG_ERR << "serialization protobuf failed: " << message.GetTypeName();
			result.clear();
		}
		return result;
	}

	// 反序列化消息.
	inline google::protobuf::Message* decode(const std::string& buf)
	{
		static const int header_len = sizeof(boost::int32_t);
		boost::int32_t len = static_cast<boost::int32_t>(buf.size());
		const char* ptr = buf.data();
		if (len != ntohl(*(boost::int32_t*)(ptr)) + header_len)							// verfiy packet length.
		{
			BOOST_ASSERT("decode protobuf failed!" && false);
			return nullptr;
		}
		const boost::int32_t name_len = ntohl(*(boost::int32_t*)(ptr + header_len));	// type name length.
		if (name_len >= len - 2 * header_len)
		{
			BOOST_ASSERT("decode protobuf failed, type name length too big!" && false);
			return nullptr;
		}
		const char* name_begin = ptr + 2 * header_len;
		const char* name_end = name_begin + name_len;
		std::string type_name(name_begin, name_end);									// type name.
		google::protobuf::Message* result = create_message(type_name);
		if (!result)
		{
			LOG_ERR << "create message failed: " << type_name;
			return result;
		}
		const char* data = name_end;
		const int data_len = len - (name_len + 2 * header_len);
		if (!result->ParseFromArray(data, data_len))										// parse protobuf message.
		{
			LOG_ERR << "parse message failed!";
			delete result;
			return nullptr;
		}
		return result;
	}
}
