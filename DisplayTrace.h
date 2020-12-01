#pragma once
#include <QVector>
#include <QPainter>
#include <QPainterPath>
#include <QString>
//#include "renderarea.h"

class RenderArea;

class DisplayTrace
{
public:
	DisplayTrace() : x0{ 0.0 }, deltax{ 0.0 }, x_unit{}, y_unit{}, data{} {};
	DisplayTrace(double X0, double DeltaX, const QString& xUnit, const QString& yUnit, const QVector<double>& Data);
	void reset();
	void render(QPainter& painter, RenderArea* display);
	bool isValid() { return !data.isEmpty(); }
	QString getXUnit() { return x_unit; }
	QString getYUnit() { return y_unit; }
//	QVector<double>& Data() { return data; }
//private:
	double x0, deltax;
	QString x_unit, y_unit;
	QVector<double> data;
};

