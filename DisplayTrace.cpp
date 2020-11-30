#include "DisplayTrace.h"

DisplayTrace::DisplayTrace(double X0, double DeltaX, const QVector<double>& Data):
	x0{ X0 }, deltax{ DeltaX }, data{ Data }
{
}

void DisplayTrace::render(QPainter& painter, RenderArea* display)
{
	QPainterPath path;
	path.moveTo(display->scaleToQPF(x0, data[0]));
	for (int i = 1; i < data.size(); ++i) {
		path.lineTo(display->scaleToQPF(x0 + i * deltax, data[i]));
	}
	painter.drawPath(path);
}
