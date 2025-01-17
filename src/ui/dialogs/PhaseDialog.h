#ifndef PhaseDialog_H
#define PhaseDialog_H

#include <QDialog>
#include <QJsonObject>
#include <QString>

class Phase;
class Label;
class Button;
class LineEdit;
class QComboBox;
class ColorPicker;


class PhaseDialog: public QDialog
{
    Q_OBJECT
public:
    PhaseDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);
    ~PhaseDialog();
    
    void setPhase(const QJsonObject& phase);
    QJsonObject getPhase();
    
    bool isValid();
    
protected slots:
    void showAppropriateTauOptions(int typeIndex);
    
protected:
    void resizeEvent(QResizeEvent* event);
    
public:
    QJsonObject mPhase;
    
    Label* mNameLab;
    Label* mColorLab;
    Label* mTauTypeLab;
    Label* mTauFixedLab;
    Label* mTauMinLab;
    Label* mTauMaxLab;
    
    LineEdit* mNameEdit;
    ColorPicker* mColorPicker;
    QComboBox* mTauTypeCombo;
    LineEdit* mTauFixedEdit;
    LineEdit* mTauMinEdit;
    LineEdit* mTauMaxEdit;
    
    Button* mOkBut;
    Button* mCancelBut;
    
    int mMargin;
    int mLineH;
    int mComboH;
    int mButH;
    int mButW;
    
    QString mError;
};

#endif
