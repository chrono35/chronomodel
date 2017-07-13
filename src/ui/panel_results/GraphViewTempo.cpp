#include "GraphViewTempo.h"
#include "GraphView.h"
#include "Phase.h"
#include "Painting.h"
#include "StdUtilities.h"
#include "QtUtilities.h"
#include "DateUtils.h"
#include "ModelUtilities.h"
#include "MainWindow.h"
#include <QtWidgets>

// Constructor / Destructor

GraphViewTempo::GraphViewTempo(QWidget *parent):GraphViewResults(parent),
mPhase(nullptr)
{
    setMainColor(QColor(50, 50, 50));
    mGraph->setBackgroundColor(QColor(210, 210, 210));

}

GraphViewTempo::~GraphViewTempo()
{
    mPhase = nullptr;
}

void GraphViewTempo::setGraphFont(const QFont& font)
{
    GraphViewResults::setFont(font);
    updateLayout();
}


void GraphViewTempo::setPhase(Phase* phase)
{
    Q_ASSERT(phase);

    mPhase = phase;
    setItemTitle(tr("Phase") + " : " + mPhase->mName);

    setItemColor(mPhase->mColor);

}

void GraphViewTempo::updateLayout()
{
        GraphViewResults::updateLayout();
}

void GraphViewTempo::paintEvent(QPaintEvent* e)
{
    GraphViewResults::paintEvent(e);
}

void GraphViewTempo::resizeEvent(QResizeEvent* )
{
    updateLayout();
}

void GraphViewTempo::generateCurves(TypeGraph typeGraph, Variable variable)
{
    //qDebug()<<"GraphViewTempo::generateCurves()";
    Q_ASSERT(mPhase);
    GraphViewResults::generateCurves(typeGraph, variable);
    
    mGraph->removeAllCurves();
    mGraph->reserveCurves(9);

    mGraph->removeAllZones();
    mGraph->clearInfos();
    mGraph->resetNothingMessage();
    
    QPen defaultPen;
    defaultPen.setWidthF(1.);
    defaultPen.setStyle(Qt::SolidLine);
    
    QColor color = mPhase->mColor;

    QString resultsText = QString("Tempo Results");//ModelUtilities::phaseResultsText(mPhase);
    QString resultsHTML = QString("Tempo Results");//ModelUtilities::phaseResultsHTML(mPhase);
    setNumericalResults(resultsHTML, resultsText);

    mGraph->setOverArrow(GraphView::eNone);
    /* -------------first tab : posterior distrib-----------------------------------
     *  Possible curves :
     *  - Post Distrib Alpha All Chains
     *  - Post Distrib Beta All Chains
     *  - HPD Alpha All Chains
     *  - HPD Beta All Chains
     *  - Duration
     *  - HPD Duration
     *  - Post Distrib Alpha i
     *  - Post Distrib Beta i
     *  - Time Range
     * ------------------------------------------------  */

    if ((typeGraph == ePostDistrib) && (variable == eDuration)) {
        mGraph->mLegendX = "Years";
        mGraph->mLegendY = "";
        mGraph->setFormatFunctX(stringWithAppSettings);
        mGraph->setFormatFunctY(nullptr);

        mTitle = tr("Phase Duration") + " : " + mPhase->mName;
        GraphCurve curveDuration;

        if (mPhase->mDuration.fullHisto().size()>1) {
            curveDuration = generateDensityCurve(mPhase->mDuration.fullHisto(),
                                                            "Post Distrib Duration All Chains",
                                                            color);

            mGraph->setRangeX(0., ceil(curveDuration.mData.lastKey()));
            color.setAlpha(255);
            GraphCurve curveDurationHPD = generateHPDCurve(mPhase->mDuration.mHPD,
                                                           "HPD Duration All Chains",
                                                           color);
            mGraph->setCanControlOpacity(true);
            mGraph->addCurve(curveDurationHPD);
            mGraph->setFormatFunctX(stringWithAppSettings);
            mGraph->setFormatFunctY(nullptr);

            mGraph->addCurve(curveDuration);


            /* ------------------------------------
             *  Theta Credibility
             * ------------------------------------
             */
            GraphCurve curveCred = generateSectionCurve(mPhase->mDuration.mCredibility,
                                                            "Credibility All Chains",
                                                            color);
            mGraph->addCurve(curveCred);

        } else
            mGraph->resetNothingMessage();


        if (!mPhase->mDuration.mChainsHistos.isEmpty())
            for (int i=0; i<mChains.size(); ++i) {
                GraphCurve curveDuration = generateDensityCurve(mPhase->mDuration.histoForChain(i),
                                                             "Post Distrib Duration " + QString::number(i),
                                                             Painting::chainColors.at(i), Qt::DotLine);

                mGraph->addCurve(curveDuration);
            }
     }

    else if ((typeGraph == ePostDistrib) && (variable == eTempo)) {
        mGraph->mLegendX = DateUtils::getAppSettingsFormatStr();
        mGraph->mLegendY = "";
        mGraph->setFormatFunctX(stringWithAppSettings);
        mGraph->setFormatFunctY(nullptr);
        mTitle = tr("Phase Tempo") + " : " + mPhase->mName;


        GraphCurve curveTempo = generateDensityCurve(mPhase->mTempo,
                                                     "Post Distrib Tempo All Chains",
                                                     color.darker(), Qt::SolidLine);

        GraphCurve curveTempoInf = generateDensityCurve(mPhase->mTempoInf,
                                                     "Post Distrib Tempo Inf All Chains",
                                                     color, Qt::SolidLine);

        GraphCurve curveTempoSup = generateDensityCurve(mPhase->mTempoSup,
                                                     "Post Distrib Tempo Sup All Chains",
                                                     color, Qt::SolidLine);


        mGraph->addCurve(curveTempoInf);
        mGraph->addCurve(curveTempo);
        mGraph->addCurve(curveTempoSup);


        mGraph->setOverArrow(GraphView::eBothOverflow);



        // ------------------------------------------------------------
        //  Add zones outside study period
        // ------------------------------------------------------------

        GraphZone zoneMin;
        zoneMin.mXStart = -INFINITY;
        zoneMin.mXEnd = mSettings.getTminFormated();
        zoneMin.mColor = QColor(217, 163, 69);
        zoneMin.mColor.setAlpha(35);
        zoneMin.mText = tr("Outside study period");
        mGraph->addZone(zoneMin);

        GraphZone zoneMax;
        zoneMax.mXStart = mSettings.getTmaxFormated();
        zoneMax.mXEnd = INFINITY;
        zoneMax.mColor = QColor(217, 163, 69);
        zoneMax.mColor.setAlpha(35);
        zoneMax.mText = tr("Outside study period");
        mGraph->addZone(zoneMax);
/*
        if (!mPhase->mAlpha.mChainsHistos.isEmpty())
            for (int i=0; i<mChains.size(); ++i) {
                GraphCurve curveAlpha = generateDensityCurve(mPhase->mAlpha.histoForChain(i),
                                                             "Post Distrib Alpha " + QString::number(i),
                                                             Painting::chainColors.at(i), Qt::DotLine);

                GraphCurve curveBeta = generateDensityCurve(mPhase->mBeta.histoForChain(i),
                                                            "Post Distrib Beta " + QString::number(i),
                                                            Painting::chainColors.at(i).darker(170), Qt::DashLine);
                mGraph->addCurve(curveAlpha);
                mGraph->addCurve(curveBeta);
            }
 */
    }

    else if (typeGraph == ePostDistrib && variable == eIntensity) {

        mGraph->mLegendX = "";
        mGraph->setFormatFunctX(stringWithAppSettings);
        mGraph->setFormatFunctY(stringWithAppSettings);

        mTitle = tr("Phase Intensity") + " : " + mPhase->mName;
        GraphCurve curveIntensity = generateDensityCurve(mPhase->mIntensity,
                                                     "Post Distrib Intensity All Chains",
                                                     color, Qt::SolidLine);

        mGraph->addCurve(curveIntensity);
    }

    /* -----------------second tab : history plot-------------------------------
     *  - Trace Alpha i
     *  - Q1 Alpha i
     *  - Q2 Alpha i
     *  - Q3 Alpha i
     *  - Trace Beta i
     *  - Q1 Beta i
     *  - Q2 Beta i
     *  - Q3 Beta i
     * ------------------------------------------------ */

    else if (typeGraph == eTrace && variable == eDuration) {
        mGraph->mLegendX = "Iterations";
        mGraph->setFormatFunctX(nullptr);
        mGraph->setFormatFunctY(stringWithAppSettings);
        mTitle = tr("Phase Duration") + " : " + mPhase->mName;

        generateTraceCurves(mChains, &(mPhase->mDuration), "Duration");
        mGraph->autoAdjustYScale(true);
    }
    /* ------------------------------------------------
     *  third tab : Acception rate
     *  fourth tab : Autocorrelation
     * ------------------------------------------------ */
    else {
       mTitle = tr("Phase") + " : " + mPhase->mName;
       mGraph->resetNothingMessage();
    }


}

void GraphViewTempo::updateCurvesToShow(bool showAllChains, const QList<bool>& showChainList, bool showCredibility, bool showCalib, bool showWiggle)
{
    Q_ASSERT(mPhase);
    GraphViewResults::updateCurvesToShow(showAllChains, showChainList, showCredibility, showCalib, showWiggle);
    
    /* --------------------first tab : posterior distrib----------------------------
     *
     *  Possible curves :
     *  - Post Distrib Alpha All Chains
     *  - Post Distrib Beta All Chains
     *  - HPD Alpha All Chains
     *  - HPD Beta All Chains
     *  - Duration
     *  - HPD Duration
     *  - Post Distrib Alpha i
     *  - Post Distrib Beta i
     * ------------------------------------------------
     */
     if (mCurrentTypeGraph == ePostDistrib && mCurrentVariable == eDuration) {
        const GraphCurve* duration = mGraph->getCurve("Post Distrib Duration All Chains");

        if ( duration && !duration->mData.isEmpty()) {

            mGraph->setCurveVisible("Post Distrib Duration All Chains", mShowAllChains);
            mGraph->setCurveVisible("HPD Duration All Chains", mShowAllChains);
            mGraph->setCurveVisible("Credibility All Chains", mShowCredibility && mShowAllChains);

            for (int i=0; i<mShowChainList.size(); ++i)
                mGraph->setCurveVisible("Post Distrib Duration " + QString::number(i), mShowChainList.at(i));

            mGraph->setTipXLab("t");
            mGraph->setTipYLab("");
            mGraph->setYAxisMode(GraphView::eHidden);
            mGraph->autoAdjustYScale(true);
        }

    }
    else if (mCurrentTypeGraph == ePostDistrib && mCurrentVariable == eTempo) {

         const GraphCurve* tempo = mGraph->getCurve("Post Distrib Tempo All Chains");

         if ( tempo && !tempo->mData.isEmpty()) {

            // mGraph->setCurveVisible("Post Distrib Tempo All Chains", mShowAllChains);
             mGraph->setCurveVisible("Post Distrib Tempo Inf All Chains", mShowAllChains);
             mGraph->setCurveVisible("Post Distrib Tempo Sup All Chains", mShowAllChains);

            /* for (int i=0; i<mShowChainList.size(); ++i)
                 mGraph->setCurveVisible("Post Distrib Duration " + QString::number(i), mShowChainList.at(i));
            */
             mGraph->setTipXLab("t");
             mGraph->setTipYLab("n");
             mGraph->setYAxisMode(GraphView::eMinMax);
             //mGraph->autoAdjustYScale(true);
             mGraph->adjustYToMinMaxValue();
         }

    }
     else if (mCurrentTypeGraph == ePostDistrib && mCurrentVariable == eIntensity) {

          const GraphCurve* intensity = mGraph->getCurve("Post Distrib Intensity All Chains");

          if ( intensity && !intensity->mData.isEmpty()) {

              mGraph->setCurveVisible("Post Distrib Intensity All Chains", mShowAllChains);

             /* for (int i=0; i<mShowChainList.size(); ++i)
                  mGraph->setCurveVisible("Post Distrib Duration " + QString::number(i), mShowChainList.at(i));
             */
              mGraph->setTipXLab("t");
              mGraph->setTipYLab("");
              mGraph->setYAxisMode(GraphView::eHidden);
              mGraph->autoAdjustYScale(true);
          }

     }
    /* ---------------- second tab : history plot--------------------------------
     *  - Alpha Trace i
     *  - Alpha Q1 i
     *  - Alpha Q2 i
     *  - Alpha Q3 i
     *  - Beta Trace i
     *  - Beta Q1 i
     *  - Beta Q2 i
     *  - Beta Q3 i
     * ------------------------------------------------ */
    else if (mCurrentTypeGraph == eTrace && mCurrentVariable == eDuration) {

        for (int i=0; i<mShowChainList.size(); ++i) {
            mGraph->setCurveVisible("Duration Trace " + QString::number(i), mShowChainList.at(i));
            mGraph->setCurveVisible("Duration Q1 " + QString::number(i), mShowChainList.at(i));
            mGraph->setCurveVisible("Duration Q2 " + QString::number(i), mShowChainList.at(i));
            mGraph->setCurveVisible("Duration Q3 " + QString::number(i), mShowChainList.at(i));
        }

        mGraph->setTipXLab("iteration");
        mGraph->setTipYLab("t");
        mGraph->setYAxisMode(GraphView::eMinMax);
        mGraph->autoAdjustYScale(true);
    }
    repaint();
}

