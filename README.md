avim
====

IM for avplayer

avim　是一个完全全新设计的IM协议，同时也是一个同名的C++实现（reference implementation）。
#　目标
目标是提供一个开源的IM实现，并且能实现文字聊天，图片聊天，语音聊天，视频聊天。这是同样开源的IRC和XMPP无法实现的。

# avim 总架构:

         avim-client(qt based, ncurser based, etc)
               |
    +----------+---------+     +----------+---------+
    |     avim-router ---+-----+----- avim-router   |
    +--------------------+     +--------------------+
										  |
										  |
								+---------+----------+
								|      avim-bot      |
								|      |      |      |
								|  avim-db  avim-log |
								+--------------------+


* avim-router 为核心协议实现， libavproto.so / libavproto.dll 提供多种语言的 wrapper 接口
* avim-bot 为聊天机器人，用来实现群聊
* avim-db 为用户信息数据库，为独立进程。可以不必跑在相同服务器上。
* avim-log 为聊天日志记录数据库，为独立进程。可以不必跑在相同服务器上。

* avim-client 为客户端，avplayer.org 计划提供基于Qt的跨平台客户端，以及　pidgin 插件。

##  计划使用的库

protobuf, boost, Qt(客户端only)

### protobuf
	protobuf 从这个地方下载

	https://github.com/google/protobuf/releases

### boost

	boost 从这个地方下载

	http://www.boost.org/users/history/version_1_57_0.html
	
### Build
``` bash
$ git clone https://github.com/avplayer/avim.git
$ cd avim && mkdir build && cmake ..
$ make -j8  # 默认带GUI; 如不需编译GUI的, 可以用 $ cmake .. -DBUILD_GUI=OFF
```

详细的编译说明

[windows 编译说明](https://github.com/avplayer/avim/wiki/Windows-%E7%BC%96%E8%AF%91%E8%AF%B4%E6%98%8E)


## 协议
协议本身将进入　public domain，允许任何人自行实现。
avplayer.org社区提供的avim协议实现avim，将采用　LGPL 许可证，以便允许商业二次开发。

# 优势
avim 作为一个开源的IM实现，同时包含了客户端和服务器端。
avplayer.org 本身会运营一个实例，用于社区成员之间的沟通。
但是，avim允许二次开发，允许第三方运行avim提供功能，
而且不同avim服务提供商之间的用户还可以相互通信。这样就把　avim变成了一个分布式的IM网络了。
任何人都可以搭建自己的服务器和聊天室。同时允许二次开发，会有更丰富的第三方应用出现。





