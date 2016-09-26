﻿#ifndef ResultsWrapper_H
#define ResultsWrapper_H

#include <QWidget>
#include <QVBoxLayout>
#include "MCMCLoopMain.h"
#include "AxisTool.h"
#include "GraphViewResults.h"

class QStackedWidget;
class QScrollArea;
class QTimer;
class QComboBox;
class QSlider;
class QScrollBar;
class QSpinBox;
class QPushButton;

class Project;
class Model;
class Tabs;
class Ruler;
class GraphView;
class GraphViewPhase;
class GraphViewEvent;
class GraphViewDate;

class Label;
class Button;
class LineEdit;
class CheckBox;
class RadioButton;
class Marker;


class ResultsView: public QWidget
{
    Q_OBJECT
public:
    ResultsView(QWidget* parent = 0, Qt::WindowFlags flags = 0);
    ~ResultsView();
    
    float mResultZoomX;
    float mResultCurrentMinX;
    float mResultCurrentMaxX;
    float mResultMinX;
    float mResultMaxX;
    float mResultMaxVariance;
    bool mHasPhases;
    
    Model* mModel;

    void doProjectConnections(Project* project);

    void updateFormatSetting(Model* model, const AppSettings* appSet);
    float getBandwidth() const;
    int getFFTLength() const;
    float getThreshold() const;


protected:
    void paintEvent(QPaintEvent* );
    void mouseMoveEvent(QMouseEvent* e);
    void resizeEvent(QResizeEvent* e);

    void createEventsScrollArea(const int idx = 0);
    void createPhasesScrollArea(const int idx = 0);
    void generateCurves(const QList<GraphViewResults*>& listGraphs);


public slots:
    void updateResults(Model* model = 0);
    void initResults(Model* model = 0);

    void changeScrollArea();
    void updateLayout();
    void updateGraphsLayout();
    
    void clearResults();
    void updateCurves();
    
    
    void updateControls();
    void updateScales();
    
    void updateModel();
    void updateResultsLog();

    void adjustDuration(bool visible);

private slots:
    void graphTypeChange();
    void updateCurvesToShow();
    
    void settingChange();
    void updateZoomX(); // Connected to slider signals
    void updateScroll(const float min, const float max); // Connected to ruler signals
    void editCurrentMinX(); // Connected to min edit signals
    void editCurrentMaxX(); // Connected to max edit signals
    void updateZoomEdit();
    void updateGraphsZoomX();
    
    void updateScaleY(int value);
    
    void updateFont();
    void updateThickness(const int value);
    void updateOpacity(const int value);
    void updateRendering(int index);
    void showInfos(bool);
    void exportFullImage();
    void exportResults();

    void previousSheet();
    void nextSheet();
    void unfoldToggle();

    // SETTER
    void setFFTLength();
    void setBandwidth();
    void setThreshold();

    
signals:
   
    void curvesGenerated();
    
    void controlsUpdated();
    void resultsLogUpdated(const QString& log);
    
    void scalesUpdated();
    
    void updateScrollAreaRequested();
    void generateCurvesRequested();

    
private:
    //QList<QRect> getGeometries(const QList<GraphViewResults*>& graphs, bool open, bool byPhases);
    

    void clearHisto();
    void clearChainHistos();

    
    Ruler* mRuler;
    

    ProjectSettings mSettings;
    MCMCSettings mMCMCSettings;
    QList<ChainSpecs> mChains;
    
    int mMargin;
    int mOptionsW;
    int mLineH;
    int mGraphLeft;
    int mRulerH;
    int mTabsH;
    int mGraphsH;
    
    Tabs* mTabs;
    int mTabEventsIndex;
    int mTabPhasesIndex;
    //Ruler* mRuler;
    Marker* mMarker;
    
    QStackedWidget* mStack;
    QScrollArea* mEventsScrollArea;
    QScrollArea* mPhasesScrollArea;
    QList<GraphViewResults*> mByEventsGraphs;
    QList<GraphViewResults*> mByPhasesGraphs;

    
    Button* mByPhasesBut;
    Button* mByEventsBut;
    
    
    QWidget* mOptionsWidget;
    
    
    Button* mUnfoldBut;
    Button* mStatsBut;
    Button* mExportImgBut;
    Button* mExportResults;
    CheckBox* mShowDataUnderPhasesCheck;

    Button* mNextSheetBut;
    Button* mPreviousSheetBut;
    
    // ------ mDisplayGroup -----
   // QWidget* mScaleGroup;
    Label* mDisplayTitle;
    QWidget* mDisplayGroup;
    Label* mXScaleLab;
    Label* mYScaleLab;
    QSlider* mXSlider;
    QSlider* mYSlider;
    LineEdit* mCurrentXMinEdit;
    LineEdit* mCurrentXMaxEdit;
    
    QFont mFont;
    QPushButton* mFontBut;
    QSpinBox* mThicknessSpin;
    QSpinBox* mOpacitySpin;
    QComboBox* mRenderCombo;
    
    
    Label* mChainsTitle;
    QWidget* mChainsGroup;
    CheckBox* mAllChainsCheck;
    QList<CheckBox*> mCheckChainChecks;
    QList<RadioButton*> mChainRadios;
    
    Label* mResultsTitle;
    QWidget* mResultsGroup;
    RadioButton* mDataThetaRadio;
    
    CheckBox* mDataCalibCheck;
    CheckBox* mWiggleCheck;
    RadioButton* mDataSigmaRadio;
    
    Label* mPostDistOptsTitle;
    QWidget* mPostDistGroup;
    Label* mThreshLab;
    CheckBox* mCredibilityCheck;
    LineEdit* mHPDEdit;
    Label* mFFTLenLab;
    QComboBox* mFFTLenCombo;
    Label* mBandwidthLab;
    LineEdit* mBandwidthEdit;
    Button* mUpdateDisplay;
    
    Label* labFont;
    Label* labThickness;
    Label* labOpacity;
    Label* labRendering;

    int mComboH;
    
    QMap<int, QPair<float, float>> mZooms;

    //propreties
    GraphViewResults::TypeGraph mCurrentTypeGraph;
    float mBandwidthUsed;
    float mThresholdUsed;
    int mNumberOfGraph;
};

#endif
