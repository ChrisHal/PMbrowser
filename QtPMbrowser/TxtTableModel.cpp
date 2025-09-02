#include "TxtTableModel.h"
#include <QTextStream>
#include <QFont>
#include <algorithm>

TxtTableModel::TxtTableModel(const QString& txt, bool hasHorizontalHeader, bool hasVerticalHeader, QObject* parent)
	: QAbstractTableModel(parent)
{
	QString tmp_string = txt;
	QTextStream s(&tmp_string, QIODeviceBase::ReadOnly);
	if (hasHorizontalHeader) {
		auto t = s.readLine();
		if (!t.isNull()) {
			horzHeader = t.split('\t');
			colCount = horzHeader.size();
		}
	}
	for (;;) {
		auto line = s.readLine();
		if (line.isNull()) break;
		auto cells = line.split('\t');
		if (hasVerticalHeader) {
			if (cells.empty()) {
				vertHeader.push_back("");
			}
			else {
				vertHeader.push_back(cells.first());
				cells.removeFirst();
			}
		}
		colCount = std::max(colCount, static_cast<int>(cells.size()));
		rows.push_back(cells);
	}
}

TxtTableModel::~TxtTableModel()
{}

int TxtTableModel::rowCount(const QModelIndex& /*parent*/) const
{
	return static_cast<int>(rows.size());
}

int TxtTableModel::columnCount(const QModelIndex& /*parent*/) const
{
	return colCount;
}

QVariant TxtTableModel::data(const QModelIndex& index, int role) const
{
	int row = index.row();
	int col = index.column();
	if (role == Qt::DisplayRole) {
		if (row < rows.size()) {
			auto& cells = rows[row];
			if (col < cells.size()) return cells[col];
		}
	}
	return QVariant();
}

QVariant TxtTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole) {
		switch (orientation) {
		case Qt::Horizontal:
			if (!horzHeader.empty()) {
				return horzHeader[section];
			}
			break;
		case Qt::Vertical:
			if (!vertHeader.empty()) {
				return vertHeader[section];
			}
			break;
		}
	}
	else if (role == Qt::FontRole && orientation == Qt::Vertical && vertHeader.size() > section
		&& vertHeader[section].endsWith(':')) {
		QFont boldFont;
		boldFont.setBold(true);
		return boldFont;
	}
	return QVariant();
}

