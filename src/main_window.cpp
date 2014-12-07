
#include <memory>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include <boost/format.hpp>

#include <QDebug>
#include <QMainWindow>
#include <QListWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

#include "main_window.hpp"
#include "ui_main_window.h"

main_window::main_window(QAbstractItemModel* buddy, QAbstractItemModel* group, QAbstractItemModel* recent)
{
	m_ui.reset(new Ui_main_window);
	m_ui->setupUi(this);

	m_ui->buddylistView->setModel(buddy);
	m_ui->grouplistView->setModel(group);
	m_ui->recentlistView->setModel(recent);

	m_ui->buddylistView->setEnabled(false);
	m_ui->grouplistView->setEnabled(false);

	connect(m_ui->buddylistView, &QListView::doubleClicked, [this, buddy](const QModelIndex & index)
	{
		Q_EMIT chat_opened(buddy->data(index, Qt::UserRole).toString().toStdString(), false);
	});

	connect(m_ui->grouplistView, &QListView::doubleClicked, [this, group](const QModelIndex & index)
	{
		Q_EMIT chat_opened(group->data(index, Qt::UserRole).toString().toStdString(), true);
	});

	QObject::connect(m_ui->add_buddyButton, &QAbstractButton::clicked, [this, buddy](bool checked)
	{
		int row = buddy->rowCount();
		buddy->insertRow(buddy->rowCount());
		QModelIndex index = buddy->index(row, 0);
		buddy->setData(index, m_ui->buddy_lineEdit->text());
		m_ui->buddy_lineEdit->clear();
	});
	QObject::connect(m_ui->add_groupButton, &QAbstractButton::clicked, [this, group](bool checked)
	{
		int row = group->rowCount();
		group->insertRow(group->rowCount());
		QModelIndex index = group->index(row, 0);
		group->setData(index, m_ui->add_groupButton->text());
		m_ui->group_lineEdit->clear();
	});
}

void main_window::closeEvent(QCloseEvent*e)
{
	Q_EMIT close_requested(e);
}

void main_window::on_login_success()
{
	// TODO 更新 GUI 向用户显示
	m_ui->buddylistView->setEnabled(true);
	m_ui->grouplistView->setEnabled(true);
}

void main_window::on_lost_connection(int reason)
{
	// TODO 更新 GUI 报告用户
}
