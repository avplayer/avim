
#include <memory>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include <QMainWindow>
#include <QListWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

#include "main_window.hpp"
#include "ui_main_window.h"

main_window::main_window()
{
	m_ui.reset(new Ui_main_window);
	m_ui->setupUi(this);

	m_list = m_ui->buddylistWidget;

	m_list->addItem("test-client@avplayer.org");
	m_list->addItem("microcai@avplayer.org");
	m_list->addItem("michael.fan@avplayer.org");

	m_ui->grouplistWidget->addItem("group@avplayer.org");

	m_list->setEnabled(false);
	m_ui->grouplistWidget->setEnabled(false);

	connect(m_list, &QListWidget::itemDoubleClicked, [this] (QListWidgetItem * item)
	{
		Q_EMIT chat_opened(item->text().toStdString());
	});

	connect(m_ui->grouplistWidget, &QListWidget::itemDoubleClicked, [this] (QListWidgetItem * item)
	{
		Q_EMIT chat_opened(item->text().toStdString());
	});

	QObject::connect(m_ui->add_buddyButton, &QAbstractButton::clicked, [this](bool checked)
	{
		m_list->addItem(m_ui->buddy_lineEdit->text());
		m_ui->buddy_lineEdit->clear();
	});
	QObject::connect(m_ui->add_groupButton, &QAbstractButton::clicked, [this](bool checked)
	{
		m_list->addItem(m_ui->group_lineEdit->text());
		m_ui->group_lineEdit->clear();
	});
}

void main_window::on_login_success()
{
	// TODO 更新 GUI 向用户显示
	m_list->setEnabled(true);
}

void main_window::on_lost_connection(int reason)
{
	// TODO 更新 GUI 报告用户
}
