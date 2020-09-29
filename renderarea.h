#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <istream>
#include "hkTree.h"

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

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setScaling(double x_0, double x_1, double y_0, double y_1);
    QPointF scaleToQPF(double x, double y);
    size_t ndatapoints;
    QVector<double> data;
    QString xunit, yunit;
    double x0, deltax, y_min, y_max;
    double a_x, b_x, a_y, b_y; // for scaling
    Ui::RenderArea *ui;
};

#endif // RENDERAREA_H
