
#ifdef _MSC_VER
#pragma comment(lib,"avproto.lib")
#endif

#if __GNUC__ >= 4
#  if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) && !defined(__CYGWIN__)
     // All Win32 development environments, including 64-bit Windows and MinGW, define
     // _WIN32 or one of its variant spellings. Note that Cygwin is a POSIX environment,
     // so does not define _WIN32 or its variants.
#    define AV_HAS_DECLSPEC
#    define AV_SYMBOL_EXPORT __attribute__((__dllexport__))
#    define AV_SYMBOL_IMPORT __attribute__((__dllimport__))
#  else
#    define AV_SYMBOL_EXPORT __attribute__((__visibility__("default")))
#    define AV_SYMBOL_IMPORT
#  endif
#  define AV_SYMBOL_VISIBLE __attribute__((__visibility__("default")))
#else
#  define AV_SYMBOL_EXPORT
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
	#define AVPROTO_API AV_SYMBOL_EXPORT
#endif

#ifdef BUILD_STATIC
#define AVPROTO_API
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
