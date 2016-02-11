#ifndef PLUGINTL_H
#define PLUGINTL_H

#if USE_PLUGIN_TL

#include "../PluginAbstract.h"

#define DATE_TL_AGE_STR "age"
#define DATE_TL_ERROR_STR "error"
#define DATE_TL_REF_YEAR_STR "ref_year"


class DATATION_SHARED_EXPORT PluginTL : public PluginAbstract
{
    Q_OBJECT
    //Q_PLUGIN_METADATA(IID "chronomodel.PluginAbstract.PluginTL")
    //Q_INTERFACES(PluginAbstract)
public:
    PluginTL();
    virtual ~PluginTL();

    //virtual function
    long double getLikelihood(const double& t, const QJsonObject& data);
    bool withLikelihoodArg() {return true; }
    QPair<long double, long double > getLikelihoodArg(const double& t, const QJsonObject& data);
    
    QString getName() const;
    QIcon getIcon() const;
    bool doesCalibration() const;
    bool wiggleAllowed() const;
    Date::DataMethod getDataMethod() const;
    QList<Date::DataMethod> allowedDataMethods() const;
    QStringList csvColumns() const;
    QJsonObject fromCSV(const QStringList& list, const QLocale &csvLocale);
    QStringList toCSV(const QJsonObject& data, const QLocale &csvLocale) const;
    QString getDateDesc(const Date* date) const;
    QJsonObject checkValuesCompatibility(const QJsonObject& values);

    QPair<double,double> getTminTmaxRefsCurve(const QJsonObject& data) const;
    
    PluginFormAbstract* getForm();
    GraphViewRefAbstract* getGraphViewRef();
    PluginSettingsViewAbstract* getSettingsView();
};

#endif
#endif
