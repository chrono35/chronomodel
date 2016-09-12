﻿#include "GraphView.h"
#include "Ruler.h"
#include "StdUtilities.h"
#include "QtUtilities.h"
#include "DateUtils.h"
#include "Painting.h"
#include "MainWindow.h"
#include <QtWidgets>
#include <algorithm>
#include <QtSvg>
#include <QLocale>

class ProjectSettings;

#pragma mark Constructor / Destructor

#if GRAPH_OPENGL
GraphView::GraphView(QWidget *parent):QOpenGLWidget(parent),
#else
GraphView::GraphView(QWidget *parent):QWidget(parent),
#endif
mStepMinWidth(100), // define secondary scale on axis
mXAxisLine(true),
mXAxisArrow(true),
mXAxisTicks(true),
mXAxisSubTicks(true),
mXAxisValues(true),
mYAxisLine(true),
mYAxisArrow(true),
mYAxisTicks(true),
mYAxisSubTicks(true),
mYAxisValues(true),
mXAxisMode(eAllTicks),
mYAxisMode(eAllTicks),
mRendering(eSD),
mAutoAdjustYScale(false),
//mFormatFuncX(0),
//mFormatFuncY(0),
mShowInfos(false),
mBackgroundColor(Qt::white),
mThickness(1),
mOpacity(100),
mCanControlOpacity(false),
mTipX(0.),
mTipY(0.),
mTipWidth(110.),
mTipHeight(40.),
mTipVisible(false),
mUseTip(true)
{
    mAxisToolX.mIsHorizontal = true;
    mAxisToolX.mShowArrow = true;
    mAxisToolY.mIsHorizontal = false;
    mAxisToolX.mShowArrow = true;
    mAxisToolY.mShowSubs = false;
    
    mTipRect.setTop(0);
    mTipRect.setLeft(0);
    mTipRect.setWidth(mTipWidth);
    mTipRect.setHeight(mTipHeight);
    
    setMouseTracking(true);
    setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred));
    
    setRangeY(0.f, 1.f);
    this->mAxisToolX.updateValues(width(), mStepMinWidth, mCurrentMinX, mCurrentMaxX);
    resetNothingMessage();

    connect(this, &GraphView::signalCurvesThickness, this, &GraphView::updateCurvesThickness);
  
}

GraphView::~GraphView()
{
   mCurves.clear();
   mZones.clear();
}

// ------------------------------------------------------
//  Zoom X
// ------------------------------------------------------
#pragma mark Zoom X
void GraphView::zoomX(const float min, const float max)
{
    if (mCurrentMinX != min || mCurrentMaxX || max) {
        mCurrentMinX = min;
        mCurrentMaxX = max;
        
        this->mAxisToolX.updateValues(width(), 10, min, max);
        if (mAutoAdjustYScale) {
            float yMax = -100000000.f;
            float yMin =  100000000.f;
            for(int curveIndex=0; curveIndex<mCurves.size(); ++curveIndex) {
                const GraphCurve& curve = mCurves.at(curveIndex);
                if (curve.mIsHorizontalLine) {
                    yMax = qMax(yMax, curve.mHorizontalValue);
                    yMin = qMin(yMin, curve.mHorizontalValue);
                    
                } else if (curve.mUseVectorData) {
                    QVector<float> subData = getVectorDataInRange(curve.mDataVector, mCurrentMinX, mCurrentMaxX, min, max);
                    yMax = qMax(yMax, vector_max_value(subData));
                    yMin = qMin(yMin, vector_min_value(subData));
                    
                } else if (!curve.mIsVertical && !curve.mIsVerticalLine && !curve.mIsHorizontalSections) {
                    QMap<float, float> subData = getMapDataInRange(curve.mData, mCurrentMinX, mCurrentMaxX);
                    yMax = qMax(yMax, map_max_value(subData));
                    yMin = qMin(yMin, map_min_value(subData));
                }
            }
            if(yMax > yMin)
                setRangeY(yMin, yMax);
        }
        repaintGraph(true);
    }
      // ici ca marche  qDebug()<<"apres GraphView::zoomX"<<mCurrentMaxX<<" max"<<max;
     //if (mCurrentMaxX!= 1000) qDebug()<<"-----------in GraphView::zoomX mCurrentMaxX"<<mCurrentMaxX;
    update();
    
}

// ------------------------------------------------------
//  Options
// ------------------------------------------------------
#pragma mark Options

void GraphView::setBackgroundColor(const QColor& aColor)
{
    mBackgroundColor = aColor;
    repaintGraph(true);
}

QColor GraphView::getBackgroundColor() const
{
    return mBackgroundColor;
}

void GraphView::addInfo(const QString& info)
{
    mInfos << info;
    if(mShowInfos)
        repaintGraph(false);
}

void GraphView::clearInfos()
{
    mInfos.clear();
    if(mShowInfos)
        repaintGraph(false);
}

void GraphView::showInfos(bool show)
{
    mShowInfos = show;
    repaintGraph(true);
}

void GraphView::setNothingMessage(const QString& message)
{
    mNothingMessage = message;
    repaintGraph(true);
}

void GraphView::resetNothingMessage()
{
    mNothingMessage = tr("Nothing to display");
    repaintGraph(true);
}

void GraphView::setRendering(GraphView::Rendering render)
{
    mRendering = render;
    repaintGraph(true);
}

GraphView::Rendering GraphView::getRendering()
{
    return mRendering;
}

void GraphView::showXAxisLine(bool show)     {if(mXAxisLine != show){mXAxisLine = show; repaintGraph(true);} }
void GraphView::showXAxisArrow(bool show)    {if(mXAxisArrow != show){mXAxisArrow = show; repaintGraph(true);} }
void GraphView::showXAxisTicks(bool show)    {if(mXAxisTicks != show){mXAxisTicks = show; repaintGraph(true);} }
void GraphView::showXAxisSubTicks(bool show) {if(mXAxisSubTicks != show){mXAxisSubTicks = show; repaintGraph(true);} }
void GraphView::showXAxisValues(bool show)   {if(mXAxisValues != show){mXAxisValues = show; repaintGraph(true);} }

void GraphView::showYAxisLine(bool show)     {if(mYAxisLine != show){mYAxisLine = show; repaintGraph(true);} }
void GraphView::showYAxisArrow(bool show)    {if(mYAxisArrow != show){mYAxisArrow = show; repaintGraph(true);} }
void GraphView::showYAxisTicks(bool show)    {if(mYAxisTicks != show){mYAxisTicks = show; repaintGraph(true);} }
void GraphView::showYAxisSubTicks(bool show) {if(mYAxisSubTicks != show){mYAxisSubTicks = show; repaintGraph(true);} }
void GraphView::showYAxisValues(bool show)   {if(mYAxisValues != show){mYAxisValues = show; repaintGraph(true);} }


void GraphView::setXAxisMode(AxisMode mode)
{
    if (mXAxisMode != mode) {
        mXAxisMode = mode;
        mAxisToolX.mShowText = (mXAxisMode!=eHidden);
        repaintGraph(true);
    }
}

void GraphView::setYAxisMode(AxisMode mode)
{
    if (mYAxisMode != mode) {
        mYAxisMode = mode;
        showYAxisValues(true);
        showYAxisTicks(true);
        showYAxisSubTicks(true);
        
        if (mYAxisMode==eMinMax) {
            showYAxisValues(true);
            showYAxisTicks(false);
            showYAxisSubTicks(false);
        }

        mAxisToolY.mMinMaxOnly = (mYAxisMode == eMinMax);
        
        if (mYAxisMode==eHidden) {
            showYAxisValues(false);
            showYAxisTicks(false);
            showYAxisSubTicks(false);
        }
        repaintGraph(true);
    }
}

/**
 * @brief If active is true, the current view automaticaly adjust Y axis to the current view.
 * @brief it's a dynamic adjustment
 */
void GraphView::autoAdjustYScale(bool active)
{
    mAutoAdjustYScale = active;
    repaintGraph(true);
}
/**
 * @brief Adjust the Y axis with 0 for the minimun and find the Maximum value in the visible curve
 */
void GraphView::adjustYToMaxValue(const qreal& marginProp)
{
    float yMax = 0;

    for (int i=0; i<mCurves.size(); ++i) {
        if (mCurves.at(i).mVisible) {
            if (!mCurves.at(i).mUseVectorData &&
                !mCurves.at(i).mIsHorizontalLine &&
                !mCurves.at(i).mIsHorizontalSections &&
                !mCurves.at(i).mIsVerticalLine &&
                !mCurves.at(i).mIsTopLineSections &&
                !mCurves.at(i).mIsVertical) {
                yMax = qMax(yMax, map_max_value(mCurves.at(i).mData));
                
            } else if (mCurves.at(i).mUseVectorData)
                yMax = qMax(yMax, vector_max_value(mCurves.at(i).mDataVector));
            
        }
    }
    setRangeY(0, yMax * (1. + marginProp));
}

void GraphView::adjustYToMinMaxValue()
{
    float yMin = 0;
    float yMax = 0;

    QList<GraphCurve>::const_iterator iter = mCurves.cbegin();
    bool firstFound = false;
    while (iter != mCurves.cend()) {
        if (iter->mVisible) {
            if (iter->mUseVectorData) {
                yMin = firstFound ? qMin(yMin, vector_min_value(iter->mDataVector)) : vector_min_value(iter->mDataVector);
                yMax = firstFound ? qMax(yMax, vector_max_value(iter->mDataVector)) : vector_max_value(iter->mDataVector);
                
            } else if (!iter->mUseVectorData &&
                    !iter->mIsHorizontalLine &&
                    !iter->mIsHorizontalSections &&
                    !iter->mIsVerticalLine &&
                    !iter->mIsVertical) {
                yMin = firstFound ? qMin(yMin, map_min_value(iter->mData)) : map_min_value(iter->mData);
                yMax = firstFound ? qMax(yMax, map_max_value(iter->mData) ): map_max_value(iter->mData);
            }
            firstFound = true;
        }
        ++iter;
    }
    setRangeY(yMin, yMax);

}

void GraphView::setGraphFont(const QFont& font)
{
    setFont(font);
    repaintGraph(true);
}
/**
 * @brief GraphView::setGraphsThickness throw signal to update thickness
 * @param value
 */
void GraphView::setGraphsThickness(int value)
{
    emit signalCurvesThickness(value);
}

/**
 * @brief GraphView::updateCurvesThickness slot which modify thickness
 * @param value
 */
void GraphView::updateCurvesThickness(int value)
{
    mThickness = value;
    repaintGraph(true);
}

void GraphView::setCurvesOpacity(int value)
{
    mOpacity = value;
    repaintGraph(true);
}

void GraphView::setCanControlOpacity(bool can)
{
    mCanControlOpacity = can;
}

void GraphView::setFormatFunctX(FormatFunc f){
    //mFormatFuncX = f;
}

void GraphView::setFormatFunctY(FormatFunc f){
    //mFormatFuncY = f;
}

/* ------------------------------------------------------
 Curves & Zones
 ------------------------------------------------------ */
#pragma mark Curves & Zones

void GraphView::addCurve(const GraphCurve& curve)
{
    mCurves.append(curve);
    repaintGraph(false);
}

void GraphView::removeCurve(const QString& name)
{
    for (int i=0; i<mCurves.size(); ++i)
        if (mCurves.at(i).mName == name) {
            mCurves.removeAt(i);
            break;
        }
    
    repaintGraph(false);
}

void GraphView::removeAllCurves()
{
    mCurves.clear();
}

void GraphView::reserveCurves(const int size)
{
    mCurves.reserve(size);
}

void GraphView::setCurveVisible(const QString& name, const bool visible)
{
    bool modified = false;
    for (int i=0; i<mCurves.size(); ++i)
        if (mCurves.at(i).mName == name && mCurves.at(i).mVisible != visible) {
            mCurves[i].mVisible = visible;
            modified = true;
            break;
        }
    
    if (modified)
        repaintGraph(false);
    
}

GraphCurve* GraphView::getCurve(const QString& name)
{
    for(int i=0; i<mCurves.size(); ++i)
        if(mCurves.at(i).mName == name)
            return &mCurves[i];

    return 0;
}

const QList<GraphCurve>& GraphView::getCurves() const
{
    return mCurves;
}

int GraphView::numCurves() const
{
    return mCurves.size();
}

void GraphView::addZone(const GraphZone& zone)
{
    mZones.append(zone);
    repaintGraph(false);
}

void GraphView::removeAllZones()
{
    mZones.clear();
    repaintGraph(false);
}

#pragma mark Mouse events & Tool Tip
void GraphView::enterEvent(QEvent* e)
{
    Q_UNUSED(e);
    mTipVisible = mUseTip;
    update();
}

void GraphView::leaveEvent(QEvent* e)
{
    Q_UNUSED(e);
    mTipVisible = false;
    update();
}

void GraphView::mouseMoveEvent(QMouseEvent* e)
{
    qreal x = e->pos().x();
    qreal y = e->pos().y();
    //std::cout << y << std::endl;
    
    if (mUseTip && x >= mMarginLeft && x <= (mMarginLeft + mGraphWidth) && y >= mMarginTop && y <= (mMarginTop + mGraphHeight))  {
        mTipVisible = true;
        QRectF old_rect = mTipRect;
        //QLocale locale;
        
        int cursorW = 15;
        int cursorH = 15;
        
        if(mMarginLeft + mGraphWidth - x <= (mTipWidth + cursorW))
            x -= mTipWidth;
        else
            x += cursorW;
        
        if (mMarginTop + mGraphHeight - y <= (mTipHeight + cursorH))
            y -= mTipHeight;
        else
            y += cursorH;
        
        mTipRect.setLeft(x - 0.5);
        mTipRect.setTop(y - 0.5);
        
        mTipRect.setWidth(mTipWidth);
        mTipRect.setHeight(mTipHeight);
        
        mTipX = getValueForX(e->x()-0.5);
        //if(mFormatFuncX)
        //    mTipX = locale.toDouble(mFormatFuncX(mTipX));
        
        mTipY = getValueForY(e->y()+0.5);
        //if(mFormatFuncY)
        //    mTipY = locale.toDouble(mFormatFuncY(mTipY));
        
        update(old_rect.adjusted(-30, -30, 30, 30).toRect());
        
    } else
        mTipVisible = false;
    
    update(mTipRect.adjusted(-30, -30, 30, 30).toRect());
    
    // forward to parent to move a marker for example
    e->ignore();
}

void GraphView::setTipXLab(const QString& lab)
{
    mTipXLab = lab =="" ? "":  lab + " = ";
}
void GraphView::setTipYLab(const QString& lab)
{
    mTipYLab = lab =="" ? "":  lab + " = ";
}

#pragma mark Resize & Paint

#if GRAPH_OPENGL
void GraphView::initializeGL()
{
    //QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    //f->glEnable(GL_MULTISAMPLE_ARB);
}
void GraphView::resizeGL(int w, int h)
{
    repaintGraph(true);
}
#else
void GraphView::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    repaintGraph(true);
}
#endif

void GraphView::updateGraphSize(int w, int h)
{
    mGraphWidth = w - mMarginLeft - mMarginRight;
    mGraphHeight = h - mMarginTop - mMarginBottom;
    mAxisToolX.updateValues(mGraphWidth, mStepMinWidth, mCurrentMinX, mCurrentMaxX);
    mAxisToolY.updateValues(mGraphHeight, 12, mMinY, mMaxY);
}

void GraphView::repaintGraph(const bool aAlsoPaintBackground)
{
    if (aAlsoPaintBackground)
        mBufferBack = QPixmap();

    update();
}
    
void GraphView::paintEvent(QPaintEvent* )
{
 //   Q_UNUSED(event);
    
    
    updateGraphSize(width(), height());
    
    // ----------------------------------------------------
    //  Nothing to draw !
    // ----------------------------------------------------
    if (mCurves.size() == 0 && mZones.size() == 0) {
        QPainter p(this);
        p.setFont(font());
        p.fillRect(0, 0, width(), height(), QColor(200, 200, 200));
        p.setPen(QColor(100, 100, 100));
        p.drawText(0, 0, width(), height(), Qt::AlignCenter, mNothingMessage);
        
        return;
    }
    
    // ----------------------------------------------------
    //  SD : draw on a buffer only if it has been reset
    // ----------------------------------------------------
    if (mBufferBack.isNull() && mRendering == eSD) {
        mBufferBack = QPixmap(width(), height());
        paintToDevice(&mBufferBack);
        
        if (mBufferBack.isNull() )
            qDebug()<< "mBufferBack.isNull()";
        
    }
    // ----------------------------------------------------
    //  HD : draw directly on widget
    // ----------------------------------------------------
    else if(mRendering == eHD)
        paintToDevice(this);
    
    // ----------------------------------------------------
    //  SD rendering : draw buffer on widget !
    // ----------------------------------------------------
    if(mRendering == eSD) {
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing);
        p.drawPixmap(mBufferBack.rect(), mBufferBack, rect());
    }
    
    // ----------------------------------------------------
    //  Tool Tip (above all) Draw horizontal and vertical red line
    // ----------------------------------------------------
    if (mTipVisible && (!mTipXLab.isEmpty() || !mTipYLab.isEmpty())) {
        QPainterPath tipPath;
        if (mTipRect.width()<2) {
            mTipRect.setLeft(20);
            mTipRect.setTop(20);
            
            mTipRect.setWidth(20);
            mTipRect.setHeight(20);
        }
      
        
        tipPath.addRoundedRect(mTipRect, 5, 5);
        
        
        QFont font;
        font.setPointSizeF(pointSize(10));
        
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing);
        p.setFont(font);
        
        p.fillPath(tipPath, QColor(0, 0, 0, 180));
        p.setPen(Qt::black);
        p.drawPath(tipPath);
        p.setPen(Qt::white);
        
        if (!mTipXLab.isEmpty() && !mTipYLab.isEmpty()) {
            p.drawText(mTipRect.adjusted(0, 0, 0, -mTipRect.height()/2), Qt::AlignCenter, mTipXLab + DateUtils::dateToString(mTipX));
            p.drawText(mTipRect.adjusted(0, (int)(mTipRect.height()/2), 0, 0), Qt::AlignCenter, mTipYLab + DateUtils::dateToString(mTipY));
            
        } else if (!mTipXLab.isEmpty())
            p.drawText(mTipRect, Qt::AlignCenter, mTipXLab + DateUtils::dateToString(mTipX));
        
        else if (!mTipYLab.isEmpty())
            p.drawText(mTipRect, Qt::AlignCenter, mTipYLab + DateUtils::dateToString(mTipY));
        
       
    }
}






/**
 * @brief draw graphics
 */
void GraphView::paintToDevice(QPaintDevice* device)
{
    QPainter p(device);    
    p.setFont(font());
    
    p.setRenderHints(QPainter::Antialiasing);
    
    // ----------------------------------------------------
    //  Background
    // ----------------------------------------------------
    p.fillRect(rect(), mBackgroundColor);
    
    QFont font = p.font();
    font.setPointSizeF(font.pointSizeF() - 2.);
    p.setFont(font);
    
    // ----------------------------------------------------
    //  Zones
    // ----------------------------------------------------
    for (int i=0; i<mZones.size(); ++i) {
        QRect r(getXForValue(mZones.at(i).mXStart),
                mMarginTop,
                getXForValue(mZones.at(i).mXEnd) - getXForValue(mZones.at(i).mXStart),
                mGraphHeight);
        
        p.save();
        p.setClipRect(r);
        
        p.fillRect(r, mZones.at(i).mColor);
        p.setPen(mZones.at(i).mColor.darker());
        QFont fontZone = p.font();
        fontZone.setWeight(QFont::Black);
        fontZone.setPointSizeF(pointSize(20));
        p.setFont(fontZone);
        p.drawText(r, Qt::AlignCenter | Qt::TextWordWrap, mZones.at(i).mText);
        p.restore();
    }
    
    // ----------------------------------------------------
    //  Curves
    // ----------------------------------------------------
    drawCurves(p);
    
    // ----------------------------------------------------
    //  Vertical Grid
    // ----------------------------------------------------
    if (!mLegendX.isEmpty()) {
        QRectF tr(mMarginLeft, mGraphHeight- mMarginBottom, mGraphWidth, mMarginBottom);
        p.setPen(Qt::black);
        p.drawText(tr, Qt::AlignRight | Qt::AlignTop, mLegendX);
    }
    
    mAxisToolX.mShowArrow = mXAxisArrow;
    mAxisToolX.mShowSubs = mXAxisTicks;
    mAxisToolX.mShowSubSubs = mXAxisSubTicks;
    mAxisToolX.mShowText = mXAxisValues;
    
    mAxisToolX.updateValues(mGraphWidth, mStepMinWidth, mCurrentMinX, mCurrentMaxX);
    QVector<qreal> linesXPos = mAxisToolX.paint(p, QRectF(mMarginLeft, mMarginTop + mGraphHeight, mGraphWidth , mMarginBottom), (qreal)7.,formatValueToAppSettingsPrecision);//, mFormatFuncX);
    
    // ----------------------------------------------------
    //  Horizontal Grid
    // ----------------------------------------------------
    mAxisToolY.mShowArrow = mYAxisArrow;
    mAxisToolY.mShowSubs = mYAxisTicks;
    mAxisToolY.mShowSubSubs = mYAxisSubTicks;
    mAxisToolY.mShowText = mYAxisValues;

    mAxisToolY.updateValues(mGraphHeight, mStepMinWidth, mMinY, mMaxY);
    QVector<qreal> linesYPos = mAxisToolY.paint(p, QRectF(0, mMarginTop, mMarginLeft, mGraphHeight), (qreal) 5.,formatValueToAppSettingsPrecision);//, mFormatFuncY);
    
    
    // ----------------------------------------------------
    //  Graph specific infos at the top right
    // ----------------------------------------------------
    if (mShowInfos) {
        font.setPointSizeF(font.pointSizeF() + 2.);
        p.setFont(font);
        p.setPen(QColor(50, 50, 50));
        int y = 0;
        int lineH = 16;
        for (int i=0; i<mInfos.size(); ++i) {
            p.drawText(mMarginLeft + 5, mMarginTop + 5 + y, mGraphWidth - 10, lineH, Qt::AlignRight | Qt::AlignTop, mInfos[i]);
            y += lineH;
        }
    }
    p.end();
}

void GraphView::drawCurves(QPainter& painter)
{
    // ---------------------------------------------------------------
    //  Curves
    // ---------------------------------------------------------------
    painter.save();
    // Draw curves inside axis only (not in margins!)
    painter.setClipRect(mMarginLeft, mMarginTop, mGraphWidth, mGraphHeight);
    
    for(int curveIndex=0; curveIndex<mCurves.size(); ++curveIndex) {
        const GraphCurve& curve = mCurves.at(curveIndex);
        
        if (curve.mVisible) {
            QPainterPath path;
            
            QPen pen = curve.mPen;
            pen.setWidth(pen.width() * mThickness);
            painter.setPen(pen);
            
            QBrush brush = curve.mBrush;

            if (mCanControlOpacity) {
                QColor c = brush.color();
                c.setAlpha(curve.mBrush.color().alpha()*mOpacity / 100);
                brush.setColor(c);
            }

            painter.setBrush(brush);
            
            if (curve.mIsHorizontalLine) {
                const qreal y = getYForValue(curve.mHorizontalValue);
                path.moveTo(mMarginLeft, y);
                path.lineTo(mMarginLeft + mGraphWidth, y);
                
                painter.strokePath(path, curve.mPen);
                
            } else if (curve.mIsVerticalLine) {
                const qreal x = getXForValue(curve.mVerticalValue, false);
                path.moveTo(x, mMarginTop + mGraphHeight);
                path.lineTo(x, mMarginTop);
                
                painter.strokePath(path, pen);
                
            } else if (curve.mIsHorizontalSections) {
                const qreal y1 = mMarginTop;
                const qreal y0 = mMarginTop + mGraphHeight;
                path.moveTo(mMarginLeft, y0);
                
                for (int i=0; i<curve.mSections.size(); ++i) {
                    qreal x1 = getXForValue(curve.mSections.at(i).first, false);
                    qreal x2 = getXForValue(curve.mSections.at(i).second, false);
                    
                    path.lineTo(x1, y0);
                    path.lineTo(x1, y1);
                    path.lineTo(x2, y1);
                    path.lineTo(x2, y0);
                }
                path.lineTo(mMarginLeft + mGraphWidth, y0);
                
                painter.setClipRect(mMarginLeft, mMarginTop, mGraphWidth, mGraphHeight);
                painter.fillPath(path, brush);
                painter.strokePath(path, curve.mPen);
                
            } else if(curve.mIsTopLineSections) {
                const qreal y1 = mMarginTop + curve.mPen.width();
                painter.setPen(curve.mPen);
                
                for(int i=0; i<curve.mSections.size(); ++i) {
                    const qreal x1 = getXForValue(curve.mSections.at(i).first, true);
                    const qreal x2 = getXForValue(curve.mSections.at(i).second, true);
                    
                    painter.drawLine(QPointF(x1, y1),QPointF(x2, y1));
                }
                
            } else if (curve.mIsVertical) {
                path.moveTo(mMarginLeft, mMarginTop + mGraphHeight);
                
                int index = 0;
                qreal last_y = 0;

                QMap<float, float>::const_iterator iter = curve.mData.cbegin();
                
                while (iter != curve.mData.cend()) {
                    float valueX = iter.value();
                    float valueY = iter.key();
                    
                    // vertical curves must be normalized (values from 0 to 1)
                    // They are drawn using a 100px width
                    qreal x = mMarginLeft + valueX * 100.f;
                    qreal y = getYForValue(valueY, false);
                    y = qMin(y, mMarginTop + mGraphHeight);
                    y = qMax(y, mMarginTop);
                    
                    if (index == 0) {
                        path.lineTo(x, y);
                        
                    } else {
                        if (curve.mIsHisto)
                            path.lineTo(x, last_y);
                        path.lineTo(x, y);
                    }
                    last_y = y;
                    ++index;
                    ++iter;
                }
                path.lineTo(mMarginLeft, mMarginTop);
                painter.drawPath(path);
                
            } else { // it's horizontal curve
            
                path.moveTo(mMarginLeft, mMarginTop + mGraphHeight);
                
                qreal last_x = 0;
                qreal last_y = 0;
                qreal last_valueY = 0;
                //float valueY ;
                
                if (curve.mUseVectorData) {
                    // Down sample vector
                    if(curve.mDataVector.isEmpty())
                        return;

                    QVector<float> subData = getVectorDataInRange(curve.mDataVector, mCurrentMinX, mCurrentMaxX, mMinX, mMaxX);
                    
                    QVector<float> lightData;
                    const float dataStep = (float)subData.size() / (float)(mGraphWidth);
                    if (dataStep > 1) {
                        for (int i = 0; i<mGraphWidth; ++i) {
                            const int idx = (int)round(i * dataStep);
                            lightData.append(subData.at(idx));
                        }
                    } else
                        lightData = subData;
                    
                    bool isFirst = true;
                    
                    for (int i = 0; i<lightData.size(); ++i) {
                        // Use "dataStep" only if lightData is different of subData !
                        const float valueX = mCurrentMinX + ((dataStep > 1) ? i * dataStep : i);
                        const float valueY = lightData.at(i);
                        
                        if (valueX >= mCurrentMinX && valueX <= mCurrentMaxX) {
                            qreal x = getXForValue(valueX, false);
                            qreal y = getYForValue(valueY, false);
                            
                             if (isFirst) {
                                path.moveTo(x, y);
                                isFirst=false;
                                 
                            } else {
                                if (curve.mIsHisto)
                                    path.lineTo(x, last_y);
                                path.lineTo(x, y);
                            }
                            last_x = x;
                            last_y = y;
                            //last_valueY = valueY;
                        }   
                    }
                    
                } else {
                    // Down sample curve for better performances
                    if (curve.mData.isEmpty())
                        continue;
                    QMap<float, float> subData = curve.mData;
                    subData = getMapDataInRange(subData, mCurrentMinX, mCurrentMaxX);

                    if (subData.isEmpty())
                        continue;

                    QMap<float, float> lightMap;

                    if (subData.size() > mGraphWidth) { //always used in the items thumbnails
                        int valuesPerPixel = subData.size() / (mGraphWidth);
                        QMap<float, float>::const_iterator iter = subData.cbegin();
                        int index = 0;
                        
                        while(iter != subData.cend()) {
                            if(index % valuesPerPixel == 0)
                                lightMap[iter.key()] = iter.value();
                            ++index;
                            ++iter;
                        }
                    } else
                        lightMap = subData;
                    
                    
                    // Draw
                    
                    QMapIterator<float, float> iter(lightMap);
                    iter.toFront();
                    if (!iter.hasNext())
                        continue;
                    
                    iter.next();
                    float valueX = iter.key();
                    float valueY = iter.value();
                    last_valueY = 0;

                    // Detect square signal front-end without null value at the begin of the QMap
                    // e.g calibration of typo-ref
                    if (iter.hasNext()) {
                        if (valueY == (iter.peekNext()).value()) {
                            if(valueX >= mCurrentMinX && valueX <= mCurrentMaxX) {
                                path.moveTo( getXForValue(valueX), getYForValue(0, true) );
                                path.lineTo( getXForValue(valueX), getYForValue(valueY, true) );
                            }
                        }
                    }
                    
                    iter.toFront();
                    bool isFirst=true;
                    
                    if (curve.mBrush != Qt::NoBrush) {
                        isFirst=false;
                        last_x = getXForValue(valueX, true);
                        last_y = getYForValue(0, false);
                    }

                    while (iter.hasNext()) {
                        iter.next();
                        valueX = iter.key();
                        valueY = iter.value();

                        if (valueX >= mCurrentMinX && valueX <= mCurrentMaxX) {
                           qreal x = getXForValue(valueX, true);
                           qreal y = getYForValue(valueY, false);
                            
                            if (isFirst) {
                                path.moveTo(x, y);
                                isFirst=false;
                                
                            } else {
                                
                                if(curve.mIsHisto) {
                                    // histo bars must be centered around x value :
                                    const qreal dx2 = (x - last_x)/2.f;
                                    path.lineTo(x - dx2, last_y);
                                    path.lineTo(x - dx2, y);
                                    
                                } else if (curve.mIsRectFromZero) {
                                    
                                    if(last_valueY != 0 && valueY != 0) {
                                        path.lineTo(x, y);
                                        
                                    } else if(last_valueY == 0 && valueY != 0) {
                                        // Draw a front end of a square signal some 0 at the begin and at the end
                                        // e.i plot the HPD surface
                                        path.lineTo(x, last_y);
                                        path.lineTo(x, y);
                                    } else if(last_valueY != 0 && valueY == 0) {
                                        // Draw a back end of a square signal some 0 at the begin and at the end
                                        // e.i plot the HPD surface
                                        path.lineTo(last_x, last_y);
                                        path.lineTo(last_x, y);
                                    }
                                
                                    
                                } else
                                    path.lineTo(x, y);
                                
                                
                            }
                            last_x = x;
                            last_y = y;

                            last_valueY = valueY;
                        }
                    }

                    if (path.elementCount()  == 1) { //there is only one value
                        last_x = getXForValue(valueX, true);
                        last_y = getYForValue(0, false);
                        path.lineTo(last_x, last_y);
                    }

                    // Detect square signal back-end without null value at the end of the QMap
                    // e.i calibration of typo-ref
                    if (lightMap.size()>1) {
                        QMapIterator<float, float> lastIter(lightMap);
                        lastIter.toBack();
                        lastIter.previous();
                        if ( lastIter.value() == lastIter.peekPrevious().value() ) {
                            float x = lastIter.key();
                            if ( x > mCurrentMinX && x < mCurrentMaxX)
                                path.lineTo(getXForValue(x, true), getYForValue(0, true) );

                        }
                    }

                }

                if (curve.mIsRectFromZero && (curve.mBrush != Qt::NoBrush) ) {
                    // Close the path on the left side
                    path.lineTo(last_x, getYForValue(0, true));

                    painter.setPen(curve.mPen);
                    painter.fillPath(path, brush);
                    
                } else
                    painter.drawPath(path);
                
            }
        }
    }
    painter.restore();
}

#pragma mark Save & Export

/**
 * @brief Export a density with locale setting and separator and specific step
 * @todo Maybe we can use QString QLocale::createSeparatedList(const QStringList & list) const
 */
void GraphView::exportCurrentDensityCurves(const QString& defaultPath, const QLocale locale, const QString& csvSep, float step) const
{
    if (step<=0)
        step=1;
    
    QString filter = tr("CSV (*.csv)");
    QString filename = QFileDialog::getSaveFileName(qApp->activeWindow(),
                                                    tr("Save graph data as..."),
                                                    defaultPath,
                                                    filter);
    QFile file(filename);
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        qDebug()<<"GraphView::exportCurrentCurves"<<" nbCurve to export"<<mCurves.size();

        QList<QStringList> rows;
        QStringList list;
        
        list <<"X Axis";
        float xMin = mCurves.cbegin()->mData.firstKey();
        float xMax = mCurves.cbegin()->mData.lastKey();;
        bool firstCurveVisible = true;
        
        for(auto iter= mCurves.cbegin(); iter != mCurves.cend(); ++iter) {
            
            if (!iter->mData.empty() &&
                !iter->mIsHorizontalLine &&
                !iter->mIsVerticalLine &&
                !iter->mIsVertical &&
                !iter->mIsHorizontalSections &&
                !iter->mUseVectorData &&
                iter->mVisible) {
                
                // 1 -Create the header
                list << iter->mName;
                // 2 - Find x Min and x Max period, on all curve, we suppose Qmap is order
                if (firstCurveVisible) {
                    xMin = iter->mData.firstKey();
                    xMax = iter->mData.lastKey();
                } else {
                    xMin = qMin(xMin, iter->mData.firstKey());
                    xMax = qMax(xMax, iter->mData.lastKey());
                }
            } else continue;
        }
        rows<<list;
        rows.reserve(ceil( (xMax - xMin)/step) );
        
        // 3 - Create Row, with each curve
        //  Create data in row
        for (float x= xMin; x <= xMax; x += step) {
            list.clear();
            /*if(mFormatFuncX) {
                list << mFormatFuncX(x);
            }
            else */
            list << QString::number(x);
            for (auto iter= mCurves.cbegin(); iter != mCurves.cend(); ++iter) {
                
                if (!iter->mData.empty() &&
                    !iter->mIsHorizontalLine &&
                    !iter->mIsVerticalLine &&
                    !iter->mIsVertical &&
                    !iter->mIsHorizontalSections &&
                    !iter->mUseVectorData &&
                    iter->mVisible) {
                    
                    const float xi = interpolateValueInQMap(x, iter->mData);
                    list<<locale.toString(xi);
                    
                } else continue;
                
            }
            rows<<list;
        }
        
        // 4 - Save Qlist
        QTextStream output(&file);
        const QString version = qApp->applicationName() + " " + qApp->applicationVersion();
        const QString projectName = tr("Project filename")+" : "+ MainWindow::getInstance()->getNameProject();

        output<<version+"\r";
        output<<projectName+ "\r";
        output<<DateUtils::getAppSettingsFormatStr()+"\r";
        
        for (int i=0; i<rows.size(); ++i) {
            output << rows.at(i).join(csvSep);
            output << "\r";
        }
        file.close();
    }
    
}


void GraphView::exportCurrentVectorCurves(const QString& defaultPath, const QLocale locale, const QString& csvSep, bool writeInRows, int offset) const
{
    Q_UNUSED(writeInRows);
    qDebug()<<"GraphView::exportCurrentVectorCurves";
    QString filter = tr("CSV (*.csv)");
    QString filename = QFileDialog::getSaveFileName(qApp->activeWindow(),
                                                    tr("Save graph data as..."),
                                                    defaultPath,
                                                    filter);
    QFile file(filename);
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        bool abscissesWritten = false;
        QList<QStringList> rows;

        rows.append(QStringList("X Axis"));
        QMap<float, QVector<float> > rowsData;
        
        int rowsCount = rows.count();
        QStringList emptyColumn;
        
        qDebug()<<"GraphView::exportCurrentVectorCurves"<<" nbCurve to export"<<mCurves.size();
        for(int idCurve=0; idCurve<mCurves.size(); ++idCurve)
        {
            if ( !mCurves[idCurve].mVisible || mCurves[idCurve].mDataVector.empty() ) continue;
            
            const QVector<float>& data = mCurves[idCurve].mDataVector;
            // the new DataVector is longer than the last, we need to expand the size of rows
            if (data.size()>rowsCount-2) {
                abscissesWritten=false;
                rowsCount = rows.count();
            }
            
            if (!abscissesWritten) {
                for(int i=offset+rowsCount; i<data.size()+1; ++i) {
                    // we add 1 to the line number, because the index of vector start to 0
                    rows.append(QStringList(locale.toString(i-rowsCount+1))+emptyColumn);
                   
                }
                abscissesWritten = true;
                
            }
            //prepare adding row
            emptyColumn<<"";
            
            if (abscissesWritten) {
                    rows[0] << mCurves[idCurve].mName;
                    for(int i=offset; i<data.size(); ++i) {
                        rows[i-offset+1]<< locale.toString(data[i]);
                    }
            }
            
        }
        
        QMapIterator<float, QVector<float> > iter2(rowsData);
        while(iter2.hasNext()) {
            iter2.next();
            QStringList list;

            list << locale.toString(iter2.key());
            
            for(int i=0; i<iter2.value().size(); ++i)
                list << locale.toString(iter2.value().at(i));
            rows.append(list);
        }
        
        QTextStream output(&file);
        for (int i=0; i<rows.size(); ++i)  {
            output << rows.at(i).join(csvSep);
            output << "\n";
        }
        file.close();
    }
}

bool GraphView::saveAsSVG(const QString& fileName, const QString& graphTitle, const QString& svgDescrition, const bool withVersion, const int versionHeight)
{
    if(fileName.isEmpty()) {
        return false;
        
    } else {
        Rendering memoRendering= mRendering;
        QRect rTotal(withVersion ? QRect(0, 0, width(), height()+versionHeight) : QRect(0, 0, width(), height()));
        
        const int graphRigthShift = 40;

        QRect rGraph(0, 0, width(), height());
        /* We can not have a svg graph in eSD Rendering Mode */
        setRendering(eHD);
        // Set SVG Generator
        QSvgGenerator svgGen;
        svgGen.setFileName(fileName);
        svgGen.setSize(rTotal.size());
        svgGen.setViewBox(rTotal);
        svgGen.setTitle(graphTitle);
        svgGen.setDescription(svgDescrition);
        
        QPainter painter;
        painter.begin(&svgGen);
        font().wordSpacing();
        
        render(&painter, QPoint(), QRegion(rGraph, QRegion::Rectangle));
        
        QFontMetrics fm(painter.font());
        painter.drawText( graphRigthShift,0, graphRigthShift+fm.width(graphTitle), versionHeight,
                         Qt::AlignCenter,
                         graphTitle);
        
        if (withVersion) {
            painter.setPen(Qt::black);
            painter.drawText(0, rGraph.y()+rGraph.height(), rGraph.width(),versionHeight,
                             Qt::AlignCenter,
                             qApp->applicationName() + " " + qApp->applicationVersion());
        }
        painter.end();
        
        setRendering(memoRendering);
        
        return true;
        
    }
    
}
QString GraphView::getInfo()
{
    
    return ( isShow() ? mInfos.join("|") : "");
}

bool GraphView::isShow()
{
    return mShowInfos;
}

