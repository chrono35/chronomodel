#ifndef PluginUniformForm_H
#define PluginUniformForm_H

#if USE_PLUGIN_UNIFORM

#include "../PluginFormAbstract.h"

class PluginUniform;
class QLabel;
class QLineEdit;


class PluginUniformForm: public PluginFormAbstract
{
    Q_OBJECT
public:
    PluginUniformForm(PluginUniform* plugin, QWidget* parent = 0, Qt::WindowFlags flags = 0);
    virtual ~PluginUniformForm();
    
    void setData(const QJsonObject& data, bool isCombined);
    QJsonObject getData();
    
    bool isValid();

private:
    QLabel* mMinLab;
    QLabel* mMaxLab;
    
    QLineEdit* mMinEdit;
    QLineEdit* mMaxEdit;
};

#endif
#endif
