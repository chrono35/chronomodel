/* ---------------------------------------------------------------------

Copyright or © or Copr. CNRS	2014 - 2020

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

#include "PluginUniformRefView.h"
#if USE_PLUGIN_UNIFORM

#include "PluginUniform.h"
#include "GraphView.h"
#include "Painting.h"
#include "StdUtilities.h"

#include <QtWidgets>

PluginUniformRefView::PluginUniformRefView(QWidget* parent):GraphViewRefAbstract(parent)
{
    mMeasureColor = QColor(56, 120, 50);
    mGraph = new GraphView(this);
    mGraph->setXAxisMode(GraphView::eAllTicks);
    mGraph->setYAxisMode(GraphView::eAllTicks);
    //mGraph->setRendering(GraphView::eHD);
    mGraph->setTipXLab("t");
    mGraph->setTipYLab("age");
    mGraph->autoAdjustYScale(true);

}

PluginUniformRefView::~PluginUniformRefView()
{

}

void PluginUniformRefView::setDate(const Date& date, const ProjectSettings& settings)
{
    Q_ASSERT(&date);
    Q_ASSERT(&settings);
    GraphViewRefAbstract::setDate(date, settings);
    
    
    if (date.mSubDates.isEmpty()) { // not a combination

        double tminDisplay;
        double tmaxDisplay;

        const double t1 = DateUtils::convertToAppSettingsFormat(mTminDisplay);
        const double t2 = DateUtils::convertToAppSettingsFormat(mTmaxDisplay);

        if (!date.isNull() && date.mIsValid) {
            const double t3 = date.getFormatedTminCalib();
            const double t4 = date.getFormatedTmaxCalib();

            tminDisplay = qMin(t1,qMin(t2,t3));
            tmaxDisplay = qMax(t1,qMax(t2,t4));
        } else {
            tminDisplay = qMin(t1, t2);
            tmaxDisplay = qMax(t1, t2);
        }

        mGraph->setRangeX(tminDisplay, tmaxDisplay);
        mGraph->setCurrentX(tminDisplay, tmaxDisplay);

        mGraph->removeAllCurves();
        mGraph->removeAllZones();
        mGraph->clearInfos();
        mGraph->showInfos(true);
        mGraph->setFormatFunctX(nullptr);

       
             
                /* ----------------------------------------------
                 *  Reference curve
                 * ---------------------------------------------- */
                GraphCurve curve;
                curve.mName = "Reference";
                curve.mPen.setColor(Painting::mainColorDark);
                curve.mIsHisto = false;
                QMap<double, double> refCurve;


                QString toFind = date.mName + date.getDesc();
                            qDebug()<<"mName"<<date.mName<<toFind;
                            
                            //Project* project = MainWindow::getInstance()->getProject();
                            //QMap<QString, CalibrationCurve>::iterator it = project->mCalibCurves.find (toFind);
                            //if ( it != project->mCalibCurves.end())
                               // date->mCalibration = & it.value();
                            
                            GraphCurve gCurve;
                            gCurve.mName = date.mName;
                
                            
                            QColor curveColor(randomColor());
                            gCurve.mPen.setColor(curveColor);
                            
                            curveColor.setAlpha(50);
                            gCurve.mBrush = curveColor;
                
                            gCurve.mIsVertical = false;
                            gCurve.mIsHisto = false;
                            gCurve.mIsRectFromZero = true;
                            
                            QMap<double, double> calib = normalize_map(getMapDataInRange(date.getRawCalibMap(), tminDisplay, tmaxDisplay));
                       
                            gCurve.mData = calib;
                            
                            mGraph->addCurve(gCurve);
                            mGraph->setRangeX(tminDisplay, tmaxDisplay);
                            mGraph->setCurrentX(tminDisplay, tmaxDisplay);


            // Y scale and RangeY are define in graphView::zommX()

    } else {
        
        
        double tminDisplay;
        double tmaxDisplay;

        const double t1 = DateUtils::convertToAppSettingsFormat(mTminDisplay);
        const double t2 = DateUtils::convertToAppSettingsFormat(mTmaxDisplay);

        for ( auto &&d : date.mSubDates ) {
            QJsonObject subDate = d.toObject();
            Date sd;
            sd.fromJson(subDate);
            QString toFind = sd.mUUID;
            
            Project* project = MainWindow::getInstance()->getProject();
            QMap<QString, CalibrationCurve>::iterator it = project->mCalibCurves.find (toFind);
            if ( it != project->mCalibCurves.end())
                sd.mCalibration = & it.value();
            
            
            if (!date.isNull() && date.mIsValid) {
                const double t3 = sd.getFormatedTminCalib();
                const double t4 = sd.getFormatedTmaxCalib();
    
                tminDisplay = qMin(t1, qMin(t2,t3));
                tmaxDisplay = qMax(t1, qMax(t2,t4));
            } else {
                tminDisplay = qMin(t1, t2);
                tmaxDisplay = qMax(t1, t2);
            }
        }
        GraphViewRefAbstract::drawSubDates(date.mSubDates, settings, tminDisplay, tmaxDisplay);
    }
    
}

void PluginUniformRefView::zoomX(const double min, const double max)
{
    mGraph->zoomX(min, max);
}
void PluginUniformRefView::setMarginRight(const int margin)
{
    mGraph->setMarginRight(margin);
}
void PluginUniformRefView::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);
    mGraph->setGeometry(0, 0, width(), height());
}

#endif
