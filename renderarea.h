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

#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QWidget>
#include <QVector>
#include <QQueue>
#include <QPointF>
#include <QPixmap>
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
    void renderTrace(hkTreeNode* trace, std::istream& infile);
    void clearTrace();
    bool isXYmode() { return xTrace.isValid(); };
    bool isSettingsModified() { return settings_modified; };
    bool isAutoscaleEnabled() { return do_autoscale_on_load; };
    void saveSettings();
    void loadSettings();

public slots:
    void showSettingsDialog();
    void autoScale();
    void toggleDoAutoscale(bool checked);
    void wipeAll() { clearTrace(); };
    void wipeBuffer();
    void setXYmode();
    void setYTmode();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event);
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void setScaling(double x_0, double x_1, double y_0, double y_1);
    QPointF scaleToQPF(double x, double y);
    void scaleFromPixToXY(int px, int py, double& x, double& y);
    void zoomIn(double x_center, double y_center, double factor);
    void drawMarquee(QPainter& painter);
    void doContextMenu(QContextMenuEvent* event);
    size_t ndatapoints;
    DisplayTrace yTrace, xTrace; // TODO at least yTrace should be a pointer?
    QQueue<DisplayTrace*> tracebuffer;
    bool background_traces_hidden;
    bool clipped; // Amp. was clipping
    double x_min, x_max, y_min, y_max;
    double a_x, b_x, a_y, b_y; // for scaling
    int numtraces; // number of traces in persistance buffer
    bool do_autoscale_on_load;
    // for marquee zoom function:
    bool isSelecting;
    QPoint selStart, selEnd;
    QPixmap* tempPixMap;
    bool settings_modified;

    friend class DisplayTrace;
};

#endif // RENDERAREA_H
