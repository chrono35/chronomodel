#include "PluginTLRefView.h"
#if USE_PLUGIN_TL

#include "PluginTL.h"
#include "GraphView.h"
#include "Painting.h"
#include "StdUtilities.h"
#include <QtWidgets>


PluginTLRefView::PluginTLRefView(QWidget* parent):GraphViewRefAbstract(parent)
{
    mMeasureColor = QColor(56, 120, 50);

    mGraph = new GraphView(this);
    mGraph->setXAxisMode(GraphView::eAllTicks);
    mGraph->setYAxisMode(GraphView::eAllTicks);
    mGraph->setRendering(GraphView::eHD);
    mGraph->setTipXLab("t");
    mGraph->setTipYLab("age");
    mGraph->autoAdjustYScale(true);
    mGraph->setMarginBottom(mGraph->font().pointSizeF() + 10. );

}

PluginTLRefView::~PluginTLRefView()
{
    
}

void PluginTLRefView::setDate(const Date& date, const ProjectSettings& settings)
{
    GraphViewRefAbstract::setDate(date, settings);

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
    mGraph->setFormatFunctX(0);
    
    if (!date.isNull()) {
        double age = date.mData.value(DATE_TL_AGE_STR).toDouble();
        double error = date.mData.value(DATE_TL_ERROR_STR).toDouble();
        double ref_year = date.mData.value(DATE_TL_REF_YEAR_STR).toDouble();
        
        // ----------------------------------------------
        //  Reference curve
        // ----------------------------------------------
        GraphCurve curve;
        curve.mName = "Reference";
        curve.mPen.setColor(Painting::mainColorDark);
        curve.mIsHisto = false;
        QMap<double, double> refCurve;

        refCurve[tminDisplay] = ref_year - DateUtils::convertFromAppSettingsFormat(tminDisplay);
        refCurve[tmaxDisplay] = ref_year - DateUtils::convertFromAppSettingsFormat(tmaxDisplay);
        curve.mData =refCurve;
        mGraph->addCurve(curve);
        
        // ----------------------------------------------
        
        double yMin ( map_min_value(curve.mData) );
        double yMax ( map_max_value(curve.mData) );

        yMin = qMin(yMin, age - error * 1.96);
        yMax = qMax(yMax, age + error * 1.96);
        
         // Y scale and RangeY are define in graphView::zommX()

        
        // ----------------------------------------------
        //  Measure curve
        // ----------------------------------------------
        
        GraphCurve curveMeasure;
        curveMeasure.mName = "Measure";
        
        curveMeasure.mPen.setColor(mMeasureColor);
        QColor curveColor(mMeasureColor);
        curveColor.setAlpha(50);
        curveMeasure.mBrush = curveColor;
        
        curveMeasure.mIsVertical = true;
        curveMeasure.mIsHisto = false;
        
        // 5000 pts are used on vertical measure
        // because the y scale auto adjusts depending on x zoom.
        // => the visible part of the measure may be very reduced !
        QMap<double, double> measureCurve;
        const double step = (yMax - yMin) / 5000.;

        for (double t=yMin; t<yMax; t += step) {
            const double v = exp(-0.5 * pow((t - age) / error, 2));
            measureCurve[t] = v;
        }
        measureCurve = normalize_map(measureCurve);
        //curveMeasure.mData = normalize_map(curveMeasure.mData);
        curveMeasure.mData = measureCurve;
        mGraph->addCurve(curveMeasure);
        
        // ----------------------------------------------
        //  Error on measure
        // ----------------------------------------------
        
        GraphCurve curveMeasureAvg;
        curveMeasureAvg.mName = "MeasureAvg";
        curveMeasureAvg.mPen.setColor(mMeasureColor);
        curveMeasureAvg.mPen.setStyle(Qt::SolidLine);
        curveMeasureAvg.mIsHorizontalLine = true;
        
        GraphCurve curveMeasureSup;
        curveMeasureSup.mName = "MeasureSup";
        curveMeasureSup.mPen.setColor(mMeasureColor);
        curveMeasureSup.mPen.setStyle(Qt::DashLine);
        curveMeasureSup.mIsHorizontalLine = true;
        
        GraphCurve curveMeasureInf;
        curveMeasureInf.mName = "MeasureInf";
        curveMeasureInf.mPen.setColor(mMeasureColor);
        curveMeasureInf.mPen.setStyle(Qt::DashLine);
        curveMeasureInf.mIsHorizontalLine = true;
        
        curveMeasureAvg.mHorizontalValue = age;
        curveMeasureSup.mHorizontalValue = age + error;
        curveMeasureInf.mHorizontalValue = age - error;
        
        mGraph->addCurve(curveMeasureAvg);
        mGraph->addCurve(curveMeasureSup);
        mGraph->addCurve(curveMeasureInf);

        // Y scale and RangeY are define in graphView::zommX()

    }
}

void PluginTLRefView::zoomX(const double min, const double max)
{
    mGraph->zoomX(min, max);
}
void PluginTLRefView::setMarginRight(const int margin)
{
    mGraph->setMarginRight(margin);
}
void PluginTLRefView::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);
    mGraph->setGeometry(0, 0, width(), height());
}

#endif

