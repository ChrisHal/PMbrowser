#include <QDialog>
#include <QFileDialog>
#include "DlgChoosePathAndPrefix.h"

DlgChoosePathAndPrefix::DlgChoosePathAndPrefix(QWidget* parent, const QString& Path) : QDialog(parent), path(Path), prefix{}, ui(new Ui::DlgChoosePathAndPrefix)
{
	ui->setupUi(this);
	ui->lineEditPath->setText(path);
	ui->lineEditPrefix->setText("PM");
	QObject::connect(ui->pushButtonChoosePath, SIGNAL(clicked()), this, SLOT(choosePath()));
}

DlgChoosePathAndPrefix::~DlgChoosePathAndPrefix()
{
}

void DlgChoosePathAndPrefix::choosePath()
{
	QFileDialog dlg;
	dlg.setFileMode(QFileDialog::Directory);
	dlg.selectFile(path);
	if (dlg.exec()) {
		ui->lineEditPath->setText(dlg.selectedFiles()[0]);
	}
}

void DlgChoosePathAndPrefix::accept()
{
	path = ui->lineEditPath->text();
	prefix = ui->lineEditPrefix->text();
	QDialog::accept();
}
