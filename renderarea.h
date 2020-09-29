#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QWidget>
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
    size_t ndatapoints;
    double* data;
    double x0, deltax, y_min, y_max;
    Ui::RenderArea *ui;
};

#endif // RENDERAREA_H
