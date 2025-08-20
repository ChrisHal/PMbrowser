#include <QString>
#include "PMparametersModel.h"

PMparametersModel::PMparametersModel(QSpan<hkLib::PMparameter> p, QObject* parent)
	: QAbstractTableModel(parent), parameters(p)
{}

PMparametersModel::~PMparametersModel()
{}

int PMparametersModel::rowCount(const QModelIndex & parent) const
{
	return parameters.size();
}

int PMparametersModel::columnCount(const QModelIndex& parent) const
{
	return 2;
}

QVariant PMparametersModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    int col = index.column();
    const auto& p = parameters[row];
    switch (role) {
    case Qt::CheckStateRole:
        if (col == 0) {
            return p.exportIBW ? Qt::Checked : Qt::Unchecked;
        }
        else if (col == 1) {
            return p.print ? Qt::Checked : Qt::Unchecked;
        }
        break;
    //case Qt::DisplayRole:
    //    return QString::fromUtf8(lables.at(col));
    //    break;
    }
    return QVariant();
}

QVariant PMparametersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Vertical) {
        return QString::fromUtf8(parameters[section].name);
    }
    else if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return QString::fromUtf8(lables.at(section));
    }
    return QVariant();
}

bool PMparametersModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::CheckStateRole) {
        if (!checkIndex(index))
            return false;
        auto& p = parameters[index.row()];
        auto state = value.value<Qt::CheckState>();
        if (index.column() == 0) {
            p.exportIBW = state != Qt::Unchecked;
        } else if (index.column() == 1) {
            p.print = state != Qt::Unchecked;
        }
        return true;
    }
    return false;
}

Qt::ItemFlags PMparametersModel::flags(const QModelIndex& index) const
{
    return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
}

