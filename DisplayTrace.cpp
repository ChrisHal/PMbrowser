#include <cmath>
#include <algorithm>
//#ifndef NDEBUG
//#include <QDebug>
//#endif
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
		//in YT-mode we speed things up by drawing only the
		//datapoints actually visible
		int N = data.size();
		int pFirst = std::max(0, int(std::floor((display->x_min - x0) / deltax)));
		int pEnd = std::min(int(std::ceil((display->x_max - x0) / deltax)), N);
//#ifndef NDEBUG
//		qDebug() << "first: " << pFirst << ", end: " << pEnd;
//#endif
		if (pFirst < pEnd) { // pFirst might even be larger than data.size(), we catch this case also here
			path.moveTo(display->scaleToQPF(x0 + pFirst * deltax, data[pFirst]));
			for (int i = 1 + pFirst; i < pEnd; ++i) {
				path.lineTo(display->scaleToQPF(x0 + i * deltax, data[i]));
			}
		}
	}
	painter.drawPath(path);
}
