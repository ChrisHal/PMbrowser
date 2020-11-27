#include "DlgTreeFilter.h"

DlgTreeFilter::DlgTreeFilter(QWidget *parent, const QString& Grp, const QString& Ser,
	const QString& Swp, const QString& Trace)
	: QDialog(parent), grp{Grp}, ser{Ser}, swp{Swp}, trace{Trace}
{
	ui.setupUi(this);
	ui.lineEditGrp->setText(grp);
	ui.lineEditSer->setText(ser);
	ui.lineEditSwp->setText(swp);
	ui.lineEditTr->setText(trace);
}

DlgTreeFilter::~DlgTreeFilter()
{
}

void DlgTreeFilter::accept()
{
	grp = ui.lineEditGrp->text();
	ser = ui.lineEditSer->text();
	swp = ui.lineEditSwp->text();
	trace = ui.lineEditTr->text();
	QDialog::accept();
}