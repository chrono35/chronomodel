#include "PluginManager.h"
#include "../PluginAbstract.h"
#include <QDebug>

// Temp:
#include "PluginMag.h"
#include "PluginTL.h"
#include "Plugin14C.h"
#include "PluginUniform.h"
#include "PluginGauss.h"


QList<PluginAbstract*> PluginManager::mPlugins = QList<PluginAbstract*>();

void PluginManager::loadPlugins()
{
    /*QDir pluginsDir = QDir(qApp->applicationDirPath());
    
#if defined(Q_OS_WIN)
    
#elif defined(Q_OS_MAC)
    if(pluginsDir.dirName() == "MacOS")
    {
        pluginsDir.cdUp();
        pluginsDir.cdUp();
        pluginsDir.cdUp();
    }
#endif
    
    foreach(QString fileName, pluginsDir.entryList(QDir::Files))
    {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject* plugin = loader.instance();
        if(plugin)
        {
            PluginAbstract* iPlugin = qobject_cast<PluginAbstract*>(plugin);
            if(iPlugin)
            {
                qDebug() << QString("Plugin ok : ") << plugin;
                mPlugins.append(iPlugin);
            }
            else
                qDebug() << "Plugin non conforme à l'interface : " << plugin;
        }
        else
            qDebug() << "Chargement échoué : " << plugin;
    }*/
    
#if USE_PLUGIN_UNIFORM
    PluginAbstract* pluginUniform = new PluginUniform();
    mPlugins.append(pluginUniform);
#endif
#if USE_PLUGIN_GAUSS
    PluginAbstract* pluginGauss = new PluginGauss();
    mPlugins.append(pluginGauss);
#endif
#if USE_PLUGIN_TL
    PluginAbstract* pluginTL = new PluginTL();
    mPlugins.append(pluginTL);
#endif
#if USE_PLUGIN_14C
    PluginAbstract* plugin14C = new Plugin14C();
    mPlugins.append(plugin14C);
#endif
#if USE_PLUGIN_AM
    PluginAbstract* pluginMag = new PluginMag();
    mPlugins.append(pluginMag);
#endif
}


PluginAbstract* PluginManager::getPluginFromId(const QString& pluginId)
{
    for(int i=0; i<mPlugins.size(); ++i)
        if(mPlugins[i]->getId() == pluginId)
            return mPlugins[i];
    return 0;
}
PluginAbstract* PluginManager::getPluginFromName(const QString& pluginName)

{
    for(int i=0; i<mPlugins.size(); ++i)
        if(mPlugins[i]->getName().toLower() == pluginName.toLower())
            return mPlugins[i];
    return 0;
}

const QList<PluginAbstract*>& PluginManager::getPlugins()
{
    return mPlugins;
}

QStringList PluginManager::getPluginsNames()
{
    QStringList names;
    for(int i=0; i<mPlugins.size(); ++i)
        names << mPlugins[i]->getName();
    return names;
}
