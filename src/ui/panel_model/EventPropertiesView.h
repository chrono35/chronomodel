#ifndef EventPropertiesView_H
#define EventPropertiesView_H

#include <QWidget>
#include <QJsonObject>

class Event;
class Date;
class Label;
class LineEdit;

class QLabel;
class QLineEdit;
class QComboBox;
class QGroupBox;
class QRadioButton;

class ColorPicker;
class DatesList;
class Button;
class RadioButton;
class GraphView;


class EventPropertiesView: public QWidget
{
    Q_OBJECT
public:
    EventPropertiesView(QWidget* parent = 0, Qt::WindowFlags flags = 0);
    ~EventPropertiesView();
    
    void updateEvent();
    const QJsonObject& getEvent() const;
    int mToolbarH;
    
public slots:
    void setEvent(const QJsonObject& event);
    
protected:
    void paintEvent(QPaintEvent* e);
    void resizeEvent(QResizeEvent* e);
    void updateLayout();
    
private slots:
    
    void updateEventName();
    void updateEventColor(QColor color);
    void updateEventMethod(int index);
    
    void createDate();
    void deleteSelectedDates();
    void recycleDates();
    
    void updateCombineAvailability();
    void sendMergeSelectedDates();
    void sendSplitDate();
    
    void updateKnownType();
    void updateKnownFixed(const QString& text);
    void updateKnownUnifStart();
    void updateKnownUnifEnd();
    void loadKnownCsv();
    
    void updateKnownGraph();
    void updateKnownControls();
    
public slots:
    void hideCalibration();
    
signals:
    void mergeDatesRequested(const int eventId, const QList<int>& dateIds);
    void splitDateRequested(const int eventId, const int dateId);
    
    void updateCalibRequested(const QJsonObject& date);
    void showCalibRequested(bool show);
    
private:
    int minimumHeight;
    QJsonObject mEvent;
    
    QWidget* mTopView;
    QWidget* mEventView;
    QWidget* mBoundView;
    
    QLabel* mNameLab;
    QLabel* mColorLab;
    QLabel* mMethodLab;
    
    QLineEdit* mNameEdit;
    ColorPicker* mColorPicker;
    QComboBox* mMethodCombo;
    
    DatesList* mDatesList;
    QList<Button*> mPluginButs1;
    QList<Button*> mPluginButs2;
    Button* mDeleteBut;
    Button* mRecycleBut;
    
    Button* mCalibBut;
    Button* mMergeBut;
    Button* mSplitBut;
    
    QRadioButton* mKnownFixedRadio;
    QRadioButton* mKnownUniformRadio;
    
    QLineEdit* mKnownFixedEdit;
    QLineEdit* mKnownStartEdit;
    QLineEdit* mKnownEndEdit;
    
    GraphView* mKnownGraph;
    
    QGroupBox* mFixedGroup;
    QGroupBox* mUniformGroup;
};

#endif
