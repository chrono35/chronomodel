﻿#include "GraphViewAbstract.h"
#include "StdUtilities.h"
#include <cmath>
#include <QDebug>


#pragma mark Constructor / Destructor

GraphViewAbstract::GraphViewAbstract():
mGraphWidth(50), mGraphHeight(50),
mMarginLeft(50), mMarginRight(10), mMarginTop(5), mMarginBottom(15),
mMinX(0), mMaxX(10),
mMinY(0), mMaxY(10),
mCurrentMinX(0),mCurrentMaxX(2000)
{
//qDebug()<<"contructor GraphViewAbstract::GraphViewAbstrac ";
}

GraphViewAbstract::~GraphViewAbstract(){}

#pragma mark Getters

float GraphViewAbstract::rangeX() const {return mMaxX - mMinX;}
float GraphViewAbstract::rangeY() const {return mMaxY - mMinY;}

float GraphViewAbstract::getCurrentMaxX() const {return mCurrentMaxX;}
float GraphViewAbstract::getCurrentMinX() const {return mCurrentMinX;}


float GraphViewAbstract::minimumX() const {return mMinX;}
float GraphViewAbstract::maximumX() const {return mMaxX;}
float GraphViewAbstract::minimumY() const {return mMinY;}
float GraphViewAbstract::maximumY() const {return mMaxY;}

qreal GraphViewAbstract::marginLeft() const {return mMarginLeft;}
qreal GraphViewAbstract::marginRight() const {return mMarginRight;}
qreal GraphViewAbstract::marginTop() const {return mMarginTop;}
qreal GraphViewAbstract::marginBottom() const {return mMarginBottom;}


#pragma mark Setters

void GraphViewAbstract::setRangeX(const float aMinX, const float aMaxX)
{
    mMinX = aMinX;
    mMaxX = aMaxX;

}

void GraphViewAbstract::setCurrentX(const float aMinX, const float aMaxX)
{
    mCurrentMinX = aMinX;
    mCurrentMaxX = aMaxX;
    repaintGraph(true);
    
}

void GraphViewAbstract::setRangeY(const float aMinY, const float aMaxY)
{
    if(aMinY != mMinY || aMaxY != mMaxY) {
        if(aMinY == aMaxY) {
            mMinY = aMinY - 1;
            mMaxY = aMaxY + 1;
            //qDebug() << "Warning : setting min == max for graph y scale : " << aMinY;
        }
        else if(mMinY > mMaxY) {
            qDebug() << "ERROR : setting min > max for graph y scale : [" << mMinY << "; " << mMaxY << "]";
        } else {
            mMinY = aMinY;
            mMaxY = aMaxY;
        }
        repaintGraph(true);
    }
}

void GraphViewAbstract::setMinimumX(const float aMinX)				{if(mMinX != aMinX){mMinX = aMinX; repaintGraph(true);}}
void GraphViewAbstract::setMaximumX(const float aMaxX)				{if(mMaxX != aMaxX){mMaxX = aMaxX; repaintGraph(true);}}
void GraphViewAbstract::setMinimumY(const float aMinY)				{if(mMinY != aMinY){mMinY = aMinY; repaintGraph(true);}}
void GraphViewAbstract::setMaximumY(const float aMaxY)				{if(mMaxY != aMaxY){mMaxY = aMaxY; repaintGraph(true);}}

void GraphViewAbstract::setMarginLeft(const qreal aMarginLeft)		{if(mMarginLeft != aMarginLeft){mMarginLeft = aMarginLeft; repaintGraph(true);}}
void GraphViewAbstract::setMarginRight(const qreal aMarginRight)		{if(mMarginRight != aMarginRight){mMarginRight = aMarginRight; repaintGraph(true);}}
void GraphViewAbstract::setMarginTop(const qreal aMarginTop)			{if(mMarginTop != aMarginTop){mMarginTop = aMarginTop; repaintGraph(true);}}
void GraphViewAbstract::setMarginBottom(const qreal aMarginBottom)	{if(mMarginBottom != aMarginBottom){mMarginBottom = aMarginBottom; repaintGraph(true);}}
void GraphViewAbstract::setMargins(const qreal aMarginLeft, const qreal aMarginRight, const qreal aMarginTop, const qreal aMarginBottom)
{
	mMarginLeft = aMarginLeft;
	mMarginRight = aMarginRight;
	mMarginTop = aMarginTop;
	mMarginBottom = aMarginBottom;
	repaintGraph(true);
}

#pragma mark Values utilities
/**
 * @brief GraphViewAbstract::getXForValue find a position on a graph for a Value in a table
 * @param aValue
 * @param aConstainResult
 * @return
 */
qreal GraphViewAbstract::getXForValue(const float aValue, const bool aConstainResult)
{
    return (qreal)(mMarginLeft + valueForProportion(aValue, mCurrentMinX, mCurrentMaxX, 0.f, (float)mGraphWidth, aConstainResult));
}

float GraphViewAbstract::getValueForX(const qreal x, const bool aConstainResult)
{
	const qreal lXFromSide = x - mMarginLeft;
    const float lValue = valueForProportion((float)lXFromSide, 0.f, (float)mGraphWidth, mCurrentMinX, mCurrentMaxX, aConstainResult);
	return lValue;
}
qreal GraphViewAbstract::getYForValue(const float aValue, const bool aConstainResult)
{
    const float lYFromBase = valueForProportion(aValue, mMinY, mMaxY, 0.f, (float)(mGraphHeight-mMarginTop), aConstainResult);
    const qreal y = mMarginTop + mGraphHeight - (qreal)lYFromBase;
	return y;	
}
float GraphViewAbstract::getValueForY(const qreal y, const bool aConstainResult)
{
	const qreal lYFromBase = mMarginTop + mGraphHeight - y;
    const float lValue = valueForProportion( (float)lYFromBase, 0.f, (float)(mGraphHeight-mMarginTop), mMinY, mMaxY, aConstainResult);
	return lValue;
}

float GraphViewAbstract::valueForProportion(const float value, const float valMin, const float valMax, const float Pmin, const float Pmax, const bool resultInBounds)
{
    float v2 = Pmin + (value - valMin) * (Pmax - Pmin) / (valMax - valMin);
    
    if(resultInBounds) {
        v2 = qBound(Pmin,v2,Pmax);
	}
	return v2;
}
qreal GraphViewAbstract::valueForProportion(const qreal value, const qreal valMin, const qreal valMax, const qreal Pmin, const qreal Pmax, const bool resultInBounds)
{
    qreal v2 = Pmin + (value - valMin) * (Pmax - Pmin) / (valMax - valMin);

    if(resultInBounds) {
        v2 = qBound(Pmin,v2,Pmax);
    }
    return v2;
}
