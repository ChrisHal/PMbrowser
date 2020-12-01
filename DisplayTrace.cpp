#include "DisplayTrace.h"
#include "renderarea.h"

DisplayTrace::DisplayTrace(double X0, double DeltaX, const QString& xUnit,
	const QString& yUnit, const QVector<double>& Data):
	x0{ X0 }, deltax{ DeltaX }, x_unit{ xUnit }, y_unit{ yUnit }, data{ Data }
{
}

void DisplayTrace::reset()
{
	data.clear();
	x_unit.clear();
	y_unit.clear();
}

void DisplayTrace::render(QPainter& painter, RenderArea* display)
{
	QPainterPath path;
	const auto& xdata = display->xTrace.data;
	if (display->isXYmode()) {
		if (xdata.size() != data.size()) {
			// skip incompatible traces for x-y-mode
			return;
		}
		path.moveTo(display->scaleToQPF(xdata[0], data[0]));
		for (int i = 1; i < data.size(); ++i) {
			path.lineTo(display->scaleToQPF(xdata[i], data[i]));
		}
	}
	else {
		path.moveTo(display->scaleToQPF(x0, data[0]));
		for (int i = 1; i < data.size(); ++i) {
			path.lineTo(display->scaleToQPF(x0 + i * deltax, data[i]));
		}
	}
	painter.drawPath(path);
}
