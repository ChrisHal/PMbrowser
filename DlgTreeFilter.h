#pragma once

#include <QDialog>
#include "ui_DlgTreeFilter.h"

class DlgTreeFilter : public QDialog
{
	Q_OBJECT

public:
	DlgTreeFilter(QWidget *parent, const QString& Grp, const QString& Ser,
		const QString& Swp, const QString& Trace);
	~DlgTreeFilter();
	QString grp, ser, swp, trace;
private slots:
	void accept() override;
private:
	Ui::DlgTreeFilter ui;
};
