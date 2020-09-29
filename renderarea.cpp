#include <QtGui>
#include "renderarea.h"
#include "ui_renderarea.h"

int test = 0;

RenderArea::RenderArea(QWidget *parent) :
    QWidget(parent)//,
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
    path.moveTo(0,0);
    path.lineTo(width(),height());
    QPainter painter(this);
    QFont font = painter.font();
    font.setPixelSize(24);
    painter.setFont(font);
    painter.drawPath(path);
    const QRect rectangle = QRect(0, 0, width(), height());
    if(test==0) {
    painter.drawText(rectangle,Qt::AlignHCenter|Qt::AlignVCenter,"no trace selected");
    } else {
        painter.drawText(rectangle,Qt::AlignHCenter|Qt::AlignVCenter,"ok");
    }
}

void RenderArea::renderTrace(hkTreeNode* trace, std::istream& infile)
{
    test = 1;//
    update();
}

void RenderArea::clearTrace()
{
    test = 0;
    update();
}
