
#ifdef __cplusplus
extern "C" {
#endif

/*
 * 启动 av 协议核心，一旦调用，那么 av 核心就启动起来了，等待 av 协议的处理
 *
 */
void av_start();

// 添加 avif 接口，用于 av_route 操作
avif * av_if_handover(avif avinterface);

/*
 * 配置 av 协议地址，类似 ifconfig 的作用。
 *
 * 传递一个 av客户端密钥和证书
 *
 * 所谓私钥啊，就是一个 RSA private key
 * 证书就是一个 RSA pubkey 和一个 avplayer.org 这个  ROOT CA 的签名
 */
bool av_config(std::string pkey, std::string  cert);

// 配置av路由表 允许使用 * 这样的地址设定默认路由
bool av_route(av_route_op op /*add mod del*/, std::string addr, std::string gateway, std::string interfacename);

// 发送数据， av层会自动调用 RSA private key 加密数据
bool av_sendto(std::string dest_address, std::string message);

// 接收数据
bool av_recvfrom(std::string dest_address, std::string message);

#ifdef __cplusplus
}
#endif
