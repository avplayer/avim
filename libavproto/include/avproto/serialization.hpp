//
// Copyright (C) 2013 Jack.
//
// Author: jack
// Email:  jack.wgm at gmail dot com
//

#pragma once

#include <string>
#include <google/protobuf/message.h>

namespace av_proto {
	// 序列化消息.
	std::string encode(const google::protobuf::Message& message);
	// 反序列化消息.
	google::protobuf::Message* decode(const std::string& buf);
}
