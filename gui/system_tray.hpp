#pragma once

#include <memory>
#include <QSystemTrayIcon>
#include <QMenu>

class avim_system_tray : public QSystemTrayIcon
{
	Q_OBJECT
public:
	avim_system_tray();
private:
	std::unique_ptr<QMenu> m_context_menu;
};
