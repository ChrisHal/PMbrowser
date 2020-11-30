#pragma once

#include <QDialog>
#include <QValidator>
#include "ui_DlgGraphSettings.h"

class DlgGraphSettings : public QDialog
{
	Q_OBJECT

public:
	DlgGraphSettings(QWidget *parent = Q_NULLPTR);
	~DlgGraphSettings();
	void setValues(bool autoscale, double xmin, double xmax,
		double ymin, double ymax, int numtraces);
	void getValues(bool& autoscale, double& xmin, double& xmax,
		double& ymin, double& ymax, int& numtraces);

private:
	Ui::DlgGraphSettings ui;
	QIntValidator validator;
	QDoubleValidator dvalidator;
};
