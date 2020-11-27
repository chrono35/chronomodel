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

#include "GraphViewCurve.h"
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

GraphViewCurve::GraphViewCurve(QWidget *parent):GraphViewResults(parent)
{
    setMainColor(Painting::borderDark);
    mGraph->setBackgroundColor(QColor(210, 210, 210));
}

GraphViewCurve::~GraphViewCurve()
{
    
}

void GraphViewCurve::setComposanteG(const PosteriorMeanGComposante& composante)
{
    mComposanteG = composante;
}

void GraphViewCurve::setComposanteGChains(const QList<PosteriorMeanGComposante>& composanteChains)
{
    mComposanteGChains = composanteChains;
}

void GraphViewCurve::paintEvent(QPaintEvent* e)
{
    GraphViewResults::paintEvent(e);
}

void GraphViewCurve::resizeEvent(QResizeEvent* )
{
    updateLayout();
}

void GraphViewCurve::generateCurves(TypeGraph typeGraph, Variable variable)
{
    GraphViewResults::generateCurves(typeGraph, variable);
    
    mGraph->removeAllCurves();
    mGraph->removeAllZones();
    mGraph->clearInfos();
    mGraph->resetNothingMessage();
    mGraph->setOverArrow(GraphView::eNone);
    mGraph->setTipXLab("t");
    mGraph->setYAxisMode(GraphView::eMinMax);
    mGraph->autoAdjustYScale(true);
    mGraph->mLegendX = DateUtils::getAppSettingsFormatStr();
    mGraph->setFormatFunctX(nullptr);
    mGraph->setBackgroundColor(QColor(230, 230, 230));
    //mGraph->reserveCurves(5);
    
    GraphCurve curveG;
    curveG.mName = tr("G");
    curveG.mPen = QPen(QColor(119, 95, 49), 1, Qt::SolidLine);
    curveG.mBrush = Qt::NoBrush;
    curveG.mIsHisto = false;
    curveG.mIsRectFromZero = false;
    
    GraphCurve curveGSup;
    curveGSup.mName = tr("G Sup");
    curveGSup.mPen = QPen(QColor(100, 100, 100), 1, Qt::SolidLine);
    curveGSup.mBrush = Qt::NoBrush;
    curveGSup.mIsHisto = false;
    curveGSup.mIsRectFromZero = false;
    
    GraphCurve curveGInf;
    curveGInf.mName = tr("G Inf");
    curveGInf.mPen = QPen(QColor(100, 100, 100), 1, Qt::SolidLine);
    curveGInf.mBrush = Qt::NoBrush;
    curveGInf.mIsHisto = false;
    curveGInf.mIsRectFromZero = false;
    
    GraphCurve curveGP;
    curveGP.mName = tr("G Prime");
    curveGP.mPen = QPen(QColor(154, 80, 225), 1, Qt::SolidLine);
    curveGP.mBrush = Qt::NoBrush;
    curveGP.mIsHisto = false;
    curveGP.mIsRectFromZero = false;
    
    GraphCurve curveGS;
    curveGS.mName = tr("G Second");
    curveGS.mPen = QPen(QColor(236, 105, 64), 1, Qt::SolidLine);
    curveGS.mBrush = Qt::NoBrush;
    curveGS.mIsHisto = false;
    curveGS.mIsRectFromZero = false;
    
    QList<GraphCurve> curveGChains;
    for(int i=0; i<mComposanteGChains.size(); ++i)
    {
        GraphCurve curveGChain;
        curveGChain.mName = QString("G Chain ") + QString::number(i);
        curveGChain.mPen = QPen(Painting::chainColors[i], 1, Qt::SolidLine);
        curveGChain.mBrush = Qt::NoBrush;
        curveGChain.mIsHisto = false;
        curveGChain.mIsRectFromZero = false;
        curveGChains.append(curveGChain);
    }
    
    for(int t=mSettings.mTmin; t<=mSettings.mTmax; ++t)
    {
        int idx = t - mSettings.mTmin;
        
        curveG.mData.insert(t, mComposanteG.vecG[idx]);
        curveGSup.mData.insert(t, mComposanteG.vecG[idx] + mComposanteG.vecGErr[idx]);
        curveGInf.mData.insert(t, mComposanteG.vecG[idx] - mComposanteG.vecGErr[idx]);
        curveGP.mData.insert(t, mComposanteG.vecGP[idx]);
        curveGS.mData.insert(t, mComposanteG.vecGS[idx]);
        
        for(int i=0; i<curveGChains.size(); ++i)
        {
            curveGChains[i].mData.insert(t, mComposanteGChains[i].vecG[idx]);
        }
    }
    
    mGraph->addCurve(curveG);
    mGraph->addCurve(curveGSup);
    mGraph->addCurve(curveGInf);
    mGraph->addCurve(curveGP);
    mGraph->addCurve(curveGS);
    
    for(int i=0; i<curveGChains.size(); ++i)
    {
        mGraph->addCurve(curveGChains[i]);
    }
}

void GraphViewCurve::updateCurvesToShowForG(bool showAllChains, QList<bool> showChainList, bool showG, bool showGError, bool showGP, bool showGS)
{
    // From GraphViewResults::updateCurvesToShow
    mShowAllChains = showAllChains;
    mShowChainList = showChainList;
    
    mShowG = showG;
    mShowGError = showGError;
    mShowGP = showGP;
    mShowGS = showGS;
    
    mGraph->setCurveVisible("G", mShowAllChains && mShowG);
    mGraph->setCurveVisible("G Sup", mShowAllChains && mShowGError);
    mGraph->setCurveVisible("G Inf", mShowAllChains && mShowGError);
    mGraph->setCurveVisible("G Prime", mShowAllChains && mShowGP);
    mGraph->setCurveVisible("G Second", mShowAllChains && mShowGS);
    
    for(int i=0; i<mShowChainList.size(); ++i)
    {
        mGraph->setCurveVisible(QString("G Chain ") + QString::number(i), mShowChainList[i] && mShowG);
    }
}
