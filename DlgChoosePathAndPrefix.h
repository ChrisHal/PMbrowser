#pragma once

#include <QWidget>
#include <QString>
#include "ui_DlgChoosePathAndPrefix.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PMbrowserWindow; }
QT_END_NAMESPACE

class DlgChoosePathAndPrefix : public QDialog
{
	Q_OBJECT

public:
	DlgChoosePathAndPrefix(QWidget* parent, const QString& Path);
	~DlgChoosePathAndPrefix();

	QString path, prefix;

private slots:
	void accept();
	void choosePath();

private:
	Ui::DlgChoosePathAndPrefix* ui;
};
