
#include "system_tray.hpp"

avim_system_tray::avim_system_tray()
	: QSystemTrayIcon()
{
	setIcon(QIcon(":/avim/logo.svg"));

	m_context_menu.reset(new QMenu());
	m_context_menu->addAction("退出(&Q)");

	setContextMenu(m_context_menu.get());
}
