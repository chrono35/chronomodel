#ifndef GraphViewResults_H
#define GraphViewResults_H

#include "ProjectSettings.h"
#include "MCMCSettings.h"
#include "MCMCLoop.h"
#include "GraphView.h"

#include <QWidget>
#include <QList>
#include <QColor>
#include <QBrush>

class Button;
class QPropertyAnimation;
class QTextEdit;
class MHVariable;
class MetropolisVariable;


class GraphViewResults: public QWidget
{
    Q_OBJECT
public:
    enum TypeGraph{
        ePostDistrib = 0,
        eTrace = 1,
        eAccept = 2,
        eCorrel = 3
    };
    enum Variable{
        eTheta = 0,
        eSigma = 1
    };
    
    explicit GraphViewResults(QWidget *parent = 0);
    virtual ~GraphViewResults();
    
    void setSettings(const ProjectSettings& settings);
    void setMCMCSettings(const MCMCSettings& mcmc, const QList<Chain>& chains);
    
    void setMainColor(const QColor& color);
    void toggle(const QRect& geometry);
    
    void setRendering(GraphView::Rendering render);
    virtual void setGraphFont(const QFont& font);
    void setGraphsThickness(int value);
    
    void setItemColor(const QColor& itemColor);
    void setItemTitle(const QString& itemTitle);
    
    virtual void setButtonsVisible(const bool visible);
    
    GraphView* mGraph;
    
    GraphCurve generateDensityCurve(const QMap<double, double>& data,
                                    const QString& name,
                                    const QColor& lineColor,
                                    const Qt::PenStyle penStyle = Qt::SolidLine,
                                    const QBrush& brush = Qt::NoBrush) const;
    
    GraphCurve generateHPDCurve(const QMap<double, double>& data,
                                const QString& name,
                                const QColor& color) const;
    
    GraphCurve generateCredibilityCurve(const QPair<double, double>& section,
                                        const QString& name,
                                        const QColor& color) const;
    
    GraphCurve generateHorizontalLine(const double yValue,
                                      const QString& name,
                                      const QColor& color,
                                      const Qt::PenStyle penStyle = Qt::SolidLine) const;
    
    void generateTraceCurves(const QList<Chain>& chains,
                             MetropolisVariable* variable,
                             const QString& name = QString());
    
    void generateAcceptCurves(const QList<Chain>& chains,
                              MHVariable* variable);
    
    void generateCorrelCurves(const QList<Chain>& chains,
                              MHVariable* variable);
    
public slots:
    void setRange(double min, double max);
    void setCurrentX(double min, double max);
    
    void zoom(double min, double max);
    void showNumericalResults(bool show);
    void setNumericalResults(const QString& resultsHTML, const QString& resultsText);
    
private slots:
    void saveAsImage();
    void imageToClipboard();
    void resultsToClipboard();
    
    
protected slots:
    virtual void saveGraphData() const; // must be accessible to modifie by GraphViewPhase
    
protected:
    
    // These methods are from QWidget and we want to modify their behavior
    virtual void paintEvent(QPaintEvent* e);
    virtual void resizeEvent(QResizeEvent* e);
    
    // This is not from QWidget : we create this function to update the layout from different places (eg: in resizeEvent()).
    // It is vitual beacause we want a different behavior in suclasses (GraphViewDate, GraphViewEvent and GraphViewPhase)
    virtual void updateLayout();
    
public:
    // This method is used to recreate all curves in mGraph.
    // It is vitual because we want a different behavior in suclasses (GraphViewDate, GraphViewEvent and GraphViewPhase)
    virtual void generateCurves(TypeGraph typeGraph, Variable variable);
    
    // This method is used to update visible existing curves in mGraph.
    // It is vitual because we want a different behavior in suclasses (GraphViewDate, GraphViewEvent and GraphViewPhase)
    virtual void updateCurvesToShow(bool showAllChains, const QList<bool>& showChainList, bool showCredibility, bool showCalib, bool showWiggle);
    
    
signals:
    void unfoldToggled(bool toggled);
    void visibilityChanged(bool visible);
    
protected:
    
    
    TypeGraph mCurrentTypeGraph;
    Variable mCurrentVariable;
    
    QString mTitle;
    
    QString mResultsText;
    
    QColor mItemColor;
    QString mItemTitle;
    
    bool mShowAllChains;
    QList<bool> mShowChainList;
    bool mShowCredibility;
    
    bool mShowCalib;
    bool mShowWiggle;
    bool mShowNumResults;
    
    ProjectSettings mSettings;
    MCMCSettings mMCMCSettings;
    QList<Chain> mChains;
    
    QColor mMainColor;
    
    QTextEdit* mTextArea;
    
    Button* mImageSaveBut;
    Button* mImageClipBut;
    Button* mResultsClipBut;
    Button* mDataSaveBut;
    
    int mMargin;
    int mLineH;
    int mGraphLeft;
    int mTopShift;
    
    bool mButtonsVisible;
    int mHeightForVisibleAxis;
    
    QPropertyAnimation* mAnimation;
};

#endif
