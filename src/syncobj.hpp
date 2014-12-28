
#pragma once

#include <QObject>
#include <boost/concept_check.hpp>
#include <functional>

void post_on_gui_thread(std::function<void()>);

class SyncObjec : public QObject
{
	Q_OBJECT

	void do_post(std::function<void()> func);

private Q_SLOTS:
	void on_post(std::function<void()> qfunc_ptr){(qfunc_ptr)();}

Q_SIGNALS:
	void post(std::function<void()>);

public:
	SyncObjec();

	friend void post_on_gui_thread(std::function<void()>);

};
