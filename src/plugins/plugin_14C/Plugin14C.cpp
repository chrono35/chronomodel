#include "Plugin14C.h"
#if USE_PLUGIN_14C

#include "QtUtilities.h"
#include "StdUtilities.h"
#include "Plugin14CForm.h"
#include "Plugin14CRefView.h"
#include "Plugin14CSettingsView.h"
#include <cstdlib>
#include <iostream>
#include <QJsonObject>
#include <QtWidgets>


Plugin14C::Plugin14C()
{
    mColor = QColor(47,46,68);
    loadRefDatas();
}

double Plugin14C::getRefValueAt(const QJsonObject& data, const double& t)
{
    QString ref_curve = data[DATE_14C_REF_CURVE_STR].toString();
    double g = 0;
    if(mRefDatas.find(ref_curve) != mRefDatas.end())
    {
        const QMap<double, double>& curve = mRefDatas[ref_curve]["G"];
        
        double tMinDef = curve.firstKey();
        double tMaxDef = curve.lastKey();
        
        if(t >= tMaxDef){
            g = interpolate(t, tMinDef, tMaxDef, curve[tMinDef], curve[tMaxDef]);
        }
        else if(t <= tMinDef){
            g = interpolate(t, tMinDef, tMaxDef, curve[tMinDef], curve[tMaxDef]);
        }
        else
        {
            double t_under = floor(t);
            double t_upper = t_under + 1;
            double g_under = curve[t_under];
            double g_upper = curve[t_upper];
            g = interpolate(t, t_under, t_upper, g_under, g_upper);
        }
    }
    return g;
}

double Plugin14C::getRefErrorAt(const QJsonObject& data, const double& t)
{
    QString ref_curve = data[DATE_14C_REF_CURVE_STR].toString();
    
    double error = 0;
    
    if(mRefDatas.find(ref_curve) != mRefDatas.end())
    {
        const QMap<double, double>& curve = mRefDatas[ref_curve]["G"];
        const QMap<double, double>& curveG95Sup = mRefDatas[ref_curve]["G95Sup"];
        
        double tMinDef = curve.firstKey();
        double tMaxDef = curve.lastKey();
        
        if(t >= tMaxDef){
            error = (curveG95Sup[tMaxDef] - curve[tMaxDef]) / 1.96f;
        }
        else if(t <= tMinDef){
            error = (curveG95Sup[tMinDef] - curve[tMinDef]) / 1.96f;
        }
        else
        {
            double t_under = floor(t);
            double t_upper = t_under + 1;
            double g_under = curve[t_under];
            double g_upper = curve[t_upper];
            double g = interpolate(t, t_under, t_upper, g_under, g_upper);
            
            double g_sup_under = curveG95Sup[t_under];
            double g_sup_upper = curveG95Sup[t_upper];
            double g_sup = interpolate(t, t_under, t_upper, g_sup_under, g_sup_upper);
            
            error = (g_sup - g) / 1.96f;
        }
    }
    return error;
}

QPair<double, double> Plugin14C::getLikelyhoodArg(const double& t, const QJsonObject& data)
{
    double age = data[DATE_14C_AGE_STR].toDouble();
    double error = data[DATE_14C_ERROR_STR].toDouble();
    double delta_r = data[DATE_14C_DELTA_R_STR].toDouble();
    double delta_r_error = data[DATE_14C_DELTA_R_ERROR_STR].toDouble();
    
    // Apply reservoir effect
    age = (age - delta_r);
    error = sqrt(error * error + delta_r_error * delta_r_error);
    
    double refValue = getRefValueAt(data, t);
    double refError = getRefErrorAt(data, t);
    
    double variance = refError * refError + error * error;
    double exponent = -0.5f * pow((age - refValue), 2.f) / variance;
    
    return qMakePair(variance, exponent);
}

double Plugin14C::getLikelyhood(const double& t, const QJsonObject& data)
{
    QPair<double, double > result = getLikelyhoodArg(t, data);
    double back = exp(result.second) / sqrt(result.first) ;
    return back;
}

QString Plugin14C::getName() const
{
    return QString("14C");
}

QIcon Plugin14C::getIcon() const
{
    return QIcon(":/14C_w.png");
}

bool Plugin14C::doesCalibration() const
{
    return true;
}

bool Plugin14C::wiggleAllowed() const
{
    return true;
}
Date::DataMethod Plugin14C::getDataMethod() const
{
    return Date::eInversion;
}
QList<Date::DataMethod> Plugin14C::allowedDataMethods() const
{
    QList<Date::DataMethod> methods;
    methods.append(Date::eMHSymetric);
    methods.append(Date::eInversion);
    methods.append(Date::eMHSymGaussAdapt);
    return methods;
}

QStringList Plugin14C::csvColumns() const
{
    QStringList cols;
    cols << "Name" << "Age" << "Error (sd)" << "Reference curve" << "ΔR" << "ΔR Error";
    return cols;
}

int Plugin14C::csvMinColumns() const{
    return csvColumns().count() - 2;
}

PluginFormAbstract* Plugin14C::getForm()
{
    Plugin14CForm* form = new Plugin14CForm(this);
    return form;
}

QJsonObject Plugin14C::fromCSV(const QStringList& list)
{
    QJsonObject json;
    if(list.size() >= csvMinColumns())
    {
        json.insert(DATE_14C_AGE_STR, list[1].toDouble());
        json.insert(DATE_14C_ERROR_STR, list[2].toDouble());
        json.insert(DATE_14C_REF_CURVE_STR, list[3].toLower());
        
        // These columns are nor mandatory in the CSV file so check if they exist :
        json.insert(DATE_14C_DELTA_R_STR, (list.size() > 4) ? list[4].toDouble() : 0);
        json.insert(DATE_14C_DELTA_R_ERROR_STR, (list.size() > 5) ? list[5].toDouble() : 0);
        
        //qDebug() << list;
        //qDebug() << json;
    }
    return json;
}

QStringList Plugin14C::toCSV(const QJsonObject& data, const QLocale& csvLocale)
{
    QStringList list;
    list << csvLocale.toString(data[DATE_14C_AGE_STR].toDouble());
    list << csvLocale.toString(data[DATE_14C_ERROR_STR].toDouble());
    list << data[DATE_14C_REF_CURVE_STR].toString();
    list << csvLocale.toString(data[DATE_14C_DELTA_R_STR].toDouble());
    list << csvLocale.toString(data[DATE_14C_DELTA_R_ERROR_STR].toDouble());
    return list;
}

QString Plugin14C::getDateDesc(const Date* date) const
{
    QLocale locale=QLocale();    
    QString result;
    if(date)
    {
        QJsonObject data = date->mData;
        
        double age = data[DATE_14C_AGE_STR].toDouble();
        double error = data[DATE_14C_ERROR_STR].toDouble();
        double delta_r = data[DATE_14C_DELTA_R_STR].toDouble();
        double delta_r_error = data[DATE_14C_DELTA_R_ERROR_STR].toDouble();
        QString ref_curve = data[DATE_14C_REF_CURVE_STR].toString().toLower();        

        result += QObject::tr("Age") + " : " + locale.toString(age);
        result += " ± " + locale.toString(error);
        if(delta_r != 0 || delta_r_error != 0){
            result += ", " + QObject::tr("ΔR") + " : " + locale.toString(delta_r);
            result += " ± " +locale.toString(delta_r_error);
        }
        if(mRefDatas.contains(ref_curve) && !mRefDatas[ref_curve].isEmpty()) {
            result += ", " + tr("Ref. curve") + " : " + ref_curve;
        }
        else {
            result += ", " + tr("ERROR") +"-> "+ tr("Ref. curve") + " : " + ref_curve;
        }
    }
    return result;
}

// ------------------------------------------------------------------


QString Plugin14C::getRefExt() const
{
    return "14c";
}

QString Plugin14C::getRefsPath() const
{
    QString path = qApp->applicationDirPath();
#ifdef Q_OS_MAC
    QDir dir(path);
    dir.cdUp();
    path = dir.absolutePath() + "/Resources";
#endif
    QString calibPath = path + "/Calib/14C";
    return calibPath;
}

QMap<QString, QMap<double, double> > Plugin14C::loadRefFile(QFileInfo refFile)
{
    QFile file(refFile.absoluteFilePath());
    QMap<QString, QMap<double, double> > curves;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMap<double, double> curveG;
        QMap<double, double> curveG95Sup;
        QMap<double, double> curveG95Inf;
        QLocale locale = QLocale(QLocale::English);

        QTextStream stream(&file);
        while(!stream.atEnd())
        {
            QString line = stream.readLine();
            if(!isComment(line))
            {
                QStringList values = line.split(",");
                if(values.size() >= 3)
                {
                    bool ok = true;
                    int t = 1950 - locale.toInt(values[0],&ok);
                    if(!ok) continue;
                    double g = locale.toDouble(values[1],&ok);
                    if(!ok) continue;
                    double gSup = g + 1.96f * locale.toDouble(values[2],&ok);
                    if(!ok) continue;
                    double gInf = g - 1.96f * locale.toDouble(values[2],&ok);
                    if(!ok) continue;
                    
                    curveG[t] = g;
                    
                    curveG95Sup[t] = gSup;
                    curveG95Inf[t] = gInf;
                }
            }
        }
        file.close();
        // it is not a valid file
        if(curveG.isEmpty()) return curves;

        // The curves do not have 1-year precision!
        // We have to interpolate in the blanks
        
        double tmin = curveG.firstKey();
        double tmax = curveG.lastKey();
        
        for(double t=tmin; t<tmax; ++t)//t+=settings.mStep)//++t)
        {
            if(curveG.find(t) == curveG.end())
            {
                // This actually return the iterator with the nearest greater key !!!
                QMap<double, double>::const_iterator iter = curveG.lowerBound(t);
                if(iter != curveG.end())
                {
                    double t_upper = iter.key();
                    --iter;
                    //if(iter != curveG.begin())
                    {
                        double t_under = iter.key();
                        
                        //qDebug() << t_under << " < " << t << " < " << t_upper;
                        
                        double g_under = curveG[t_under];
                        double g_upper = curveG[t_upper];
                        
                        double gsup_under = curveG95Sup[t_under];
                        double gsup_upper = curveG95Sup[t_upper];
                        
                        double ginf_under = curveG95Inf[t_under];
                        double ginf_upper = curveG95Inf[t_upper];
                        
                        curveG[t] = interpolate(t, t_under, t_upper, g_under, g_upper);
                        curveG95Sup[t] = interpolate(t, t_under, t_upper, gsup_under, gsup_upper);
                        curveG95Inf[t] = interpolate(t, t_under, t_upper, ginf_under, ginf_upper);
                    }
                    /*else
                     {
                     curveG[t] = 0;
                     curveG95Sup[t] = 0;
                     curveG95Inf[t] = 0;
                     }*/
                }
                else
                {
                    /*curveG[t] = 0;
                     curveG95Sup[t] = 0;
                     curveG95Inf[t] = 0;*/
                }
            }
        }
        
        // Store the resulting curves :
        
        curves["G"] = curveG;
        curves["G95Sup"] = curveG95Sup;
        curves["G95Inf"] = curveG95Inf;
    }
    return curves;
}

QPair<double,double> Plugin14C::getTminTmaxRefsCurve(const QJsonObject& data) const
{
    double tmin = 0;
    double tmax = 0;
    QString ref_curve = data[DATE_14C_REF_CURVE_STR].toString().toLower();

    if( mRefDatas.contains(ref_curve)  && !mRefDatas[ref_curve].isEmpty() ) {
       tmin = mRefDatas[ref_curve]["G"].firstKey();
       tmax = mRefDatas[ref_curve]["G"].lastKey();
    }
    return qMakePair<double,double>(tmin,tmax);
}

// ------------------------------------------------------------------
GraphViewRefAbstract* Plugin14C::getGraphViewRef()
{
    //if(!mRefGraph) mRefGraph = new Plugin14CRefView();
    
    if(mRefGraph) delete mRefGraph;
    mRefGraph = new Plugin14CRefView();
    
    return mRefGraph;
}

PluginSettingsViewAbstract* Plugin14C::getSettingsView()
{
    return new Plugin14CSettingsView(this);
}

QList<QHash<QString, QVariant>> Plugin14C::getGroupedActions()
{
    QList<QHash<QString, QVariant>> result;
    
    QHash<QString, QVariant> groupedAction;
    groupedAction.insert("pluginId", getId());
    groupedAction.insert("title", tr("Selected events with 14C: change Ref. Curves"));
    groupedAction.insert("label", tr("Change 14C Ref. Curves for all 14C data in selected events :"));
    groupedAction.insert("inputType", "combo");
    groupedAction.insert("items", getRefsNames());
    groupedAction.insert("valueKey", DATE_14C_REF_CURVE_STR);
    
    result.append(groupedAction);
    return result;
}

QJsonObject Plugin14C::checkValuesCompatibility(const QJsonObject& values){
    QJsonObject result = values;

    if(result.find(DATE_14C_DELTA_R_STR) == result.end())
        result[DATE_14C_DELTA_R_STR] = 0;
    
    if(result.find(DATE_14C_DELTA_R_ERROR_STR) == result.end())
        result[DATE_14C_DELTA_R_ERROR_STR] = 0;
    
    // Force curve name to lower case :
    result[DATE_14C_REF_CURVE_STR] = result[DATE_14C_REF_CURVE_STR].toString().toLower();
    
    return result;
}

bool Plugin14C::isDateValid(const QJsonObject& data, const ProjectSettings& settings){
    
    QString ref_curve = data[DATE_14C_REF_CURVE_STR].toString().toLower();
    if(!mRefDatas.contains(ref_curve))
    {
        qDebug()<<"in Plugin14C::isDateValid() unkowned curve"<<ref_curve;
        //QMessageBox::warning(qApp->activeWindow(),tr("Curve error"),tr("in Plugin14C unkowned curve : ")+ref_curve);
        return false;
    }
    double age = data[DATE_14C_AGE_STR].toDouble();
    double error = data[DATE_14C_ERROR_STR].toDouble();
    double delta_r = data[DATE_14C_DELTA_R_STR].toDouble();
    double delta_r_error = data[DATE_14C_DELTA_R_ERROR_STR].toDouble();
    
    // Apply reservoir effect
    age = (age - delta_r);
    error = sqrt(error * error + delta_r_error * delta_r_error);
    
    double min = 0;
    double max = 0;
    
    if(mLastRefsMinMax.find(ref_curve) != mLastRefsMinMax.end() &&
       mLastRefsMinMax[ref_curve].first.first == settings.mTmin &&
       mLastRefsMinMax[ref_curve].first.second == settings.mTmax)
    {
        min = mLastRefsMinMax[ref_curve].second.first;
        max = mLastRefsMinMax[ref_curve].second.second;
    }
    else
    {
        const QMap<double, double>& curveG95Inf = mRefDatas[ref_curve]["G95Inf"];
        const QMap<double, double>& curveG95Sup = mRefDatas[ref_curve]["G95Sup"];
        
        // Find max
        QMap<double, double>::const_iterator iter = curveG95Sup.constFind(settings.mTmin);
        if(iter == curveG95Sup.constEnd()){
            double t1 = curveG95Sup.firstKey();
            if(t1 < settings.mTmax){
                iter = curveG95Sup.constBegin();
            }
        }
        if(iter != curveG95Sup.constEnd()){
            max = iter.value();
            while(iter != curveG95Sup.constEnd() && iter.key() <= settings.mTmax){
                max = qMax(max, iter.value());
                ++iter;
            }
        }
        
        // Find min
        iter = curveG95Inf.constFind(settings.mTmin);
        if(iter == curveG95Inf.constEnd()){
            double t1 = curveG95Inf.firstKey();
            if(t1 < settings.mTmax){
                iter = curveG95Inf.constBegin();
            }
        }
        if(iter != curveG95Inf.constEnd()){
            min = iter.value();
            while(iter != curveG95Inf.constEnd() && iter.key() <= settings.mTmax){
                min = qMin(min, iter.value());
                ++iter;
            }
        }
        
        // Store min & max
        mLastRefsMinMax[ref_curve].first.first = settings.mTmin;
        mLastRefsMinMax[ref_curve].first.second = settings.mTmax;
        mLastRefsMinMax[ref_curve].second.first = min;
        mLastRefsMinMax[ref_curve].second.second = max;
    }
    return !(min == 0 && max == 0) && ((age - 1.96f*error < max) && (age + 1.96f*error > min));
}

bool Plugin14C::areDatesMergeable(const QJsonArray& dates)
{
    QString refCurve;
    for(int i=0; i<dates.size(); ++i)
    {
        QJsonObject date = dates[i].toObject();
        QJsonObject data = date[STATE_DATE_DATA].toObject();
        QString curve = data[DATE_14C_REF_CURVE_STR].toString();
        
        if(refCurve.isEmpty())
            refCurve = curve;
        else if(refCurve != curve)
            return false;
    }
    return true;
}

QJsonObject Plugin14C::mergeDates(const QJsonArray& dates)
{
    QJsonObject result;
    if(dates.size() > 1){
        // Verify all dates have the same ref curve :
        QJsonObject firstDate = dates[0].toObject();
        QJsonObject firstDateData = firstDate[STATE_DATE_DATA].toObject();
        QString firstCurve = firstDateData[DATE_14C_REF_CURVE_STR].toString();
        
        for(int i=1; i<dates.size(); ++i){
            QJsonObject date = dates[i].toObject();
            QJsonObject dateData = date[STATE_DATE_DATA].toObject();
            QString curve = dateData[DATE_14C_REF_CURVE_STR].toString();
            if(firstCurve != curve){
                result["error"] = tr("All combined data must use the same reference curve !");
                return result;
            }
        }
        
        double sum_vi = 0;
        double sum_mi_vi = 0;
        double sum_1_vi = 0;
      //  double sum_mi_2 = 0;
        QStringList names;
        
        for(int i=0; i<dates.size(); ++i){
            QJsonObject date = dates[i].toObject();
            QJsonObject data = date[STATE_DATE_DATA].toObject();
            
            names.append(date[STATE_NAME].toString());
            double a = data[DATE_14C_AGE_STR].toDouble();
            double e = data[DATE_14C_ERROR_STR].toDouble();
            double r = data[DATE_14C_DELTA_R_STR].toDouble();
            double re = data[DATE_14C_DELTA_R_ERROR_STR].toDouble();
            
            // Reservoir effet
            double m = a - r;
            double v = e * e + re * re;
            
            sum_vi += v;
            sum_mi_vi += m/v;
            sum_1_vi += 1/v;
         //   sum_mi_2 += m*m;
        }
        
        result = dates[0].toObject();
        result[STATE_NAME] = "Combined (" + names.join(" | ") + ")";
        
        QJsonObject mergedData = result[STATE_DATE_DATA].toObject();
        mergedData[DATE_14C_AGE_STR] = sum_mi_vi / sum_1_vi;
        mergedData[DATE_14C_ERROR_STR] = sqrt(1 / sum_1_vi);
        mergedData[DATE_14C_DELTA_R_STR] = 0;
        mergedData[DATE_14C_DELTA_R_ERROR_STR] = 0;
        
        qDebug() << mergedData;
        
        result[STATE_DATE_DATA] = mergedData;
        result[STATE_DATE_SUB_DATES] = dates;
    }else{
        result["error"] = tr("Combine needs at least 2 data !");
    }
    return result;
    
}

#endif
