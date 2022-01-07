/*
    Copyright 2020 - 2022 Christian R. Halaszovich

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
#include <QMenu>
#include <QDebug>
#include <QSettings>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <memory>
#include <cmath>
#include "DlgGraphSettings.h"
#include "renderarea.h"
#include "ui_renderarea.h"
#include "DatFile.h"
#include <QMessageBox>
#include "DisplayTrace.h"
#include "qstring_helper.h"

RenderArea::RenderArea(QWidget* parent) :
    QWidget(parent), ndatapoints{}, 
    xTrace{}, yTrace{}, tracebuffer{}, background_traces_hidden{ false },
    clipped{ false },
    x_min{ 0.0 }, x_max{ 0.0 },
    y_min{}, y_max{}, a_x{}, b_x{}, a_y{}, b_y{}, numtraces{ 10 },
    do_autoscale_on_load{ true },
    isSelecting{ false }, selStart{}, selEnd{}, tempPixMap{ nullptr },
    settings_modified{ false }
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
            assert(tempPixMap != nullptr);
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
                if (!background_traces_hidden) {
                    // paint traces in persistance buffer
                    painter.setPen(QColor(128, 128, 128)); // grey
                    for (auto trace : tracebuffer) {
                        trace->render(painter, this);
                    }
                    painter.setPen(QColor(0, 0, 0)); // black
                }

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
        if (dataindex >= 0 && 
           static_cast<std::size_t>(dataindex) < yTrace.data.size()) {
            datay = yTrace.data.at(dataindex);
        }
        QString txt;
        if (isXYmode()) {
            txt = QString("(%1%2/%3%4)").arg(x).arg(xTrace.getYUnit()).arg(y).arg(yTrace.getYUnit());
        }
        else {
            txt = QString("(%1%2/%3%4)\ndata: %5%6").arg(x).arg(yTrace.getXUnit()).arg(y).arg(yTrace.getYUnit()).arg(datay).arg(yTrace.getYUnit());
        }
        QToolTip::showText(event->globalPos(), txt, this, rect());
        event->accept();
    }
    else if (isSelecting && event->buttons() == Qt::MouseButton::LeftButton) {
        selEnd = event->pos();
        update();
        event->accept();
    }
    else {
        event->ignore();
    }
}

void RenderArea::doContextMenu(QContextMenuEvent* event)
{
    QMenu menu(this);
    auto actZoomOut = menu.addAction("zoom out");
    auto actShrinkV = menu.addAction("vertical shrink");
    auto actAutoScale = menu.addAction("autoscale");
    auto actCopy = menu.addAction("copy");
    menu.addSeparator();
    // auto actWipeBK = menu.addAction("wipe background traces");
    auto actASOL = menu.addAction("autoscale on load");
    actASOL->setCheckable(true);
    actASOL->setChecked(do_autoscale_on_load);
    QAction* actToggleBK = menu.addAction("show background traces");
    actToggleBK->setCheckable(true);
    actToggleBK->setChecked(!background_traces_hidden);
    
    auto response = menu.exec(event->globalPos());
    if (response == actZoomOut) {
        double x, y;
        scaleFromPixToXY(event->x(), event->y(), x, y);
        zoomIn(x, y, 0.5);
        event->accept();
    }
    else if (response == actShrinkV) {
        double nymin = 1.5 * y_min - 0.5 * y_max,
            nymax = 1.5 * y_max - 0.5 * y_min;
        y_min = nymin;
        y_max = nymax;
        update();
        event->accept();
    }
    else if (response == actAutoScale) {
        autoScale();
        event->accept();
        }
    else if (response == actASOL) {
        do_autoscale_on_load = !do_autoscale_on_load;
        // settings_modified = true;
    }
    else if (response == actToggleBK) {
        background_traces_hidden = !background_traces_hidden;
        update();
        event->accept();
    } else if (response == actCopy) {
        copyToClipboard();
        event->accept();
    }
}

void RenderArea::mousePressEvent(QMouseEvent* event)
{
    // This should be habdked by a context menu event!
    //if (yTrace.isValid() && (event->button() == Qt::MouseButton::RightButton) && !isSelecting) {
    //    doContextMenu(event);
    //    return;
    //}
    if (!yTrace.isValid() || (event->button() != Qt::MouseButton::LeftButton)) {
        event->ignore();
        return;
    }
    // start selecting for zoom
    setCursor(Qt::CrossCursor);
    tempPixMap = new QPixmap(grab());
    isSelecting = true;
    selEnd = selStart = event->pos();
    event->accept();
 }

void RenderArea::contextMenuEvent(QContextMenuEvent* event)
{
    if (yTrace.isValid() && !isSelecting) {
        doContextMenu(event);
    }
    else {
        event->ignore();
    }
}

void RenderArea::mouseReleaseEvent(QMouseEvent* event)
{
    if (!yTrace.isValid()) {
        event->ignore();
        return;
    }

    if (isSelecting && event->button() == Qt::MouseButton::LeftButton) {
        double x, y;
        scaleFromPixToXY(event->x(), event->y(), x, y);
        isSelecting = false;
        delete tempPixMap; tempPixMap = nullptr;
        unsetCursor();
        // only zoom if there is a meanigful selection:
        if (event->x() != selStart.x() && event->y() != selStart.y()) {
            double xs, ys;
            scaleFromPixToXY(selStart.x(), selStart.y(), xs, ys);
            x_min = std::min(x, xs);
            x_max = std::max(x, xs);
            y_min = std::min(y, ys);
            y_max = std::max(y, ys);
        }
        update();
        event->accept();
        return;
    }
    event->ignore();
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


void RenderArea::autoScale()
{
    if (isXYmode()) {
        x_min = *std::min_element(xTrace.data.cbegin(), xTrace.data.cend());
        x_max = *std::max_element(xTrace.data.cbegin(), xTrace.data.cend());
    }
    else {
        x_min = yTrace.x0;
        x_max = yTrace.x0 + (yTrace.data.size() - 1) * yTrace.deltax;
    }
    y_min = *std::min_element(yTrace.data.cbegin(), yTrace.data.cend());
    y_max = *std::max_element(yTrace.data.cbegin(), yTrace.data.cend());
    update();
}

void RenderArea::toggleDoAutoscale(bool checked)
{
    do_autoscale_on_load = checked;
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

void RenderArea::copyToClipboard()
{
    QPixmap pixmap(grab());
    QGuiApplication::clipboard()->setPixmap(pixmap);
}

void RenderArea::showSettingsDialog()
{
    DlgGraphSettings dlg(this);
    dlg.setValues(do_autoscale_on_load, x_min, x_max, y_min, y_max, numtraces);
    if (dlg.exec()) {
        settings_modified = true;
        dlg.getValues(do_autoscale_on_load, x_min, x_max, y_min, y_max, numtraces);
        // if numtraces has been reduced we want to get rid of excess traces
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
        tracebuffer.enqueue(new DisplayTrace(std::move(yTrace)));
        while (tracebuffer.size() > numtraces) {
            delete tracebuffer.dequeue();
        }
    }
    char dataformat = TrRecord->getChar(TrDataFormat);
    uint16_t tracedatakind = TrRecord->extractUInt16(TrDataKind);
    clipped = tracedatakind & ClipBit;
    yTrace.y_unit = qs_from_sv(TrRecord->getString(TrYUnit)); // assuming the string is zero terminated...
    yTrace.x_unit = qs_from_sv(TrRecord->getString(TrXUnit));
    yTrace.x0 = TrRecord->extractLongReal(TrXStart), yTrace.deltax = TrRecord->extractLongReal(TrXInterval);
    ndatapoints = TrRecord->extractInt32(TrDataPoints);
    yTrace.data.resize(ndatapoints);
	try {
		if (dataformat == DFT_int16) {
			ReadScaleAndConvert<int16_t>(infile, *TrRecord, ndatapoints, yTrace.data.data());
		}
		else if (dataformat == DFT_int32) {
			ReadScaleAndConvert<int32_t>(infile, *TrRecord, ndatapoints, yTrace.data.data());
		}
		else if (dataformat == DFT_float) {
			ReadScaleAndConvert<float>(infile, *TrRecord, ndatapoints, yTrace.data.data());
		}
		else if (dataformat == DFT_double) {
			ReadScaleAndConvert<double>(infile, *TrRecord, ndatapoints, yTrace.data.data());
		}
		else {
			QMessageBox::warning(this, QString("Data Format Error"), QString("Unknown Dataformat"));
			return;
		}
	}
	catch (const std::exception& e) {
		yTrace.data.clear();
		QMessageBox::warning(nullptr, "File Error", e.what());
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

void RenderArea::loadSettings()
{
    QSettings s;
    s.beginGroup("renderarea");
    do_autoscale_on_load = s.value("do_autoscale_on_load", int(do_autoscale_on_load)).toInt();;
    numtraces = s.value("numtraces", numtraces).toInt();
    s.endGroup();
}

void RenderArea::saveSettings()
{
    QSettings s;
    s.beginGroup("renderarea");
    s.setValue("do_autoscale_on_load", int(do_autoscale_on_load));
    s.setValue("numtraces", numtraces);
    s.endGroup();
}