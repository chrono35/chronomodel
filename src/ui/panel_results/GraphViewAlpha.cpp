/* ---------------------------------------------------------------------

Copyright or © or Copr. CNRS	2014 - 2018

Authors :
	Philippe LANOS
	Helori LANOS
 	Philippe DUFRESNE

This software is a computer program whose purpose is to
create chronological models of archeological data using Bayesian statistics.

This software is governed by the CeCILL V2.1 license under French law and
abiding by the rules of distribution of free software.  You can  use,
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability.

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or
data to be ensured and,  more generally, to use and operate it in the
same conditions as regards security.

The fact that you are presently reading this means that you have had
knowledge of the CeCILL V2.1 license and that you accept its terms.
--------------------------------------------------------------------- */

#include "GraphViewAlpha.h"
#include "GraphView.h"
#include "ModelChronocurve.h"
#include "Painting.h"
#include "StdUtilities.h"
#include "QtUtilities.h"
#include "DateUtils.h"
#include "ModelUtilities.h"
#include "MainWindow.h"
#include <QtWidgets>

// Constructor / Destructor

GraphViewAlpha::GraphViewAlpha(QWidget *parent):GraphViewResults(parent),
mModel(nullptr)
{
    setMainColor(Painting::borderDark);
    mGraph->setBackgroundColor(QColor(210, 210, 210));

}

GraphViewAlpha::~GraphViewAlpha()
{
    mModel = nullptr;
}


void GraphViewAlpha::setModel(ModelChronocurve* model)
{
    Q_ASSERT(model);
    mModel = model;
}


void GraphViewAlpha::paintEvent(QPaintEvent* e)
{
    GraphViewResults::paintEvent(e);
}

void GraphViewAlpha::resizeEvent(QResizeEvent* )
{
    updateLayout();
}

void GraphViewAlpha::generateCurves(TypeGraph typeGraph, Variable variable)
{
    Q_ASSERT(mModel);

    GraphViewResults::generateCurves(typeGraph, variable);
    
    mGraph->removeAllCurves();
    mGraph->removeAllZones();
    mGraph->clearInfos();
    mGraph->resetNothingMessage();
    mGraph->setOverArrow(GraphView::eNone);
    mGraph->reserveCurves(6);

    QPen defaultPen;
    defaultPen.setWidthF(1);
    defaultPen.setStyle(Qt::SolidLine);

    QColor color = Qt::blue;
    //QString resultsText = ModelUtilities::eventResultsText(mEvent, false);
    //QString resultsHTML = ModelUtilities::eventResultsHTML(mEvent, false);
    //setNumericalResults(resultsHTML, resultsText);
    
    // ------------------------------------------------
    //  First tab : Posterior distrib
    // ------------------------------------------------
    if(typeGraph == ePostDistrib)
    {
        mGraph->mLegendX = DateUtils::getAppSettingsFormatStr();
        mGraph->setFormatFunctX(nullptr);//DateUtils::convertToAppSettingsFormat);
        mGraph->setFormatFunctY(nullptr);
        mGraph->setBackgroundColor(QColor(230, 230, 230));
        mGraph->setOverArrow(GraphView::eBothOverflow);
        
        mTitle = tr("Alpha Lissage");

        // ------------------------------------
        //  Post distrib All Chains
        // ------------------------------------
        //GraphCurve curvePostDistrib = generateDensityCurve(mModel->mAlphaLissage.fullHisto(), "Post Distrib All Chains", color);
        
        GraphCurve curvePostDistrib;
        curvePostDistrib.mName = "Post Distrib All Chains";
        curvePostDistrib.mData = mModel->mAlphaLissage.fullHisto();
        curvePostDistrib.mPen = QPen(color, 1, Qt::SolidLine);
        curvePostDistrib.mBrush = Qt::NoBrush;
        curvePostDistrib.mIsHisto = false;
        //curvePostDistrib.mIsRectFromZero = true; // for Unif-typo. calibs., invisible for others!
        
        mGraph->addCurve(curvePostDistrib);
        
        //qDebug() << mModel->mAlphaLissage.fullHisto();
        //qDebug() << curvePostDistrib.mData;

        // ------------------------------------
        //  HPD All Chains
        // ------------------------------------
        GraphCurve curveHPD = generateHPDCurve(mModel->mAlphaLissage.mHPD, "HPD All Chains", color);
        mGraph->addCurve(curveHPD);

        // ------------------------------------
        //  Post Distrib Chain i
        // ------------------------------------
        if(!mModel->mAlphaLissage.mChainsHistos.isEmpty())
        {
            for (int i=0; i<mChains.size(); ++i)
            {
                GraphCurve curvePostDistribChain = generateDensityCurve(mModel->mAlphaLissage.histoForChain(i),
                                                                        "Post Distrib Chain " + QString::number(i),
                                                                        Painting::chainColors.at(i),
                                                                        Qt::SolidLine,
                                                                        Qt::NoBrush);
                mGraph->addCurve(curvePostDistribChain);
            }
        }
        
        // ------------------------------------
        //  Theta Credibility
        // ------------------------------------
        GraphCurve curveCred = generateSectionCurve(mModel->mAlphaLissage.mCredibility,
                                                    "Credibility All Chains",
                                                    color);
        mGraph->addCurve(curveCred);
    }
    // -------------------------------------------------
    //  History plots
    // -------------------------------------------------
    else if(typeGraph == eTrace)
    {
        mGraph->mLegendX = "Iterations";
        mGraph->setFormatFunctX(nullptr);
        mTitle = tr("Alpha lissage trace");

        generateTraceCurves(mChains, &(mModel->mAlphaLissage));
    }
    // -------------------------------------------------
    //  Acceptance rate
    // -------------------------------------------------
    else if (typeGraph == eAccept)
    {
        mGraph->mLegendX = "Iterations";
        mGraph->setFormatFunctX(nullptr);
        mTitle = tr("Alpha lissage acceptation");

        mGraph->addCurve(generateHorizontalLine(44, "Accept Target", QColor(180, 10, 20), Qt::DashLine));

        generateAcceptCurves(mChains, &(mModel->mAlphaLissage));
        mGraph->repaint();
    }

    // -------------------------------------------------
    //  Autocorrelation
    // -------------------------------------------------
    else if (typeGraph == eCorrel)
    {
        mGraph->mLegendX = "";
        mGraph->setFormatFunctX(nullptr);
        mTitle = tr("Alpha lissage autocorrelation");

        generateCorrelCurves(mChains, &(mModel->mAlphaLissage));
        mGraph->setXScaleDivision(10, 10);
    }
    else
    {
        mTitle = tr("Alpha lissage");
        mGraph->resetNothingMessage();
    }
}

void GraphViewAlpha::updateCurvesToShow(bool showAllChains, const QList<bool>& showChainList, bool showCredibility, bool showError, bool showWiggle)
{
    Q_ASSERT(mModel);

    GraphViewResults::updateCurvesToShow(showAllChains, showChainList, showCredibility, showError, showWiggle);

    if (mCurrentTypeGraph == ePostDistrib)
    {
        mGraph->setTipYLab("");
        
        mGraph->setCurveVisible("Post Distrib All Chains", mShowAllChains);
        mGraph->setCurveVisible("HPD All Chains", mShowAllChains);
        mGraph->setCurveVisible("Credibility All Chains", mShowCredibility && mShowAllChains);

        for (int i=0; i<mShowChainList.size(); ++i){
            mGraph->setCurveVisible("Post Distrib Chain " + QString::number(i), mShowChainList.at(i));
        }
        
        mGraph->setTipXLab("t");
        mGraph->setYAxisMode(GraphView::eHidden);
        mGraph->showInfos(false);
        mGraph->clearInfos();
    }

    else if (mCurrentTypeGraph == eTrace)
    {
        for (int i=0; i<mShowChainList.size(); ++i)
        {
            mGraph->setCurveVisible("Trace " + QString::number(i), mShowChainList.at(i));
            mGraph->setCurveVisible("Q1 " + QString::number(i), mShowChainList.at(i));
            mGraph->setCurveVisible("Q2 " + QString::number(i), mShowChainList.at(i));
            mGraph->setCurveVisible("Q3 " + QString::number(i), mShowChainList.at(i));
        }

        mGraph->setTipXLab(tr("Iteration"));
        mGraph->setTipYLab("t");

        mGraph->setYAxisMode(GraphView::eMinMaxHidden);
        mGraph->showInfos(true);
        mGraph->autoAdjustYScale(true);
    }

      /* ----------------------Third tab : Acceptance rate--------------------------
       *  Possible curves :
       *  - Accept i
       *  - Accept Target
       * ------------------------------------------------  */
      /*else if ((mCurrentTypeGraph == eAccept) && (mCurrentVariable == eTheta) && ((mEvent->mMethod == Event::eMHAdaptGauss) || (mEvent->mMethod == Event::eFixe))) {
          mGraph->setCurveVisible("Accept Target", true);
          for (int i=0; i<mShowChainList.size(); ++i)
              mGraph->setCurveVisible("Accept " + QString::number(i), mShowChainList.at(i));

          mGraph->setTipXLab(tr("Iteration"));
          mGraph->setTipYLab(tr("Rate"));

          mGraph->setYAxisMode(GraphView::eMinMax);
          mGraph->showInfos(false);
          mGraph->clearInfos();
          mGraph->autoAdjustYScale(false); // do  repaintGraph()
          mGraph->setRangeY(0, 100);
      }*/
      /* ----------------------fourth tab : Autocorrelation--------------------------
       *  Possible curves :
       *  - Correl i
       *  - Correl Limit Lower i
       *  - Correl Limit Upper i
       * ------------------------------------------------   */
      else if (mCurrentTypeGraph == eCorrel && mCurrentVariable == eTheta) {
          for (int i=0; i<mShowChainList.size(); ++i) {
              mGraph->setCurveVisible("Correl " + QString::number(i), mShowChainList.at(i));
              mGraph->setCurveVisible("Correl Limit Lower " + QString::number(i), mShowChainList.at(i));
              mGraph->setCurveVisible("Correl Limit Upper " + QString::number(i), mShowChainList.at(i));
          }
          mGraph->setTipXLab("h");
          mGraph->setTipYLab(tr("Value"));
          mGraph->setYAxisMode(GraphView::eMinMax);
          mGraph->showInfos(false);
          mGraph->clearInfos();
          mGraph->autoAdjustYScale(false); // do  repaintGraph()
          mGraph->setRangeY(-1, 1);
      }

      update();
}
