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

struct im_message{
	bool is_encrypted_message;
	bool is_group_message;
	bool is_control_message;
	bool is_message;

	std::string sender;

	message::message_packet impkt;
};


// 检测是否经过了对称密钥的加密
bool is_encrypted_message(const std::string& payload);

im_message decode_message(const std::string& payload);

// 解码用的 key 是个 base64 编码的字符串. 加密类型和加密密钥都在里面. 这个 key 字符串由管理员在你进群的时候发送过来
im_message decode_message(const std::string& encryption_key, const std::string& payload);

std::string encode_message(const message::message_packet&);
std::string encode_message(const std::string& encryption_key, const message::message_packet&);

// 序列化群消息以便 avkernel.send 使用
std::string encode_group_message(const std::string& sender, const std::string& encryption_key, const message::message_packet&);

// 序列化控制消息, 以便 avkernel.send 使用
std::string encode_control_message(const std::string& sender, const message::control_message&);
