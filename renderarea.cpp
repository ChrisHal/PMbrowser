/*
    Copyright 2020 Christian R. Halaszovich

     This file is part of PMbrowser.

    PMbrowser is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PMbrowser is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PMbrowser.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <QtGui>
#include <QToolTip>
#include <qdebug.h>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <cmath>
#include "renderarea.h"
#include "ui_renderarea.h"
#include "DatFile.h"
#include <QMessageBox>


RenderArea::RenderArea(QWidget* parent) :
    QWidget(parent), ndatapoints{}, data{}, xunit{}, yunit{}, clipped{ false },
    x0{}, deltax{}, x_min{ 0.0 }, x_max{ 0.0 },
    y_min{}, y_max{}, a_x{}, b_x{}, a_y{}, b_y{},
    ui{ nullptr }
//    ui(new Ui::RenderArea)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    //ui->setupUi(this);
}

RenderArea::~RenderArea()
{
    //delete ui;
}

void RenderArea::paintEvent(QPaintEvent * /* event */)
{
    QPainterPath path;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QFont font = painter.font();
    font.setPixelSize(24);
    painter.setFont(font);
    //painter.drawPath(path);
    const QRect rectangle = QRect(0, 0, width(), height());
    if(data.size()==0) {
    painter.drawText(rectangle,Qt::AlignHCenter|Qt::AlignVCenter,"no trace selected");
    } else {
        setScaling(x_min, x_max, y_min, y_max);
        path.moveTo(scaleToQPF(x0,data[0]));
        for(int i=0; i<data.size(); ++i) {
            path.lineTo(scaleToQPF(x0+i*deltax, data[i]));
        }
        painter.drawPath(path);
        font = painter.font();
        font.setPixelSize(16);
        painter.setFont(font);
        painter.setPen(QColor(200, 0, 0)); // some red
        QString label = QString("%1 %2").arg(y_max).arg(yunit);
        painter.drawText(rectangle, Qt::AlignHCenter | Qt::AlignTop, label);
        label = QString("%1 %2").arg(y_min).arg(yunit);
        painter.drawText(rectangle, Qt::AlignHCenter | Qt::AlignBottom, label);
        label = QString("%1 %2").arg(x_min).arg(xunit);
        painter.drawText(rectangle, Qt::AlignVCenter | Qt::AlignLeft, label);
        label = QString("%1 %2").arg(x_max).arg(xunit);
        painter.drawText(rectangle, Qt::AlignVCenter | Qt::AlignRight, label);
        if (clipped) {
            painter.setBrush(QColor(200, 0, 0));
            painter.setPen(QColor(200, 0, 0));
            auto br = painter.boundingRect(rectangle, Qt::AlignCenter, QString("clipping"));
            painter.drawRect(br);
            painter.setPen(QColor(255, 255, 255));
            painter.drawText(rectangle, Qt::AlignCenter, QString("clipping"));
        }
    }
}

void RenderArea::mouseMoveEvent(QMouseEvent* event)
{
    if (data.size() > 0 && event->buttons() == Qt::NoButton) {
        double x, y;
        scaleFromPixToXY(event->x(), event->y(), x, y);
        long dataindex = std::lrint((x - x0) / deltax);
        double datay = std::numeric_limits<double>::quiet_NaN();
        if (dataindex >= 0 && dataindex < data.size()) {
            datay = data.at(dataindex);
        }
        QToolTip::showText(event->globalPos(), 
            QString("(%1%2/%3%4)\ndata: %5%6\nclick L/R: zoom in/out").arg(x).arg(xunit).arg(y).arg(yunit).arg(datay).arg(yunit),
            this, rect());
        event->accept();
    }
    else {
        event->ignore();
    }
}

void RenderArea::mouseReleaseEvent(QMouseEvent* event)
{
    if (data.size() == 0) {
        event->ignore();
        return;
    }
    double x, y;
    scaleFromPixToXY(event->x(), event->y(), x, y);
    if (event->button() == Qt::MouseButton::LeftButton) {
        zoomIn(x, y, 2.0);
    }
    else if (event->button() == Qt::MouseButton::RightButton) {
        // zoom out
        zoomIn(x, y, 0.5);
    }
    event->accept();
}

void RenderArea::wheelEvent(QWheelEvent* event)
{
    auto pos = event->pos();
    auto delta = event->angleDelta().y();
    if (delta == 0) {
        event->ignore();
        return;
    }
    double factor = 1.0 + std::abs(delta)/360.0;
    if (delta < 0) {
        factor = 1.0 / factor;
    }
    double x, y;
    scaleFromPixToXY(pos.x(), pos.y(), x, y);
    zoomIn(x, y, factor);
    event->accept();
}


template<typename T> void ReadScaleAndConvert(std::istream& infile,
 bool need_swap,
    double datascaler, size_t ndatapoints, int interleavesize, int interleaveskip, QVector<double>& data)
{
    T* source = new T[ndatapoints];
    if (interleavesize == 0) {
        infile.read((char*)source, sizeof(T) * ndatapoints);
    }
    else {
        assert(interleaveskip >= interleavesize);
        size_t bytesremaining = sizeof(T) * ndatapoints;
        int bytestoskip = interleaveskip - interleavesize; // interleaveskip is from block-start to block-start!
        char* p = (char*)source;
        while (bytesremaining > 0) {
            int bytestoread = std::min(bytesremaining, size_t(interleavesize));
            infile.read(p, bytestoread);
            if (!infile) { break; }
            p += bytestoread;
            bytesremaining -= bytestoread;
            if (bytesremaining > 0) {
                infile.seekg(bytestoskip, std::ios::cur); // skip to next block
            }
        }
    }
    if (!need_swap) {
        for (size_t i = 0; i < ndatapoints; ++i) {
            data.push_back(datascaler * source[i]);
        }
    }
    else {
        for (size_t i = 0; i < ndatapoints; ++i) {
            data.push_back(datascaler * swap_bytes(source[i]));
        }
    }
    delete[] source; source = nullptr;
}

void RenderArea::autoScale()
{
    x_min = x0;
    x_max = x0 + (ndatapoints - 1) * deltax;
    y_min = *std::min_element(data.constBegin(), data.constEnd());
    y_max = *std::max_element(data.constBegin(), data.constEnd());
    update();
}

void RenderArea::zoomIn(double x_center, double y_center, double factor)
{
    double  x_offset = (x_max - x_min) / factor / 2.0,
        y_offset = (y_max - y_min) / factor / 2.0;
    x_min = x_center - x_offset;
    x_max = x_center + x_offset;
    y_min = y_center - y_offset;
    y_max = y_center + y_offset;
    update();
}

void RenderArea::renderTrace(hkTreeNode* TrRecord, std::istream& infile)
{
    char dataformat = TrRecord->getChar(TrDataFormat);
    int32_t     interleavesize = TrRecord->extractValue<int32_t>(TrInterleaveSize ,0),
                inerleaveskip = TrRecord->extractValue<int32_t>(TrInterleaveSkip, 0);
    uint16_t tracedatakind = TrRecord->extractUInt16(TrDataKind);
    bool need_swap = !(tracedatakind & LittleEndianBit);
    clipped = tracedatakind & ClipBit;
    yunit = TrRecord->getString(TrYUnit).c_str(); // assuming the string is zero terminated...
    xunit = TrRecord->getString(TrXUnit).c_str();
    x0 = TrRecord->extractLongReal(TrXStart), deltax = TrRecord->extractLongReal(TrXInterval);
    double datascaler = TrRecord->extractLongReal(TrDataScaler);
    int32_t trdata = TrRecord->extractInt32(TrData);
    ndatapoints = TrRecord->extractInt32(TrDataPoints);
    data.clear();
    infile.seekg(trdata);
    if (dataformat == DFT_int16) {
        ReadScaleAndConvert<int16_t>(infile, need_swap,  datascaler,  ndatapoints, interleavesize, inerleaveskip, data);
    }
    else if (dataformat == DFT_int32) {
        ReadScaleAndConvert<int32_t>(infile, need_swap, datascaler, ndatapoints, interleavesize, inerleaveskip, data);
    }
    else if (dataformat == DFT_float) {
        ReadScaleAndConvert<float>(infile, need_swap, datascaler, ndatapoints, interleavesize, inerleaveskip, data);
    }
    else if (dataformat == DFT_double) {
        ReadScaleAndConvert<double>(infile, need_swap, datascaler, ndatapoints, interleavesize, inerleaveskip, data);
    }
    else {
        QMessageBox::warning(this, QString("Data Format Error"), QString("Unknown Dataformat"));
        return;
    }
    autoScale();
    setMouseTracking(true);
    }

void RenderArea::clearTrace()
{
    ndatapoints = 0;
    data.clear();
    setMouseTracking(false);
    update();
}

void RenderArea::setScaling(double x_0, double x_1, double y_0, double y_1)
{
    double h = height() - 1, w = width() - 1;
    a_x = -w*x_0/(x_1-x_0);
    b_x = w/(x_1-x_0);
    a_y = h*y_1/(y_1-y_0);
    b_y = -h/(y_1-y_0);
}

QPointF RenderArea::scaleToQPF(double x, double y)
{
    return QPointF(a_x+b_x*x, a_y+b_y*y);
}

void RenderArea::scaleFromPixToXY(int px, int py, double& x, double& y)
{
    x = x_min + double(px) / double(width()) * (x_max - x_min);
    y = y_max - double(py) / double(height()) * (y_max - y_min);
}
