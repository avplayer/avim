说明：
protobuf 2.x版本:仅支持armeabi-v7a,armeabi,mips,x86,x86_64，不支持arm64-v8a,mips64
具体见\protobuf\src\google\protobuf\stubs下的platform_macros.h，实践证明也确实如此
protobuf3.x版本都支持（Application.mk选择APP_ABI := all时都可以编译成功）,目前最新版本是v3.0.0-alpha-1

1、如何编译？
安装NDK环境，我的NDK版本是：android-ndk-r10c
切换到jni目录，执行:ndk-build即可

2、libprotobuf-cpp-lite 和 libprotobuf-cpp-full的区别？
libprotobuf-cpp-lite 是轻量版，如果只是用protobuf的封包/解包等简单操作的话可以使用这个版本
libprotobuf-cpp-full 是完整版，包含了反射等特性，该版本依赖于libprotobuf-cpp-lite
