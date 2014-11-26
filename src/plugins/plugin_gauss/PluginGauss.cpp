#include "PluginGauss.h"
#if USE_PLUGIN_GAUSS

#include "StdUtilities.h"
#include "PluginGaussForm.h"
#include "PluginGaussRefView.h"
#include <cstdlib>
#include <iostream>
#include <QJsonObject>
#include <QtWidgets>


PluginGauss::PluginGauss()
{
    
}

float PluginGauss::getLikelyhood(const float& t, const QJsonObject& data)
{
    float age = data[DATE_GAUSS_AGE_STR].toDouble();
    float error = data[DATE_GAUSS_ERROR_STR].toDouble();
    float a = data[DATE_GAUSS_A_STR].toDouble();
    float b = data[DATE_GAUSS_B_STR].toDouble();
    float c = data[DATE_GAUSS_C_STR].toDouble();
    
    float v = exp(-0.5f * powf((age - (a * t * t + b * t + c)) / error, 2.f)) / (error * sqrt(2.f * M_PI));
    return v;
}

QString PluginGauss::getName() const
{
    return QString("Gauss");
}
QIcon PluginGauss::getIcon() const
{
    return QIcon(":/gauss_w.png");
}
bool PluginGauss::doesCalibration() const
{
    return true;
}
bool PluginGauss::wiggleAllowed() const
{
    return false;
}
Date::DataMethod PluginGauss::getDataMethod() const
{
    return Date::eMHIndependant;
}
QList<Date::DataMethod> PluginGauss::allowedDataMethods() const
{
    QList<Date::DataMethod> methods;
    methods.append(Date::eMHIndependant);
    methods.append(Date::eInversion);
    methods.append(Date::eMHSymGaussAdapt);
    return methods;
}
QString PluginGauss::csvHelp() const
{
    return "Calibration : g(t) = at^2 + bt + c\n";
}
QStringList PluginGauss::csvColumns() const
{
    QStringList cols;
    cols << "Name" << "Age" << "Error" << "a" << "b" << "c";
    return cols;
}


PluginFormAbstract* PluginGauss::getForm()
{
    PluginGaussForm* form = new PluginGaussForm(this);
    return form;
}

QJsonObject PluginGauss::dataFromList(const QStringList& list)
{
    QJsonObject json;
    if(list.size() >= csvMinColumns())
    {
        json.insert(DATE_GAUSS_AGE_STR, list[1].toDouble());
        json.insert(DATE_GAUSS_ERROR_STR, list[2].toDouble());
        json.insert(DATE_GAUSS_A_STR, list[3].toDouble());
        json.insert(DATE_GAUSS_B_STR, list[4].toDouble());
        json.insert(DATE_GAUSS_C_STR, list[5].toDouble());
    }
    return json;
}

QString PluginGauss::getDateDesc(const Date* date) const
{
    QString result;
    if(date)
    {
        QJsonObject data = date->mData;
        result += QObject::tr("Age") + " : " + QString::number(data[DATE_GAUSS_AGE_STR].toDouble());
        result += " +- " + QString::number(data[DATE_GAUSS_ERROR_STR].toDouble());
        result += ", " + QObject::tr("Ref. curve") + " = " +
            QString::number(data[DATE_GAUSS_A_STR].toDouble()) + "t^2 + ";
            QString::number(data[DATE_GAUSS_B_STR].toDouble()) + "t + ";
            QString::number(data[DATE_GAUSS_C_STR].toDouble());
    }
    return result;
}

// ------------------------------------------------------------------

GraphViewRefAbstract* PluginGauss::getGraphViewRef()
{
    if(!mRefGraph)
        mRefGraph = new PluginGaussRefView();
    return mRefGraph;
}

#endif
