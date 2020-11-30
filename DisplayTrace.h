#pragma once
#include <QVector>
#include <QPainter>
#include <QPainterPath>
#include "renderarea.h"

class DisplayTrace
{
public:
	DisplayTrace(double X0, double DeltaX, const QVector<double>& Data);
	void render(QPainter& painter, RenderArea* display);
private:
	double x0, deltax;
	QVector<double> data;
};

