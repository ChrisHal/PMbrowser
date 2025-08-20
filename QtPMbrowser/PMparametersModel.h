#pragma once

#include <QAbstractTableModel>
#include <QSpan>
#include "PMparameters.h"

class PMparametersModel  : public QAbstractTableModel
{
	Q_OBJECT

private:
	static constexpr std::array<const char*, 2> lables{ "export", "print" };
	QSpan<hkLib::PMparameter> parameters;

public:
	explicit PMparametersModel(QSpan<hkLib::PMparameter> PMparameters, QObject *parent = nullptr);
	~PMparametersModel();
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
};

