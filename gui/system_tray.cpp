
#include "system_tray.hpp"

avim_system_tray::avim_system_tray()
	: QSystemTrayIcon()
{
#ifndef _WIN32
	setIcon(QIcon(":/avim/logo.svg"));
#else
	HICON hicon = (HICON)::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(1),
		IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_LOADTRANSPARENT);
	setIcon(QIcon(QPixmap::fromWinHICON(hicon)));
	::DestroyIcon(hIcon);
#endif
	m_context_menu.reset(new QMenu());
	auto action = m_context_menu->addAction("退出(&Q)");

	QObject::connect(action, &QAction::triggered, this, &avim_system_tray::menu_request_quit);
	setContextMenu(m_context_menu.get());
}
