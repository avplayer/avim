
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

main_window::main_window(avimApp* _avimapp)
	: m_avimapp(_avimapp)
{
	setCentralWidget(new QWidget);
	auto vlayout = new QVBoxLayout(centralWidget());
	m_list = new QListWidget(centralWidget());
	vlayout->addWidget(m_list);

	m_list->addItem("test@avplayer.org");
	m_list->addItem("test-client@avplayer.org");
	m_list->addItem("microcai@avplayer.org");
	m_list->addItem("group@avplayer.org");

	m_list->setEnabled(false);

	connect(m_list, &QListWidget::itemDoubleClicked, [this] (QListWidgetItem * item)
	{
		Q_EMIT chat_opened(item->text().toStdString());
	});

	auto adder_p = new QWidget(centralWidget());
	auto editor = new QLineEdit(adder_p);
	auto bt = new QPushButton(adder_p);

	auto hlayout = new QHBoxLayout(adder_p);
	hlayout->addWidget(editor);
	hlayout->addWidget(bt);

	vlayout->addWidget(adder_p);

	bt->setText("add buddy");

	QObject::connect(bt, &QAbstractButton::clicked, [this, editor](bool checked)\
	{
		m_list->addItem(editor->text());
		editor->clear();
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
