#include "buddymodel.hpp"
#include <boost/concept_check.hpp>

BuddyModel::BuddyModel(std::vector<avbuddy>* _data, QObject* parent)
	: QAbstractListModel(parent)
	, m_data(_data)
{

}

QVariant BuddyModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::DisplayRole || role == Qt::UserRole)
	{
		avbuddy buddy = (*m_data)[index.row()];
		auto addr = av_address_to_string(buddy.av_addr);
		return QString::fromStdString(addr);
	}
	return QVariant();
}

int BuddyModel::rowCount(const QModelIndex& parent) const
{
	int count = m_data->size();
	return count;
}

bool BuddyModel::insertRows(int row, int count, const QModelIndex& parent)
{
	beginInsertRows(parent, row, row + count);
	m_data->resize(row + count);
	endInsertRows();
	return true;
}

bool BuddyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	auto row = index.row();
	(*m_data)[row] = avbuddy(value.toString().toStdString());
	dataChanged(index, index);
	return true;
}


