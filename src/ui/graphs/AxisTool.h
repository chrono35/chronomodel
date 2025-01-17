#ifndef AXISTOOL_H
#define AXISTOOL_H

#include <QVector>
#include <QPainter>
#include <QRectF>
#include <QWidget>
#include "StdUtilities.h"


class AxisTool
{
public:
    AxisTool();
    void updateValues(double totalPix, double minDeltaPix, double minVal, double maxVal);
    QVector<qreal> paint(QPainter& p, const QRectF& r, qreal heigthSize, QString (*valueFormatFunc)(double) = NULL);
    
public:
    bool mIsHorizontal;
    bool mShowSubs;
    bool mShowSubSubs;
    bool mShowText;
    bool mMinMaxOnly;
    bool mShowArrow;
    
    double mDeltaVal;
    double mDeltaPix;
    double mStartVal;
    double mEndVal;
    double mStartPix;
    double mPixelsPerUnit;
    QFont    mfont;
    
    QColor mAxisColor;
    QString mLegend;
};

class AxisWidget: public QWidget, public AxisTool{
public:
    AxisWidget(FormatFunc funct = 0, QWidget* parent = 0);
    
protected:
    void paintEvent(QPaintEvent* e);
    
private:
    FormatFunc mFormatFunct;
    
public:
    int mMarginLeft;
    int mMarginRight;
};

#endif
