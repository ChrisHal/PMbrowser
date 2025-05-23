/*
    Copyright 2020 - 2022, 2025 Christian R. Halaszovich

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

#include <QGuiApplication>
#include <QClipboard>
#include <QToolTip>
#include <QMenu>
#include <QDebug>
#include <QSettings>
#include <QSizePolicy>
#include <QMessageBox>
#include <algorithm>
#include <memory>
#include <utility>
#include <cmath>

#include "DlgGraphSettings.h"
#include "renderarea.h"
#include "DatFile.h"
#include "DisplayTrace.h"
#include "qstring_helper.h"


RenderArea::RenderArea(QWidget* parent) :
    QWidget(parent),
    btnWipe{"wipe", this},
    btnAutoScale{"auto", this},
    btnVertShrink{"v.shrink", this},
    btnHrzShrink{"h.shrink", this},
    chkAutoScale{"autoscale on load", this},
    chkOverlay{"overlay", this},
    ndatapoints{}, 
    xTrace{}, yTrace{}, tracebuffer{}, background_traces_hidden{ false },
    clipped{ false },
    x_min{ 0.0 }, x_max{ 0.0 },
    a_x{}, b_x{}, a_y{}, b_y{}, numtraces{ 10 },
    do_autoscale_on_load{ true }, global_autoscale{ true },
    isTraceDragging{ false }, isPinching{false},
    isSelecting{ false }, selStart{}, selEnd{}, tempPixMap{ nullptr },
    settings_modified{ false }
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::WheelFocus);
    grabGesture(Qt::GestureType::PinchGesture);

    QObject::connect(&btnWipe, &QPushButton::clicked, this, &RenderArea::wipeAll);
    QObject::connect(&btnAutoScale, &QPushButton::clicked, this, &RenderArea::autoScale);
    QObject::connect(&btnVertShrink, &QPushButton::clicked, this, &RenderArea::verticalShrink);
    QObject::connect(&btnHrzShrink, &QPushButton::clicked, this, &RenderArea::horizontalShrink);
    QObject::connect(&chkAutoScale, &QCheckBox::checkStateChanged, this, &RenderArea::toggleDoAutoscale2);
    QObject::connect(&chkOverlay, &QCheckBox::checkStateChanged, this, &RenderArea::toggleOverlay);


    //auto btnstyle = p_btnstyle.get();
    my_layout = new QGridLayout;
    this->setLayout(my_layout);
    my_layout->addWidget(&btnWipe, 0, 0);
    my_layout->addWidget(&btnAutoScale, 0, 1);
    my_layout->addWidget(&btnVertShrink, 0, 2);
    my_layout->addWidget(&btnHrzShrink, 0, 3);
    my_layout->addWidget(&chkAutoScale, 1, 0, 1, 4);
    my_layout->addWidget(&chkOverlay, 2, 0, 1, 4);
    my_layout->addItem(new QSpacerItem(0, 0), 3, 0, 1, 4); // spacer at bottom
    my_layout->addItem(new QSpacerItem(0, 0), 0, 4, 4, 1); // spacer at rhs
    my_layout->setRowStretch(3, 1);
    my_layout->setColumnStretch(4, 1);
    my_layout->setContentsMargins(0, 0, 0, 0);
    my_layout->setSpacing(1);

    //btnWipe.setStyle(btnstyle);
    btnWipe.setToolTip("clear display");
    //btnAutoScale.setStyle(btnstyle);
    btnAutoScale.setToolTip("auto scale");
    //btnVertShrink.setStyle(btnstyle);
    btnVertShrink.setToolTip("expand y-axis range");
    //btnHrzShrink.setStyle(btnstyle);
    btnHrzShrink.setToolTip("expand x-axis range");
    chkAutoScale.setChecked(do_autoscale_on_load);
    //chkAutoScale.setStyle(btnstyle);
    chkOverlay.setChecked(!background_traces_hidden);
    //chkOverlay.setStyle(btnstyle);
    chkOverlay.setToolTip("overlay traces");

    currentYscale = &yScales["A"];

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

void RenderArea::paint(QPainter& painter, const QRect& rectangle)
{
    painter.setRenderHint(QPainter::Antialiasing, true);
    QFont font = painter.font();
    font.setPixelSize(24);
    if(button_row_height<0){
        QWidget* w=my_layout->itemAtPosition(0,0)->widget();
        assert(w!=nullptr);
        button_row_height = w->frameGeometry().bottom();
    }
    painter.setFont(font);
    if (noData()) {
        painter.drawText(rectangle, Qt::AlignHCenter | Qt::AlignVCenter, "no data to display");
    }
    else {
        if (isSelecting) {
            assert(tempPixMap != nullptr);
            painter.drawPixmap(rect(), *tempPixMap);
            drawMarquee(painter);
        }
        else {
            if (isXYmode() && xTrace.size() != yTrace.size()) {
                font.setPixelSize(16);
                painter.setFont(font);
                painter.drawText(rectangle, Qt::AlignHCenter | Qt::AlignVCenter,
                    "x- and y-trace: numbers of datapoints\nnot equal\n(required for YX-mode)");
            }
            else {
                const double y_min = currentYscale->y_min;
                const double y_max = currentYscale->y_max;
                setScaling(x_min, x_max, y_min, y_max);
                drawGrid(painter, show_grid_horz, show_grid_vert);
                if (!background_traces_hidden) {
                    // paint traces in persistance buffer
                    painter.setPen(color_bktrace);
                    for (auto trace : std::as_const(tracebuffer)) {
                        priv_Scale ys = yScales[trace->getYUnit()];
                        setScaling(x_min, x_max, ys.y_min, ys.y_max);
                        trace->render(painter, this);
                    }
                    painter.setPen(color_trace);
                }
                setScaling(x_min, x_max, y_min, y_max);
                yTrace.render(painter, this);

                font = painter.font();
                font.setPixelSize(16);
                painter.setFont(font);
                painter.setPen(QColor(200, 0, 0)); // some red
                QString label = QString("%L1 %2").arg(y_max).arg(yTrace.getYUnit());
                painter.drawText(rectangle, Qt::AlignHCenter | Qt::AlignTop, label);
                label = QString("%L1 %2").arg(y_min).arg(yTrace.getYUnit());
                painter.drawText(rectangle, Qt::AlignHCenter | Qt::AlignBottom, label);
                if (isXYmode()) {
                    label = QString("%L1 %2").arg(x_min).arg(xTrace.getYUnit());
                    painter.drawText(rectangle, Qt::AlignVCenter | Qt::AlignLeft, label);
                    label = QString("%L1 %2").arg(x_max).arg(xTrace.getYUnit());
                    painter.drawText(rectangle, Qt::AlignVCenter | Qt::AlignRight, label);
                }
                else {
                    label = QString("%L1 %2").arg(x_min).arg(yTrace.getXUnit());
                    painter.drawText(rectangle, Qt::AlignVCenter | Qt::AlignLeft, label);
                    label = QString("%L1 %2").arg(x_max).arg(yTrace.getXUnit());
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

void RenderArea::drawGrid(QPainter& painter, bool horizontal, bool vertical)
{
    auto zero_point = scaleToQPF(0.0, 0.0);
    painter.save();
    QPen penSolid = painter.pen();
    penSolid.setColor(color_grid);
    QPen penDashed{ penSolid };
    penDashed.setStyle(Qt::DotLine);
    QString info{};
    //penDashed.setWidthF(0.5);
    
    if (vertical && x_min < x_max) {
        double d_height = height();
        double vert_step = std::pow(10.0, std::floor(std::log10(x_max - x_min)));
        int vert_divs = static_cast<int>((x_max - x_min) / vert_step) + 1 ;
        if (vert_divs < 4) {
            vert_divs *= 4;
            vert_step /= 4.0;
        }
        info.append(QString("x: %L1/div").arg(vert_step));
        auto line_0 = std::ceil(x_min / vert_step) * vert_step;
        painter.setPen(penDashed);
        for (int i = 0; i < vert_divs; ++i) {
            double x = line_0 + i * vert_step;
            if (x == 0.0) continue;
            auto px = scaleToQPF(x, 0.0).x();
            painter.drawLine(QPointF(px, 0.0), QPointF(px, d_height));
        }
        if (x_min < 0.0 && x_max > 0.0) {
            // draw zero line
            painter.setPen(penSolid);
            painter.drawLine(QPointF(zero_point.x(),0.0),
                QPointF(zero_point.x(), static_cast<double>(height())));
        }
    }
    if (horizontal && currentYscale->y_min < currentYscale->y_max) {
        double d_width = width();
        double horz_step = std::pow(10.0, std::floor(std::log10(currentYscale->y_max - currentYscale->y_min)));
        if (info.length() > 0) {
            info.append(" | ");
        }
        info.append(QString("y: %L1/div").arg(horz_step));
        int horz_divs = static_cast<int>((currentYscale->y_max - currentYscale->y_min) / horz_step) + 1;
        auto line_0 = std::ceil(currentYscale->y_min / horz_step) * horz_step;
        painter.setPen(penDashed);
        for (int i = 0; i < horz_divs; ++i) {
            double y = line_0 + i * horz_step;
            if (y == 0.0) continue;
            auto py = scaleToQPF(0.0, y).y();
            painter.drawLine(QPointF(0.0, py), QPointF(d_width, py));
        }
        if (currentYscale->y_min < 0.0 && currentYscale->y_max>0.0) {
            // draw zero line
            painter.setPen(penSolid);
            painter.drawLine(QPointF(0.0, zero_point.y()),
                QPointF(d_width, zero_point.y()));
        }
    }

    if (info.length() > 0) {
        painter.setPen(penSolid);
        QFont font = painter.font();
        font.setPixelSize(12);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignLeft | Qt::AlignBottom, info);
    }
    painter.restore();
}

bool RenderArea::event(QEvent* event)
{
    if (event->type() == QEvent::Gesture)
        return gestureEvent(static_cast<QGestureEvent*>(event));
    return QWidget::event(event);
}

void RenderArea::paintEvent(QPaintEvent* event)
{
    (void)event;
    QPainter painter(this);
    paint(painter, QRect(0, button_row_height, width(), height() - button_row_height));
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        scaleFromPixToXY(event->x(), event->y(), x, y);
#else
        scaleFromPixToXY(event->position(), x, y);
#endif

        QString txt;
        if (isXYmode()) {
            txt = QString("(%1%2/%3%4)").arg(x).arg(xTrace.getYUnit()).arg(y).arg(yTrace.getYUnit());
        }
        else {
            double datay = yTrace.interp(x); //= std::numeric_limits<double>::quiet_NaN();
            txt = QString("(%1%2/%3%4)\ndata: %5%6").arg(x).arg(yTrace.getXUnit()).arg(y).arg(yTrace.getYUnit()).arg(datay).arg(yTrace.getYUnit());
        }
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        QToolTip::showText(event->globalPos(), txt, this, rect());
#else
        QToolTip::showText(event->globalPosition().toPoint(), txt, this, rect());
#endif
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
        chkOverlay.setChecked(!background_traces_hidden);
        update();
    } else if (response == actCopy) {
        copyToClipboard();
    }
    event->accept();
}

void RenderArea::mousePressEvent(QMouseEvent* event)
{
	if (noData() || isPinching ||
        (event->button() != Qt::MouseButton::LeftButton)) {
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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void RenderArea::enterEvent(QEvent* event)
#else
void RenderArea::enterEvent(QEnterEvent* event)
#endif
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

void RenderArea::mouseReleaseEvent(QMouseEvent* event)
{
    if (noData()) {
        event->ignore();
        return;
    }

    if (isSelecting && event->button() == Qt::MouseButton::LeftButton) {
        double x, y;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        scaleFromPixToXY(event->x(), event->y(), x, y);
#else
        scaleFromPixToXY(event->position(), x, y);
#endif
        isSelecting = false;
        delete tempPixMap; tempPixMap = nullptr;
        unsetCursor();
        // only zoom if there is a meanigful selection:
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        QPoint pe = event->pos();
#else
        QPoint pe = event->position().toPoint();
#endif
        if (pe.x() != selStart.x() && pe.y() != selStart.y()) {
            double xs, ys;
            scaleFromPixToXY(selStart.x(), selStart.y(), xs, ys);
            x_min = std::min(x, xs);
            x_max = std::max(x, xs);
            currentYscale->y_min = std::min(y, ys);
            currentYscale->y_max = std::max(y, ys);
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
    if (noData() || isPinching) {
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

/// <summary>
/// find min and max value while igoring NaNs
/// </summary>
/// <param name="first">const iterator of start of range</param>
/// <param name="last">const itereator one past last element</param>
/// <param name="min_val">ret: min val. of range, NaN if only NaNs are found</param>
/// <param name="max_val">ret: max val. of range, NaN if only NaNs are found</param>
static void find_min_max(std::vector<double>::const_iterator first,
    std::vector<double>::const_iterator last,
    double& min_val, double& max_val)
{
    min_val = max_val = *first;
    for (; first != last; ++first) {
        if (!std::isnan(*first)) {
            min_val = max_val = *first;
            break;
        }
    }
    for (; first != last; ++first) {
        auto val = *first;
        if (!std::isnan(val)) {
            min_val = std::min(min_val, val);
            max_val = std::max(max_val, val);
        }
    }
}

void RenderArea::autoScale()
{
    if (noData()) return;

    double g_x_min{std::numeric_limits<double>::max()},
        g_x_max{std::numeric_limits<double>::min()},
        g_y_min{std::numeric_limits<double>::max()},
        g_y_max{std::numeric_limits<double>::min()};
    // first find min/max for x
    if(isXYmode()){
        find_min_max(xTrace.data().cbegin(), xTrace.data().cend(), g_x_min, g_x_max);
    }
    else if (yTrace.has_x_trace()) {
        find_min_max(yTrace.x_data().cbegin(), yTrace.x_data().cend(), g_x_min, g_x_max);
    }
    else
    {
        g_x_min = yTrace.x0();
        g_x_max = yTrace.x0() + static_cast<double>(yTrace.size() - 1) * yTrace.deltax();
    }
    if(global_autoscale && !background_traces_hidden){
        for(const auto* t: tracebuffer){
            if(t->has_x_trace()){
                double minx, maxx;
                find_min_max(t->x_data().cbegin(), t->x_data().cend(), minx, maxx);
                g_x_min=std::min(g_x_min,minx);
                g_x_max=std::max(g_x_max, maxx);
            }
        }
    }
    x_min=g_x_min;
    x_max=g_x_max;


    yTrace.getDataMinMax(g_y_min, g_y_max);
    if(global_autoscale && !background_traces_hidden){
        for(const auto* t: tracebuffer){
            // only touch scaling for curent y-unit
            if(t->getYUnit()!=yTrace.getYUnit()) continue;
            auto [miny, maxy] = t->getDataMinMax();
            g_y_min = std::min(g_y_min, miny);
            g_y_max = std::max(g_y_max, maxy);
            }
    }
    currentYscale->y_min=g_y_min;
    currentYscale->y_max=g_y_max;
    update();
}

void RenderArea::verticalShrink()
{
    double nymin = 1.5 * currentYscale->y_min - 0.5 * currentYscale->y_max,
        nymax = 1.5 * currentYscale->y_max - 0.5 * currentYscale->y_min;
    currentYscale->y_min = nymin;
    currentYscale->y_max = nymax;
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

void RenderArea::toggleOverlay(int state)
{
    background_traces_hidden = state == Qt::CheckState::Unchecked;
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

void RenderArea::copyToClipboard()
{
    QPixmap pixmap(this->size());
    pixmap.fill(); // fill with white
    QPainter painter(&pixmap);
    paint(painter, pixmap.rect());
    QGuiApplication::clipboard()->setPixmap(pixmap);
}

void RenderArea::showSettingsDialog()
{
    DlgGraphSettings dlg(this);
    dlg.setValues(do_autoscale_on_load, global_autoscale,
        x_min, x_max, currentYscale->y_min, currentYscale->y_max, numtraces,
        show_grid_horz, show_grid_vert, shift_all_y_scales, color_grid, color_trace, color_bktrace);
    if (dlg.exec()) {
        settings_modified = true;
        dlg.getValues(do_autoscale_on_load, global_autoscale,
            x_min, x_max, currentYscale->y_min, currentYscale->y_max, numtraces,
            show_grid_horz, show_grid_vert, shift_all_y_scales, color_grid, color_trace, color_bktrace);
        // if numtraces has been reduced we want to get rid of excess traces
        while (tracebuffer.size() > numtraces) {
            delete tracebuffer.dequeue();
        }
        update();
    }
}

//static double zoom_tranform(double v_max, double v, double factor)
//{
//    return v + (v_max - v) / factor;
//}

void RenderArea::zoomIn(double x_center, double y_center, double factor)
{
    x_min = x_center - (x_center - x_min) / factor;
    x_max = x_center + (x_max - x_center) / factor;
    currentYscale->y_min = y_center - (y_center - currentYscale->y_min) / factor;
    currentYscale->y_max = y_center + (currentYscale->y_max - y_center) / factor;
    update();
}

void RenderArea::renderTrace(const hkLib::hkTreeNode* TrRecord, std::istream& infile)
{
    using namespace hkLib;
    char dataformat = TrRecord->getChar(TrDataFormat);
    uint16_t tracedatakind = TrRecord->extractUInt16(TrDataKind);
    clipped = tracedatakind & ClipBit;
    ndatapoints = TrRecord->extractValue<uint32_t>(TrDataPoints);
	try {
        std::vector<double> new_data(ndatapoints);
		if (dataformat == DFT_int16) {
			ReadScaleAndConvert<int16_t>(infile, *TrRecord, ndatapoints, new_data.data());
		}
		else if (dataformat == DFT_int32) {
			ReadScaleAndConvert<int32_t>(infile, *TrRecord, ndatapoints, new_data.data());
		}
		else if (dataformat == DFT_float) {
			ReadScaleAndConvert<float>(infile, *TrRecord, ndatapoints, new_data.data());
		}
		else if (dataformat == DFT_double) {
			ReadScaleAndConvert<double>(infile, *TrRecord, ndatapoints, new_data.data());
		}
		else {
			QMessageBox::warning(this, QString("Data Format Error"), QString("Unknown Dataformat"));
			return;
		}
        
        addTrace(DisplayTrace(
            qs_from_sv(TrRecord->getString<8>(TrXUnit)),
            qs_from_sv(TrRecord->getString<8>(TrYUnit)),
            TrRecord->extractLongReal(TrXStart),
            TrRecord->extractLongReal(TrXInterval),
            std::move(new_data)
            )
        );
	}
	catch (const std::exception& e) {
//        newYtrace.data.clear();
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
    currentYscale = &yScales[yTrace.getYUnit()];
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

bool RenderArea::gestureEvent(QGestureEvent* event)
{
    if (noData()) return false;
    if (QGesture* pinch = event->gesture(Qt::PinchGesture))
        pinchTriggered(static_cast<QPinchGesture*>(pinch));
    return true;
}

void RenderArea::pinchTriggered(QPinchGesture* gesture)
{
    auto change_flags = gesture->changeFlags();
    auto state = gesture->state();
    if (state == Qt::GestureStarted) { isPinching = true; }
    else if (state == Qt::GestureFinished || state == Qt::GestureCanceled) {
        isPinching = false;
    }
    if (change_flags & QPinchGesture::ScaleFactorChanged) {
        auto scale = gesture->scaleFactor();
        auto center = mapFromGlobal(gesture->centerPoint());
        double x_c{}, y_c{};
        scaleFromPixToXY(center, x_c, y_c);

        //scaleFromPixToXY(width() / 2, height() / 2, x_c, y_c);
        zoomIn(x_c, y_c, scale);       
    }
}

void RenderArea::setScaling(double x_0, double x_1, double y_0, double y_1)
{
    double h = height() - 1 - button_row_height, w = width() - 1;
    a_x = -w*x_0/(x_1-x_0);
    b_x = w/(x_1-x_0);
    a_y = h*y_1/(y_1-y_0) + button_row_height;
    b_y = -h/(y_1-y_0);
}

QPointF RenderArea::scaleToQPF(double x, double y)
{
    return QPointF(a_x+b_x*x, a_y+b_y*y);
}

void RenderArea::scaleFromPixToXY(double px, double py, double& x, double& y)
{
    x = x_min + px / double(width()) * (x_max - x_min);
    y = currentYscale->y_max - (py - button_row_height) / double(height() - button_row_height) * (currentYscale->y_max - currentYscale->y_min);
}

void RenderArea::scaleFromPixToXY(const QPointF& p, double& x, double& y)
{
    scaleFromPixToXY(p.x(), p.y(), x, y);
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
        if (shift_all_y_scales) {
            for (auto& ys : yScales) {
                //shift each y scale
                auto dy = -(ys.y_max - ys.y_min) / height() * double(shift.y());
                ys.y_max += dy;
                ys.y_min += dy;
            }
        }
        else {
            auto& ys = *currentYscale;
            auto dy = -(ys.y_max - ys.y_min) / height() * double(shift.y());
            ys.y_max += dy;
            ys.y_min += dy;
        }
        update();
    }
}

void RenderArea::loadSettings()
{
    QSettings s;
    s.beginGroup("renderarea");
    do_autoscale_on_load = s.value("do_autoscale_on_load", do_autoscale_on_load).toBool();
    global_autoscale = s.value("global_autoscale", global_autoscale).toBool();
    shift_all_y_scales = s.value("shift_all_y_scales", int(shift_all_y_scales)).toInt();
    show_grid_horz = s.value("show_grid_horz", int(show_grid_horz)).toInt();
    show_grid_vert = s.value("show_grid_vert", int(show_grid_vert)).toInt();
    background_traces_hidden = !s.value("overlay", 1).toInt();
    chkAutoScale.setChecked(do_autoscale_on_load);
    chkOverlay.setChecked(!background_traces_hidden);
    numtraces = s.value("numtraces", numtraces).toInt();
    color_grid = s.value("color_grid", color_grid).value<QColor>();
    color_trace = s.value("color_trace", color_trace).value<QColor>();
    color_bktrace = s.value("color_bktrace", color_bktrace).value<QColor>();
    s.endGroup();
}

void RenderArea::saveSettings()
{
    QSettings s;
    s.beginGroup("renderarea");
    s.setValue("do_autoscale_on_load", do_autoscale_on_load);
    s.setValue("global_autoscale", global_autoscale);
    s.setValue("shift_all_y_scales", int(shift_all_y_scales));
    s.setValue("show_grid_horz", int(show_grid_horz));
    s.setValue("show_grid_vert", int(show_grid_vert));
    s.setValue("overlay", int(!background_traces_hidden));
    s.setValue("numtraces", numtraces);
    s.setValue("color_grid", color_grid);
    s.setValue("color_trace", color_trace);
    s.setValue("color_bktrace", color_bktrace);
    s.endGroup();
}
