
#include "system_tray.hpp"

avim_system_tray::avim_system_tray(const QIcon& ico)
	: QSystemTrayIcon(ico)
{
	m_context_menu.reset(new QMenu());
	auto action = m_context_menu->addAction(QStringLiteral("退出(&Q)"));

	QObject::connect(action, &QAction::triggered, this, &avim_system_tray::menu_request_quit);
	setContextMenu(m_context_menu.get());
}
