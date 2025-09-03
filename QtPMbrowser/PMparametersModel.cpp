#include <QString>
#include <QFont>
#include "PMparametersModel.h"

int PMparametersModel::countCheckedPrint() const
{
    int count{ 0 };
    for (const auto& p : parameters) {
        if (p.print) ++count;
    }
    return count;
}

int PMparametersModel::countCheckedExport() const
{
    int count{ 0 };
    for (const auto& p : parameters) {
        if (p.exportIBW) ++count;
    }
    return count;
}

PMparametersModel::PMparametersModel(QSpan<hkLib::PMparameter> p, QObject* parent)
	: QAbstractTableModel(parent), parameters(p)
{}

PMparametersModel::~PMparametersModel()
{}

int PMparametersModel::rowCount(const QModelIndex&) const
{
	return parameters.size() + 1;
}

int PMparametersModel::columnCount(const QModelIndex&) const
{
	return 2;
}

QVariant PMparametersModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    int col = index.column();
    if (row == 0) {
        switch (role) {
        case Qt::CheckStateRole:
        {
            int N{ 0 };
            if (col == 0) {
                N = countCheckedExport();
            }
            else if (col == 1) {
                N = countCheckedPrint();
            }
            if (N == 0) {
                return Qt::Unchecked;
            }
            else if (N == parameters.size()) {
                return Qt::Checked;
            }
            else {
                return Qt::PartiallyChecked;
            }
        }
            break;
        case Qt::DisplayRole:
            return QString::fromUtf8(lables.at(col));
            break;
        case Qt::FontRole:
            QFont bf;
            bf.setBold(true);
            return bf;
            break;
        }
    }
    else {
        --row;
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
        case Qt::DisplayRole:
            return QString::fromUtf8(lables.at(col));
            break;
        }
    }
    return QVariant();
}

QVariant PMparametersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Vertical) {
        if (section == 0) {
            return QString("(All Parameters)");
        }
        else {
            return QString::fromUtf8(parameters[section - 1].name);
        }
    }
    else if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return QString::fromUtf8(lables.at(section));
    }
    else if (role == Qt::FontRole && orientation == Qt::Vertical && section == 0) {
        QFont bf;
        bf.setBold(true);
        return bf;
    }
    return QVariant();
}

bool PMparametersModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::CheckStateRole) {
        if (!checkIndex(index))
            return false;
        auto state = value.value<Qt::CheckState>();
        int row = index.row();
        if (row == 0) {
            if (state == Qt::Checked || state == Qt::Unchecked) {
                if (index.column() == 0) {
                    for (auto& p : parameters) {
                        p.exportIBW = state == Qt::Checked;
                    }
                }
                else if (index.column() == 1) {
                    for (auto& p : parameters) {
                        p.print = state == Qt::Checked;
                    }
                }
            }
            else return false;
            emit dataChanged(this->index(0, index.column()), this->index(parameters.size() + 1, index.column()));
            return true;
        }
        --row;
        auto& p = parameters[row];
        if (index.column() == 0) {
            p.exportIBW = state != Qt::Unchecked;
        } else if (index.column() == 1) {
            p.print = state != Qt::Unchecked;
        }
        emit dataChanged(this->index(0, index.column()), this->index(parameters.size() + 1, index.column()));
        return true;
    }
    return false;
}

Qt::ItemFlags PMparametersModel::flags(const QModelIndex& index) const
{
    return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
}

