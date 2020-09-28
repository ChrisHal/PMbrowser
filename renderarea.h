#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QWidget>

namespace Ui {
class RenderArea;
}

class RenderArea : public QWidget
{
    Q_OBJECT

public:
    explicit RenderArea(QWidget *parent = nullptr);
    ~RenderArea();

private:
    Ui::RenderArea *ui;
};

#endif // RENDERAREA_H
