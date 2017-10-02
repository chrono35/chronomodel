#ifndef CALIBRATIONVIEW_H
#define CALIBRATIONVIEW_H

#include <QWidget>
#include <QJsonObject>

#include "Date.h"
#include "ProjectSettings.h"
#include "GraphView.h"

class GraphViewRefAbstract;
class Date;
class Marker;
class LineEdit;
class Button;
class QTextEdit;
class QLabel;
class Label;
class QSlider;
class QScrollBar;
class QFrame;
class QGraphicsScene;
class QGraphicsView;
class CalibrationDrawing;


class CalibrationView: public QWidget
{
    Q_OBJECT
public:
    CalibrationView(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
    ~CalibrationView();
    
    void setDate(const QJsonObject& date);
    void setFont(const QFont& font);
    void initScale (const double &majorScale, const int &minorScale) { mMajorScale= majorScale; mMinorScale = minorScale;}
    void initScale (const Scale &s) { mMajorScale = s.mark; mMinorScale = s.tip;}
    
protected:
    void paintEvent(QPaintEvent* e);
    void resizeEvent(QResizeEvent* e);
    void updateLayout();

public slots:
    virtual void setVisible(bool visible);
    void updateScaleX();

private slots:
    void updateGraphs();
    void updateZoom();
    void updateScroll();
    void exportImage();
    void copyImage();
    void copyText();


signals:
    void closed();
    
public:
    Date mDate;
    ProjectSettings mSettings;
    
private:

    CalibrationDrawing* mDrawing;

    GraphView* mCalibGraph;
    GraphViewRefAbstract* mRefGraphView;
    
    QTextEdit* mResultsText;
    qreal mResultsHeight;

    qreal mButtonWidth;

    Button* mImageSaveBut;
    Button* mImageClipBut;
    Button* mResultsClipBut;
    
    QFrame* frameSeparator;

    Label* mHPDLab;
    LineEdit* mHPDEdit;

    Label* mStartLab;
    LineEdit* mStartEdit;

    Label* mEndLab;
    LineEdit* mEndEdit;

    Label* mMajorScaleLab;
    LineEdit* mMajorScaleEdit;

    Label* mMinorScaleLab;
    LineEdit* mMinorScaleEdit;

    double mTminDisplay;
    double mTmaxDisplay;

    double mMajorScale;
    int mMinorScale;
};

#endif
