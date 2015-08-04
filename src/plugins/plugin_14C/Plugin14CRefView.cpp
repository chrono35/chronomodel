#include "Plugin14CRefView.h"
#if USE_PLUGIN_14C

#include "Plugin14C.h"
#include "GraphView.h"
#include "StdUtilities.h"
#include <QtWidgets>


Plugin14CRefView::Plugin14CRefView(QWidget* parent):GraphViewRefAbstract(parent),
mGraph(0)
{
    mGraph = new GraphView(this);
    
    mGraph->setXAxisMode(GraphView::eAllTicks);
    mGraph->setYAxisMode(GraphView::eAllTicks);
    mGraph->setRendering(GraphView::eHD);
    mGraph->autoAdjustYScale(true);
}

Plugin14CRefView::~Plugin14CRefView()
{
    
}

void Plugin14CRefView::setDate(const Date& d, const ProjectSettings& settings)
{
    GraphViewRefAbstract::setDate(d, settings);
    Date date = d;
    
    mGraph->removeAllCurves();
    mGraph->clearInfos();
    mGraph->showInfos(true);
    mGraph->setRangeX(mSettings.mTmin, mSettings.mTmax);
    mGraph->setCurrentX(mSettings.mTmin, mSettings.mTmax);
    
    if(!date.isNull())
    {
        double age = date.mData.value(DATE_14C_AGE_STR).toDouble();
        double error = date.mData.value(DATE_14C_ERROR_STR).toDouble();
        double delta_r = date.mData.value(DATE_14C_DELTA_R_STR).toDouble();
        double delta_r_error = date.mData.value(DATE_14C_DELTA_R_ERROR_STR).toDouble();
        QString ref_curve = date.mData.value(DATE_14C_REF_CURVE_STR).toString().toLower();
        
        // ----------------------------------------------
        //  Reference curve
        // ----------------------------------------------
        
        QColor color2(150, 150, 150);
        
        Plugin14C* plugin = (Plugin14C*)date.mPlugin;

        const QMap<QString, QMap<double, double> >& curves = plugin->getRefData(ref_curve);
        
        
        QMap<double, double> curveG;
        QMap<double, double> curveG95Sup;
        QMap<double, double> curveG95Inf;
        
        //qDebug() << curves["G"][0];
        double yMin = curves["G95Inf"][mSettings.mTmin];
        double yMax = curves["G95Sup"][mSettings.mTmin];
        
        for(double t=mSettings.mTmin; t<=mSettings.mTmax; ++t) { //t+=mSettings.mStep) {
            curveG[t] = curves["G"][t];
            curveG95Sup[t] = curves["G95Sup"][t];
            curveG95Inf[t] = curves["G95Inf"][t];
            
            yMin = qMin(yMin, curveG95Inf[t]);
            yMax = qMax(yMax, curveG95Sup[t]);
        }
        
        GraphCurve graphCurveG;
        graphCurveG.mName = "G";
        graphCurveG.mData = curveG;
        graphCurveG.mPen.setColor(Qt::blue);
        graphCurveG.mIsHisto = false;
        mGraph->addCurve(graphCurveG);
        
        GraphCurve graphCurveG95Sup;
        graphCurveG95Sup.mName = "G95Sup";
        graphCurveG95Sup.mData = curveG95Sup;
        graphCurveG95Sup.mPen.setColor(QColor(180, 180, 180));
        graphCurveG95Sup.mIsHisto = false;
        mGraph->addCurve(graphCurveG95Sup);
        
        GraphCurve graphCurveG95Inf;
        graphCurveG95Inf.mName = "G95Inf";
        graphCurveG95Inf.mData = curveG95Inf;
        graphCurveG95Inf.mPen.setColor(QColor(180, 180, 180));
        graphCurveG95Inf.mIsHisto = false;
        mGraph->addCurve(graphCurveG95Inf);
        
        // Display reference curve name
        mGraph->addInfo(tr("Ref : ") + ref_curve);
        
        // ----------------------------------------------
        
       // double yMin = map_min_value(curveG95Inf);
       // double yMax = map_max_value(curveG95Sup);
        
        yMin = qMin(yMin, age);
        yMin = floor(yMin/10)*10;
        
        yMax = qMax(yMax, age);
        yMax = ceil(yMax/10)*10;
        
        mGraph->setRangeY(yMin, yMax);
       // qDebug()<<"Plugin14CRefView::setDate yMin"<<yMin;
       // qDebug()<<"Plugin14CRefView::setDate yMax"<<yMax;
        
        // ----------------------------------------------
        //  Measure curve
        // ----------------------------------------------
        
        GraphCurve curveMeasure;
        curveMeasure.mName = "Measure";
        
        QColor penColor(mMeasureColor);
        QColor brushColor(mMeasureColor);
        
        // Lower opacity in case of delta r not null
        if(delta_r != 0 && delta_r_error != 0){
            penColor.setAlpha(100);
            brushColor.setAlpha(15);
        }else{
            penColor.setAlpha(255);
            brushColor.setAlpha(50);
        }
        curveMeasure.mPen.setColor(penColor);
        curveMeasure.mBrush.setColor(brushColor);
        
        curveMeasure.mFillUnder = true;
        curveMeasure.mIsVertical = true;
        curveMeasure.mIsHisto = false;
        
        // 5000 pts are used on vertical measure
        // because the y scale auto adjusts depending on x zoom.
        // => the visible part of the measure may be very reduced !
        double step = (yMax - yMin) / 5000.;
        for(double t=yMin; t<yMax; t += step)
        {
            double v = exp(-0.5 * pow((age - t) / error, 2));
            curveMeasure.mData[t] = v;
        }
        curveMeasure.mData = normalize_map(curveMeasure.mData);
        mGraph->addCurve(curveMeasure);
        
        // Infos to write :
        QString info = tr("Age BP : ") + QString::number(age) + " ± " + QString::number(error);
        
        // ----------------------------------------------
        //  Delta R curve
        // ----------------------------------------------
        if(delta_r != 0 && delta_r_error != 0)
        {
            // Apply reservoir effect
            age = (age - delta_r);
            error = sqrt(error * error + delta_r_error * delta_r_error);
            
            GraphCurve curveDeltaR;
            curveDeltaR.mName = "Delta R";
            
            penColor = mMeasureColor;
            brushColor = mMeasureColor;
            brushColor.setAlpha(50);
            
            curveDeltaR.mPen.setColor(penColor);
            curveDeltaR.mBrush.setColor(brushColor);
            curveDeltaR.mFillUnder = true;
            curveDeltaR.mIsVertical = true;
            curveDeltaR.mIsHisto = false;
            
            // 5000 pts are used on vertical measure
            // because the y scale auto adjusts depending on x zoom.
            // => the visible part of the measure may be very reduced !
            step = (yMax - yMin) / 5000.;
            for(double t=yMin; t<yMax; t += step)
            {
                double v = exp(-0.5 * pow((age - t) / error, 2));
                curveDeltaR.mData[t] = v;
            }
            curveDeltaR.mData = normalize_map(curveDeltaR.mData);
            mGraph->addCurve(curveDeltaR);
            
            info += tr(", ΔR : ") + QString::number(delta_r) + " ± " + QString::number(delta_r_error);
        }

        // ----------------------------------------------
        //  Textual info
        // ----------------------------------------------
        
        mGraph->addInfo(info);
        
        // ----------------------------------------------
        //  Error on measure (horizontal lines)
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
    }
}

void Plugin14CRefView::zoomX(double min, double max)
{
    mGraph->zoomX(min, max);
}

void Plugin14CRefView::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);
    mGraph->setGeometry(rect());
}

#endif
