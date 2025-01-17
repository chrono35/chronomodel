#include "GraphViewPhase.h"
#include "GraphView.h"
#include "Phase.h"
#include "Painting.h"
#include "StdUtilities.h"
#include "QtUtilities.h"
#include "DateUtils.h"
#include "ModelUtilities.h"
#include "MainWindow.h"
#include "Button.h"
#include <QtWidgets>



#pragma mark Constructor / Destructor

GraphViewPhase::GraphViewPhase(QWidget *parent):GraphViewResults(parent),
mPhase(0)
{
    setMainColor(QColor(50, 50, 50));
    mGraph->setBackgroundColor(QColor(210, 210, 210));
    //mGraph->setRangeX(mSettings.mTmin, mSettings.mTmax); // it's done in GraphViewResults
    
    mDurationGraph = new GraphView(this);
    mDurationGraph -> setBackgroundColor(QColor(230, 230, 230));
    mDurationGraph -> addInfo(tr("WARNING : this graph scale is NOT the study period!"));
    mDurationGraph -> mLegendX = "";
    mDurationGraph -> setFormatFunctX(formatValueToAppSettingsPrecision);
    
    mDurationGraph->showXAxisArrow(true);
    mDurationGraph->showXAxisTicks(true);
    mDurationGraph->showXAxisSubTicks(true);
    mDurationGraph->showXAxisValues(true);
    
    mDurationGraph->showYAxisArrow(true);
    mDurationGraph->showYAxisTicks(false);
    mDurationGraph->showYAxisSubTicks(false);
    mDurationGraph->showYAxisValues(false);
    
    mDurationGraph -> setXAxisMode(GraphView::eAllTicks);
    mDurationGraph -> setYAxisMode(GraphView::eHidden);
    mDurationGraph -> setBackgroundColor(QColor(210, 210, 210));
    
    mDurationGraph->setMargins(50, 10, 5, 30);
    mDurationGraph->setRangeY(0, 1);
    
    mDurationGraph->setVisible(false);
    
    mShowDuration = new Button(tr("Show Duration"), this);
    mShowDuration->setCheckable(true);
    mShowDuration->setFlatHorizontal();
    connect(mShowDuration, SIGNAL(toggled(bool)), this, SLOT(showDuration(bool)));
    //disconnect(mDataSaveBut, SIGNAL(clicked()), GraphViewResults, SLOT(saveGraphData()));
    //connect(mDataSaveBut, SIGNAL(clicked()), this, SLOT(saveGraphData()));
}

GraphViewPhase::~GraphViewPhase()
{
    mPhase = 0;
}

void GraphViewPhase::setGraphFont(const QFont& font)
{
    GraphViewResults::setGraphFont(font);
    mDurationGraph->setGraphFont(font);
}

void GraphViewPhase::setButtonsVisible(const bool visible)
{
    GraphViewResults::setButtonsVisible(visible);
    mShowDuration->setVisible(mButtonsVisible);
}

void GraphViewPhase::setPhase(Phase* phase)
{
    if(phase)
    {
        mPhase = phase;
        setItemTitle(tr("Phase") + " : " + mPhase->getName());
        setItemColor(mPhase->getColor());
    }
    update();
}

void GraphViewPhase::updateLayout()
{
    GraphViewResults::updateLayout();
    
    int h = height();
    int leftShift = mButtonsVisible ? mGraphLeft : 0;
    QRect graphRect(leftShift, mTopShift, this->width() - leftShift, height()-mTopShift);
    
    bool axisVisible = (h > mHeightForVisibleAxis);
    mDurationGraph->showXAxisValues(axisVisible);
    mDurationGraph->setMarginBottom(axisVisible ? mDurationGraph->font().pointSizeF() + 10 : 10);
    
    if(mButtonsVisible)
    {
        int butInlineMaxH = 50;
        int bh = (height() - mLineH) / 2;
        bh = qMin(bh, butInlineMaxH);
        mShowDuration->setGeometry(0, mLineH + bh, mGraphLeft, bh);
    }
    mDurationGraph->setGeometry(graphRect.adjusted(0, 0, 0, mShowNumResults ? -graphRect.height()/2 : 0));
}

void GraphViewPhase::paintEvent(QPaintEvent* e)
{
    GraphViewResults::paintEvent(e);
}

void GraphViewPhase::generateCurves(TypeGraph typeGraph, Variable variable)
{
    GraphViewResults::generateCurves(typeGraph, variable);
    
    mGraph->removeAllCurves();
    mGraph->removeAllZones();
    mGraph->clearInfos();
    mGraph->resetNothingMessage();
    
    mGraph->autoAdjustYScale(typeGraph == eTrace);
    
    mDurationGraph->removeAllCurves();
    QPen defaultPen;
    defaultPen.setWidthF(1);
    defaultPen.setStyle(Qt::SolidLine);
    
    
    if(mPhase)
    {
        const QColor color = mPhase->getColor();
        
        // ------------------------------------------------
        //  first tab : posterior distrib
        //  Possible curves :
        //  - Post Distrib Alpha All Chains
        //  - Post Distrib Beta All Chains
        //  - HPD Alpha All Chains
        //  - HPD Beta All Chains
        //  - Duration
        //  - HPD Duration
        //  - Post Distrib Alpha i
        //  - Post Distrib Beta i
        // ------------------------------------------------
        if(typeGraph == ePostDistrib && variable == eTheta)
        {
            mGraph->mLegendX = DateUtils::getAppSettingsFormat();
            mGraph->mLegendY = "";
            mGraph->setFormatFunctX(DateUtils::convertToAppSettingsFormatStr);
            mGraph->setFormatFunctY(formatValueToAppSettingsPrecision);
            mTitle =  tr("Phase") + " : " + mPhase->getName();
            
            mGraph->setRangeX(mSettings.mTmin, mSettings.mTmax);
            
            mShowDuration->setVisible(true);
            showDuration(mShowDuration->isChecked());
            
            QString resultsText = ModelUtilities::phaseResultsText(mPhase);
            QString resultsHTML = ModelUtilities::phaseResultsHTML(mPhase);
            setNumericalResults(resultsHTML, resultsText);
            
            GraphCurve curveAlpha = generateDensityCurve(mPhase->mAlpha.fullHisto(),
                                                         "Post Distrib Alpha All Chains",
                                                         color, Qt::DotLine);
            
            GraphCurve curveBeta = generateDensityCurve(mPhase->mBeta.fullHisto(),
                                                         "Post Distrib Beta All Chains",
                                                         color, Qt::DashLine);
            
            
            GraphCurve curveAlphaHPD = generateHPDCurve(mPhase->mAlpha.mHPD,
                                                         "HPD Alpha All Chains",
                                                         color);
            
            GraphCurve curveBetaHPD = generateHPDCurve(mPhase->mBeta.mHPD,
                                                       "HPD Beta All Chains",
                                                       color);
            
            GraphCurve curveDuration = generateDensityCurve(mPhase->mDuration.fullHisto(),
                                                            "Duration",
                                                            color);
            mGraph->addCurve(curveBeta);
            mGraph->addCurve(curveAlpha);
            mGraph->addCurve(curveAlphaHPD);
            mGraph->addCurve(curveBetaHPD);
            mDurationGraph->addCurve(curveDuration);
            
            if(!curveDuration.mData.isEmpty())
            {
                GraphCurve curveDurationHPD = generateHPDCurve(mPhase->mDuration.mHPD,
                                                               "HPD Duration",
                                                               color);
                mDurationGraph->addCurve(curveDurationHPD);
                mDurationGraph->setFormatFunctX(formatValueToAppSettingsPrecision);
                mDurationGraph->setFormatFunctY(formatValueToAppSettingsPrecision);

             }
            
            
            for(int i=0; i<mChains.size(); ++i)
            {
                GraphCurve curveAlpha = generateDensityCurve(mPhase->mAlpha.histoForChain(i),
                                                             "Post Distrib Alpha " + QString::number(i),
                                                             color, Qt::DotLine);
                
                GraphCurve curveBeta = generateDensityCurve(mPhase->mBeta.histoForChain(i),
                                                            "Post Distrib Beta " + QString::number(i),
                                                            color, Qt::DashLine);
                mGraph->addCurve(curveAlpha);
                mGraph->addCurve(curveBeta);
                
            }
           
        }
        
    
        // ------------------------------------------------
        //  second tab : history plot
        //  - Trace Alpha i
        //  - Q1 Alpha i
        //  - Q2 Alpha i
        //  - Q3 Alpha i
        //  - Trace Beta i
        //  - Q1 Beta i
        //  - Q2 Beta i
        //  - Q3 Beta i
        // ------------------------------------------------
        else if(typeGraph == eTrace && variable == eTheta)
        {
            mGraph->mLegendX = "Iterations";
            mGraph->setFormatFunctX(0);
            mGraph->setFormatFunctY(DateUtils::convertToAppSettingsFormatStr);
            
            mShowDuration->setVisible(false);
            mShowDuration->setChecked(false);
            showDuration(false);
            
            generateTraceCurves(mChains, &(mPhase->mAlpha), "Alpha");
            generateTraceCurves(mChains, &(mPhase->mBeta), "Beta");
            
        }
    }
}

void GraphViewPhase::updateCurvesToShow(bool showAllChains, const QList<bool>& showChainList, bool showCredibility, bool showCalib, bool showWiggle)
{
    GraphViewResults::updateCurvesToShow(showAllChains, showChainList, showCredibility, showCalib, showWiggle);
    
    if(mPhase)
    {
        // ------------------------------------------------
        //  first tab : posterior distrib
        //  Possible curves :
        //  - Post Distrib Alpha All Chains
        //  - Post Distrib Beta All Chains
        //  - HPD Alpha All Chains
        //  - HPD Beta All Chains
        //  - Duration
        //  - HPD Duration
        //  - Post Distrib Alpha i
        //  - Post Distrib Beta i
        // ------------------------------------------------
        if(mCurrentTypeGraph == ePostDistrib && mCurrentVariable == eTheta)
        {
            mGraph->setTipYLab("");

            mGraph->setCurveVisible("Post Distrib Alpha All Chains", mShowAllChains);
            mGraph->setCurveVisible("Post Distrib Beta All Chains", mShowAllChains);
            mGraph->setCurveVisible("HPD Alpha All Chains", mShowAllChains);
            mGraph->setCurveVisible("HPD Beta All Chains", mShowAllChains);
            
            for(int i=0; i<mShowChainList.size(); ++i)
            {
                mGraph->setCurveVisible("Post Distrib Alpha " + QString::number(i), mShowChainList[i]);
                mGraph->setCurveVisible("Post Distrib Beta " + QString::number(i), mShowChainList[i]);
            }
            mGraph->adjustYToMaxValue();
            
            mDurationGraph->setCurveVisible("Duration", mShowAllChains);
            mDurationGraph->setCurveVisible("HPD Duration", mShowAllChains);
            
            mDurationGraph->adjustYToMaxValue();
            mGraph->setTipXLab("t");
            mGraph->setYAxisMode(GraphView::eHidden);
        }
        
        // ------------------------------------------------
        //  second tab : history plot
        //  - Alpha Trace i
        //  - Alpha Q1 i
        //  - Alpha Q2 i
        //  - Alpha Q3 i
        //  - Beta Trace i
        //  - Beta Q1 i
        //  - Beta Q2 i
        //  - Beta Q3 i
        // ------------------------------------------------
        else if(mCurrentTypeGraph == eTrace && mCurrentVariable == eTheta)
        {
            for(int i=0; i<mShowChainList.size(); ++i){
                mGraph->setCurveVisible("Alpha Trace " + QString::number(i), mShowChainList[i]);
                mGraph->setCurveVisible("Alpha Q1 " + QString::number(i), mShowChainList[i]);
                mGraph->setCurveVisible("Alpha Q2 " + QString::number(i), mShowChainList[i]);
                mGraph->setCurveVisible("Alpha Q3 " + QString::number(i), mShowChainList[i]);
                
                mGraph->setCurveVisible("Beta Trace " + QString::number(i), mShowChainList[i]);
                mGraph->setCurveVisible("Beta Q1 " + QString::number(i), mShowChainList[i]);
                mGraph->setCurveVisible("Beta Q2 " + QString::number(i), mShowChainList[i]);
                mGraph->setCurveVisible("Beta Q3 " + QString::number(i), mShowChainList[i]);
            }
            mGraph->adjustYToMinMaxValue();
            mGraph->setTipXLab("iteration");
            mGraph->setTipYLab("t");
            mGraph->setYAxisMode(GraphView::eMinMax);
        }
    }
}


void GraphViewPhase::showDuration(bool show)
{
    mDurationGraph->setVisible(show);
    mGraph->setVisible(!show);
    mShowDuration->raise();
    
}

void GraphViewPhase::saveGraphData() const
{
    if(mShowDuration->isChecked()) {
        AppSettings settings = MainWindow::getInstance()->getAppSettings();
        QString currentPath = MainWindow::getInstance()->getCurrentPath();
        QString csvSep = settings.mCSVCellSeparator;

        QLocale csvLocal = settings.mCSVCellSeparator == "." ? QLocale::English : QLocale::French;
        csvLocal.setNumberOptions(QLocale::OmitGroupSeparator);
        
        int offset = 0;
        
        if(mCurrentTypeGraph == eTrace || mCurrentTypeGraph == eAccept)
        {
            QMessageBox messageBox;
            messageBox.setWindowTitle(tr("Save all trace"));
            messageBox.setText(tr("Do you want the entire trace from the beginning of the process or only the aquisition part"));
            QAbstractButton *allTraceButton = messageBox.addButton(tr("All the process"), QMessageBox::YesRole);
            QAbstractButton *acquireTraceButton = messageBox.addButton(tr("Only aquisition part"), QMessageBox::NoRole);
            
            messageBox.exec();
            if (messageBox.clickedButton() == allTraceButton)  {
                mDurationGraph->exportCurrentVectorCurves(currentPath, csvLocal, csvSep, false, 0);
            }
            else if (messageBox.clickedButton() == acquireTraceButton) {
                int chainIdx = -1;
                for(int i=0; i<mShowChainList.size(); ++i)
                    if(mShowChainList[i]) chainIdx = i;
                if(chainIdx != -1) {
                    offset = mChains[chainIdx].mNumBurnIter + mChains[chainIdx].mBatchIndex * mChains[chainIdx].mNumBatchIter;
                }
                mDurationGraph->exportCurrentVectorCurves(currentPath, csvLocal, csvSep, false, offset);
            }
            else return;
        }
        
        else if(mCurrentTypeGraph == eCorrel) {
            mDurationGraph->exportCurrentVectorCurves(currentPath, csvLocal, csvSep, false, 0);
        }
        
        // All visible curves are saved in the same file, the credibility bar is not save
        
        else if(mCurrentTypeGraph == ePostDistrib) {
            mDurationGraph->exportCurrentDensityCurves(currentPath, csvLocal, csvSep,  mSettings.mStep);
        }
    }
    else {
        GraphViewResults::saveGraphData();
    }
}
