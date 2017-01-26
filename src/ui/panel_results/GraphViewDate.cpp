#include "GraphViewDate.h"
#include "GraphView.h"
#include "Date.h"
#include "Event.h"
#include "Painting.h"
#include "StdUtilities.h"
#include "QtUtilities.h"
#include "ModelUtilities.h"
#include <QtWidgets>
#include "../PluginAbstract.h"
#include "../GraphViewRefAbstract.h"



#pragma mark Constructor / Destructor

GraphViewDate::GraphViewDate(QWidget *parent):GraphViewResults(parent),
mDate(nullptr),
mColor(Qt::blue)
{
    setMainColor(QColor(155, 155, 155));
    mGraph->setBackgroundColor(Qt::white);
}

GraphViewDate::~GraphViewDate()
{
    mDate = nullptr;
}

void GraphViewDate::setDate(Date* date)
{
    Q_ASSERT(date);
   // if (date) {
        mDate = date;
        setItemTitle(QString(tr("Data") + " : " + mDate->mName));
        setItemColor(mDate->mColor);
   // }
    update();
}

void GraphViewDate::setColor(const QColor& color)
{
    mColor = color;
    setItemColor(mColor);
    update();
}

void GraphViewDate::paintEvent(QPaintEvent* e)
{
    GraphViewResults::paintEvent(e);
}

// not used since v1.4
QColor GraphViewDate::getEventColor()
{
    return mDate->getEventColor();
}

void GraphViewDate::generateCurves(TypeGraph typeGraph, Variable variable)
{
    //qDebug()<<"GraphViewDate::generateCurves()";
    GraphViewResults::generateCurves(typeGraph, variable);
   
    /* ------------------------------------------------
     *  Reset the graph object settings
     * ------------------------------------------------
     */
    mGraph->removeAllCurves();
    mGraph->reserveCurves(6);

    mGraph->removeAllZones();
    mGraph->clearInfos();
    mGraph->resetNothingMessage();
    
    if (mDate) {
        QColor color = mDate->mColor;
        QPen defaultPen;
        defaultPen.setWidthF(1);
        defaultPen.setStyle(Qt::SolidLine);
        QString resultsText = ModelUtilities::dateResultsText(mDate);
        QString resultsHTML = ModelUtilities::dateResultsHTML(mDate);
        setNumericalResults(resultsHTML, resultsText);
        
        /* ------------------------------------------------
         *  Are we working on calendar date or std dev ?
         * ------------------------------------------------
         */
        MHVariable* variableDate = &(mDate->mTheta);
        if (variable == eTheta)
            variableDate = &(mDate->mTheta);

        else if(variable == eSigma)
            variableDate = &(mDate->mSigma);
       
        /* ------------------------------------------------
         *  First tab : Posterior distrib.
         * ------------------------------------------------
         */
        if (typeGraph == ePostDistrib) {
            
            /* ------------------------------------------------
             *  Possible Curves :
             *  - Post Distrib All Chains
             *  - Post Distrib Chain i
             *  - HPD All Chains
             *  - Credibility All Chains
             *  - Calibration
             *  - Wiggle
             * ------------------------------------------------
             */
            if (variable == eTheta) {
                mTitle = QString(tr("Data") + " : " + mDate->mName);


                mGraph->mLegendX = DateUtils::getAppSettingsFormatStr();
                mGraph->setFormatFunctX(stringWithAppSettings);
                mGraph->setFormatFunctY(nullptr);


                //  Post Distrib All Chains
                GraphCurve curvePostDistrib = generateDensityCurve(variableDate->fullHisto(),
                                                             "Post Distrib All Chains",
                                                             color,
                                                             Qt::SolidLine,
                                                             Qt::NoBrush);                
                mGraph->addCurve(curvePostDistrib);
                
                // Post Distrib Chain i
                if (!variableDate->mChainsHistos.isEmpty())
                    for (int i=0; i<mChains.size(); ++i) {
                        GraphCurve curvePostDistribChain = generateDensityCurve(variableDate->histoForChain(i),
                                                                                "Post Distrib Chain " + QString::number(i),
                                                                                Painting::chainColors.at(i),
                                                                                Qt::SolidLine,
                                                                                Qt::NoBrush);
                        mGraph->addCurve(curvePostDistribChain);
                    }
                
                // HPD All Chains
                GraphCurve curveHPD = generateHPDCurve(variableDate->mHPD,
                                                       "HPD All Chains",
                                                        color);
                mGraph->addCurve(curveHPD);
                
                // Calibration
                const QMap<double,double> formatedCalib = mDate->getFormatedCalibMap();

                GraphCurve curveCalib = generateDensityCurve(formatedCalib,
                                                             "Calibration",
                                                             QColor(150, 150, 150),
                                                             Qt::SolidLine,
                                                             Qt::NoBrush);
                mGraph->addCurve(curveCalib);
                
                // Wiggle
                GraphCurve curveWiggle = generateDensityCurve(mDate->mWiggle.fullHisto(),
                                                              "Wiggle",
                                                              mColor,
                                                              Qt::DashLine,
                                                              Qt::NoBrush);
                mGraph->addCurve(curveWiggle);
                
                // Credibility (must be the last created curve because uses yMax!
                GraphCurve curveCred = generateSectionCurve(variableDate->mCredibility,
                                                                "Credibility All Chains",
                                                                color);
                mGraph->addCurve(curveCred);

            }
            
            /* ------------------------------------------------
             *  Possible Curves :
             *  - Sigma All Chains
             *  - Sigma Chain i
             * ------------------------------------------------
             */
            else if (variable == eSigma) {
                mTitle = QString(tr("Std") + " : " + mDate->mName);

                mGraph->mLegendX = "";
                mGraph->setFormatFunctX(nullptr);
                mGraph->setFormatFunctY(stringWithAppSettings);
                
                //  Post Distrib All Chains
                GraphCurve curvePostDistrib = generateDensityCurve(variableDate->fullHisto(),
                                                                   "Sigma all Chains",
                                                                   color,
                                                                   Qt::SolidLine,
                                                                   Qt::NoBrush);
                mGraph->addCurve(curvePostDistrib); 
                
                // Post Distrib Chain i
                if (!variableDate->mChainsHistos.isEmpty())
                    for (int i=0; i<mChains.size(); ++i) {
                        GraphCurve curvePostDistribChain = generateDensityCurve(variableDate->histoForChain(i),
                                                                                "Sigma for Chain " + QString::number(i),
                                                                                Painting::chainColors.at(i),
                                                                                Qt::SolidLine,
                                                                                Qt::NoBrush);
                        mGraph->addCurve(curvePostDistribChain);
                    }
                
                // Check if necessary ?
                mGraph->autoAdjustYScale(true);
            }

           mGraph->autoAdjustYScale(true); // do repaintGraph()
        }
        /* ------------------------------------------------
         *  Second tab : History plots.
         *  Possible Curves (could be for theta or sigma):
         *  - Trace i
         *  - Q1 i
         *  - Q2 i
         *  - Q3 i
         * ------------------------------------------------
         */
        else if (typeGraph == eTrace) {
            mGraph->mLegendX = "Iterations";
            mGraph->setFormatFunctX(nullptr);
            mGraph->setFormatFunctY(DateUtils::convertToAppSettingsFormatStr);

            generateTraceCurves(mChains, variableDate);
            mGraph->autoAdjustYScale(true); // do repaintGraph()
        }
        /* ------------------------------------------------
         *  Third tab : Acceptation rate.
         *  Possible curves (could be for theta or sigma):
         *  - Accept i
         *  - Accept Target
         * ------------------------------------------------
         */
        else if (typeGraph == eAccept) {
            mGraph->mLegendX = "Iterations";
            mGraph->setFormatFunctX(nullptr);
            mGraph->setFormatFunctY(nullptr);
            mGraph->autoAdjustYScale(true);
          
            mGraph->addCurve( generateHorizontalLine(44, "Accept Target", QColor(180, 10, 20), Qt::DashLine) );
            generateAcceptCurves(mChains, variableDate);

            mGraph->autoAdjustYScale(true); // do repaintGraph()
        }
        
        /* ------------------------------------------------
         *  fourth tab : Autocorrelation
         *  Possible curves (could be for theta or sigma):
         *  - Correl i
         *  - Correl Limit Lower i
         *  - Correl Limit Upper i
         * ------------------------------------------------
         */
        else if (typeGraph == eCorrel) {
            mGraph->mLegendX = "";
            mGraph->setFormatFunctX(nullptr);
            mGraph->setFormatFunctY(stringWithAppSettings);

            generateCorrelCurves(mChains, variableDate);
            //mGraph->setRangeY(-1, 1);

            mGraph->autoAdjustYScale(true); // do repaintGraph()
        }
    }
}

void GraphViewDate::updateCurvesToShow(bool showAllChains, const QList<bool>& showChainList, bool showCredibility, bool showCalib, bool showWiggle)
{
    GraphViewResults::updateCurvesToShow(showAllChains, showChainList, showCredibility, showCalib, showWiggle);
    
    // ------------------------------------------------
    //  First Tab : Posterior distrib.
    // ------------------------------------------------
    if (mCurrentTypeGraph == ePostDistrib) {
        mGraph->setTipYLab("");
        /* ------------------------------------------------
         *  Possible Curves :
         *  - Post Distrib All Chains
         *  - Post Distrib Chain i
         *  - HPD All Chains
         *  - Credibility All Chains
         *  - Calibration
         *  - Wiggle
         * ------------------------------------------------
         */
        if (mCurrentVariable == eTheta) {
            
            mGraph->setCurveVisible("Post Distrib All Chains", mShowAllChains);
            mGraph->setCurveVisible("HPD All Chains", mShowAllChains);
            mGraph->setCurveVisible("Credibility All Chains", mShowAllChains && mShowCredibility);
            mGraph->setCurveVisible("Calibration", mShowCalib);
            mGraph->setCurveVisible("Wiggle", mShowWiggle);
            for (int i=0; i<mShowChainList.size(); ++i)
                mGraph->setCurveVisible("Post Distrib Chain " + QString::number(i), mShowChainList[i]);
            
            mGraph->setTipXLab("t");

            mGraph->setYAxisMode(GraphView::eHidden);
        }
        /* ------------------------------------------------
         *  Possible Curves :
         *  - Sigma All Chains
         *  - Sigma Chain i
         * ------------------------------------------------
         */
        else if (mCurrentVariable == eSigma) {
            mGraph->setCurveVisible("Sigma all Chains", mShowAllChains);
            //mGraph->setCurveVisible("Post Distrib All Chains", mShowAllChains);
            for (int i=0; i<mShowChainList.size(); ++i)
                mGraph->setCurveVisible("Sigma for Chain " + QString::number(i), mShowChainList[i]);
            
            mGraph->setTipXLab("sigma");
            mGraph->setYAxisMode(GraphView::eHidden);
        }
        mGraph->autoAdjustYScale(true);
    }
    /* ------------------------------------------------
     *  Second tab : History plots.
     *  Possible Curves (could be for theta or sigma):
     *  - Trace i
     *  - Q1 i
     *  - Q2 i
     *  - Q3 i
     * ------------------------------------------------
     */
    else if (mCurrentTypeGraph == eTrace) {
        for (int i=0; i<mShowChainList.size(); ++i) {
            mGraph->setCurveVisible("Trace " + QString::number(i), mShowChainList.at(i));
            mGraph->setCurveVisible("Q1 " + QString::number(i), mShowChainList.at(i));
            mGraph->setCurveVisible("Q2 " + QString::number(i), mShowChainList.at(i));
            mGraph->setCurveVisible("Q3 " + QString::number(i), mShowChainList.at(i));
        }

        mGraph->setTipXLab("iterations");
        mGraph->setTipYLab("t");
        mGraph->setYAxisMode(GraphView::eMinMax);
        mGraph->autoAdjustYScale(true); // do  repaintGraph()
    }
    /* ------------------------------------------------
     *  Third tab : Acceptation rate.
     *  Possible curves (could be for theta or sigma):
     *  - Accept i
     *  - Accept Target
     * ------------------------------------------------
     */
    else if (mCurrentTypeGraph == eAccept) {

        mGraph->setCurveVisible("Accept Target", true);
        for (int i=0; i<mShowChainList.size(); ++i)
            mGraph->setCurveVisible("Accept " + QString::number(i), mShowChainList.at(i));
        
        mGraph->setTipXLab("iterations");
        mGraph->setTipYLab("rate");

        mGraph->setYAxisMode(GraphView::eMinMax);
        mGraph->autoAdjustYScale(true); // do  repaintGraph()
    }
    
    /* ------------------------------------------------
     *  fourth tab : Autocorrelation
     *  Possible curves (could be for theta or sigma):
     *  - Correl i
     *  - Correl Limit Lower i
     *  - Correl Limit Upper i
     * ------------------------------------------------
     */
    else if (mCurrentTypeGraph == eCorrel) {
        for (int i=0; i<mShowChainList.size(); ++i) {
            mGraph->setCurveVisible("Correl " + QString::number(i), mShowChainList.at(i));
            mGraph->setCurveVisible("Correl Limit Lower " + QString::number(i), mShowChainList.at(i));
            mGraph->setCurveVisible("Correl Limit Upper " + QString::number(i), mShowChainList.at(i));
        }
        mGraph->setTipXLab("h");
        mGraph->setTipYLab("value");
        mGraph->setYAxisMode(GraphView::eMinMax);
    }
   repaint();
}
