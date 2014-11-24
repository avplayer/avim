#pragma once

#include <memory>
#include <boost/noncopyable.hpp>

#include <QObject>

class avjackif;
class AVConnection :  public QObject,  boost::noncopyable
{
    Q_OBJECT

public:
    AVConnection();

private:

	std::shared_ptr<avjackif> m_avif;
};
