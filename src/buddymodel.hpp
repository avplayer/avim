#pragma once

#include <QAbstractItemModel>

#include "avbuddy.hpp"

class BuddyModel : public QAbstractListModel
{
    Q_OBJECT
public:
    BuddyModel(std::shared_ptr<std::vector<avbuddy>>, QObject *parent = 0);

public:
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
private:

	std::shared_ptr<std::vector<avbuddy>> m_data;
};
