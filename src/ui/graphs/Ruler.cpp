#include "Ruler.h"
#include "Painting.h"
#include "AxisTool.h"
#include "StdUtilities.h"
#include <QtWidgets>
#include <iostream>


Ruler::Ruler(QWidget* parent, Qt::WindowFlags flags):QWidget(parent, flags),
mCurrentMin(0),
mCurrentMax(1000),
mMin(0),
mMax(1000),
mZoomProp(1.),
mStepMinWidth(50),//define secondary scale
mStepWidth(100)
{
    mScrollBarHeight = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    
    setMouseTracking(true);
    
    mScrollBar = new QScrollBar(Qt::Horizontal, this);
    mScrollBar->setRange(0, 0);
    mScrollBar->setSingleStep(1);
    mScrollBar->setPageStep(10000);
    mScrollBar->setTracking(true);
    
    connect(mScrollBar, SIGNAL(valueChanged(int)), this, SLOT(updateScroll()));
    //connect(mScrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollValueChanged(int)));
    
    mAxisTool.mIsHorizontal = true;
    mAxisTool.mShowArrow = false;
    
    /*
     mScrollBarHeight = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
     
     setMouseTracking(true);
     
     mScrollBar = new QScrollBar(Qt::Horizontal, this);
     mScrollBar->setRange(0, 0);
     mScrollBar->setSingleStep(1);
     mScrollBar->setPageStep(10000);
     mScrollBar->setTracking(true);
     
     connect(mScrollBar, SIGNAL(valueChanged(int)), this, SLOT(updateScroll()));
     }
     */
    
    
    
}

Ruler::~Ruler()
{
    
}

#pragma mark Areas
void Ruler::clearAreas()
{
    mAreas.clear();
    update();
}

void Ruler::addArea(double start, double end, const QColor& color)
{
    RulerArea area;
    area.mStart = start;
    area.mStop = end;
    area.mColor = color;
    mAreas.append(area);
    update();
}

#pragma mark Range & Zoom & Scroll & Current

double Ruler::getRealValue()
{
    return realPosition;
}

void Ruler::scrollValueChanged(int value)
{
    emit valueChanged(value);
}

void Ruler::setRange(const double min, const double max)
{
    if(mMin != min || mMax || max)
    {
        mMin = min;
        mMax = max;
    }
}

void Ruler::setCurrent(const double min, const double max)
{
    //qDebug()<<" Ruler::setCurrent mCurrentMin 0" << mCurrentMin<<" "<< mCurrentMax;
    //qDebug()<<" Ruler::setCurrent min 1" << min<<" "<< max;
    
    mCurrentMin = min;
    mCurrentMax = max;
 
    int fullScrollSteps = 1000;
    mScrollBar->setRange(0, fullScrollSteps);
    
    double position = mCurrentMin;
    position = (position - mMin) / (mMax-mMin) * double (mScrollBar->maximum());
    mScrollBar->setTracking(false);
    mScrollBar->setSliderPosition(floor(position));//  setValue(floor(position));
    mScrollBar->setTracking(true);
    
  //
    mAxisTool.updateValues(mRulerRect.width(), mStepMinWidth, mCurrentMin, mCurrentMax);
   // qDebug()<<" Ruler::setCurrent mCurrentMin 3" << mCurrentMin<<" "<< mCurrentMax<<" position"<<position;

    
    update();

}
void Ruler::currentChanged(const double min, const double max)
{    
    setCurrent(min, max);
    
    emit positionChanged(mCurrentMin, mCurrentMax);
    
    update();
    
}
double Ruler::getZoom()
{
    return (100.-mZoomProp);
}


void Ruler::setZoom(int prop)
{
    // Ici, 10 correspond à la différence minimale de valeur (quand le zoom est le plus fort)
    double minProp = 1. / (mMax - mMin);   //10. / (mMax - mMin);
    
  //  mZoomProp = (100. - prop) / 100.;
    mZoomProp = prop /100;
    if(mZoomProp < minProp) mZoomProp = minProp;
    
    if(mZoomProp != 1)
    {
        // Remember old scroll position
        double posProp = 0;
        double rangeBefore = (double)mScrollBar->maximum();
        if(rangeBefore > 0)
            posProp = (double)mScrollBar->value() / rangeBefore;
        
        // Update Scroll Range
        int fullScrollSteps = 1000;
        int scrollSteps = (1. - mZoomProp) * (double)fullScrollSteps;
        mScrollBar->setRange(0, scrollSteps);
        mScrollBar->setPageStep(fullScrollSteps);
        
        // Set scroll to correct position
        double pos = 0;
        double rangeAfter = (double)mScrollBar->maximum();
        if(rangeAfter > 0)
            pos = floor(posProp * rangeAfter);
        mScrollBar->setValue(pos);
       /* mScrollBar->setTracking(false);
        mScrollBar->setSliderPosition(pos);
        mScrollBar->setTracking(true);*/
    }
    else
    {
        mScrollBar->setRange(0, 0);
    }
    //updateScroll();
    
    
    
 /*   if(mZoomProp != 1)
    {
        double delta = mZoomProp * (mMax - mMin);
        double deltaStart = (mMax - mMin) - delta;
        
        mCurrentMin = mMin + deltaStart * ((double)mScrollBar->value() / (double)mScrollBar->maximum());
        mCurrentMax = mCurrentMin + delta;
    }
    else
    {
        mCurrentMin = mMin;
        mCurrentMax = mMax;
    }
    qDebug()<<"Ruler::setZoom() mCurrentMin"<< mCurrentMin<<" "<<mCurrentMax;
    mAxisTool.updateValues(mRulerRect.width(), mStepMinWidth, mCurrentMin, mCurrentMax);
   */
  
    
    update();
    
}

void Ruler::updateScroll()
{
    //qDebug()<<"Ruler::updateScroll() mCurrentMin"<< mCurrentMin<<" mCurrentMax"<<mCurrentMax;
    //if(mZoomProp != 1)
    if( (mCurrentMax - mCurrentMin) != (mMax - mMin))
    {
        double delta = mCurrentMax - mCurrentMin;
        double deltaStart = (mMax - mMin)-delta;
        
    //qDebug()<<"Ruler::updateScroll() delta"<< delta<<" deltaStart"<<deltaStart;
        
        mCurrentMin = mMin + deltaStart * ((double)mScrollBar->value() / (double)mScrollBar->maximum());
        mCurrentMin = floor( inRange(mMin, mCurrentMin, mMax) );
        mCurrentMax = mCurrentMin + delta;
        
    }
    else
    {
        mCurrentMin = mMin;
        mCurrentMax = mMax;
    }

    //qDebug()<<"Ruler::updateScroll() mCurrentMin"<< mCurrentMin<<" "<<mCurrentMax<<" value"<<mScrollBar->value()<<" maximun"<<mScrollBar->maximum();
    mAxisTool.updateValues(mRulerRect.width(), mStepMinWidth, mCurrentMin, mCurrentMax);
    
    emit positionChanged(mCurrentMin, mCurrentMax);
    
    update();
    
    /* original code by HL
     void Ruler::updateScroll()
     {
     if(mZoomProp != 1)
     {
     double delta = mZoomProp * (mMax - mMin);
     double deltaStart = (mMax - mMin) - delta;
     
     mCurrentMin = mMin + deltaStart * ((double)mScrollBar->value() / (double)mScrollBar->maximum());
     mCurrentMax = mCurrentMin + delta;
     }
     else
     {
     mCurrentMin = mMin;
     mCurrentMax = mMax;
     }
     mAxisTool.updateValues(mRulerRect.width(), mStepMinWidth, mCurrentMin, mCurrentMax);
     
     emit positionChanged(mCurrentMin, mCurrentMax);
     
     update();
     }
     */
    
}

#pragma mark Layout & Paint
void Ruler::layout()
{
    int w = width();
    int h = height();
    
    mScrollBar->setGeometry(0, 0, w, mScrollBarHeight);
    mRulerRect = QRectF(0, mScrollBarHeight, w, h - mScrollBarHeight);
    
    mAxisTool.updateValues(mRulerRect.width(), mStepMinWidth, mCurrentMin, mCurrentMax);
    update();
}

void Ruler::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);
    layout();
}

void Ruler::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);
    
    double w = mRulerRect.width();
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    painter.fillRect(mRulerRect, Qt::white);
    
    // ----------------------------------------------
    //  Areas
    // ----------------------------------------------
    
    for(int i=0; i<mAreas.size(); ++i)
    {
        if(mAreas[i].mStart < mCurrentMax && mAreas[i].mStop > mCurrentMin)
        {
            double x1 = w * (mAreas[i].mStart - mCurrentMin) / (mCurrentMax - mCurrentMin);
            double x2 = w;
            if(mAreas[i].mStop < mCurrentMax)
                x2 = w * (mAreas[i].mStop - mCurrentMin) / (mCurrentMax - mCurrentMin);
            
            painter.setPen(mAreas[i].mColor);
            painter.setBrush(mAreas[i].mColor);
            painter.drawRect(mRulerRect.x() + x1, mRulerRect.y(), x2 - x1, mRulerRect.height());
        }
    }

    painter.setPen(Qt::black);
    
    // ----------------------------------------------
    //  Axis
    // ----------------------------------------------
    
    QFont font = painter.font();
    font.setPointSizeF(pointSize(9.));
    painter.setFont(font);
    
    //mAxisTool.paint(painter, mRulerRect, mStepMinWidth);
    mAxisTool.paint(painter, mRulerRect, 10);
    
}

