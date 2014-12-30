#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include "GraphViewAbstract.h"
#include "GraphCurve.h"
#include "GraphZone.h"
#include "Ruler.h"

#include <QWidget>
#include <QString>
#include <QFont>
#include <QColor>
#include <QPixmap>


class GraphView: public QWidget, public GraphViewAbstract
{
    Q_OBJECT
public:
    enum Rendering{
        eSD = 0,
        eHD = 1
    };
    enum AxisMode{
        eHidden = 0,
        eMinMax = 1,
        eMainTicksOnly = 2,
        eAllTicks = 3
    };
    
    GraphView(QWidget* parent = 0);
    virtual ~GraphView();
    
    // Options
    
    void setBackgroundColor(const QColor& aColor);
    QColor getBackgroundColor() const;
    
    void addInfo(const QString& info);
    void clearInfos();
    void showInfos(bool show);
    
    void setRendering(Rendering render);
    void showAxisArrows(bool show);
    void showAxisLines(bool show);
    void showVertGrid(bool show);
    void showHorizGrid(bool show);
    void setXAxisMode(AxisMode mode);
    void setYAxisMode(AxisMode mode);
    
    // Manage Curves
    
    void addCurve(const GraphCurve& curve);
    void removeCurve(const QString& name);
    void removeAllCurves();
    GraphCurve* getCurve(const QString& name);
    int numCurves() const;
    
    void addZone(const GraphZone& zone);
    void removeAllZones();
    
    // Paint
    
    void paintToDevice(QPaintDevice* device);
    
public slots:
    void zoomX(const double min, const double max);
    void exportCurrentCurves(const QString& defaultPath, const QString& csvSep, bool writeInRows) const;
    
protected:
    void adaptMarginBottom();
    
    void updateGraphSize(int w, int h);
    void repaintGraph(const bool aAlsoPaintBackground);
    void drawCurves(QPainter& painter);

    void resizeEvent(QResizeEvent* aEvent);
    void paintEvent(QPaintEvent* aEvent);
    void enterEvent(QEvent* e);
    void leaveEvent(QEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    
protected:
    QPixmap	mBufferBack;
    
    AxisTool mAxisToolX;
    AxisTool mAxisToolY;
    
    Rendering mRendering;
    bool mShowAxisArrows;
    bool mShowAxisLines;
    bool mShowVertGrid;
    bool mShowHorizGrid;
    AxisMode mXAxisMode;
    AxisMode mYAxisMode;
    
    bool mShowInfos;
    QStringList mInfos;
    
    QColor	mBackgroundColor;
    
    QRectF  mTipRect;
    double  mTipX;
    double  mTipY;
    double  mTipWidth;
    double  mTipHeight;
    double  mTipVisible;
    double  mUseTip;
    
    int mCurveMaxResolution;
    QList<GraphCurve> mCurves;
    QList<GraphZone> mZones;
};

#endif
