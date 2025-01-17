#ifndef PluginGaussForm_H
#define PluginGaussForm_H

#if USE_PLUGIN_GAUSS

#include "../PluginFormAbstract.h"

class PluginGauss;
class QLineEdit;
class QLabel;
class QRadioButton;
class QComboBox;


class PluginGaussForm: public PluginFormAbstract
{
    Q_OBJECT
public:
    PluginGaussForm(PluginGauss* plugin, QWidget* parent = 0, Qt::WindowFlags flags = 0);
    virtual ~PluginGaussForm();
    
    void setData(const QJsonObject& data, bool isCombined);
    QJsonObject getData();
    
    bool isValid();
    
protected slots:
    void updateVisibleElements();

private:
    QLabel* mAverageLab;
    QLabel* mErrorLab;
    QLabel* mCalibLab;
    
    QLineEdit* mAverageEdit;
    QLineEdit* mErrorEdit;
    
    QWidget* mEqWidget;
    QLabel* mEq1Lab;
    QLabel* mEq2Lab;
    QLabel* mEq3Lab;
    QLineEdit* mAEdit;
    QLineEdit* mBEdit;
    QLineEdit* mCEdit;
    
    QRadioButton* mEquationRadio;
    QRadioButton* mCurveRadio;
    QRadioButton* mNoneRadio;
    
    QComboBox* mCurveCombo;
    int mComboH;
};

#endif
#endif
