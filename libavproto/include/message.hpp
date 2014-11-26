#pragma once
/*
 * 这个文件决定了 avpakcet->payload 里面的数据的解释格式
 */
#include <string>
#include <exception>
#include "avim_proto/im.pb.h"

struct im_decode_error : std::runtime_error
{
    explicit im_decode_error(int t, const std::string& __arg)
		: std::runtime_error(__arg), error_code(t){}

	int error_code;
};

// 检测是否经过了对称密钥的加密
bool is_encrypted_message(const std::string& payload);

proto::avim_message_packet decode_message(const std::string& payload);

// 解码用的 key 是个 base64 编码的字符串. 加密类型和加密密钥都在里面. 这个 key 字符串由管理员在你进群的时候发送过来
proto::avim_message_packet decode_message(const std::string& encryption_key, const std::string& payload);

std::string encode_message(const proto::avim_message_packet&);
std::string encode_message(const std::string& encryption_key, const proto::avim_message_packet&);
