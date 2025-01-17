#ifndef PLUGINSETTINGSVIEWABSTRACT_H
#define PLUGINSETTINGSVIEWABSTRACT_H

#include <QWidget>

class PluginAbstract;


class PluginSettingsViewAbstract: public QWidget
{
    Q_OBJECT
public:
    PluginSettingsViewAbstract(PluginAbstract* plugin, QWidget* parent = 0, Qt::WindowFlags flags = 0):QWidget(parent, flags),
    mPlugin(plugin){}
    
    virtual ~PluginSettingsViewAbstract(){}
    
protected slots:
    virtual void onAccepted(){}
    virtual void onRejected(){}
    
public:
    PluginAbstract* mPlugin;
};

#endif

