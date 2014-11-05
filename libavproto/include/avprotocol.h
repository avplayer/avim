
#ifdef _MSC_VER
#pragma comment(lib,"avproto.lib")
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
	#ifdef DLL_EXPORTS
	#define AVPROTO_API    __declspec(dllexport)
	#else
	#define AVPROTO_API    __declspec(dllimport)
	#endif
#else
	#define AVPROTO_API extern
#endif

/*
 * 启动 av 协议核心，一旦调用，那么 av 核心就启动起来了，等待 av 协议的处理
 */
AVPROTO_API void av_start();

/*
 * 停止核心
 */
AVPROTO_API void av_stop();

// port = NULL 表示使用默认端口
AVPROTO_API int connect_to_avrouter(const char * key, const char * cert, const char * self_addr, const char * host, const char * port);

// 发送数据， av层会自动调用 RSA private key 加密数据
AVPROTO_API int av_sendto(const char * dest_address, const char * message, int len);

// 接收数据
AVPROTO_API int av_recvfrom(char * dest_address, char * message, int & len);

#ifdef __cplusplus
}
#endif
