#pragma once

#include <memory>
#include <QSystemTrayIcon>
#include <QMenu>

class avim_system_tray : public QSystemTrayIcon
{
	Q_OBJECT
Q_SIGNALS:
	void menu_request_quit();
public:
	avim_system_tray();
private:
	std::unique_ptr<QMenu> m_context_menu;
};
