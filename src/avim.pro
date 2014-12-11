
QT       += core gui widgets
CXXFLAGS += "-std=c++11 -DANDROID"

INCLUDEPATH += /home/cai/android/boost_1_57_0
INCLUDEPATH += /home/cai/android/protobuf/include

LIBS += -lcrypto -lprotobuf
LIBS += -lboost_system -lboost_thread -lboost_filesystem -lboost_coroutine -lboost_thread -lboost_context -lboost_regex

LIBPATH += $$PWD/../android-build/libs/
LIBPATH +=  /home/cai/android/boost_1_57_0/stage/lib/

INCLUDEPATH += $$PWD/../android-build/include
INCLUDEPATH += $$PWD/../libavim/include
INCLUDEPATH += $$PWD/widgets/chatwidget/
INCLUDEPATH += $$PWD/widgets/qrichtext/

TARGET = avim
TEMPLATE = app subdirs

SOURCES	+= app.cpp avconnection.cpp login_dialog.cpp main_window.cpp syncobj.cpp avbuddy.cpp buddymodel.cpp main.cpp register_dialog.cpp system_tray.cpp

SOURCES += widgets/chatwidget/chatwidget.cpp
SOURCES += widgets/qrichtext/qbubbleframe.cpp widgets/qrichtext/qrichedit.cpp widgets/qrichtext/qrichtext.cpp widgets/qrichtext/qrichtextlayout.cpp

HEADERS	+= app.hpp avbuddy.hpp avconnection.hpp buddymodel.hpp login_dialog.hpp main_window.hpp register_dialog.hpp syncobj.hpp system_tray.hpp
HEADERS += widgets/chatwidget/chatwidget.hpp
HEADERS += widgets/qrichtext/qbubbleframe.hpp  widgets/qrichtext/qrichedit.hpp  widgets/qrichtext/qrichtext.hpp  widgets/qrichtext/qrichtextlayout.hpp

FORMS	+= login_dialog.ui main_window.ui register_dialog.ui
FORMS	+= widgets/chatwidget/chatwidget.ui
FORMS	+= widgets/qrichtext/chat_segment_widget.ui


CONFIG += mobility no_keywords
MOBILITY =

SOURCES += ../libavim/src/avjackif.cpp ../libavim/src/avkernel.cpp ../libavim/src/interface.cpp ../libavim/src/message.cpp ../libavim/src/root_ca.cpp ../libavim/src/serialization.cpp

SOURCES += ../build/avim_proto/address.pb.cc ../build/avim_proto/ca.pb.cc ../build/avim_proto/group.pb.cc ../build/avim_proto/im.pb.cc ../build/avim_proto/packet.pb.cc ../build/avim_proto/user.pb.cc
