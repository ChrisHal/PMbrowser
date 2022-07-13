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

#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QtGlobal>
#include <QWidget>
#include <QVector>
#include <QQueue>
#include <QPointF>
#include <QPixmap>
#include <QPushButton>
#include <QCheckBox>
//#include <QStyle>
//#include <QStyleFactory>
#include <QGridLayout>
#include <istream>
#include "hkTree.h"
#include "DisplayTrace.h"
class DisplayTrace;

namespace Ui {
class RenderArea;
}

class RenderArea : public QWidget
{
    Q_OBJECT

public:
    explicit RenderArea(QWidget *parent = nullptr);
    ~RenderArea();
    bool noData() { return !yTrace.isValid(); };
    void renderTrace(hkTreeNode* trace, std::istream& infile);
    void addTrace(DisplayTrace&& dt);

    /// <summary>
    /// dt_x is a x-y-trace (usually representing a stimulus)
    /// that is to be used as a template to create a trace
    /// that is used as the x-trace. This sets the renderares
    /// to xy-mode.
    /// Especiually usefull for ramps where the voltage trace
    /// has not been recorded.
    /// </summary>
    /// <param name="dt_x">x-y-trace, usually a stimulus</param>
    void createInterpolatedXtrace(DisplayTrace&& dt_x);
    void clearTrace();

    /// <summary>
    /// checks if we display is in x-y-mode,
    /// i.e. a data-trace is displayed versus a
    /// x-traces
    /// </summary>
    /// <returns>true id in xy-mode</returns>
    bool isXYmode() { return xTrace.isValid(); };

    bool YtraceHasX() { return (yTrace.isValid() && yTrace.has_x_trace()); };
    bool isSettingsModified() { return settings_modified; };
    bool isAutoscaleEnabled() { return do_autoscale_on_load; };
    void saveSettings();
    void loadSettings();

public slots:
    void showSettingsDialog();
    void autoScale();
    void verticalShrink();
    void horizontalShrink();
    void toggleDoAutoscale(bool checked);
    void toggleDoAutoscale2(int checked);
    void toggleOverlay(int checked);
    void wipeAll() { clearTrace(); };
    void wipeBuffer();
    void setXYmode();
    void setYTmode();
    void copyToClipboard();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEvent* event) override;
#else
    void enterEvent(QEnterEvent* event) override;
#endif
    void leaveEvent(QEvent* event) override;
    //void resizeEvent(QResizeEvent* event) override;

private:
    void setScaling(double x_0, double x_1, double y_0, double y_1);
    QPointF scaleToQPF(double x, double y);
    void scaleFromPixToXY(int px, int py, double& x, double& y);
    void scaleFromPixToXY(const QPointF& p, double& x, double& y);
    void shiftByPixel(QPoint shift);
    void zoomIn(double x_center, double y_center, double factor);
    void drawMarquee(QPainter& painter);
    void doContextMenu(QContextMenuEvent* event);

    //std::unique_ptr<QStyle> p_btnstyle{ QStyleFactory::create("fusion") };
    QPushButton btnWipe, btnAutoScale, btnVertShrink, btnHrzShrink;
    QCheckBox chkAutoScale, chkOverlay;
    QGridLayout* my_layout{};
    int button_row_height{};

    size_t ndatapoints;
    DisplayTrace xTrace, yTrace; // TODO at least yTrace should be a pointer?
    QQueue<DisplayTrace*> tracebuffer;
    bool background_traces_hidden;
    bool clipped; // Amp. was clipping
    double x_min, x_max, y_min, y_max;
    double a_x, b_x, a_y, b_y; // for scaling
    int numtraces; // number of traces in persistance buffer
    bool do_autoscale_on_load;
    bool isTraceDragging;
    // for marquee zoom function:
    bool isSelecting;
    QPoint selStart, selEnd;
    QPixmap* tempPixMap;
    bool settings_modified;

    friend class DisplayTrace;
};

#endif // RENDERAREA_H
