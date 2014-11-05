
#ifdef __cplusplus
extern "C" {
#endif

/*
 * 启动 av 协议核心，一旦调用，那么 av 核心就启动起来了，等待 av 协议的处理
 */
void av_start();

/*
 * 停止核心
 */
void av_stop();

// port = NULL 表示使用默认端口
int connect_to_avrouter(const char * key, const char * cert, const char * self_addr, const char * host, const char * port);

// 发送数据， av层会自动调用 RSA private key 加密数据
int av_sendto(const char * dest_address, const char * message, int len);

// 接收数据
int av_recvfrom(char * dest_address, char * message, int & len);

#ifdef __cplusplus
}
#endif
