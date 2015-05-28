#ifndef RULER_H
#define RULER_H

#include <QWidget>
#include "AxisTool.h"


class QScrollBar;

struct RulerArea{
    double mStart;
    double mStop;
    QColor mColor;
};

class Ruler: public QWidget
{
    Q_OBJECT
public:
    Ruler(QWidget* parent = 0, Qt::WindowFlags flags = 0);
    ~Ruler();
    
    void setRange(const double min, const double max);
    void setCurrent(const double min, const double max);
    void currentChanged(const double min, const double max);
    double getZoom();
    double getRealValue();
    
    void clearAreas();
    void addArea(double start, double end, const QColor& color);
    
    double realPosition;
    double mCurrentMin;
    double mCurrentMax;
    double mMin;
    double mMax;
    double mZoomProp;
    
    AxisTool mAxisTool;
    
protected:
    void layout();
    void resizeEvent(QResizeEvent* e);
    void paintEvent(QPaintEvent* e);
    
public slots:
    void setZoom(int prop);// HL
//    double setZoom(const double prop); //PhD
    void updateScroll();
    void scrollValueChanged(int value);
    
signals:
    void positionChanged(double min, double max);
    void valueChanged(int value);
    
private:
    QScrollBar* mScrollBar;
    qreal mScrollBarHeight;
    
    QRectF mRulerRect;
    
    //double mMin;
    //double mMax;
    //double mCurrentMin;
    //double mCurrentMax;
    //double mZoomProp;
    
    qreal mStepMinWidth;
    qreal mStepWidth;
    
    //AxisTool mAxisTool;
    
    QVector<RulerArea> mAreas;
};

#endif
