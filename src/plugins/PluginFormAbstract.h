#ifndef PLUGINFORMABSTRACT_H
#define PLUGINFORMABSTRACT_H

#include "GroupBox.h"
#include "Date.h"
#include <QString>

class PluginAbstract;


class PluginFormAbstract: public GroupBox
{
    Q_OBJECT
public:
    PluginFormAbstract(PluginAbstract* plugin, const QString& title, QWidget* parent = 0, Qt::WindowFlags flags = 0):GroupBox(title, parent, flags),
    mPlugin(plugin){}
    
    virtual ~PluginFormAbstract(){}
    
    virtual void setData(const QJsonObject& data, bool isCombined) = 0;
    virtual QJsonObject getData() = 0;

    virtual bool isValid() = 0;
    
public:
    PluginAbstract* mPlugin;
    QString mError;
protected:
  //  FormatFunc mFormatFuncX;
};

#endif

