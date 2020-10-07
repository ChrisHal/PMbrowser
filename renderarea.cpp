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
#include <stdexcept>
#include <algorithm>
#include "renderarea.h"
#include "ui_renderarea.h"
#include "DatFile.h"


RenderArea::RenderArea(QWidget *parent) :
    QWidget(parent), ndatapoints{}, data{}, xunit{}, yunit{}, clipped{ false }, x0{}, deltax{},
    y_min{}, y_max{}, a_x{}, b_x{}, a_y{}, b_y{},
    ui{ nullptr }
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
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QFont font = painter.font();
    font.setPixelSize(24);
    painter.setFont(font);
    //painter.drawPath(path);
    const QRect rectangle = QRect(0, 0, width(), height());
    if(data.size()==0) {
    painter.drawText(rectangle,Qt::AlignHCenter|Qt::AlignVCenter,"no trace selected");
    } else {
        double x1 = x0 + (ndatapoints - 1) * deltax;
        setScaling(x0, x1, y_min, y_max);
        path.moveTo(scaleToQPF(x0,data[0]));
        for(int i=0; i<data.size(); ++i) {
            path.lineTo(scaleToQPF(x0+i*deltax, data[i]));
        }
        painter.drawPath(path);
        font = painter.font();
        font.setPixelSize(16);
        painter.setFont(font);
        painter.setPen(QColor(200, 0, 0)); // some red
        QString label = QString("%1 %2").arg(y_max).arg(yunit);
        painter.drawText(rectangle, Qt::AlignHCenter | Qt::AlignTop, label);
        label = QString("%1 %2").arg(y_min).arg(yunit);
        painter.drawText(rectangle, Qt::AlignHCenter | Qt::AlignBottom, label);
        label = QString("%1 %2").arg(x0).arg(xunit);
        painter.drawText(rectangle, Qt::AlignVCenter | Qt::AlignLeft, label);
        label = QString("%1 %2").arg(x1).arg(xunit);
        painter.drawText(rectangle, Qt::AlignVCenter | Qt::AlignRight, label);
        if (clipped) {
            painter.setBrush(QColor(200, 0, 0));
            painter.setPen(QColor(200, 0, 0));
            auto br = painter.boundingRect(rectangle, Qt::AlignCenter, QString("clipping"));
            painter.drawRect(br);
            painter.setPen(QColor(255, 255, 255));
            painter.drawText(rectangle, Qt::AlignCenter, QString("clipping"));
        }
    }
}

void RenderArea::renderTrace(hkTreeNode* TrRecord, std::istream& infile)
{
    char dataformat = TrRecord->getChar(TrDataFormat);
    if (dataformat != DFT_int16) {
        throw std::runtime_error("can't export data that is not int16");
    }
    int32_t interleavesize;
    try {
        interleavesize = TrRecord->extractInt32(TrInterleaveSize);
    }
    catch (std::out_of_range& e) { // fileformat too old to have interleave entry
        (void)e;
        interleavesize = 0;
    }
    assert(interleavesize == 0);
    uint16_t tracedatakind = TrRecord->extractUInt16(TrDataKind);
    bool need_swap = !(tracedatakind & LittleEndianBit);
    clipped = tracedatakind & ClipBit;
    yunit = TrRecord->getString(TrYUnit).c_str(); // assuming the string is zero terminated...
    xunit = TrRecord->getString(TrXUnit).c_str();
    x0 = TrRecord->extractLongReal(TrXStart), deltax = TrRecord->extractLongReal(TrXInterval);
    double datascaler = TrRecord->extractLongReal(TrDataScaler);
    int32_t trdata = TrRecord->extractInt32(TrData);
    ndatapoints = TrRecord->extractInt32(TrDataPoints);
    int16_t* source = new int16_t[ndatapoints];
    data.clear();
    infile.seekg(trdata);
    infile.read((char*)source, sizeof(int16_t) * ndatapoints);
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

    y_min = *std::min_element(data.constBegin(), data.constEnd());
    y_max = *std::max_element(data.constBegin(), data.constEnd());

    delete[] source; source = nullptr;

    update();
}

void RenderArea::clearTrace()
{
    ndatapoints = 0;
    data.clear();
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
