#include "GraphViewEvent.h"
#include "GraphView.h"
#include "Event.h"
#include "EventKnown.h"
#include "StdUtilities.h"
#include "QtUtilities.h"
#include "ModelUtilities.h"
#include "Painting.h"
#include "MainWindow.h"
#include <QtWidgets>



#pragma mark Constructor / Destructor

GraphViewEvent::GraphViewEvent(QWidget *parent):GraphViewResults(parent),
mEvent(0)
{
    //setMainColor(QColor(100, 100, 120));
    setMainColor(QColor(100, 100, 100));
    mGraph->setBackgroundColor(QColor(230, 230, 230));

    
    
}

GraphViewEvent::~GraphViewEvent()
{
    mEvent = 0;
}

void GraphViewEvent::setEvent(Event* event)
{
    if(event)
    {
        mEvent = event;
        if (mEvent->type()==Event::eKnown) {
            mTitle = tr("Bound ") + " : " + mEvent->mName;
        }
        else mTitle = tr("Event") + " : " + mEvent->mName;
    }
    update();
}

void GraphViewEvent::refresh()
{
    mGraph->removeAllCurves();
    mGraph->removeAllZones();
    mGraph->clearInfos();
    mGraph->resetNothingMessage();
    
    mGraph->autoAdjustYScale(mCurrentTypeGraph == eTrace);
    
    QPen defaultPen;
    defaultPen.setWidthF(1);
    defaultPen.setStyle(Qt::SolidLine);
    
    if(mEvent)
    {
        QColor color = mEvent->mColor;
        
        bool isFixedBound = false;
        bool isUnifBound = false;
        EventKnown* bound = 0;
        if(mEvent->type() == Event::eKnown)
        {
            bound = dynamic_cast<EventKnown*>(mEvent);
            if(bound)
            {
                if(bound->knownType() == EventKnown::eFixed)
                    isFixedBound = true;
                else if(bound->knownType() == EventKnown::eUniform)
                    isUnifBound = true;
            }
        }
        
        QString results = ModelUtilities::eventResultsText(mEvent, false);
        setNumericalResults(results);
        
        if(mCurrentTypeGraph == eHisto)
        {
            if (mEvent->type()==Event::eKnown) {
                mTitle = tr("Bound ") + " : " + mEvent->mName;
            }
            else mTitle = tr("Event") + " : " + mEvent->mName;
            
            mGraph->setBackgroundColor(QColor(230, 230, 230));
            if(mCurrentVariable == eTheta)
            {
                mGraph->setRangeX(mSettings.mTmin, mSettings.mTmax);
                
                if(isFixedBound)
                {
                    mGraph->setRangeY(0, 1.f);
                    
                    GraphCurve curve;
                    curve.mName = mTitle+" : "+"Fixed Bound";
                    curve.setPen(defaultPen);
                    curve.mPen.setColor(color);
                    curve.mIsHisto = false;
                    curve.mIsRectFromZero = true;
                    
                    for(int t=mSettings.mTmin; t< mSettings.mTmax; ++t)
                        curve.mData[t] = 0.f;
                    curve.mData[floor(bound->fixedValue())] = 1.f;
                    
                    mGraph->addCurve(curve);
                }
                else
                {
                    mGraph->setRangeY(0, 0.00001f);
                    //setNumericalResults(mTitle + "\n" + mEvent->mTheta.resultsText());
                    
                    if(isUnifBound && mShowCalib)
                    {
                        GraphCurve curve;
                        curve.mName = mTitle+" : "+"Uniform Bound";
                        curve.setPen(defaultPen);
                        curve.mPen.setColor(QColor(120, 120, 120));
                        curve.mIsHisto = true;
                        curve.mIsRectFromZero = true;
                        curve.mData = bound->mValues;
                        mGraph->addCurve(curve);
                        
                        double yMax = 1.1f * map_max_value(curve.mData);
                        mGraph->setRangeY(0, qMax(mGraph->maximumY(), yMax));
                    }
                    if(mShowAllChains)
                    {
                        if(mShowRawResults)
                        {
                           GraphCurve curveRaw;
                            curveRaw.mName = "raw histo full";
                            curveRaw.setPen(defaultPen);
                            curveRaw.mPen.setColor(Qt::red);
                            //curveRaw.mData = equal_areas(mEvent->mTheta.fullRawHisto(), 1.f);
                            curveRaw.mData = mEvent->mTheta.fullRawHisto();
                            curveRaw.mIsHisto = true;
                            mGraph->addCurve(curveRaw);
                            
                            double yMax2 = 1.1f * map_max_value(curveRaw.mData);
                            mGraph->setRangeY(0, qMax(mGraph->maximumY(), yMax2));
                        }
                        
                       GraphCurve curve;
                        curve.mName = "histo full";
                        curve.setPen(defaultPen);
                        curve.mPen.setColor(color);
                        //curve.mData = equal_areas(mEvent->mTheta.fullHisto(), 1.f);
                        curve.mData = mEvent->mTheta.fullHisto();
                        curve.mIsHisto = false;
                        mGraph->addCurve(curve);
                        
                        double yMax = 1.1f * map_max_value(curve.mData);
                        mGraph->setRangeY(0, qMax(mGraph->maximumY(), yMax));
                        
                        GraphCurve curveHPD;
                        curveHPD.mName = "histo HPD full";
                        curveHPD.setPen(defaultPen);
                        curveHPD.mPen.setColor(color);
                        curveHPD.mFillUnder = true;
                        QColor HPDColor(color);
                        HPDColor.setAlpha(100);
                        curveHPD.mBrush.setStyle(Qt::SolidPattern);
                        curveHPD.mBrush.setColor(HPDColor);
                        curveHPD.mIsHisto = false;
                        curveHPD.mIsRectFromZero = true;
                        //double realThresh = map_area(mEvent->mTheta.mHPD) / map_area(mEvent->mTheta.fullHisto());
                        
                        //curveHPD.mData = equal_areas(mEvent->mTheta.mHPD, realThresh);
                        curveHPD.mData = mEvent->mTheta.mHPD;
                        
                       mGraph->addCurve(curveHPD);
                    }
                    for(int i=0; i<mShowChainList.size(); ++i)
                    {
                        if(mShowChainList[i])
                        {
                            QColor col = Painting::chainColors[i];
                            
                            GraphCurve curve;
                            curve.mName = QString("histo chain " + QString::number(i));
                            curve.setPen(defaultPen);
                            curve.mPen.setColor(col);
                            curve.mIsHisto = false;
                            //curve.mData = equal_areas(mEvent->mTheta.histoForChain(i), 1.f);
                            curve.mData = mEvent->mTheta.histoForChain(i);
                            mGraph->addCurve(curve);
                            
                            double yMax = 1.1f * map_max_value(curve.mData);
                            mGraph->setRangeY(0, qMax(mGraph->maximumY(), yMax));
                        }
                    }
                    if(mShowAllChains && mShowHPD)
                    {
                        GraphCurve curveCred;
                        curveCred.mName = "credibility full";
                        curveCred.mSections.append(mEvent->mTheta.mCredibility);
                        curveCred.mHorizontalValue = mGraph->maximumY();
                        
                        curveCred.mPen.setColor(color);
                        curveCred.mPen.setWidth(4);
                        curveCred.mPen.setCapStyle(Qt::FlatCap);
                        curveCred.mIsHorizontalSections = true;
                        mGraph->addCurve(curveCred);
                    }
                }
            }
            else if(mCurrentVariable == eSigma)
            {
                // On est en train de regarder les variances des data
                // On affiche donc ici la superposition des variances (et pas le résultat de theta f)
                if (mEvent->type()==Event::eKnown) {
                    mTitle = tr("Bound ") + " : " + mEvent->mName;
                }
                else mTitle = tr("Std") + " : " + mEvent->mName;
                mGraph->setBackgroundColor(QColor(Qt::white));
                mGraph->autoAdjustYScale(true);
               // mGraph->setCurrentX(0, mSettings.mTmax-mSettings.mTmin);
              
                mGraph->setRangeX(0, mSettings.mTmax - mSettings.mTmin);
                double yMax = 0;
                
                for(int i=0; i<mEvent->mDates.size(); ++i)
                {
                    Date& date = mEvent->mDates[i];
                    if(mShowAllChains)
                    {
                        GraphCurve curve;
                        curve.mName = "histo full date " + QString::number(i);
                        curve.setPen(defaultPen);
                        curve.mPen.setColor(color);
                        curve.mIsHisto = false;
                        //curve.mData = equal_areas(date.mSigma.fullHisto(), 1.f);
                        curve.mData = date.mSigma.fullHisto();
                        mGraph->addCurve(curve);
                        
                        yMax = qMax(yMax, 1.1f * map_max_value(curve.mData));
                        mGraph->setRangeY(0, yMax);
                    }
                    for(int j=0; j<mShowChainList.size(); ++j)
                    {
                        if(mShowChainList[j])
                        {
                            QColor col = Painting::chainColors[j];
                            
                            GraphCurve curve;
                            curve.mName = QString("histo sigma data " + QString::number(i) + " for chain" + QString::number(j));
                            curve.setPen(defaultPen);
                            curve.mPen.setColor(col);
                            curve.mIsHisto = false;
                            //curve.mData = equal_areas(date.mSigma.histoForChain(j), 1.f);
                            curve.mData = date.mSigma.histoForChain(j);
                            mGraph->addCurve(curve);
                            
                            yMax = 1.1f * map_max_value(curve.mData);
                            mGraph->setRangeY(0, qMax(mGraph->maximumY(), yMax));
                        }
                    }
                }
            }
        }
        else if(mCurrentTypeGraph == eTrace && mCurrentVariable == eTheta)
        {
            int chainIdx = -1;
            for(int i=0; i<mShowChainList.size(); ++i)
                if(mShowChainList[i])
                    chainIdx = i;
            
            if(chainIdx != -1)
            {
                Chain& chain = mChains[chainIdx];
                //mGraph->setCurrentX(0, chain.mNumBurnIter + chain.mNumBatchIter * chain.mBatchIndex + chain.mNumRunIter / chain.mThinningInterval);
                mGraph->setRangeX(0, chain.mNumBurnIter + chain.mNumBatchIter * chain.mBatchIndex + chain.mNumRunIter / chain.mThinningInterval);
                
                GraphCurve curve;
                curve.mUseVectorData = true;
                curve.mName = QString("trace chain " + QString::number(chainIdx)).toUtf8();
                curve.mDataVector = mEvent->mTheta.fullTraceForChain(mChains, chainIdx);
                curve.setPen(defaultPen);
                curve.mPen.setColor(Painting::chainColors[chainIdx]);
                curve.mIsHisto = false;
                mGraph->addCurve(curve);
                
                const Quartiles& quartiles = mEvent->mTheta.mResults.quartiles;
                
                GraphCurve curveQ1;
                curveQ1.mIsHorizontalLine = true;
                curveQ1.mHorizontalValue = quartiles.Q1;
                curveQ1.mName = QString("Q1");
                curveQ1.setPen(defaultPen);
                curveQ1.mPen.setColor(Qt::green);
                mGraph->addCurve(curveQ1);
                
                GraphCurve curveQ2;
                curveQ2.mIsHorizontalLine = true;
                curveQ2.mHorizontalValue = quartiles.Q2;
                curveQ2.mName = QString("Q2");
                curveQ2.setPen(defaultPen);
                curveQ2.mPen.setColor(Qt::red);
                mGraph->addCurve(curveQ2);
                
                GraphCurve curveQ3;
                curveQ3.mIsHorizontalLine = true;
                curveQ3.mHorizontalValue = quartiles.Q3;
                curveQ3.mName = QString("Q3");
                curveQ3.setPen(defaultPen);
                curveQ3.mPen.setColor(Qt::green);
                mGraph->addCurve(curveQ3);
                
                double min = vector_min_value(curve.mDataVector);
                double max = vector_max_value(curve.mDataVector);
                mGraph->setRangeY(floor(min), ceil(max));
            }
        }
        else if(mCurrentTypeGraph == eAccept && mCurrentVariable == eTheta && mEvent->mMethod == Event::eMHAdaptGauss)
        {
            int chainIdx = -1;
            for(int i=0; i<mShowChainList.size(); ++i)
                if(mShowChainList[i])
                    chainIdx = i;
            
            if(chainIdx != -1)
            {
                Chain& chain = mChains[chainIdx];
                //mGraph->setCurrentX(0, chain.mNumBurnIter + chain.mNumBatchIter * chain.mBatchIndex + chain.mNumRunIter / chain.mThinningInterval);
                mGraph->setRangeX(0, chain.mNumBurnIter + chain.mNumBatchIter * chain.mBatchIndex + chain.mNumRunIter / chain.mThinningInterval);
              
               mGraph->setRangeY(0, 100);
                
                GraphCurve curve;
                curve.mName = QString("accept history chain " + QString::number(chainIdx));
                curve.mDataVector = mEvent->mTheta.acceptationForChain(mChains, chainIdx);
                curve.setPen(defaultPen);
                curve.mPen.setColor(Painting::chainColors[chainIdx]);
                curve.mUseVectorData = true;
                curve.mIsHisto = false;
                mGraph->addCurve(curve);
                
                GraphCurve curveTarget;
                curveTarget.mName = "target";
                curveTarget.mIsHorizontalLine = true;
                curveTarget.mHorizontalValue = 44;
                curveTarget.setPen(defaultPen);
                curveTarget.mPen.setStyle(Qt::DashLine);
                curveTarget.mPen.setColor(QColor(180, 10, 20));
                mGraph->addCurve(curveTarget);
            }
        }
        else if(mCurrentTypeGraph == eCorrel && mCurrentVariable == eTheta && !isFixedBound)
        {
            int chainIdx = -1;
            for(int i=0; i<mShowChainList.size(); ++i)
                if(mShowChainList[i])
                    chainIdx = i;
            
            if(chainIdx != -1 && chainIdx < mChains.size())
            {
                if(true)//mChains[chainIdx].mThinningInterval == 1)
                {
                    GraphCurve curve;
                    curve.mName = QString("correlation chain " + QString::number(chainIdx));
                    curve.mDataVector = mEvent->mTheta.correlationForChain(chainIdx);
                    curve.mUseVectorData = true;
                    curve.setPen(defaultPen);
                    curve.mPen.setColor(Painting::chainColors[chainIdx]);
                    curve.mIsHisto = false;
                    mGraph->addCurve(curve);
                    
                    mGraph->setRangeX(0, 100);
                    mGraph->setRangeY(-1, 1);
                    
                    double n = mEvent->mTheta.runTraceForChain(mChains, chainIdx).size();
                    double limit = 1.96f / sqrt(n);
                    
                    //qDebug() << n << ", " <<limit;
                    
                    GraphCurve curveLimitLower;
                    curveLimitLower.mName = QString("correlation limit lower " + QString::number(chainIdx));
                    curveLimitLower.mIsHorizontalLine = true;
                    curveLimitLower.mHorizontalValue = -limit;
                    curveLimitLower.setPen(defaultPen);
                    curveLimitLower.mPen.setColor(Qt::red);
                    curveLimitLower.mPen.setStyle(Qt::DotLine);
                    mGraph->addCurve(curveLimitLower);
                    
                    GraphCurve curveLimitUpper;
                    curveLimitUpper.mName = QString("correlation limit upper " + QString::number(chainIdx));
                    curveLimitUpper.mIsHorizontalLine = true;
                    curveLimitUpper.mHorizontalValue = limit;
                    curveLimitUpper.setPen(defaultPen);
                    curveLimitUpper.mPen.setColor(Qt::red);
                    curveLimitUpper.mPen.setStyle(Qt::DotLine);
                    mGraph->addCurve(curveLimitUpper);
                }
                else
                {
                    // -----------------------------------------------
                    //  Important : auto-correlation must be calculated on  ALL TRACE VALUES !!
                    //  Otherwise, it is false. => The thinning must be 1!
                    //  TODO : find a solution to calculate auto-correlation on all trace points
                    //  without having to store all trace points (with thinning > 1)
                    // -----------------------------------------------
                    
                    mGraph->setNothingMessage(tr("The thinning interval must be 1 to display this curve."));
                }
            }
        }
    }
}

void GraphViewEvent::paintEvent(QPaintEvent* e)
{
   
    //QPainter p(this);
    
    
    if(mEvent)
    {
        
        this->setItemColor(mEvent->mColor);
        
        QString evenTitle = ( (mEvent->mType == Event::eDefault) ? tr("Event") : tr("Bound") ) ;
        this->setItemTitle(evenTitle + " : " + mEvent->mName);
       /* QColor backCol = mEvent->mColor;
        QColor foreCol = getContrastedColor(backCol);
        
        QRect topRect(0, 0, mGraphLeft, mLineH);
        p.setPen(backCol);
        p.setBrush(backCol);
        p.drawRect(topRect);
        
        p.setPen(Qt::black);
        p.drawLine(0, height(), mGraphLeft, height());
        
        p.setPen(foreCol);
        QFont font;
        font.setPointSizeF(pointSize(11));
        p.setFont(font);
        
        QString type = (mEvent->mType == Event::eDefault) ? tr("Event") : tr("Bound");
        p.drawText(topRect.adjusted(mMargin, 0, -mMargin, 0),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   type + " : " + mEvent->mName);
       */ 
       
    }
     GraphViewResults::paintEvent(e);
     //p.end();
}
