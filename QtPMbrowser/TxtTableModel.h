#pragma once

#include <QAbstractTableModel>
#include <QList>
#include <QStringList>
#include <QString>

class TxtTableModel  : public QAbstractTableModel
{
	Q_OBJECT

		int colCount{ 0 };
		QList<QStringList> rows;
		QStringList horzHeader;
		QStringList vertHeader;
public:
	TxtTableModel(const QString& txt, bool hasHorizontalHeader, bool hasVerticalHeader, QObject *parent = nullptr);
	~TxtTableModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

