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
#include "DlgGraphSettings.h"
#include "renderarea.h"
#include "ui_renderarea.h"
#include "DatFile.h"
#include <QMessageBox>
#include "DisplayTrace.h"

RenderArea::RenderArea(QWidget* parent) :
    QWidget(parent), ndatapoints{}, 
    xTrace{}, yTrace{}, tracebuffer{},
    clipped{ false },
    x_min{ 0.0 }, x_max{ 0.0 },
    y_min{}, y_max{}, a_x{}, b_x{}, a_y{}, b_y{}, numtraces{ 10 },
    do_autoscale_on_load{ true },
    isSelecting{ false }, selStart{}, selEnd{}, tempPixMap{ nullptr }
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    //ui->setupUi(this);
}

RenderArea::~RenderArea()
{
    //delete ui;
    while (tracebuffer.size() > 0) {
        delete tracebuffer.dequeue();
    }
}

void RenderArea::drawMarquee(QPainter& painter)
{
    //painter.save();
    painter.setPen(QColor(0, 0, 200));
    QPainterPath path;
    path.moveTo(selStart);
    path.lineTo(selEnd.x(), selStart.y());
    path.lineTo(selEnd);
    path.lineTo(selStart.x(), selEnd.y());
    path.lineTo(selStart);
    painter.drawPath(path);
    //painter.restore();
}

void RenderArea::paintEvent(QPaintEvent* event)
{
    (void)event;
    QPainterPath path;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QFont font = painter.font();
    font.setPixelSize(24);
    painter.setFont(font);
    //painter.drawPath(path);
    const QRect rectangle = QRect(0, 0, width(), height());
    if(!yTrace.isValid()) {
    painter.drawText(rectangle,Qt::AlignHCenter|Qt::AlignVCenter,"no data to display");
    } else {
        if (isSelecting) {
            painter.drawPixmap(rect(), *tempPixMap);
            drawMarquee(painter);
        }
        else {

            if (isXYmode() && xTrace.data.size() != yTrace.data.size()) {
                font.setPixelSize(16);
                painter.setFont(font);
                painter.drawText(rectangle, Qt::AlignHCenter | Qt::AlignVCenter,
                    "x- and y-trace: numbers of datapoints\nnot equal\n(required for YX-mode)");
            }
            else {
                setScaling(x_min, x_max, y_min, y_max);
                // paint traces in persistance buffer
                painter.setPen(QColor(128, 128, 128)); // grey
                for (auto trace : tracebuffer) {
                    trace->render(painter, this);
                }
                painter.setPen(QColor(0, 0, 0)); // black
                yTrace.render(painter, this);

                font = painter.font();
                font.setPixelSize(16);
                painter.setFont(font);
                painter.setPen(QColor(200, 0, 0)); // some red
                QString label = QString("%1 %2").arg(y_max).arg(yTrace.getYUnit());
                painter.drawText(rectangle, Qt::AlignHCenter | Qt::AlignTop, label);
                label = QString("%1 %2").arg(y_min).arg(yTrace.getYUnit());
                painter.drawText(rectangle, Qt::AlignHCenter | Qt::AlignBottom, label);
                if (isXYmode()) {
                    label = QString("%1 %2").arg(x_min).arg(xTrace.getYUnit());
                    painter.drawText(rectangle, Qt::AlignVCenter | Qt::AlignLeft, label);
                    label = QString("%1 %2").arg(x_max).arg(xTrace.getYUnit());
                    painter.drawText(rectangle, Qt::AlignVCenter | Qt::AlignRight, label);
                }
                else {
                    label = QString("%1 %2").arg(x_min).arg(yTrace.getXUnit());
                    painter.drawText(rectangle, Qt::AlignVCenter | Qt::AlignLeft, label);
                    label = QString("%1 %2").arg(x_max).arg(yTrace.getXUnit());
                    painter.drawText(rectangle, Qt::AlignVCenter | Qt::AlignRight, label);
                }
            }
            if (clipped) {
                painter.save();
                painter.setBrush(QColor(200, 0, 0));
                painter.setPen(QColor(200, 0, 0));
                auto br = painter.boundingRect(rectangle, Qt::AlignCenter, QString("clipping"));
                painter.drawRect(br);
                painter.setPen(QColor(255, 255, 255));
                painter.drawText(rectangle, Qt::AlignCenter, QString("clipping"));
                painter.restore();
            }
        }
    }
}

void RenderArea::mouseMoveEvent(QMouseEvent* event)
{
    if (yTrace.isValid() && event->buttons() == Qt::NoButton) {
        double x, y;
        scaleFromPixToXY(event->x(), event->y(), x, y);
        long dataindex = std::lrint((x - yTrace.x0) / yTrace.deltax);
        double datay = std::numeric_limits<double>::quiet_NaN();
        if (dataindex >= 0 && dataindex < yTrace.data.size()) {
            datay = yTrace.data.at(dataindex);
        }
        QString txt;
        if (isXYmode()) {
            txt = QString("(%1%2/%3%4)\nclick L/R: zoom in/out").arg(x).arg(xTrace.getYUnit()).arg(y).arg(yTrace.getYUnit());
        }
        else {
            txt = QString("(%1%2/%3%4)\ndata: %5%6\nclick L/R: zoom in/out").arg(x).arg(yTrace.getXUnit()).arg(y).arg(yTrace.getYUnit()).arg(datay).arg(yTrace.getYUnit());
        }
        QToolTip::showText(event->globalPos(), txt, this, rect());
        event->accept();
    }
    else if (isSelecting && event->buttons() == Qt::MouseButton::LeftButton) {
        selEnd = event->pos();
        //QPainter painter(this);
        //painter.drawPixmap(rect(), tempPixMap);
        //drawMarquee(painter);
        //painter.end();
        update();
        event->accept();
    }
    else {
        event->ignore();
    }
}

void RenderArea::mousePressEvent(QMouseEvent* event)
{
    if (!yTrace.isValid() || (event->button() != Qt::MouseButton::LeftButton)) {
        event->ignore();
        return;
    }
    setCursor(Qt::CrossCursor);
    tempPixMap = new QPixmap(grab());
    isSelecting = true;
    selEnd = selStart = event->pos();
    event->accept();
 }

void RenderArea::mouseReleaseEvent(QMouseEvent* event)
{
    if (!yTrace.isValid()) {
        event->ignore();
        return;
    }
    double x, y;
    scaleFromPixToXY(event->x(), event->y(), x, y);
    if (isSelecting && event->button() == Qt::MouseButton::LeftButton) {
        isSelecting = false;
        delete tempPixMap; tempPixMap = nullptr;
        unsetCursor();
        double xs, ys;
        scaleFromPixToXY(selStart.x(), selStart.y(), xs, ys);
        x_min = std::min(x, xs);
        x_max = std::max(x, xs);
        y_min = std::min(y, ys);
        y_max = std::max(y, ys);
        update();
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
            size_t bytestoread = std::min(bytesremaining, size_t(interleavesize));
            infile.read(p, bytestoread);
            if (!infile) { break; }
            p += bytestoread;
            bytesremaining -= bytestoread;
            if (bytesremaining > 0) {
                infile.seekg(bytestoskip, std::ios::cur); // skip to next block
            }
        }
    }
    if (!infile) {
        delete[]source;
        data.clear();
        QMessageBox::warning(nullptr, "File Error", "error while reading datafile");
        return;
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
    if (isXYmode()) {
        x_min = *std::min_element(xTrace.data.constBegin(), xTrace.data.constEnd());
        x_max = *std::max_element(xTrace.data.constBegin(), xTrace.data.constEnd());
    }
    else {
        x_min = yTrace.x0;
        x_max = yTrace.x0 + (yTrace.data.size() - 1) * yTrace.deltax;
    }
    y_min = *std::min_element(yTrace.data.constBegin(), yTrace.data.constEnd());
    y_max = *std::max_element(yTrace.data.constBegin(), yTrace.data.constEnd());
    update();
}

void RenderArea::wipeBuffer()
{
    while (tracebuffer.size() > 0) {
        delete tracebuffer.dequeue();
    }
    update();
}

// enter x-y mode, curent yTrace becomes x-trace
// will always autoscale
void RenderArea::setXYmode()
{
//    if (!xTrace.isValid()) {
//        // make sure not already in x-y-mode
        xTrace = yTrace;
        autoScale();
//    }
}

// leave XYmode
void RenderArea::setYTmode()
{
    if (xTrace.isValid()) {
        xTrace.reset();
        autoScale();
    }
}

void RenderArea::showSettingsDialog()
{
    DlgGraphSettings dlg(this);
    dlg.setValues(do_autoscale_on_load, x_min, x_max, y_min, y_max, numtraces);
    if (dlg.exec()) {
        dlg.getValues(do_autoscale_on_load, x_min, x_max, y_min, y_max, numtraces);
        // if numtraces has been reduced we ant to get rid of excess traces
        while (tracebuffer.size() > numtraces) {
            delete tracebuffer.dequeue();
        }
        update();
    }
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
    if (yTrace.isValid()) {
        tracebuffer.enqueue(new DisplayTrace(yTrace));
        while (tracebuffer.size() > numtraces) {
            delete tracebuffer.dequeue();
        }
    }
    char dataformat = TrRecord->getChar(TrDataFormat);
    int32_t     interleavesize = TrRecord->extractValue<int32_t>(TrInterleaveSize ,0),
                inerleaveskip = TrRecord->extractValue<int32_t>(TrInterleaveSkip, 0);
    uint16_t tracedatakind = TrRecord->extractUInt16(TrDataKind);
    bool need_swap = !(tracedatakind & LittleEndianBit);
    clipped = tracedatakind & ClipBit;
    yTrace.y_unit = TrRecord->getString(TrYUnit).c_str(); // assuming the string is zero terminated...
    yTrace.x_unit = TrRecord->getString(TrXUnit).c_str();
    yTrace.x0 = TrRecord->extractLongReal(TrXStart), yTrace.deltax = TrRecord->extractLongReal(TrXInterval);
    double datascaler = TrRecord->extractLongReal(TrDataScaler);
    int32_t trdata = TrRecord->extractInt32(TrData);
    ndatapoints = TrRecord->extractInt32(TrDataPoints);
    yTrace.data.clear();
    infile.seekg(trdata);
    if (dataformat == DFT_int16) {
        ReadScaleAndConvert<int16_t>(infile, need_swap,  datascaler,  ndatapoints, interleavesize, inerleaveskip, yTrace.data);
    }
    else if (dataformat == DFT_int32) {
        ReadScaleAndConvert<int32_t>(infile, need_swap, datascaler, ndatapoints, interleavesize, inerleaveskip, yTrace.data);
    }
    else if (dataformat == DFT_float) {
        ReadScaleAndConvert<float>(infile, need_swap, datascaler, ndatapoints, interleavesize, inerleaveskip, yTrace.data);
    }
    else if (dataformat == DFT_double) {
        ReadScaleAndConvert<double>(infile, need_swap, datascaler, ndatapoints, interleavesize, inerleaveskip, yTrace.data);
    }
    else {
        QMessageBox::warning(this, QString("Data Format Error"), QString("Unknown Dataformat"));
        return;
    }
    if (do_autoscale_on_load) { autoScale(); }
    else { update(); } // update is usually done within autoScale()
    setMouseTracking(true);
    }

void RenderArea::clearTrace()
{
    ndatapoints = 0;
    yTrace.reset();
    xTrace.reset();
    while (tracebuffer.size() > 0) {
        delete tracebuffer.dequeue();
    }
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
