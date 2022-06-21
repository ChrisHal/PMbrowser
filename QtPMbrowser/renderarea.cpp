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
#include <QGuiApplication>
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

constexpr auto BUTTON_HEIGHT = 23, BUTTON_WIDTH = 55;

RenderArea::RenderArea(QWidget* parent) :
    QWidget(parent),
    btnWipe{"wipe", this},
    btnAutoScale{"auto", this},
    btnVertShrink{"v.shrink", this},
    btnHrzShrink{"h.shrink", this},
    chkAutoScale{"scale on load", this},
    ndatapoints{}, 
    xTrace{}, yTrace{}, tracebuffer{}, background_traces_hidden{ false },
    clipped{ false },
    x_min{ 0.0 }, x_max{ 0.0 },
    y_min{}, y_max{}, a_x{}, b_x{}, a_y{}, b_y{}, numtraces{ 10 },
    do_autoscale_on_load{ true }, isTraceDragging{ false },
    isSelecting{ false }, selStart{}, selEnd{}, tempPixMap{ nullptr },
    settings_modified{ false }
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::WheelFocus);

    QObject::connect(&btnWipe, &QPushButton::clicked, this, &RenderArea::wipeAll);
    QObject::connect(&btnAutoScale, &QPushButton::clicked, this, &RenderArea::autoScale);
    QObject::connect(&btnVertShrink, &QPushButton::clicked, this, &RenderArea::verticalShrink);
    QObject::connect(&btnHrzShrink, &QPushButton::clicked, this, &RenderArea::horizontalShrink);
    QObject::connect(&chkAutoScale, &QCheckBox::stateChanged, this, &RenderArea::toggleDoAutoscale2);


    auto btnstyle = p_btnstyle.get();

    btnWipe.setGeometry(0, 0, BUTTON_WIDTH, BUTTON_HEIGHT);
    btnWipe.setStyle(btnstyle);
    btnAutoScale.setGeometry(BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT);
    btnAutoScale.setStyle(btnstyle);
    btnVertShrink.setGeometry(2 * BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT);
    btnVertShrink.setStyle(btnstyle);
    btnHrzShrink.setGeometry(3 * BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT);
    btnHrzShrink.setStyle(btnstyle);
    chkAutoScale.setChecked(do_autoscale_on_load);
    chkAutoScale.setGeometry(4 * BUTTON_WIDTH, 0, chkAutoScale.sizeHint().width(), BUTTON_HEIGHT);
    chkAutoScale.setStyle(btnstyle);


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
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QFont font = painter.font();
    font.setPixelSize(24);
    painter.setFont(font);
    //painter.drawPath(path);
    const QRect rectangle = QRect(0, BUTTON_HEIGHT, width(), height() - BUTTON_HEIGHT);
    if(noData()) {
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
                    for (auto trace : qAsConst(tracebuffer)) {
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

void RenderArea::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Shift && !isTraceDragging) {
        setCursor(Qt::OpenHandCursor);
    }
    else {
        QWidget::keyPressEvent(event);
    }
}

void RenderArea::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Shift && !isTraceDragging) {
        unsetCursor();
    }
    else {
        QWidget::keyPressEvent(event);
    }
}

void RenderArea::mouseMoveEvent(QMouseEvent* event)
{
    if (!noData() && event->buttons() == Qt::NoButton) {
        double x, y;
        scaleFromPixToXY(event->x(), event->y(), x, y);

        QString txt;
        if (isXYmode()) {
            txt = QString("(%1%2/%3%4)").arg(x).arg(xTrace.getYUnit()).arg(y).arg(yTrace.getYUnit());
        }
        else {
            double datay = yTrace.interp(x); //= std::numeric_limits<double>::quiet_NaN();
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
    else if (isTraceDragging && event->buttons() == Qt::MouseButton::LeftButton) {
        auto pos_curr = event->pos();
        shiftByPixel(selStart - pos_curr);
        selStart = pos_curr;
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
    auto actShrinkH = menu.addAction("horizontal shrink");
    auto actAutoScale = menu.addAction("autoscale");
    auto actCopy = menu.addAction("copy");
    menu.addSeparator();
    // auto actWipeBK = menu.addAction("wipe background traces");
    auto actASOL = menu.addAction("autoscale on load");

    QObject::connect(actShrinkH, &QAction::triggered, this, &RenderArea::horizontalShrink);
    QObject::connect(actShrinkV, &QAction::triggered, this, &RenderArea::verticalShrink);
    QObject::connect(actAutoScale, &QAction::triggered, this, &RenderArea::autoScale);

    actASOL->setCheckable(true);
    actASOL->setChecked(do_autoscale_on_load);
    QAction* actToggleBK = menu.addAction("overlay traces");
    actToggleBK->setCheckable(true);
    actToggleBK->setChecked(!background_traces_hidden);
    
    auto response = menu.exec(event->globalPos());
    if (response == actZoomOut) {
        double x, y;
        scaleFromPixToXY(event->x(), event->y(), x, y);
        zoomIn(x, y, 0.5);
    }
    else if (response == actASOL) {
        toggleDoAutoscale(!do_autoscale_on_load);
    }
    else if (response == actToggleBK) {
        background_traces_hidden = !background_traces_hidden;
        update();
    } else if (response == actCopy) {
        copyToClipboard();
    }
    event->accept();
}

void RenderArea::mousePressEvent(QMouseEvent* event)
{
	if (noData() || (event->button() != Qt::MouseButton::LeftButton)) {
		event->ignore();
		return;
	}
	auto keymod = QGuiApplication::keyboardModifiers();
	if (keymod == Qt::ShiftModifier) {
        isTraceDragging = true;
        setCursor(Qt::ClosedHandCursor);
        selEnd = selStart = event->pos();
        event->accept();
    }
	else if (keymod == Qt::NoModifier) {
		// start selecting for zoom
		setCursor(Qt::CrossCursor);
		tempPixMap = new QPixmap(grab());
		isSelecting = true;
		selEnd = selStart = event->pos();
		event->accept();
	}
	else {
		event->ignore();
	}
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

void RenderArea::enterEvent(QEvent* event)
{
    setFocus();
    if (!noData() && !isTraceDragging && !isSelecting) {
        if (QGuiApplication::keyboardModifiers()
            == Qt::ShiftModifier) {
            setCursor(Qt::OpenHandCursor);
        }
        else {
            unsetCursor();
        }
    }
    event->accept();
}

void RenderArea::leaveEvent(QEvent* event)
{
    clearFocus();
    event->accept();
}

//void RenderArea::resizeEvent(QResizeEvent* event)
//{
//    btnWipe.move(0, 0);
//    auto h = btnAutoScale.height();
//    btnAutoScale.move(0, h);
//    QWidget::resizeEvent(event);
//}

void RenderArea::mouseReleaseEvent(QMouseEvent* event)
{
    if (noData()) {
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
    else if (isTraceDragging && event->button() == Qt::MouseButton::LeftButton) {
        isTraceDragging = false;
        if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier) {
            setCursor(Qt::OpenHandCursor);
        }
        else {
            unsetCursor();
        }
        event->accept();
        return;
    }
    event->ignore();
}

void RenderArea::wheelEvent(QWheelEvent* event)
{
    if (noData()) {
        event->ignore();
        return;
    }
    auto shift = event->pixelDelta();
    if (!shift.isNull()) {
        event->accept();
        shiftByPixel(shift);
        return;
    }
    auto delta = event->angleDelta();
    if (delta.isNull()) {
        event->ignore();
        return;
    }
    int s_x{}, s_y{};
    if (delta.y()) {
        s_y = -delta.y();
        if (event->inverted()) {
            // make behavior more "natural" for "real" mouse wheels?
            s_y *= -1;
        }
    }
    if (delta.x()) {
        s_x = -delta.x();
    }
    shiftByPixel({ s_x,s_y });
    event->accept();
}


void RenderArea::autoScale()
{
    if (noData()) return;
    if (isXYmode()) {
        x_min = *std::min_element(xTrace.data.cbegin(), xTrace.data.cend());
        x_max = *std::max_element(xTrace.data.cbegin(), xTrace.data.cend());
    }
    else if (yTrace.has_x_trace()) {
        x_min = *std::min_element(yTrace.p_xdata->cbegin(), yTrace.p_xdata->cend());
        x_max = *std::max_element(yTrace.p_xdata->cbegin(), yTrace.p_xdata->cend());
    }
    else
    {
        x_min = yTrace.x0;
        x_max = yTrace.x0 + (yTrace.data.size() - 1) * yTrace.deltax;
    }
    y_min = *std::min_element(yTrace.data.cbegin(), yTrace.data.cend());
    y_max = *std::max_element(yTrace.data.cbegin(), yTrace.data.cend());
    update();
}

void RenderArea::verticalShrink()
{
    double nymin = 1.5 * y_min - 0.5 * y_max,
        nymax = 1.5 * y_max - 0.5 * y_min;
    y_min = nymin;
    y_max = nymax;
    update();
}

void RenderArea::horizontalShrink()
{
    double nxmin = 1.5 * x_min - 0.5 * x_max,
        nxmax = 1.5 * x_max - 0.5 * x_min;
    x_min = nxmin;
    x_max = nxmax;
    update();
}

void RenderArea::toggleDoAutoscale(bool new_state)
{
    do_autoscale_on_load = new_state;
    chkAutoScale.setChecked(new_state);
}

void RenderArea::toggleDoAutoscale2(int checked)
{
    do_autoscale_on_load = checked != Qt::CheckState::Unchecked;
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
    yTrace.y_unit = qs_from_sv(TrRecord->getString<8>(TrYUnit));
    yTrace.x_unit = qs_from_sv(TrRecord->getString<8>(TrXUnit));
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

void RenderArea::addTrace(DisplayTrace&& dt)
{
    if (yTrace.isValid()) {
        tracebuffer.enqueue(new DisplayTrace(std::move(yTrace)));
        while (tracebuffer.size() > numtraces) {
            delete tracebuffer.dequeue();
        }
    }
    yTrace = std::move(dt);
    if (do_autoscale_on_load) { autoScale(); }
    setMouseTracking(true);
    update();
}

void RenderArea::createInterpolatedXtrace(DisplayTrace&& dt_x)
{
    if (yTrace.isValid()) {
        xTrace = std::move(dt_x);
        xTrace.convertToInterpolated(yTrace);
        if (do_autoscale_on_load) { autoScale(); }
        update();
    }
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
    double h = height() - 1 - BUTTON_HEIGHT, w = width() - 1;
    a_x = -w*x_0/(x_1-x_0);
    b_x = w/(x_1-x_0);
    a_y = h*y_1/(y_1-y_0) + BUTTON_HEIGHT;
    b_y = -h/(y_1-y_0);
}

QPointF RenderArea::scaleToQPF(double x, double y)
{
    return QPointF(a_x+b_x*x, a_y+b_y*y);
}

void RenderArea::scaleFromPixToXY(int px, int py, double& x, double& y)
{
    x = x_min + double(px) / double(width()) * (x_max - x_min);
    y = y_max - double(py - BUTTON_HEIGHT) / double(height() - BUTTON_HEIGHT) * (y_max - y_min);
}

void RenderArea::shiftByPixel(QPoint shift)
{
    if (shift.x() != 0) {
        auto dx = (x_max - x_min) / width() * double(shift.x());
        x_max += dx;
        x_min += dx;
        update();
    }
    if (shift.y() != 0) {
        auto dy = -(y_max - y_min) / height() * double(shift.y());
        y_max += dy;
        y_min += dy;
        update();
    }
}

void RenderArea::loadSettings()
{
    QSettings s;
    s.beginGroup("renderarea");
    do_autoscale_on_load = s.value("do_autoscale_on_load", int(do_autoscale_on_load)).toInt();
    chkAutoScale.setChecked(do_autoscale_on_load);
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
