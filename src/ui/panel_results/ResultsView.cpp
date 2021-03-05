/* ---------------------------------------------------------------------

Copyright or © or Copr. CNRS	2014 - 2018

Authors :
	Philippe LANOS
	Helori LANOS
 	Philippe DUFRESNE

This software is a computer program whose purpose is to
create chronological models of archeological data using Bayesian statistics.

This software is governed by the CeCILL V2.1 license under French law and
abiding by the rules of distribution of free software.  You can  use,
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability.

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or
data to be ensured and,  more generally, to use and operate it in the
same conditions as regards security.

The fact that you are presently reading this means that you have had
knowledge of the CeCILL V2.1 license and that you accept its terms.
--------------------------------------------------------------------- */

#include "ResultsView.h"

#include "GraphView.h"
#include "GraphViewDate.h"
#include "GraphViewEvent.h"
#include "GraphViewPhase.h"
#include "GraphViewTempo.h"
#include "GraphViewAlpha.h"
#include "GraphViewCurve.h"

#include "Tabs.h"
#include "Ruler.h"
#include "Marker.h"

#include "Date.h"
#include "Event.h"
#include "EventKnown.h"
#include "Phase.h"

#include "Label.h"
#include "Button.h"
#include "LineEdit.h"
#include "CheckBox.h"
#include "RadioButton.h"
#include "Painting.h"

#include "MainWindow.h"
#include "Project.h"

#include "QtUtilities.h"
#include "StdUtilities.h"
#include "ModelUtilities.h"
#include "DoubleValidator.h"

#include "../PluginAbstract.h"

#include "AppSettings.h"

#include "ModelChronocurve.h"

#include <QtWidgets>
#include <iostream>
#include <QtSvg>
#include <QFontDialog>



ResultsView::ResultsView(QWidget* parent, Qt::WindowFlags flags):QWidget(parent, flags),
mModel(nullptr),

mMargin(5),
mOptionsW(250),
mMarginLeft(40),
mMarginRight(40),

mCurrentTypeGraph(GraphViewResults::ePostDistrib),
mCurrentVariable(GraphViewResults::eTheta),

mHasPhases(false),
mResultZoomX(1.),
mResultMinX(0.),
mResultMaxX(0.),
mResultCurrentMinX(0.),
mResultCurrentMaxX(0.),

//mResultMaxVariance(1000.),
//mResultMaxDuration(0.),

mMajorScale(100),
mMinorCountScale(4),

mCurrentPage(0),
mGraphsPerPage(APP_SETTINGS_DEFAULT_SHEET),
mMaximunNumberOfVisibleGraph(0)
{
    setMouseTracking(true);

    // -----------------------------------------------------------------
    //  Left part : Tabs, Ruler, Stack
    // -----------------------------------------------------------------
    mGraphTypeTabs = new Tabs(this);
    mGraphTypeTabs->addTab(tr("Posterior Distrib."));
    mGraphTypeTabs->addTab(tr("History Plot"));
    mGraphTypeTabs->addTab(tr("Acceptance Rate"));
    mGraphTypeTabs->addTab(tr("Autocorrelation"));
    mGraphTypeTabs->setTab(0, false);
    mGraphTypeTabs->setFixedHeight(mGraphTypeTabs->tabHeight());

    mRuler = new Ruler(this);
    mRuler->setMarginLeft(mMarginLeft);
    mRuler->setMarginRight(mMarginRight);

    mMarker = new Marker(this);

    mEventsScrollArea = new QScrollArea(this);
    mEventsScrollArea->setMouseTracking(true);
    QWidget* eventsWidget = new QWidget();
    eventsWidget->setMouseTracking(true);
    mEventsScrollArea->setWidget(eventsWidget);

    mPhasesScrollArea = new QScrollArea(this);
    mPhasesScrollArea->setMouseTracking(true);
    QWidget* phasesWidget = new QWidget();
    phasesWidget->setMouseTracking(true);
    mPhasesScrollArea->setWidget(phasesWidget);

    mTempoScrollArea = new QScrollArea(this);
    mTempoScrollArea->setMouseTracking(true);
    QWidget* tempoWidget = new QWidget();
    tempoWidget->setMouseTracking(true);
    mTempoScrollArea->setWidget(tempoWidget);

    mCurveScrollArea = new QScrollArea(this);
    mCurveScrollArea->setMouseTracking(true);
    QWidget* curveWidget = new QWidget();
    curveWidget->setMouseTracking(true);
    mCurveScrollArea->setWidget(curveWidget);

    connect(mGraphTypeTabs, static_cast<void (Tabs::*)(const int&)>(&Tabs::tabClicked), this, &ResultsView::applyGraphTypeTab);
    connect(mRuler, &Ruler::positionChanged, this, &ResultsView::applyRuler);


    // -----------------------------------------------------------------
    //  Right part
    // -----------------------------------------------------------------
    mOptionsScroll = new QScrollArea(this);
    mOptionsWidget = new QWidget();
    mOptionsScroll->setWidget(mOptionsWidget);
    int h = 16;

    // -----------------------------------------------------------------
    //  Results Group (if graph list tab = events or phases)
    // -----------------------------------------------------------------
    mResultsGroup = new QWidget();

    mEventsfoldCheck = new CheckBox(tr("Unfold Events"), mResultsGroup);
    mEventsfoldCheck->setFixedHeight(h);
    mEventsfoldCheck->setToolTip(tr("Display phases' events"));

    mDatesfoldCheck = new CheckBox(tr("Unfold Data"), mResultsGroup);
    mDatesfoldCheck->setFixedHeight(h);
    mDatesfoldCheck->setToolTip(tr("Display Events' data"));

    mDataThetaRadio = new RadioButton(tr("Calendar Dates"), mResultsGroup);
    mDataThetaRadio->setFixedHeight(h);
    mDataThetaRadio->setChecked(true);

    mDataSigmaRadio = new RadioButton(tr("Ind. Std. Deviations"), mResultsGroup);
    mDataSigmaRadio->setFixedHeight(h);
    
    mDataVGRadio = new RadioButton(tr("Variance G"), mResultsGroup);
    mDataVGRadio->setFixedHeight(h);

    mDataCalibCheck = new CheckBox(tr("Individual Calib. Dates"), mResultsGroup);
    mDataCalibCheck->setFixedHeight(h);
    mDataCalibCheck->setChecked(true);

    mWiggleCheck = new CheckBox(tr("Wiggle shifted"), mResultsGroup);
    mWiggleCheck->setFixedHeight(h);

    mStatCheck = new CheckBox(tr("Show Stat."), mResultsGroup);
    mStatCheck->setFixedHeight(h);
    mStatCheck->setToolTip(tr("Display numerical results computed on posterior densities below all graphs."));

    QVBoxLayout* resultsGroupLayout = new QVBoxLayout();
    resultsGroupLayout->setContentsMargins(10, 10, 10, 10);
    resultsGroupLayout->setSpacing(15);
    resultsGroupLayout->addWidget(mDataThetaRadio);
    resultsGroupLayout->addWidget(mDataSigmaRadio);
    resultsGroupLayout->addWidget(mDataVGRadio);
    resultsGroupLayout->addWidget(mEventsfoldCheck);
    resultsGroupLayout->addWidget(mDatesfoldCheck);
    resultsGroupLayout->addWidget(mDataCalibCheck);
    resultsGroupLayout->addWidget(mWiggleCheck);
    resultsGroupLayout->addWidget(mStatCheck);
    mResultsGroup->setLayout(resultsGroupLayout);


    // -----------------------------------------------------------------
    //  Tempo Group (if graph list tab = duration)
    // -----------------------------------------------------------------
    mTempoGroup = new QWidget();

    mDurationRadio = new RadioButton(tr("Phase Duration"));
    mDurationRadio->setFixedHeight(h);
    mDurationRadio->setChecked(true);

    mTempoRadio = new RadioButton(tr("Phase Tempo"), mTempoGroup);
    mTempoRadio->setFixedHeight(h);

    mTempoCredCheck = new CheckBox(tr("Tempo Cred."), mTempoGroup);
    mTempoCredCheck->setFixedHeight(h);

    mTempoErrCheck = new CheckBox(tr("Tempo Error"), mTempoGroup);
    mTempoErrCheck->setFixedHeight(h);

    mActivityRadio = new RadioButton(tr("Phase Activity"), mTempoGroup);
    mActivityRadio->setFixedHeight(h);

    mTempoStatCheck = new CheckBox(tr("Show Tempo Stat."), mTempoGroup);
    mTempoStatCheck->setFixedHeight(h);
    mTempoStatCheck->setToolTip(tr("Display numerical results computed on posterior densities below all graphs."));

    QVBoxLayout* tempoGroupLayout = new QVBoxLayout();
    tempoGroupLayout->setContentsMargins(10, 10, 10, 10);
    tempoGroupLayout->setSpacing(15);
    tempoGroupLayout->addWidget(mDurationRadio);
    tempoGroupLayout->addWidget(mTempoRadio);
    tempoGroupLayout->addWidget(mTempoCredCheck);
    tempoGroupLayout->addWidget(mTempoErrCheck);
    tempoGroupLayout->addWidget(mActivityRadio);
    tempoGroupLayout->addWidget(mTempoStatCheck);
    mTempoGroup->setLayout(tempoGroupLayout);

    // -----------------------------------------------------------------
    //  Curves Group (if graph list tab = curve)
    // -----------------------------------------------------------------
    mCurvesGroup = new QWidget();
    
    mCurveGRadio = new RadioButton(tr("G"));
    mCurveGRadio->setFixedHeight(h);
    mCurveGRadio->setChecked(true);
    
    mCurveErrorCheck = new CheckBox(tr("Error envelope"));
    mCurveErrorCheck->setFixedHeight(h);
    mCurveErrorCheck->setChecked(false);
    
    mCurvePointsCheck = new CheckBox(tr("Ref points"));
    mCurvePointsCheck->setFixedHeight(h);
    mCurvePointsCheck->setChecked(false);

    mCurveGPRadio = new RadioButton(tr("G Prime"));
    mCurveGPRadio->setFixedHeight(h);
    
    mCurveGSRadio = new RadioButton(tr("G Second"));
    mCurveGSRadio->setFixedHeight(h);
    
    mAlphaRadio = new RadioButton(tr("Alpha"));
    mAlphaRadio->setFixedHeight(h);
    
    QVBoxLayout* curveGroupLayout = new QVBoxLayout();
    curveGroupLayout->setContentsMargins(10, 10, 10, 10);
    curveGroupLayout->setSpacing(15);
    curveGroupLayout->addWidget(mCurveGRadio);
    curveGroupLayout->addWidget(mCurveErrorCheck);
    curveGroupLayout->addWidget(mCurvePointsCheck);
    curveGroupLayout->addWidget(mCurveGPRadio);
    curveGroupLayout->addWidget(mCurveGSRadio);
    curveGroupLayout->addWidget(mAlphaRadio);
    mCurvesGroup->setLayout(curveGroupLayout);

    // -----------------------------------------------------------------
    //  Connections
    // -----------------------------------------------------------------
    connect(mDataThetaRadio, &RadioButton::clicked, this, &ResultsView::applyCurrentVariable);
    connect(mDataSigmaRadio, &RadioButton::clicked, this, &ResultsView::applyCurrentVariable);
    connect(mDataVGRadio, &RadioButton::clicked, this, &ResultsView::applyCurrentVariable);
    connect(mDurationRadio, &RadioButton::clicked, this, &ResultsView::applyCurrentVariable);
    connect(mTempoRadio, &RadioButton::clicked, this, &ResultsView::applyCurrentVariable);
    connect(mActivityRadio, &RadioButton::clicked, this, &ResultsView::applyCurrentVariable);

    connect(mEventsfoldCheck, &CheckBox::clicked, this, &ResultsView::applyUnfoldEvents);
    connect(mDatesfoldCheck, &CheckBox::clicked, this, &ResultsView::applyUnfoldDates);

    connect(mDataCalibCheck, &CheckBox::clicked, this, &ResultsView::updateCurvesToShow);
    connect(mWiggleCheck, &CheckBox::clicked, this, &ResultsView::updateCurvesToShow);

    connect(mStatCheck, &CheckBox::clicked, this, &ResultsView::showInfos);
    connect(mTempoStatCheck, &CheckBox::clicked, this, &ResultsView::showInfos);

    connect(mTempoCredCheck, &CheckBox::clicked, this, &ResultsView::updateCurvesToShow);
    connect(mTempoErrCheck, &CheckBox::clicked, this, &ResultsView::updateCurvesToShow);
    
    connect(mCurveGRadio, &CheckBox::clicked, this, &ResultsView::applyCurrentVariable);
    connect(mCurveGPRadio, &CheckBox::clicked, this, &ResultsView::applyCurrentVariable);
    connect(mCurveGSRadio, &CheckBox::clicked, this, &ResultsView::applyCurrentVariable);
    connect(mAlphaRadio, &CheckBox::clicked, this, &ResultsView::applyCurrentVariable);
    connect(mCurveErrorCheck, &CheckBox::clicked, this, &ResultsView::updateCurvesToShow);
    connect(mCurvePointsCheck, &CheckBox::clicked, this, &ResultsView::updateCurvesToShow);

    // -----------------------------------------------------------------
    //  Graph List tab (has to be created after mResultsGroup and mTempoGroup)
    // -----------------------------------------------------------------
    mGraphListTab = new Tabs();
    mGraphListTab->setFixedHeight(mGraphListTab->tabHeight());
    mGraphListTab->addTab(tr("Events"));
    mGraphListTab->addTab(tr("Phases"));
    mGraphListTab->addTab(tr("Tempo"));
    mGraphListTab->addTab(tr("Curve"));
    mGraphListTab->setFixedHeight(mGraphListTab->tabHeight());

    connect(mGraphListTab, static_cast<void (Tabs::*)(const int&)>(&Tabs::tabClicked), this, &ResultsView::applyGraphListTab);

    // -----------------------------------------------------------------
    //  Tabs : Display / Distrib. Options
    // -----------------------------------------------------------------
    mTabDisplayMCMC = new Tabs();
    mTabDisplayMCMC->setFixedHeight(mTabDisplayMCMC->tabHeight());

    mTabDisplayMCMC->addTab(tr("Display"));
    mTabDisplayMCMC->addTab(tr("Distrib. Options"));

    // Necessary to reposition all elements inside the selected tab :
    connect(mTabDisplayMCMC, static_cast<void (Tabs::*)(const int&)>(&Tabs::tabClicked), this, &ResultsView::applyDisplayTab);

    // -----------------------------------------------------------------
    //  Display Options layout
    // -----------------------------------------------------------------
    mTabDisplay = new QWidget();
    mSpanGroup  = new QWidget();
    h = 20;

    mSpanTitle = new Label(tr("Span Options"), mTabDisplay);
    mSpanTitle->setFixedHeight(25);
    mSpanTitle->setIsTitle(true);

    mDisplayStudyBut = new Button(tr("Study Period Display"), mSpanGroup);
    mDisplayStudyBut->setFixedHeight(25);
    mDisplayStudyBut->setToolTip(tr("Restore view with the study period span"));

    mSpanLab = new QLabel(tr("Span"), mSpanGroup);
    mSpanLab->setFixedHeight(h);
    //mSpanLab->setAdjustText(false);

    mCurrentXMinEdit = new LineEdit(mSpanGroup);
    mCurrentXMinEdit->setFixedHeight(h);
    mCurrentXMinEdit->setToolTip(tr("Enter a minimal value to display the curves"));

    mCurrentXMaxEdit = new LineEdit(mSpanGroup);
    mCurrentXMaxEdit->setFixedHeight(h);
    mCurrentXMaxEdit->setToolTip(tr("Enter a maximal value to display the curves"));

    mXLab = new QLabel(tr("X"), mSpanGroup);
    mXLab->setFixedHeight(h);
    mXLab->setAlignment(Qt::AlignCenter);
    //mXLab->setAdjustText(false);

    mXSlider = new QSlider(Qt::Horizontal, mSpanGroup);
    mXSlider->setFixedHeight(h);
    mXSlider->setRange(-100, 100);
    mXSlider->setTickInterval(1);
    mXSlider->setValue(0);

    mXSpin = new QDoubleSpinBox(mSpanGroup);
    mXSpin->setFixedHeight(h);
    mXSpin->setRange(pow(10., double (mXSlider->minimum()/100.)),pow(10., double (mXSlider->maximum()/100.)));
    mXSpin->setSingleStep(.01);
    mXSpin->setDecimals(3);
    mXSpin->setValue(sliderToZoom(mXSlider->value()));
    mXSpin->setToolTip(tr("Enter zoom value to magnify the curves on X span"));

    mMajorScaleLab = new QLabel(tr("Major Interval"), mSpanGroup);
    mMajorScaleLab->setFixedHeight(h);
    mMajorScaleLab->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    mMajorScaleEdit = new LineEdit(mSpanGroup);
    mMajorScaleEdit->setFixedHeight(h);
    mMajorScaleEdit->setText(QString::number(mMajorScale));
    mMajorScaleEdit->setToolTip(tr("Enter a interval for the main division of the axes under the curves, upper than 1"));

    mMinorScaleLab = new Label(tr("Minor Interval Count"), mSpanGroup);
    mMinorScaleLab->setFixedHeight(h);
    mMinorScaleLab->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    mMinorScaleEdit = new LineEdit(mSpanGroup);
    mMinorScaleEdit->setFixedHeight(h);
    mMinorScaleEdit->setText(QString::number(mMinorCountScale));
    mMinorScaleEdit->setToolTip(tr("Enter a interval for the subdivision of the Major Interval for the scale under the curves, upper than 1"));

    connect(mDisplayStudyBut, static_cast<void (Button::*)(bool)>(&Button::clicked), this, &ResultsView::applyStudyPeriod);
    connect(mCurrentXMinEdit, &LineEdit::editingFinished, this, &ResultsView::applyXRange);
    connect(mCurrentXMaxEdit, &LineEdit::editingFinished, this, &ResultsView::applyXRange);
    connect(mXSlider, &QSlider::valueChanged, this, &ResultsView::applyXSlider);
    connect(mXSpin, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &ResultsView::applyXSpin);
    connect(mMajorScaleEdit, static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textEdited), this, &ResultsView::applyXIntervals);
    connect(mMinorScaleEdit, static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textEdited), this, &ResultsView::applyXIntervals);

    QHBoxLayout* spanLayout0 = new QHBoxLayout();
    spanLayout0->setContentsMargins(0, 0, 0, 0);
    spanLayout0->addWidget(mDisplayStudyBut);

    QHBoxLayout* spanLayout1 = new QHBoxLayout();
    spanLayout1->setContentsMargins(0, 0, 0, 0);
    //spanLayout1->setSpacing(mMargin);
    spanLayout1->addWidget(mCurrentXMinEdit);
    spanLayout1->addWidget(mSpanLab);
    spanLayout1->addWidget(mCurrentXMaxEdit);

    QHBoxLayout* spanLayout2 = new QHBoxLayout();
    spanLayout2->setContentsMargins(0, 0, 0, 0);
    //spanLayout2->setSpacing(mMargin);
    spanLayout2->addWidget(mXLab);
    spanLayout2->addWidget(mXSlider);
    spanLayout2->addWidget(mXSpin);

    QHBoxLayout* spanLayout3 = new QHBoxLayout();
    spanLayout3->setContentsMargins(0, 0, 0, 0);
    //spanLayout3->setSpacing(mMargin);
    spanLayout3->addWidget(mMajorScaleLab);
    spanLayout3->addWidget(mMajorScaleEdit);

    QHBoxLayout* spanLayout4 = new QHBoxLayout();
    spanLayout4->setContentsMargins(0, 0, 0, 0);
    //spanLayout4->setSpacing(mMargin);
    spanLayout4->addWidget(mMinorScaleLab);
    spanLayout4->addWidget(mMinorScaleEdit);

    QVBoxLayout* spanLayout = new QVBoxLayout();
    spanLayout->setContentsMargins(10, 10, 10, 10);
    spanLayout->setSpacing(5);
    spanLayout->addLayout(spanLayout0);
    spanLayout->addLayout(spanLayout1);
    spanLayout->addLayout(spanLayout2);
    spanLayout->addLayout(spanLayout3);
    spanLayout->addLayout(spanLayout4);

    mSpanGroup->setLayout(spanLayout);

    // ------------------------------------
    //  Graphic Options
    // ------------------------------------
    mGraphicTitle = new Label(tr("Graphic Options"), mTabDisplay);
    mGraphicTitle->setFixedHeight(25);
    mGraphicTitle->setIsTitle(true);

    mGraphicGroup = new QWidget();

    mYLab = new QLabel(tr("Y"), mGraphicGroup);
    mYLab->setAlignment(Qt::AlignCenter);
    //mYLab->setAdjustText(false);

    mYSlider = new QSlider(Qt::Horizontal, mGraphicGroup);
    mYSlider->setRange(10, 500);
    mYSlider->setTickInterval(1);
    mYSlider->setValue(100);

    mYSpin = new QSpinBox(mGraphicGroup);
    mYSpin->setRange(mYSlider->minimum(), mYSlider->maximum());
    mYSpin->setValue(mYSlider->value());
    mYSpin->setToolTip(tr("Enter zoom value to magnify the curves on Y scale"));

    mLabFont = new QLabel(tr("Font"), mGraphicGroup);
    mLabFont->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    mFontBut = new Button(font().family() + ", " + QString::number(font().pointSizeF()), mGraphicGroup);
    mFontBut->setFixedHeight(25);
    mFontBut->setToolTip(tr("Click to change the font on the drawing"));

    mLabThickness = new QLabel(tr("Thickness"), mGraphicGroup);
    mLabThickness->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    mThicknessCombo = new QComboBox(mGraphicGroup);
    mThicknessCombo->addItem("1 px");
    mThicknessCombo->addItem("2 px");
    mThicknessCombo->addItem("3 px");
    mThicknessCombo->addItem("4 px");
    mThicknessCombo->addItem("5 px");
    mThicknessCombo->setToolTip(tr("Select to change the thickness of the drawing"));

    mLabOpacity = new QLabel(tr("Opacity"), mGraphicGroup);
    mLabOpacity->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    mOpacityCombo = new QComboBox(mGraphicGroup);
    mOpacityCombo->addItem("0 %");
    mOpacityCombo->addItem("10 %");
    mOpacityCombo->addItem("20 %");
    mOpacityCombo->addItem("30 %");
    mOpacityCombo->addItem("40 %");
    mOpacityCombo->addItem("50 %");
    mOpacityCombo->addItem("60 %");
    mOpacityCombo->addItem("70 %");
    mOpacityCombo->addItem("80 %");
    mOpacityCombo->addItem("90 %");
    mOpacityCombo->addItem("100 %");
    mOpacityCombo->setToolTip(tr("Select to change the opacity of the drawing"));
    mOpacityCombo->setCurrentIndex(5);

    connect(mYSlider, &QSlider::valueChanged, this, &ResultsView::applyYSlider);
    connect(mYSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ResultsView::applyYSpin);
    connect(mFontBut, &QPushButton::clicked, this, &ResultsView::applyFont);
    connect(mThicknessCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ResultsView::applyThickness);
    connect(mOpacityCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ResultsView::applyOpacity);

    QHBoxLayout* graphicLayout1 = new QHBoxLayout();
    graphicLayout1->setContentsMargins(0, 0, 0, 0);
    //graphicLayout1->setSpacing(mMargin);
    graphicLayout1->addWidget(mYLab);
    graphicLayout1->addWidget(mYSlider);
    graphicLayout1->addWidget(mYSpin);

    QHBoxLayout* graphicLayout2 = new QHBoxLayout();
    graphicLayout2->setContentsMargins(0, 0, 0, 0);
    //graphicLayout2->setSpacing(mMargin);
    graphicLayout2->addWidget(mLabFont);
    graphicLayout2->addWidget(mFontBut);

    QHBoxLayout* graphicLayout3 = new QHBoxLayout();
    graphicLayout3->setContentsMargins(0, 0, 0, 0);
    //graphicLayout3->setSpacing(mMargin);
    graphicLayout3->addWidget(mLabThickness);
    graphicLayout3->addWidget(mThicknessCombo);

    QHBoxLayout* graphicLayout4 = new QHBoxLayout();
    graphicLayout4->setContentsMargins(0, 0, 0, 0);
    //graphicLayout4->setSpacing(mMargin);
    graphicLayout4->addWidget(mLabOpacity);
    graphicLayout4->addWidget(mOpacityCombo);

    QVBoxLayout* graphicLayout = new QVBoxLayout();
    graphicLayout->setContentsMargins(10, 10, 10, 10);
    graphicLayout->setSpacing(5);
    graphicLayout->addLayout(graphicLayout1);
    graphicLayout->addLayout(graphicLayout2);
    graphicLayout->addLayout(graphicLayout3);
    graphicLayout->addLayout(graphicLayout4);

    mGraphicGroup->setLayout(graphicLayout);

    QVBoxLayout* displayLayout = new QVBoxLayout();
    displayLayout->setContentsMargins(0, 0, 0, 0);
    displayLayout->setSpacing(0);
    displayLayout->addWidget(mSpanTitle);
    displayLayout->addWidget(mSpanGroup);
    displayLayout->addWidget(mGraphicTitle);
    displayLayout->addWidget(mGraphicGroup);
    mTabDisplay->setLayout(displayLayout);

    // ------------------------------------
    //  MCMC Chains
    //  Note : mChainChecks and mChainRadios are populated by createChainsControls()
    // ------------------------------------
    mChainsTitle = new Label(tr("MCMC Chains"));
    mChainsTitle->setFixedHeight(25);
    mChainsTitle->setIsTitle(true);

    mChainsGroup = new QWidget();

    mAllChainsCheck = new CheckBox(tr("Chain Concatenation"), mChainsGroup);
    mAllChainsCheck->setFixedHeight(16);
    mAllChainsCheck->setChecked(true);

    connect(mAllChainsCheck, &CheckBox::clicked, this, &ResultsView::updateCurvesToShow);

    QVBoxLayout* chainsLayout = new QVBoxLayout();
    chainsLayout->setContentsMargins(10, 10, 10, 10);
    chainsLayout->setSpacing(15);
    chainsLayout->addWidget(mAllChainsCheck);
    mChainsGroup->setLayout(chainsLayout);

    // ------------------------------------
    //  Density Options
    // ------------------------------------
    mDensityOptsTitle = new Label(tr("Density Options"));
    mDensityOptsTitle->setFixedHeight(25);
    mDensityOptsTitle->setIsTitle(true);

    mDensityOptsGroup = new QWidget();

    mCredibilityCheck = new CheckBox(tr("Show Confidence Bar"), mDensityOptsGroup);
    mCredibilityCheck->setFixedHeight(16);
    mCredibilityCheck->setChecked(true);

    mThreshLab = new Label(tr("Confidence Level (%)"), mDensityOptsGroup);
    mThreshLab->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    mThresholdEdit = new LineEdit(mDensityOptsGroup);
    DoubleValidator* percentValidator = new DoubleValidator();
    percentValidator->setBottom(0.0);
    percentValidator->setTop(100.0);
    percentValidator->setDecimals(1);
    mThresholdEdit->setValidator(percentValidator);

    mFFTLenLab = new Label(tr("Grid Length"), mDensityOptsGroup);
    mFFTLenLab->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    mFFTLenCombo = new QComboBox(mDensityOptsGroup);
    mFFTLenCombo->addItem("32");
    mFFTLenCombo->addItem("64");
    mFFTLenCombo->addItem("128");
    mFFTLenCombo->addItem("256");
    mFFTLenCombo->addItem("512");
    mFFTLenCombo->addItem("1024");
    mFFTLenCombo->addItem("2048");
    mFFTLenCombo->addItem("4096");
    mFFTLenCombo->addItem("8192");
    mFFTLenCombo->addItem("16384");

    mBandwidthLab = new Label(tr("Bandwidth Const."), mDensityOptsGroup);
    mBandwidthLab->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    mBandwidthSpin = new QDoubleSpinBox(mDensityOptsGroup);
    mBandwidthSpin->setDecimals(2);
    
    connect(mCredibilityCheck, &CheckBox::clicked, this, &ResultsView::updateCurvesToShow);
    connect(mFFTLenCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ResultsView::applyFFTLength);
    connect(mBandwidthSpin, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &ResultsView::applyBandwidth);
    connect(mThresholdEdit, &LineEdit::editingFinished, this, &ResultsView::applyThreshold);

    QHBoxLayout* densityLayout0 = new QHBoxLayout();
    densityLayout0->setContentsMargins(0, 0, 0, 0);
    //densityLayout0->setSpacing(mMargin);
    densityLayout0->addWidget(mCredibilityCheck);

    QHBoxLayout* densityLayout1 = new QHBoxLayout();
    densityLayout1->setContentsMargins(0, 0, 0, 0);
    //densityLayout1->setSpacing(mMargin);
    densityLayout1->addWidget(mThreshLab);
    densityLayout1->addWidget(mThresholdEdit);

    QHBoxLayout* densityLayout2 = new QHBoxLayout();
    densityLayout2->setContentsMargins(0, 0, 0, 0);
    //densityLayout2->setSpacing(mMargin);
    densityLayout2->addWidget(mFFTLenLab);
    densityLayout2->addWidget(mFFTLenCombo);

    QHBoxLayout* densityLayout3 = new QHBoxLayout();
    densityLayout3->setContentsMargins(0, 0, 0, 0);
    //densityLayout3->setSpacing(mMargin);
    densityLayout3->addWidget(mBandwidthLab);
    densityLayout3->addWidget(mBandwidthSpin);
    
    QVBoxLayout* densityLayout = new QVBoxLayout();
    densityLayout->setContentsMargins(10, 10, 10, 10);
    densityLayout->setSpacing(5);
    densityLayout->addLayout(densityLayout0);
    densityLayout->addLayout(densityLayout1);
    densityLayout->addLayout(densityLayout2);
    densityLayout->addLayout(densityLayout3);
    mDensityOptsGroup->setLayout(densityLayout);

    // ------------------------------------
    //  Tab MCMC (Distrib. Options)
    // ------------------------------------
    mTabMCMC = new QWidget();

    QVBoxLayout* mcmcLayout = new QVBoxLayout();
    mcmcLayout->setContentsMargins(0, 0, 0, 0);
    mcmcLayout->setSpacing(0);
    mcmcLayout->addWidget(mChainsTitle);
    mcmcLayout->addWidget(mChainsGroup);
    mcmcLayout->addWidget(mDensityOptsTitle);
    mcmcLayout->addWidget(mDensityOptsGroup);
    mTabMCMC->setLayout(mcmcLayout);

    // ------------------------------------
    //  Pagination / Saving
    // ------------------------------------
    mPageWidget = new QWidget();
    mToolsWidget = new QWidget();

    mTabPageSaving = new Tabs();
    mTabPageSaving->setFixedHeight(mTabPageSaving->tabHeight());
    mTabPageSaving->addTab(tr("Page"));
    mTabPageSaving->addTab(tr("Saving"));

    connect(mTabPageSaving, static_cast<void (Tabs::*)(const int&)>(&Tabs::tabClicked), this, &ResultsView::applyPageSavingTab);

    // ------------------------------------
    //  Pagination
    // ------------------------------------
    mPageEdit = new LineEdit(mPageWidget);
    mPageEdit->setEnabled(false);
    mPageEdit->setReadOnly(true);
    mPageEdit->setAlignment(Qt::AlignCenter);
    mPageEdit->setText(QString::number(mMaximunNumberOfVisibleGraph));

    mPreviousPageBut = new Button(tr("Prev."), mPageWidget);
    mPreviousPageBut->setCheckable(false);
    mPreviousPageBut->setFlatHorizontal();
    mPreviousPageBut->setToolTip(tr("Display previous data"));
    mPreviousPageBut->setIconOnly(false);

    mNextPageBut  = new Button(tr("Next"), mPageWidget);
    mNextPageBut->setCheckable(false);
    mNextPageBut->setFlatHorizontal();
    mNextPageBut->setToolTip(tr("Display next data"));
    mNextPageBut->setIconOnly(false);

    mGraphsPerPageLab = new Label(tr("Nb Densities / Page"), mPageWidget);
    mGraphsPerPageLab->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    mGraphsPerPageSpin = new QSpinBox(mPageWidget);
    mGraphsPerPageSpin->setRange(1, 100);
    mGraphsPerPageSpin->setValue(mGraphsPerPage);
    mGraphsPerPageSpin->setToolTip(tr("Enter the maximum densities to display on a sheet"));
    
    QHBoxLayout* pageNavigationLayout = new QHBoxLayout();
    pageNavigationLayout->setContentsMargins(0, 0, 0, 0);
    pageNavigationLayout->setSpacing(0);
    pageNavigationLayout->addWidget(mPreviousPageBut);
    pageNavigationLayout->addWidget(mPageEdit);
    pageNavigationLayout->addWidget(mNextPageBut);
    
    QHBoxLayout* pageCountLayout = new QHBoxLayout();
    pageCountLayout->setContentsMargins(0, 0, 0, 0);
    pageCountLayout->setSpacing(0);
    pageCountLayout->addWidget(mGraphsPerPageLab);
    pageCountLayout->addWidget(mGraphsPerPageSpin);
    
    QVBoxLayout* pageLayout = new QVBoxLayout();
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);
    pageLayout->addLayout(pageNavigationLayout);
    pageLayout->addLayout(pageCountLayout);
    mPageWidget->setLayout(pageLayout);

    connect(mPreviousPageBut, &Button::pressed, this, &ResultsView::applyPreviousPage);
    connect(mNextPageBut, &Button::pressed, this, &ResultsView::applyNextPage);
    connect(mGraphsPerPageSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ResultsView::applyGraphsPerPage);

    // ------------------------------------
    //  Tools Buttons (multiple graphs)
    // ------------------------------------
    mExportImgBut = new Button(tr("Capture"), mToolsWidget);
    mExportImgBut->setFlatHorizontal();
    mExportImgBut->setIcon(QIcon(":picture_save.png"));
    mExportImgBut->setToolTip(tr("Save all currently visible results as an image.<br><u>Note</u> : If you want to copy textual results, see the Log tab."));

    mExportResults = new Button(tr("Results"), mToolsWidget);
    mExportResults->setFlatHorizontal();
    mExportResults->setIcon(QIcon(":csv.png"));
    mExportResults->setToolTip(tr("Export all result in several files"));

    connect(mExportImgBut, static_cast<void (Button::*)(bool)>(&Button::clicked), this, &ResultsView::exportFullImage);
    connect(mExportResults, static_cast<void (Button::*)(bool)>(&Button::clicked), this, &ResultsView::exportResults);

    // ------------------------------------
    //  Tools Buttons (single graph)
    // ------------------------------------
    mImageSaveBut = new Button(tr("Save"), mToolsWidget);
    mImageSaveBut->setIcon(QIcon(":picture_save.png"));
    mImageSaveBut->setFlatVertical();
    mImageSaveBut->setToolTip(tr("Save image as file"));

    mImageClipBut = new Button(tr("Copy"), mToolsWidget);
    mImageClipBut->setIcon(QIcon(":clipboard_graph.png"));
    mImageClipBut->setFlatVertical();
    mImageClipBut->setToolTip(tr("Copy image to clipboard"));

    mResultsClipBut = new Button(tr("Copy"), mToolsWidget);
    mResultsClipBut->setIcon(QIcon(":text.png"));
    mResultsClipBut->setFlatVertical();
    mResultsClipBut->setToolTip(tr("Copy text results to clipboard"));

    mDataSaveBut = new Button(tr("Save"), mToolsWidget);
    mDataSaveBut->setIcon(QIcon(":data.png"));
    mDataSaveBut->setFlatVertical();
    mDataSaveBut->setToolTip(tr("Save graph data to file"));

    connect(mImageSaveBut, static_cast<void (Button::*)(bool)>(&Button::clicked), this, &ResultsView::saveAsImage);
    connect(mImageClipBut, static_cast<void (Button::*)(bool)>(&Button::clicked), this, &ResultsView::imageToClipboard);
    connect(mResultsClipBut, static_cast<void (Button::*)(bool)>(&Button::clicked), this, &ResultsView::resultsToClipboard);
    connect(mDataSaveBut, static_cast<void (Button::*)(bool)>(&Button::clicked), this, &ResultsView::saveGraphData);

    // ---------------------------------------------------------------------------
    //  Right Layout (Options)
    // ---------------------------------------------------------------------------
    QVBoxLayout* optionsLayout = new QVBoxLayout();
    optionsLayout->setContentsMargins(mMargin, mMargin, 0, 0);
    optionsLayout->setSpacing(0);
    optionsLayout->addWidget(mGraphListTab);
    optionsLayout->addWidget(mResultsGroup);
    optionsLayout->addWidget(mTempoGroup);
    optionsLayout->addWidget(mCurvesGroup);
    optionsLayout->addWidget(mTabDisplayMCMC);
    optionsLayout->addWidget(mTabDisplay);
    optionsLayout->addWidget(mTabMCMC);
    optionsLayout->addWidget(mTabPageSaving);
    optionsLayout->addWidget(mPageWidget);
    optionsLayout->addWidget(mToolsWidget);
    optionsLayout->addStretch();
    mOptionsWidget->setLayout(optionsLayout);

    // ---------------------------------------------------------------------------
    //  Inititialize tabs indexes
    // ---------------------------------------------------------------------------
    mGraphTypeTabs->setTab(0, false);
    mGraphListTab->setTab(0, false);
    mTabDisplayMCMC->setTab(0, false);
    mTabPageSaving->setTab(0, false);

    mEventsScrollArea->setVisible(true);
    mPhasesScrollArea->setVisible(false);
    mTempoScrollArea->setVisible(false);
    mCurveScrollArea->setVisible(false);

    GraphViewResults::mHeightForVisibleAxis = 4 * AppSettings::heigthUnit();
    mGraphHeight = GraphViewResults::mHeightForVisibleAxis;

    mMarker->raise();

    updateControls();
}

ResultsView::~ResultsView()
{
    mModel = nullptr;
}

#pragma mark Project & Model

void ResultsView::setProject(Project* project)
{
    /* Starting MCMC calculation does a mModel.clear() at first, and recreate it.
     * Then, it fills its elements (events, ...) with calculated data (trace, ...)
     * If the process is canceled, we only have unfinished data in storage.
     * => The previous nor the new results can be displayed so we must start by clearing the results view! */

    clearResults();
    updateModel(project->mModel);
    connect(project, &Project::mcmcStarted, this, &ResultsView::clearResults);
}

void ResultsView::clearResults()
{
    deleteChainsControls();
    deleteAllGraphsInList(mByEventsGraphs);
    deleteAllGraphsInList(mByPhasesGraphs);
    deleteAllGraphsInList(mByTempoGraphs);
    deleteAllGraphsInList(mByCurveGraphs);
}

void ResultsView::updateModel(Model* model)
{
    if (mModel == nullptr) {
        disconnect(mModel, &Model::newCalculus, this, &ResultsView::generateCurves);
    }

    mModel = model;
    connect(mModel, &Model::newCalculus, this, &ResultsView::generateCurves);

    Scale xScale;
    xScale.findOptimal(mModel->mSettings.mTmin, mModel->mSettings.mTmax, 7);
    mMajorScale = xScale.mark;
    mMinorCountScale = 4;

    mRuler->setRange(mModel->mSettings.getTminFormated(), mModel->mSettings.getTmaxFormated());
    mRuler->setCurrent(mModel->mSettings.getTminFormated(), mModel->mSettings.getTmaxFormated());
    mRuler->setScaleDivision(mMajorScale, mMinorCountScale);

    QLocale locale = QLocale();
    mMajorScaleEdit->setText(locale.toString(mMajorScale));
    mMinorScaleEdit->setText(locale.toString(mMinorCountScale));

    mHasPhases = (mModel->mPhases.size() > 0);

    // ----------------------------------------------------
    //  Create Chains option controls (radio and checkboxes under "MCMC Chains")
    // ----------------------------------------------------
    createChainsControls();
    
    mCurrentVariableMaxX = 0.;
    
    mCurrentTypeGraph = GraphViewResults::ePostDistrib;
    mCurrentVariable = GraphViewResults::eTheta;

    mFFTLenCombo->setCurrentText(QString::number(mModel->getFFTLength()));
    mBandwidthSpin->setValue(mModel->getBandwidth());
    mThresholdEdit->setText(QString::number(mModel->getThreshold()));
    
    applyStudyPeriod();
    updateControls();
    createGraphs();
    updateLayout();
    
    showInfos(false);
}

#pragma mark Layout

void ResultsView::mouseMoveEvent(QMouseEvent* e)
{
    updateMarkerGeometry(e->pos().x());
}

void ResultsView::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);
    updateLayout();
}

void ResultsView::updateMarkerGeometry(const int x)
{
    const int markerXPos = inRange(0, x, mRuler->x() + mRuler->width());
    mMarker->setGeometry(markerXPos, mGraphTypeTabs->height() + mMargin, mMarker->thickness(), height() - mGraphTypeTabs->height() - mMargin);
}

void ResultsView::updateLayout()
{
    // The scroll bar extent (width or height depending on the orientation)
    // depends on the native platform, and must be taken into account.
    const int sbe = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);

    //const QFontMetrics fm(font());

    int leftWidth = width() - mOptionsW - sbe;
    int graphWidth = leftWidth;
    int tabsH = mGraphTypeTabs->tabHeight();
    int rulerH = Ruler::sHeight;
    int stackH = height() - mMargin - tabsH - rulerH;

    if (mStatCheck->isChecked() || mTempoStatCheck->isChecked()) {
        graphWidth = (2./3.) * leftWidth;
    }

    // ----------------------------------------------------------
    //  Left layout
    // ----------------------------------------------------------
    mGraphTypeTabs->setGeometry(mMargin, mMargin, leftWidth, tabsH);
    mRuler->setGeometry(0, mMargin + tabsH, graphWidth, rulerH);

    QRect graphScrollGeometry(0, mMargin + tabsH + rulerH, leftWidth, stackH);

    mEventsScrollArea->setGeometry(graphScrollGeometry);
    mPhasesScrollArea->setGeometry(graphScrollGeometry);
    mTempoScrollArea->setGeometry(graphScrollGeometry);
    mCurveScrollArea->setGeometry(graphScrollGeometry);

    updateGraphsLayout();
    updateMarkerGeometry(mMarker->pos().x());

    // --------------------------------------------------------
    //  Pagination / Saving Tabs
    // --------------------------------------------------------
    int buttonHeight = 40;
    int buttonSide = mOptionsW / 4;

    //mTabPageSaving->resize(mOptionsW, tabsH);
    //mPageWidget->resize(mOptionsW, 2*buttonHeight + mMargin);
    mToolsWidget->resize(mOptionsW, buttonSide);

    // --------------------------------------------------------
    //  Tools layout
    // --------------------------------------------------------
    mImageSaveBut->setGeometry(0, 0, buttonSide, buttonHeight);
    mImageClipBut->setGeometry(buttonSide, 0, buttonSide, buttonHeight);
    mResultsClipBut->setGeometry(2 * buttonSide, 0, buttonSide, buttonHeight);
    mDataSaveBut->setGeometry(3 * buttonSide, 0, buttonSide, buttonHeight);
    
    mExportImgBut->setGeometry(0, 0, 2*buttonSide, buttonHeight);
    mExportResults->setGeometry(2*buttonSide, 0, 2*buttonSide, buttonHeight);
    
    // --------------------------------------------------------
    //  Right layout
    // --------------------------------------------------------
    mOptionsScroll->setGeometry(leftWidth, 0, mOptionsW, height());
    mOptionsWidget->setGeometry(0, 0, mOptionsW - sbe, 800);
}


void ResultsView::updateGraphsLayout()
{
    if (mGraphListTab->currentIndex() == 0) {
        updateGraphsLayout(mEventsScrollArea, mByEventsGraphs);

    } else if (mGraphListTab->currentIndex() == 1) {
        updateGraphsLayout(mPhasesScrollArea, mByPhasesGraphs);

    } else if (mGraphListTab->currentIndex() == 2) {
        updateGraphsLayout(mTempoScrollArea, mByTempoGraphs);

    } else if (mGraphListTab->currentIndex() == 3) {
        updateGraphsLayout(mCurveScrollArea, mByCurveGraphs);
    }
}

void ResultsView::updateGraphsLayout(QScrollArea* scrollArea, QList<GraphViewResults*> graphs)
{
    const int sbe = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    QWidget* widget = scrollArea->widget();

    /*QPalette palette = widget->palette();
    palette.setBrush(QPalette::Background, Qt::blue);
    widget->setPalette(palette);*/

    if (widget) {
        widget->resize(width() - mOptionsW - sbe, graphs.size() * mGraphHeight);

        for (int i = 0; i<graphs.size(); ++i) {
            graphs[i]->setGeometry(0, i * mGraphHeight, width() - mOptionsW - sbe, mGraphHeight);
            graphs[i]->setVisible(true);
            graphs[i]->update();
        }
    }
}

#pragma mark Tabs changes listeners

/**
 * This is called when mGraphTypeTabs is clicked.
 * Changing the "graph type" means switching between "Posterior distrib", "History Plot", "Acceptance Rate" and "Autocorrelation".
 * It only makes sense when working on MH variable.
 *
 * Changing it requires the following steps :
 * 1) updateControls : the available options have to be modified according to the graph type
 * 2) generateCurves : no need to call createGraphs because we are only changing the curves displayed in the current graphs list.
 * 3) updateLayout : always needed at the end to refresh the display
*/
void ResultsView::applyGraphTypeTab()
{
    mCurrentTypeGraph = (GraphViewResults::TypeGraph) mGraphTypeTabs->currentIndex();

    updateControls();
    generateCurves();
    updateLayout();
}

/**
 * This is called when mGraphListTab is clicked.
 * MCMC display options are only available for mCurrentVariable in : eTheta, eSigma, eDuration
 * so mTabDisplayMCMC must be displayed accordingly
 */
void ResultsView::applyGraphListTab()
{
    int currentIndex = mGraphListTab->currentIndex();
    
    // Display the scroll area corresponding to the selected tab :
    mEventsScrollArea->setVisible(currentIndex == 0);
    mPhasesScrollArea->setVisible(currentIndex == 1);
    mTempoScrollArea->setVisible(currentIndex == 2);
    mCurveScrollArea->setVisible(currentIndex == 3);
    
    // Update the current variable to the most appropriate for this list :
    if (currentIndex == 0) {
        mDataThetaRadio->setChecked(true);

    } else if (currentIndex == 1) {
        mDataThetaRadio->setChecked(true);

    } else if (currentIndex == 2) {
        mDurationRadio->setChecked(true);

    } else if (currentIndex == 3) {
        mCurveGRadio->setChecked(true);
    }
    updateCurrentVariable();
    
    // Set the current graph type to Posterior distrib :
    mGraphTypeTabs->setTab(0, false);
    mCurrentTypeGraph = (GraphViewResults::TypeGraph) mGraphTypeTabs->currentIndex();
    
    // Changing the graphs list implies to go back to page 1 :
    mCurrentPage = 0;
    
    updateControls();
    createGraphs();
    updateLayout();
}

void ResultsView::updateCurrentVariable()
{
    if (mGraphListTab->currentIndex() == 0) {
        if (mDataThetaRadio->isChecked()) {
            mCurrentVariable = GraphViewResults::eTheta;

        } else if (mDataSigmaRadio->isChecked()) {
            mCurrentVariable = GraphViewResults::eSigma;

        } else if (mDataVGRadio->isChecked()) {
            mCurrentVariable = GraphViewResults::eVG;
        }

    } else if (mGraphListTab->currentIndex() == 1) {
        if (mDataThetaRadio->isChecked()) {
            mCurrentVariable = GraphViewResults::eTheta;

        } else if (mDataSigmaRadio->isChecked()) {
            mCurrentVariable = GraphViewResults::eSigma;
        }

    } else if (mGraphListTab->currentIndex() == 2) {
        if (mDurationRadio->isChecked()) {
            mCurrentVariable = GraphViewResults::eDuration;

        } else if (mTempoRadio->isChecked()) {
            mCurrentVariable = GraphViewResults::eTempo;
        }

        if (mActivityRadio->isChecked()) {
            mCurrentVariable = GraphViewResults::eActivity;
        }

    } else if (mGraphListTab->currentIndex() == 3) {

        if (mAlphaRadio->isChecked()) {
            mCurrentVariable = GraphViewResults::eAlpha;

        } else {
            mCurrentVariable = GraphViewResults::eG;
        }
    }
}

void ResultsView::applyCurrentVariable()
{
    updateCurrentVariable();
    
    updateControls();
    createGraphs();
    updateLayout();
}

void ResultsView::applyUnfoldEvents()
{
    updateControls();
    createGraphs();
    updateLayout();
}

void ResultsView::applyUnfoldDates()
{
    updateControls();
    createGraphs();
    updateLayout();
}

void ResultsView::applyDisplayTab()
{
    updateControls();
    updateLayout();
}

void ResultsView::applyPageSavingTab()
{
    updateControls();
    updateLayout();
}

#pragma mark Chains controls

void ResultsView::createChainsControls()
{
    if (mModel->mChains.size() != mChainChecks.size())
        deleteChainsControls();

    for (int i=0; i<mModel->mChains.size(); ++i) {
        CheckBox* check = new CheckBox(tr("Chain %1").arg(QString::number(i+1)));
        check->setFixedHeight(16);
        check->setVisible(true);
        mChainChecks.append(check);

        connect(check, &CheckBox::clicked, this, &ResultsView::updateCurvesToShow);

        RadioButton* radio = new RadioButton(tr("Chain %1").arg(QString::number(i+1)));
        radio->setFixedHeight(16);
        radio->setChecked(i == 0);
        radio->setVisible(true);
        mChainRadios.append(radio);

        connect(radio, &RadioButton::clicked, this, &ResultsView::updateCurvesToShow);

        mChainsGroup->layout()->addWidget(check);
        mChainsGroup->layout()->addWidget(radio);
    }
}

void ResultsView::deleteChainsControls()
{
    for (CheckBox*& check : mChainChecks) {
        disconnect(check, &CheckBox::clicked, this, &ResultsView::updateCurvesToShow);
        delete check;
    }
    mChainChecks.clear();

    for (RadioButton*& radio : mChainRadios) {
        disconnect(radio, &RadioButton::clicked, this, &ResultsView::updateCurvesToShow);
        delete radio;
    }
    mChainRadios.clear();
}

#pragma mark Graphs UI

void ResultsView::createGraphs()
{
    if (!mModel) {
        return;
    }

    if (mGraphListTab->currentIndex() == 0) {
        createByEventsGraphs();
    } else if (mGraphListTab->currentIndex() == 1) {
        createByPhasesGraphs();
    } else if (mGraphListTab->currentIndex() == 2) {
        createByTempoGraphs();
    } else if (mGraphListTab->currentIndex() == 3) {
        createByCurveGraph();
    }
    
    generateCurves();
}

void ResultsView::updateTotalGraphs()
{
    if (!mModel) {
        mMaximunNumberOfVisibleGraph = 0;
        return;
    }
    
    int totalGraphs = 0;
    
    if (mGraphListTab->currentIndex() == 0) {
        bool showAllEvents = ! mModel->hasSelectedEvents();
        for (int i = 0; i<mModel->mEvents.size(); ++i) {
            Event* event = mModel->mEvents[i];
            if (event->mIsSelected || showAllEvents) {
                ++totalGraphs;
                
                if (mDatesfoldCheck->isChecked()) {
                    for (int j = 0; j<event->mDates.size(); ++j) {
                        ++totalGraphs;
                    }
                }
            }
        }
    } else if (mGraphListTab->currentIndex() == 1) {
        bool showAllPhases = ! mModel->hasSelectedPhases();

        for (int i=0; i<mModel->mPhases.size(); ++i) {
            Phase* phase = mModel->mPhases[i];
            if (phase->mIsSelected || showAllPhases) {
                ++totalGraphs;
                
                if (mEventsfoldCheck->isChecked())  {
                    for (int j=0; j<phase->mEvents.size(); ++j)  {
                        ++totalGraphs;
                        
                        Event* event = phase->mEvents[j];
                        if (mDatesfoldCheck->isChecked()) {
                            for (int k=0; k<event->mDates.size(); ++k)  {
                                ++totalGraphs;
                            }
                        }
                    }
                }
            }
        }
    } else if (mGraphListTab->currentIndex() == 2) {
        bool showAllPhases = ! mModel->hasSelectedPhases();
        
        for (int i=0; i<mModel->mPhases.size(); ++i) {
            Phase* phase = mModel->mPhases[i];
            if (phase->mIsSelected || showAllPhases) {
                ++totalGraphs;
            }
        }

    } else if (mGraphListTab->currentIndex() == 3) {
        if (mAlphaRadio->isChecked()) {
            ++totalGraphs;

        } else {
            ModelChronocurve* model = modelChronocurve();
            bool hasY = (model->mChronocurveSettings.mProcessType != ChronocurveSettings::eProcessTypeUnivarie);
            bool hasZ = (model->mChronocurveSettings.mProcessType == ChronocurveSettings::eProcessTypeVectoriel);
            
            ++totalGraphs;
            if (hasY) ++totalGraphs;
            if (hasZ) ++totalGraphs;
        }
    }
    
    mMaximunNumberOfVisibleGraph = totalGraphs;
}

/**
 * this method (re-)creates all the events graphs
 */
void ResultsView::createByEventsGraphs()
{
    Q_ASSERT(mModel);

    // ----------------------------------------------------------------------
    //  Disconnect and delete existing graphs
    // ----------------------------------------------------------------------
    deleteAllGraphsInList(mByEventsGraphs);

    // ----------------------------------------------------------------------
    // Show all events unless at least one is selected
    // ----------------------------------------------------------------------
    bool showAllEvents = ! mModel->hasSelectedEvents();

    // ----------------------------------------------------------------------
    //  Iterate through all events and create corresponding graphs
    // ----------------------------------------------------------------------
    QWidget* eventsWidget = mEventsScrollArea->widget();
    int graphIndex = 0;

    for(auto& event : mModel->mEvents) {
    //for(int i = 0; i < mModel->mEvents.size(); ++i) {
      //  Event* event = mModel->mEvents[i];
        if (event->mIsSelected || showAllEvents) {
            if (graphIndexIsInCurrentPage(graphIndex)) {
                GraphViewEvent* graph = new GraphViewEvent(eventsWidget);
                graph->setSettings(mModel->mSettings);
                graph->setMCMCSettings(mModel->mMCMCSettings, mModel->mChains);
                graph->setEvent(event);
                graph->setGraphFont(font());
                graph->setGraphsThickness(mThicknessCombo->currentIndex());
                graph->changeXScaleDivision(mMajorScale, mMinorCountScale);
                graph->setMarginLeft(mMarginLeft);
                graph->setMarginRight(mMarginRight);

                mByEventsGraphs.append(graph);
                connect(graph, &GraphViewResults::selected, this, &ResultsView::updateLayout);
            }
            ++graphIndex;
                
            if (mDatesfoldCheck->isChecked()) {
                for (auto& date : event->mDates) {
               // for (int j = 0; j < event->mDates.size(); ++j) {
                    if (graphIndexIsInCurrentPage(graphIndex)) {
                 //       Date& date = event->mDates[j];

                        GraphViewDate* graph = new GraphViewDate(eventsWidget);
                        graph->setSettings(mModel->mSettings);
                        graph->setMCMCSettings(mModel->mMCMCSettings, mModel->mChains);
                        graph->setDate(&date);
                        graph->setGraphFont(font());
                        graph->setGraphsThickness(mThicknessCombo->currentIndex());
                        graph->changeXScaleDivision(mMajorScale, mMinorCountScale);
                        graph->setColor(event->mColor);
                        graph->setGraphsOpacity(mOpacityCombo->currentIndex()*10);
                        graph->setMarginLeft(mMarginLeft);
                        graph->setMarginRight(mMarginRight);

                        mByEventsGraphs.append(graph);
                        connect(graph, &GraphViewResults::selected, this, &ResultsView::updateLayout);
                    }
                    ++graphIndex;
                }
            }
        }
    }
}

void ResultsView::createByPhasesGraphs()
{
    Q_ASSERT(mModel);

    // ----------------------------------------------------------------------
    //  Disconnect and delete existing graphs
    // ----------------------------------------------------------------------
    deleteAllGraphsInList(mByPhasesGraphs);

    // ----------------------------------------------------------------------
    // Show all, unless at least one is selected
    // ----------------------------------------------------------------------
    bool showAllPhases = ! mModel->hasSelectedPhases();

    // ----------------------------------------------------------------------
    //  Iterate through all, and create corresponding graphs
    // ----------------------------------------------------------------------
    QWidget* phasesWidget = mPhasesScrollArea->widget();
    int graphIndex = 0;

    for (auto& phase : mModel->mPhases) {
   // for (int i = 0; i<mModel->mPhases.size(); ++i) {
       // Phase* phase = mModel->mPhases[i];
        if (phase->mIsSelected || showAllPhases) {
            if (graphIndexIsInCurrentPage(graphIndex)) {
                GraphViewPhase* graph = new GraphViewPhase(phasesWidget);
                graph->setSettings(mModel->mSettings);
                graph->setMCMCSettings(mModel->mMCMCSettings, mModel->mChains);
                graph->setPhase(phase);
                graph->setGraphFont(font());
                graph->setGraphsThickness(mThicknessCombo->currentIndex());
                graph->changeXScaleDivision(mMajorScale, mMinorCountScale);
                graph->setMarginLeft(mMarginLeft);
                graph->setMarginRight(mMarginRight);

                mByPhasesGraphs.append(graph);
                connect(graph, &GraphViewResults::selected, this, &ResultsView::updateLayout);
            }
            ++graphIndex;
            
            if (mEventsfoldCheck->isChecked()) {
                for (auto& event : phase->mEvents) {
               // for (int j=0; j<phase->mEvents.size(); ++j)  {
                 //   Event* event = phase->mEvents[j];

                    if (graphIndexIsInCurrentPage(graphIndex)) {
                        GraphViewEvent* graph = new GraphViewEvent(phasesWidget);
                        graph->setSettings(mModel->mSettings);
                        graph->setMCMCSettings(mModel->mMCMCSettings, mModel->mChains);
                        graph->setEvent(event);
                        graph->setGraphFont(font());
                        graph->setGraphsThickness(mThicknessCombo->currentIndex());
                        graph->changeXScaleDivision(mMajorScale, mMinorCountScale);
                        graph->setMarginLeft(mMarginLeft);
                        graph->setMarginRight(mMarginRight);

                        mByPhasesGraphs.append(graph);
                        connect(graph, &GraphViewResults::selected, this, &ResultsView::updateLayout);
                    }
                    ++graphIndex;
                        
                    if (mDatesfoldCheck->isChecked()) {
                        for (auto& date : event->mDates) {
                        //for (int k=0; k<event->mDates.size(); ++k)  {
                            if (graphIndexIsInCurrentPage(graphIndex))  {
                               // Date& date = event->mDates[k];

                                GraphViewDate* graph = new GraphViewDate(phasesWidget);
                                graph->setSettings(mModel->mSettings);
                                graph->setMCMCSettings(mModel->mMCMCSettings, mModel->mChains);
                                graph->setDate(&date);
                                graph->setGraphFont(font());
                                graph->setGraphsThickness(mThicknessCombo->currentIndex());
                                graph->changeXScaleDivision(mMajorScale, mMinorCountScale);
                                graph->setColor(event->mColor);
                                graph->setGraphsOpacity(mOpacityCombo->currentIndex()*10);
                                graph->setMarginLeft(mMarginLeft);
                                graph->setMarginRight(mMarginRight);

                                mByPhasesGraphs.append(graph);
                                connect(graph, &GraphViewResults::selected, this, &ResultsView::updateLayout);
                            }
                            ++graphIndex;
                        }
                    }
                }
            }
        }
    }
}

void ResultsView::createByTempoGraphs()
{
    Q_ASSERT(mModel);

    // ----------------------------------------------------------------------
    //  Disconnect and delete existing graphs
    // ----------------------------------------------------------------------
    deleteAllGraphsInList(mByTempoGraphs);

    // ----------------------------------------------------------------------
    // Show all, unless at least one is selected
    // ----------------------------------------------------------------------
    bool showAllPhases = ! mModel->hasSelectedPhases();

    // ----------------------------------------------------------------------
    //  Iterate through all, and create corresponding graphs
    // ----------------------------------------------------------------------
    QWidget* tempoWidget = mTempoScrollArea->widget();
    int graphIndex = 0;

    for (auto& phase : mModel->mPhases) {
    //for (int i = 0; i < mModel->mPhases.size(); ++i) {
     //   Phase* phase = mModel->mPhases[i];
        if (phase->mIsSelected || showAllPhases) {
            if (graphIndexIsInCurrentPage(graphIndex)) {
                GraphViewTempo* graph = new GraphViewTempo(tempoWidget);
                graph->setSettings(mModel->mSettings);
                graph->setMCMCSettings(mModel->mMCMCSettings, mModel->mChains);
                graph->setPhase(phase);
                graph->setGraphFont(font());
                graph->setGraphsThickness(mThicknessCombo->currentIndex());
                graph->changeXScaleDivision(mMajorScale, mMinorCountScale);
                graph->setMarginLeft(mMarginLeft);
                graph->setMarginRight(mMarginRight);

                mByTempoGraphs.append(graph);
                connect(graph, &GraphViewResults::selected, this, &ResultsView::updateLayout);
            }
            ++graphIndex;
        }
    }
}

void ResultsView::createByCurveGraph()
{
    Q_ASSERT(isChronocurve());

    ModelChronocurve* model = modelChronocurve();
    
    // ----------------------------------------------------------------------
    //  Disconnect and delete existing graphs
    // ----------------------------------------------------------------------
    deleteAllGraphsInList(mByCurveGraphs);
    
    QWidget* widget = mCurveScrollArea->widget();

    if (mAlphaRadio->isChecked())  {
        GraphViewAlpha* graphAlpha = new GraphViewAlpha(widget);
        graphAlpha->setSettings(mModel->mSettings);
        graphAlpha->setMCMCSettings(mModel->mMCMCSettings, mModel->mChains);
        graphAlpha->setGraphFont(font());
        graphAlpha->setGraphsThickness(mThicknessCombo->currentIndex());
        graphAlpha->changeXScaleDivision(mMajorScale, mMinorCountScale);
        graphAlpha->setMarginLeft(mMarginLeft);
        graphAlpha->setMarginRight(mMarginRight);
        //graphAlpha->setModel(model);
        graphAlpha->setTitle(tr("Alpha smoothing"));
        graphAlpha->setModel(model);
        
        mByCurveGraphs.append(graphAlpha);
        connect(graphAlpha, &GraphViewResults::selected, this, &ResultsView::updateLayout);

    } else  {
        bool hasY = (model->mChronocurveSettings.mProcessType != ChronocurveSettings::eProcessTypeUnivarie);
        bool hasZ = (model->mChronocurveSettings.mProcessType == ChronocurveSettings::eProcessTypeVectoriel);

        // insert refpoints for X
        const double thresh = 64.5; //80;
        QVector<RefPoint> refPts;

        for (auto& event : modelChronocurve()->mEvents) {
            RefPoint rf;
            if (event->mType == Event::eDefault) {
                double tmin = HUGE_VAL;
                double tmax = -HUGE_VAL;

                for (auto&& date: event->mDates) {
                    QMap<double, double> calibMap = date.getRawCalibMap();//  getFormatedCalibMap();

                    QMap<double, double> subData = getMapDataInRange(calibMap, mModel->mSettings.mTmin, mModel->mSettings.mTmax);// mSettings.getTminFormated(), mSettings.getTmaxFormated());
                    QMap<double, double> hpd = create_HPD(subData, thresh);

                    QMapIterator<double, double> it(hpd);
                    it.toFront();
                    while (it.hasNext()) {
                        it.next();
                        if (it.value() != 0) {
                            tmin = std::min(tmin, it.key());
                            break;
                        }
                    }
                    it.toBack();
                    while (it.hasPrevious()) {
                        it.previous();
                        if (it.value() != 0) {
                            tmax = std::max(tmax, it.key());
                            break;
                        }
                    }
                }
                const double tmoy = DateUtils::convertToAppSettingsFormat((tmax + tmin) / 2.);
                const double terr = (tmax - tmin) / 2.;
                //tPts[tmoy] = terr;

                rf.Xmean = tmoy;
                rf.Xerr = terr;

            } else {
                rf.Xmean = event->mTheta.mX; // always the same value
                rf.Xerr = 0.;
            }

            if (!hasY) {
                switch (model->mChronocurveSettings.mVariableType) {
                case ChronocurveSettings::eVariableTypeInclinaison :
                    rf.Ymean = event-> mYInc;
                    rf.Yerr = event->mSInc;
                case ChronocurveSettings::eVariableTypeDeclinaison :
                    rf.Ymean = event-> mYDec;
                    rf.Yerr = event->mSInc / cos(event->mYInc * M_PI /180.);
                case ChronocurveSettings::eVariableTypeIntensite :
                    rf.Ymean = event-> mYInc;
                    rf.Yerr = event->mSInc;
                case ChronocurveSettings::eVariableTypeProfondeur :
                    rf.Ymean = event-> mYInc;
                    rf.Yerr = event->mSInc;
                case ChronocurveSettings::eVariableTypeAutre :
                    rf.Ymean = event-> mYInc;
                    rf.Yerr = event->mSInc;
                }

            } else {
                rf.Ymean = event-> mYInc;
                rf.Yerr = event->mSInc;
            }

            rf.color = event->mColor;

            refPts.append(rf);
        }


        GraphViewCurve* graphX = new GraphViewCurve(widget);
        graphX->setSettings(mModel->mSettings);
        graphX->setMCMCSettings(mModel->mMCMCSettings, mModel->mChains);
        graphX->setGraphFont(font());
        graphX->setGraphsThickness(mThicknessCombo->currentIndex());
        graphX->changeXScaleDivision(mMajorScale, mMinorCountScale);
        graphX->setMarginLeft(mMarginLeft);
        graphX->setMarginRight(mMarginRight);

        if (model->mChronocurveSettings.mProcessType == ChronocurveSettings::eProcessTypeUnivarie ) {
            switch (model->mChronocurveSettings.mVariableType) {
                 case ChronocurveSettings::eVariableTypeInclinaison :
                      graphX->setTitle(tr("Mean Inclination"));
                 case ChronocurveSettings::eVariableTypeDeclinaison :
                       graphX->setTitle(tr("Mean Declination"));
                 case ChronocurveSettings::eVariableTypeIntensite:
                      graphX->setTitle(tr("Mean Intensity"));
                 case ChronocurveSettings::eVariableTypeProfondeur:
                      graphX->setTitle(tr("Mean Detph"));
                 case ChronocurveSettings::eVariableTypeAutre:
                       graphX->setTitle(tr("Mean"));
            }
        } else {
            graphX->setTitle(tr("Mean Inclination"));
        }

        graphX->setComposanteG(modelChronocurve()->mPosteriorMeanG.gx);
        graphX->setComposanteGChains(modelChronocurve()->getChainsMeanGComposanteX());
        graphX->setEvents(modelChronocurve()->mEvents);
        graphX->setRefPoints(refPts);


        mByCurveGraphs.append(graphX);
        
        connect(graphX, &GraphViewResults::selected, this, &ResultsView::updateLayout);
        
        if (hasY) {
            GraphViewCurve* graphY = new GraphViewCurve(widget);
            graphY->setSettings(mModel->mSettings);
            graphY->setMCMCSettings(mModel->mMCMCSettings, mModel->mChains);
            graphY->setGraphFont(font());
            graphY->setGraphsThickness(mThicknessCombo->currentIndex());
            graphY->changeXScaleDivision(mMajorScale, mMinorCountScale);
            graphY->setMarginLeft(mMarginLeft);
            graphY->setMarginRight(mMarginRight);

            graphY->setTitle(tr("Mean Declination"));
            //graphY->setTitle(tr("Mean G (composante y)"));
            
            graphY->setComposanteG(modelChronocurve()->mPosteriorMeanG.gy);
            graphY->setComposanteGChains(modelChronocurve()->getChainsMeanGComposanteY());

            graphY->setEvents(modelChronocurve()->mEvents);
            // change the values of the Y and the error, with the values of the declination and the error, we keep tmean
            int i = 0;
            for (auto& event : modelChronocurve()->mEvents) {
                refPts[i].Ymean = event-> mYDec;
                refPts[i].Yerr = event->mSInc / cos(event->mYInc * M_PI /180.);
                ++i;
            }
            graphY->setRefPoints(refPts);
            mByCurveGraphs.append(graphY);
            
            connect(graphY, &GraphViewResults::selected, this, &ResultsView::updateLayout);
        }
        
        if (hasZ) {
            GraphViewCurve* graphZ = new GraphViewCurve(widget);
            graphZ->setSettings(mModel->mSettings);
            graphZ->setMCMCSettings(mModel->mMCMCSettings, mModel->mChains);
            graphZ->setGraphFont(font());
            graphZ->setGraphsThickness(mThicknessCombo->currentIndex());
            graphZ->changeXScaleDivision(mMajorScale, mMinorCountScale);
            graphZ->setMarginLeft(mMarginLeft);
            graphZ->setMarginRight(mMarginRight);

            graphZ->setTitle(tr("Mean Intensity"));
            
            graphZ->setComposanteG(modelChronocurve()->mPosteriorMeanG.gz);
            graphZ->setComposanteGChains(modelChronocurve()->getChainsMeanGComposanteZ());

            graphZ->setEvents(modelChronocurve()->mEvents);
            int i = 0;
            for (auto& event : modelChronocurve()->mEvents) {
                refPts[i].Ymean = event-> mYInt;
                refPts[i].Yerr = event->mSInt;
                ++i;
            }
            graphZ->setRefPoints(refPts);
            mByCurveGraphs.append(graphZ);
            
            connect(graphZ, &GraphViewResults::selected, this, &ResultsView::updateLayout);
        }
    }
}

void ResultsView::deleteAllGraphsInList(QList<GraphViewResults*>& list)
{
    for (auto&& graph : list) {
        disconnect(graph, &GraphViewResults::selected, this, &ResultsView::updateLayout);
        delete graph;
    }
    list.clear();
}

QList<GraphViewResults*> ResultsView::allGraphs()
{
    QList<GraphViewResults*> graphs;
    graphs.append(mByEventsGraphs);
    graphs.append(mByPhasesGraphs);
    graphs.append(mByTempoGraphs);
    graphs.append(mByCurveGraphs);
    return graphs;
}

bool ResultsView::hasSelectedGraphs()
{
    return (currentGraphs(true).size() > 0);
}

QList<GraphViewResults*> ResultsView::currentGraphs(bool onlySelected)
{
    QList<GraphViewResults*> graphs;

    if (mGraphListTab->currentIndex() == 0) {
        for (auto&& graph : mByEventsGraphs) {
            if (!onlySelected || graph->isSelected()) {
                graphs.append(graph);
            }
        }

    } else if (mGraphListTab->currentIndex() == 1) {
        for (auto&& graph : mByPhasesGraphs) {
            if (!onlySelected || graph->isSelected()) {
                graphs.append(graph);
            }
        }

    } else if (mGraphListTab->currentIndex() == 2) {
        for (auto&& graph : mByTempoGraphs) {
            if (!onlySelected || graph->isSelected()) {
                graphs.append(graph);
            }
        }

    } else if (mGraphListTab->currentIndex() == 3) {
        for (auto&& graph : mByCurveGraphs) {
            if (!onlySelected || graph->isSelected()) {
                graphs.append(graph);
            }
        }
    }
    return graphs;
}


#pragma mark Pagination


bool ResultsView::graphIndexIsInCurrentPage(int graphIndex)
{
    int firstIndexToShow = mCurrentPage * mGraphsPerPage;
    return (graphIndex >= firstIndexToShow) && (graphIndex < (firstIndexToShow + mGraphsPerPage));
}


#pragma mark Curves generation

/**
*  @brief re-generate all curves in graph views form model data.
*  @brief Each curve is given a name. This name will be used by updateCurvesToShow() to decide whether the curve is visible or not.
*  @param listGraphs is the list of existings graph for which we want to generate curves. It may be mByEventsGraphs, mByPhasesGraphs, etc...
*  Depending on the selected tab, options may differ. For example, we don't have the same display options for events and durations !
*  We thus have to check which tab is selected, to gather the corresponding options, and generate the curves.
*/
void ResultsView::generateCurves()
{
    // -----------------------------------------------------------------
    //  Generate all graphs curves in the current list
    // -----------------------------------------------------------------
    QList<GraphViewResults*> listGraphs = currentGraphs(false);
    for (GraphViewResults*& graph : listGraphs) {
        graph->generateCurves(GraphViewResults::TypeGraph(mCurrentTypeGraph), mCurrentVariable);
    }
    
    updateCurvesToShow();
    updateGraphsMax();
    updateScales();
}

void ResultsView::updateGraphsMax()
{
    QList<GraphViewResults*> listGraphs = currentGraphs(false);
    
    if (mCurrentVariable == GraphViewResults::eSigma) {
        mCurrentVariableMaxX = getGraphsMax(listGraphs, "Sigma", 100.);

    } else if (mCurrentVariable == GraphViewResults::eDuration) {
        mCurrentVariableMaxX = getGraphsMax(listGraphs, "Post Distrib Duration", 100.);

    }else if (mCurrentVariable == GraphViewResults::eVG) {
        mCurrentVariableMaxX = getGraphsMax(listGraphs, "VG", 100.);

    } else if (mCurrentVariable == GraphViewResults::eAlpha) {
        mCurrentVariableMaxX = getGraphsMax(listGraphs, "Alpha", 100.);
    }
}

double ResultsView::getGraphsMax(const QList<GraphViewResults*>& graphs, const QString& title, double maxFloor)
{
    double max = 0.;
    
    QList<GraphViewResults*>::const_iterator it = graphs.cbegin();
    QList<GraphViewResults*>::const_iterator itEnd = graphs.cend();

    while (it != itEnd)  {
        GraphViewResults* graphWrapper = (*it);
        QList<GraphCurve> curves = graphWrapper->getGraph()->getCurves();
        for (auto&& curve : curves) {
            //if(graphWrapper->getCurrentVariable() == variable)
            if (curve.mName.contains(title) && (curve.mVisible == true)) {
                max = ceil(qMax(max, curve.mData.lastKey()));
            }
        }
        ++it;
    }
    max = std::max(maxFloor, max);
    return max;
}

/**
 *  @brief Decide which curve graphs must be show, based on currently selected options.
 *  @brief This function does NOT remove or create any curve in graphs! It only checks if existing curves should be visible or not.
 */
void ResultsView::updateCurvesToShow()
{
    // --------------------------------------------------------
    //  Gather selected chain options
    // --------------------------------------------------------
    bool showAllChains = mAllChainsCheck->isChecked();

    // --------------------------------------------------------
    //  showChainList is a list of booleans describing which chains are visible or not.
    //  For Post distribs, multiple chains can be visible at once (checkboxes)
    //  In other cases, only one chain can be displayed (radios)
    // --------------------------------------------------------
    QList<bool> showChainList;
    if (mCurrentTypeGraph == GraphViewResults::ePostDistrib) {
        for (CheckBox*& cbButton : mChainChecks) {
            showChainList.append(cbButton->isChecked());
        }

    } else {
        for (RadioButton*& rButton : mChainRadios) {
            showChainList.append(rButton->isChecked());
        }
    }

    // --------------------------------------------------------
    //  Find the currently selected list of graphs
    // --------------------------------------------------------
    QList<GraphViewResults*> listGraphs = currentGraphs(false);

    // --------------------------------------------------------
    //  Options for "Curve"
    // --------------------------------------------------------
    if ((mGraphListTab->currentIndex() == 3) && !mAlphaRadio->isChecked()) {
        bool showG = mCurveGRadio->isChecked();
        bool showGError = mCurveErrorCheck->isChecked();
        bool showGPoints = mCurvePointsCheck->isChecked();
        bool showGP = mCurveGPRadio->isChecked();
        bool showGS = mCurveGSRadio->isChecked();
        
        // --------------------------------------------------------
        //  Update Graphs with selected options
        // --------------------------------------------------------
        for (GraphViewResults*& graph : listGraphs) {
            GraphViewCurve* graphCurve = static_cast<GraphViewCurve*>(graph);
            graphCurve->setShowNumericalResults(false);
            graphCurve->updateCurvesToShowForG(showAllChains, showChainList, showG, showGError, showGPoints, showGP, showGS);
        }
    }
    // --------------------------------------------------------
    //  All others
    // --------------------------------------------------------
    else {
        bool showCalib = mDataCalibCheck->isChecked();
        bool showWiggle = mWiggleCheck->isChecked();
        bool showCredibility = mCredibilityCheck->isChecked();
        bool showStat = mStatCheck->isChecked();

        if (mCurrentVariable == GraphViewResults::eTempo) {
            showCalib = mTempoErrCheck->isChecked();
            showCredibility = mTempoCredCheck->isChecked();

        } else if (mCurrentVariable == GraphViewResults::eAlpha){
            //showCalib = mTempoErrCheck->isChecked();
            //showCredibility = mTempoCredCheck->isChecked();
        }
        
        // --------------------------------------------------------
        //  Update Graphs with selected options
        // --------------------------------------------------------
        for (GraphViewResults*& graph : listGraphs) {
            graph->setShowNumericalResults(showStat);
            graph->updateCurvesToShow(showAllChains, showChainList, showCredibility, showCalib, showWiggle);
        }
    }
}

/**
 *  @brief
 *  This method does the following :
 *  - Defines [mResultMinX, mResultMaxX]
 *  - Defines [mResultCurrentMinX, mResultCurrentMaxX] (based on saved zoom if any)
 *  - Computes mResultZoomX
 *  - Set Ruler Areas
 *  - Set Ruler and graphs range and zoom
 *  - Update mXMinEdit, mXMaxEdit, mXSlider, mXSpin, mMajorScaleEdit, mMinorScaleEdit
 */
void ResultsView::updateScales()
{
    if (!mModel) {
        return;
    }

    ProjectSettings s = mModel->mSettings;

    // ------------------------------------------------------------------
    //  Get X range based on current options used to calculate zoom
    // ------------------------------------------------------------------

    // ------------------------------------------------------------------
    //  Chronocurve results graphs ("Curve" tab)
    // ------------------------------------------------------------------
    if ((mGraphListTab->currentIndex() == 3) && !mAlphaRadio->isChecked())  {
        // Study period min and max
        mResultMinX = s.getTminFormated();
        mResultMaxX = s.getTmaxFormated();

        // The zoom slider and spin are linear.
        const int zoomLevels = ceil((mResultMaxX - mResultMinX) / 10);
        mXSlider->setRange(1, zoomLevels);
        mXSpin->setRange(1, zoomLevels);
        mXSpin->setSingleStep(1.);
        mXSpin->setDecimals(0);

        mRuler->setRange(mResultMinX, mResultMaxX);
        mRuler->setFormatFunctX(nullptr);
    }
    // ------------------------------------------------------------------
    //  All other modes
    // ------------------------------------------------------------------
    else if (mCurrentTypeGraph == GraphViewResults::ePostDistrib) {
        // ------------------------------------------------------------------
        // These graphs use the time in the X axis
        // ------------------------------------------------------------------
        if (mCurrentVariable == GraphViewResults::eTheta
             || mCurrentVariable == GraphViewResults::eTempo
             || mCurrentVariable == GraphViewResults::eActivity) {

            // Study period min and max
            mResultMinX = s.getTminFormated();
            mResultMaxX = s.getTmaxFormated();

            // The X zoom uses a log scale on the spin box and can be controlled by the linear slider
            mXSlider->setRange(-100, 100);
            mXSpin->setRange(sliderToZoom(-100), sliderToZoom(100));
            mXSpin->setSingleStep(.01);
            mXSpin->setDecimals(3);

            // The Ruler range is much wider based on the minimal zoom
            const double tCenter = (mResultMinX + mResultMaxX) / 2.;
            const double tSpan = mResultMaxX - mResultMinX;
            const double tRangeMin = tCenter - ((tSpan/2.) / sliderToZoom(mXSlider->minimum()));
            const double tRangeMax = tCenter + ((tSpan/2.) / sliderToZoom(mXSlider->minimum()));

            mRuler->setRange(tRangeMin, tRangeMax);
            mRuler->setFormatFunctX(nullptr);
        }
        // ------------------------------------------------------------------
        // These graphs use the variance in the X axis
        // ------------------------------------------------------------------
        else if ((mCurrentVariable == GraphViewResults::eSigma) ||
                 (mCurrentVariable == GraphViewResults::eDuration) ||
                 (mCurrentVariable == GraphViewResults::eVG) ||
                 (mCurrentVariable == GraphViewResults::eAlpha))
        {
            // Variance min and max
            mResultMinX = 0.;
            mResultMaxX = mCurrentVariableMaxX;
            
            // The X zoom uses a log scale on the spin box and can be controlled by the linear slider
            mXSlider->setRange(-100, 100);
            mXSpin->setRange(sliderToZoom(-100), sliderToZoom(100));
            mXSpin->setSingleStep(.01);
            mXSpin->setDecimals(3);

            // The Ruler range is much wider based on the minimal zoom
            const double tRangeMax = mResultMaxX / sliderToZoom(mXSlider->minimum());
            
            mRuler->setRange(0, tRangeMax);
            mRuler->setFormatFunctX(nullptr);
        }
    }
    // ------------------------------------------------------------------
    //  Trace and Acceptation
    // ------------------------------------------------------------------
    else if ((mCurrentTypeGraph == GraphViewResults::eTrace) || (mCurrentTypeGraph == GraphViewResults::eAccept)) {
        // The min is always 0
        mResultMinX = 0.;

        // We look for the selected chain (only one possible) and set the max to the number of iterations
        for (int i = 0; i < mChainRadios.size(); ++i) {
            if (mChainRadios.at(i)->isChecked()) {
                const ChainSpecs& chain = mModel->mChains.at(i);
                mResultMaxX = 1 + chain.mNumBurnIter + (chain.mBatchIndex * chain.mNumBatchIter) + chain.mNumRunIter / chain.mThinningInterval;
                break;
            }
        }

        // The zoom slider and spin are linear.
        // The number of zoom levels depends on the number of iterations (by a factor 100)
        // e.g. 400 iterations => 4 levels
        const int zoomLevels = (int) mResultMaxX / 100;
        mXSlider->setRange(1, zoomLevels);
        mXSpin->setRange(1, zoomLevels);
        mXSpin->setSingleStep(1.);
        mXSpin->setDecimals(0);

        // The Ruler range is set exactly to the min and max (impossible to scroll outside)
        mRuler->setRange(mResultMinX, mResultMaxX);
        mRuler->setFormatFunctX(nullptr);
    }
    // ------------------------------------------------------------------
    //  Autocorrelation
    // ------------------------------------------------------------------
    else if (mCurrentTypeGraph == GraphViewResults::eCorrel) {
        // The x axis represents h, always in [0, 40]
        mResultMinX = 0.;
        mResultMaxX = 40.;

        // The zoom slider and spin are linear.
        // Always 5 zoom levels
        mXSlider->setRange(1, 5);
        mXSpin->setRange(1, 5);
        mXSpin->setSingleStep(1.);
        mXSpin->setDecimals(0);

        mRuler->setRange(mResultMinX, mResultMaxX);
        mRuler->setFormatFunctX(nullptr);
    }

    // ------------------------------------------------------------------
    //  Define mResultCurrentMinX and mResultCurrentMaxX
    //  + Restore last zoom values if any
    // ------------------------------------------------------------------

    // The key of the saved zooms map is as long as that :
    QPair<GraphViewResults::Variable, GraphViewResults::TypeGraph> key(mCurrentVariable, mCurrentTypeGraph);

    // Anyway, let's check if we have a saved zoom value for this key :
    if (mZooms.find(key) != mZooms.end()) {
        // Get the saved (unformatted) values
        double tMin = mZooms.value(key).first;
        double tMax = mZooms.value(key).second;

        // These graphs needs formating since the x axis represents the time
        if (xScaleRepresentsTime()) {
            double tMinFormatted = DateUtils::convertToAppSettingsFormat(tMin);
            double tMaxFormatted = DateUtils::convertToAppSettingsFormat(tMax);

            // Min and max may be inverted due to formatting, so we use std::minmax
            std::pair<double, double> currentMinMax = std::minmax(tMinFormatted, tMaxFormatted);

            mResultCurrentMinX = currentMinMax.first;
            mResultCurrentMaxX = currentMinMax.second;

        } else {
            mResultCurrentMinX = tMin;
            mResultCurrentMaxX = tMax;
        }

    } else {
        mResultCurrentMinX = mResultMinX;
        mResultCurrentMaxX = mResultMaxX;
    }
    
    // Now, let's check if we have a saved scale (ticks interval) value for this key :
    if (mScales.find(key) != mScales.end()) {
        mMajorScale = mScales.value(key).first;
        mMinorCountScale = mScales.value(key).second;

    } else {
        // For correlation graphs, ticks intervals are not an available option
        if (mCurrentTypeGraph == GraphViewResults::eCorrel) {
            mMajorScale = 10.;
            mMinorCountScale = 10;
        }
        // All other cases (default behavior)
        else {
            Scale xScale;
            xScale.findOptimal(mResultCurrentMinX, mResultCurrentMaxX, 10);
            mMajorScale = xScale.mark;
            mMinorCountScale = 10;
        }
    }

    // ------------------------------------------------------------------
    //  Compute mResultZoomX
    // ------------------------------------------------------------------
    mResultZoomX = (mResultMaxX - mResultMinX) / (mResultCurrentMaxX - mResultCurrentMinX);

    // ------------------------------------------------------------------
    //  Set Ruler Areas (Burn, Adapt, Run)
    // ------------------------------------------------------------------
    mRuler->clearAreas();

    if ((mCurrentTypeGraph == GraphViewResults::eTrace) || (mCurrentTypeGraph == GraphViewResults::eAccept)) {
        for (int i=0; i<mChainRadios.size(); ++i) {
            if (mChainRadios.at(i)->isChecked()) {
                const ChainSpecs& chain = mModel->mChains.at(i);
                int adaptSize = chain.mBatchIndex * chain.mNumBatchIter;
                int runSize = int (chain.mNumRunIter / chain.mThinningInterval);

                mRuler->addArea(0., chain.mNumBurnIter, QColor(235, 115, 100));
                mRuler->addArea(chain.mNumBurnIter, chain.mNumBurnIter + adaptSize, QColor(250, 180, 90));
                mRuler->addArea(chain.mNumBurnIter + adaptSize, chain.mNumBurnIter + adaptSize + runSize, QColor(130, 205, 110));

                break;
            }
        }
    }

    // ------------------------------------------------------------------
    //  Apply to Ruler
    // ------------------------------------------------------------------
    mRuler->setCurrent(mResultCurrentMinX, mResultCurrentMaxX);
    mRuler->setScaleDivision(mMajorScale, mMinorCountScale);

    // -------------------------------------------------------
    //  Apply to all graphs
    // -------------------------------------------------------
    QList<GraphViewResults*> graphs = currentGraphs(false);
    for (GraphViewResults*& graph : graphs) {
        graph->setRange(mRuler->mMin, mRuler->mMax);
        graph->setCurrentX(mResultCurrentMinX, mResultCurrentMaxX);
        graph->changeXScaleDivision(mMajorScale, mMinorCountScale);
        graph->zoom(mResultCurrentMinX, mResultCurrentMaxX);
    }

    // -------------------------------------------------------
    //  Set options UI components values
    // -------------------------------------------------------
    setXRange();
    setXSlider(zoomToSlider(mResultZoomX));
    setXSpin(mResultZoomX);
    setXIntervals();
}

void ResultsView::updateControls()
{
    bool isPostDistrib = isPostDistribGraph();

    // -------------------------------------------------------------------------------------
    //  Update graph list tab
    // -------------------------------------------------------------------------------------
    if (isChronocurve()) {
        mGraphListTab->setTabVisible(1, false); // Phases
        mGraphListTab->setTabVisible(2, false); // Tempo
        mGraphListTab->setTabVisible(3, true); // Curve
        
        // If the current tab index is "Phases" or "Tempo",
        // then go to "Events" tab which is a good default choice in "Curve" mode
        if (mGraphListTab->currentIndex() == 1 || mGraphListTab->currentIndex() == 2) {
            mGraphListTab->setTab(0, false);
        }
    } else {
        mGraphListTab->setTabVisible(1, mHasPhases); // Phases
        mGraphListTab->setTabVisible(2, mHasPhases); // Tempo
        mGraphListTab->setTabVisible(3, false); // Curve
        
        // If the current tab is not currently visible :
        // - Show the "Phases" tab (1) which is a good default choice if the model has phases.
        // - Show the "Events" tab (0) which is a good default choice if the model doesn't have phases.
        
        if (mHasPhases && mGraphListTab->currentIndex() >= 3) {
            mGraphListTab->setTab(1, false);

        } else if(!mHasPhases && mGraphListTab->currentIndex() >= 1) {
            mGraphListTab->setTab(1, false);
        }
    }

    // -------------------------------------------------------------------------------------
    //  Update controls depending on current graph list
    // -------------------------------------------------------------------------------------
    if (mGraphListTab->currentIndex() == 0) {
        mGraphTypeTabs->setTabVisible(1, true); // History Plot
        mGraphTypeTabs->setTabVisible(2, true); // Acceptance Rate
        mGraphTypeTabs->setTabVisible(3, true); // Autocorrelation

        mResultsGroup->setVisible(true);
        mTempoGroup->setVisible(false);
        mCurvesGroup->setVisible(false);

        mEventsfoldCheck->setVisible(false);

        if (isChronocurve()) {
            mDataVGRadio->setVisible(true);
        } else {
            mDataVGRadio->setVisible(false);
        }
        
        if (mCurrentVariable == GraphViewResults::eVG)  {
            mDatesfoldCheck->setChecked(false);
            mDatesfoldCheck->setVisible(false);
            mDataCalibCheck->setVisible(false);
            mWiggleCheck->setVisible(false);
        } else {
            mDatesfoldCheck->setVisible(true);
            
            bool showCalibControl = isPostDistrib && mDataThetaRadio->isChecked() && mDatesfoldCheck->isChecked();
            
            mDataCalibCheck->setVisible(showCalibControl);
            mWiggleCheck->setVisible(showCalibControl);
        }
            
    } else if (mGraphListTab->currentIndex() == 1) {
        mGraphTypeTabs->setTabVisible(1, true); // History Plot
        mGraphTypeTabs->setTabVisible(2, true); // Acceptance Rate
        mGraphTypeTabs->setTabVisible(3, true); // Autocorrelation

        mResultsGroup->setVisible(true);
        mTempoGroup->setVisible(false);
        mCurvesGroup->setVisible(false);

        mEventsfoldCheck->setVisible(true);
        mDatesfoldCheck->setVisible(mEventsfoldCheck->isChecked());

        if (!mEventsfoldCheck->isChecked()) {
            mDatesfoldCheck->setChecked(false);
        }

        bool showCalibControl = isPostDistrib && mEventsfoldCheck->isChecked() && mDatesfoldCheck->isChecked() && mDataThetaRadio->isChecked();

        mDataCalibCheck->setVisible(showCalibControl);
        mWiggleCheck->setVisible(showCalibControl);

    } else if(mGraphListTab->currentIndex() == 2) {
        mGraphTypeTabs->setTabVisible(1, false); // History Plot
        mGraphTypeTabs->setTabVisible(2, false); // Acceptance Rate
        mGraphTypeTabs->setTabVisible(3, false); // Autocorrelation
        mGraphTypeTabs->setTab(0, true);

        mResultsGroup->setVisible(false);
        mTempoGroup->setVisible(true);
        mCurvesGroup->setVisible(false);

        mTempoCredCheck->setVisible(mTempoRadio->isChecked());
        mTempoErrCheck->setVisible(mTempoRadio->isChecked());

    } else if (mGraphListTab->currentIndex() == 3) {
        bool isAlpha = mAlphaRadio->isChecked();
        
        mGraphTypeTabs->setTabVisible(1, isAlpha); // History Plot
        mGraphTypeTabs->setTabVisible(2, isAlpha); // Acceptance Rate
        mGraphTypeTabs->setTabVisible(3, isAlpha); // Autocorrelation
        
        mResultsGroup->setVisible(false);
        mTempoGroup->setVisible(false);
        mCurvesGroup->setVisible(true);
        
        bool showGOptions = mCurveGRadio->isChecked();
        mCurveErrorCheck->setVisible(showGOptions);
        mCurvePointsCheck->setVisible(showGOptions);
        if (!showGOptions) {
            mCurveErrorCheck->setChecked(false);
            mCurvePointsCheck->setChecked(false);
        }
    }

    // -------------------------------------------------------------------------------------
    //  Update controls depending on current display tab
    // -------------------------------------------------------------------------------------
    mTabDisplay->setVisible(mTabDisplayMCMC->currentIndex() == 0);
    mTabMCMC->setVisible(mTabDisplayMCMC->currentIndex() == 1);

    if (mCurrentVariable == GraphViewResults::eTheta
        || mCurrentVariable == GraphViewResults::eTempo
        || mCurrentVariable == GraphViewResults::eActivity
        || mCurrentVariable == GraphViewResults::eDuration
        || mCurrentVariable == GraphViewResults::eSigma
        || mCurrentVariable == GraphViewResults::eVG
        || mCurrentVariable == GraphViewResults::eAlpha)
    {
        mDisplayStudyBut->setVisible(true);
    } else {
        mDisplayStudyBut->setVisible(false);
    }

    // -------------------------------------------------------------------------------------
    //  MCMC Display options are not visible for mCurrentVariable = Tempo or Activity
    // -------------------------------------------------------------------------------------
    if (mCurrentVariable == GraphViewResults::eTempo || mCurrentVariable == GraphViewResults::eActivity) {
        mTabDisplayMCMC->setTabVisible(1, false);
        mTabDisplayMCMC->setTab(0, false);

    } else {
        mTabDisplayMCMC->setTabVisible(1, true);
    }

    // ------------------------------------
    //  MCMC Chains
    //  Switch between checkBoxes or Radio-buttons for chains
    // ------------------------------------
    mAllChainsCheck->setVisible(isPostDistrib);

    for (auto&& checkChain : mChainChecks) {
        checkChain->setVisible(isPostDistrib);
    }

    for (auto&& chainRadio : mChainRadios) {
        chainRadio->setVisible(!isPostDistrib);
    }

    // ------------------------------------
    //  Density Options
    // ------------------------------------
    bool showDensityOptions = isPostDistrib && (mCurrentVariable != GraphViewResults::eTempo && mCurrentVariable != GraphViewResults::eActivity);

    mDensityOptsTitle->setVisible(showDensityOptions);
    mDensityOptsGroup->setVisible(showDensityOptions);

    // ------------------------------------
    //  Display Options
    // ------------------------------------
    mDisplayStudyBut->setText(xScaleRepresentsTime() ? tr("Study Period Display") : tr("Fit Display"));

    // -------------------------------------------------------------------------------------
    //  Pagination options
    // -------------------------------------------------------------------------------------
    bool showPagination = (mTabPageSaving->currentIndex() == 0);

    mPageWidget->setVisible(showPagination);
    mToolsWidget->setVisible(!showPagination);

    QList<GraphViewResults*> graphsSelected = currentGraphs(true);
    bool hasSelection = (graphsSelected.size() > 0);

    mImageSaveBut->setVisible(hasSelection);
    mImageClipBut->setVisible(hasSelection);
    mResultsClipBut->setVisible(hasSelection);
    mDataSaveBut->setVisible(hasSelection);

    mExportImgBut->setVisible(!hasSelection);
    mExportResults->setVisible(!hasSelection);


    // -------------------------------------------------------------------------------------
    //  - Update the total number of graphs for all pages
    //  - Check if the current page is still lower than the number of pages
    //  - Update the pagination display
    //  => All this must be done BEFORE calling createGraphs, which uses theses params to build the graphs
    // -------------------------------------------------------------------------------------
    updateTotalGraphs();
    
    int numPages = ceil((double)mMaximunNumberOfVisibleGraph / (double)mGraphsPerPage);
    if (mCurrentPage >= numPages) {
        mCurrentPage = 0;
    }
    
    mPageEdit->setText(locale().toString(mCurrentPage + 1) + "/" + locale().toString(numPages));
}

/*void ResultsView::updateResultsLog()
{
    QString log;
    try {
        for (auto &&phase : mModel->mPhases)
            log += ModelUtilities::phaseResultsHTML(phase);

        for (auto &&phase : mModel->mPhases)
            log += ModelUtilities::tempoResultsHTML(phase);

        for (auto &&phaseConstraint : mModel->mPhaseConstraints) {
            log += ModelUtilities::constraintResultsHTML(phaseConstraint);

         for (auto &&event : mModel->mEvents)
              log += ModelUtilities::eventResultsHTML(event, true, mModel);

          log += "<hr>";
        }


    } catch (std::exception const & e) {
        log = tr("Impossible to compute");
    }

    emit resultsLogUpdated(log);
}*/

#pragma mark Utilities

bool ResultsView::isPostDistribGraph()
{
    return (mCurrentTypeGraph == GraphViewResults::ePostDistrib);
}

bool ResultsView::xScaleRepresentsTime()
{
    return isPostDistribGraph() && (mCurrentVariable == GraphViewResults::eTheta
                                    || mCurrentVariable == GraphViewResults::eTempo
                                    || mCurrentVariable == GraphViewResults::eActivity);
}

double ResultsView::sliderToZoom(const int &coef)
{
    return isPostDistribGraph() ? pow(10., double (coef/100.)) : coef;
}

int ResultsView::zoomToSlider(const double &zoom)
{
    return isPostDistribGraph() ? int (round(log10(zoom) * (100.))) : int(zoom);
}

void ResultsView::updateGraphsHeight()
{
    const double min = 2 * AppSettings::heigthUnit();
    const double origin = GraphViewResults::mHeightForVisibleAxis;
    const double prop = mYSpin->value() / 100.;
    mGraphHeight = min + prop * (origin - min);
    updateGraphsLayout();
}

void ResultsView::updateZoomX()
{
    // Pick the value from th spin or the slider
    double zoom = mXSpin->value();

    mResultZoomX = 1./zoom;

    const double tCenter = (mResultCurrentMaxX + mResultCurrentMinX)/2.;
    const double span = (mResultMaxX - mResultMinX)* (1./ zoom);

    double curMin = tCenter - span/2.;
    double curMax = tCenter + span/2.;

    if (curMin < mRuler->mMin) {
        curMin = mRuler->mMin;
        curMax = curMin + span;

    } else if (curMax > mRuler->mMax) {
        curMax = mRuler->mMax;
        curMin = curMax - span;
    }

    mResultCurrentMinX = curMin;
    mResultCurrentMaxX = curMax;

    mRuler->setCurrent(mResultCurrentMinX, mResultCurrentMaxX);

    setXRange();

    updateGraphsZoomX();
}

void ResultsView::updateGraphsZoomX()
{
    // ------------------------------------------------------
    //  Update graphs zoom and scale
    // ------------------------------------------------------
    QList<GraphViewResults*> graphs = currentGraphs(false);
    for (GraphViewResults*& graph : graphs) {
        graph->changeXScaleDivision(mMajorScale, mMinorCountScale);
        graph->zoom(mResultCurrentMinX, mResultCurrentMaxX);
    }

    // ------------------------------------------------------
    //  Store zoom and scale for this type of graph
    // ------------------------------------------------------
    QPair<GraphViewResults::Variable, GraphViewResults::TypeGraph> key(mCurrentVariable, mCurrentTypeGraph);

    if (xScaleRepresentsTime()) {
        double minFormatted = DateUtils::convertFromAppSettingsFormat(mResultCurrentMinX);
        double maxFormatted = DateUtils::convertFromAppSettingsFormat(mResultCurrentMaxX);

        std::pair<double, double> resultMinMax = std::minmax(minFormatted, maxFormatted);

        mZooms[key] = QPair<double, double>(resultMinMax.first, resultMinMax.second);

    } else {
        mZooms[key] = QPair<double, double>(mResultCurrentMinX, mResultCurrentMaxX);
    }

    mScales[key] = QPair<double, int>(mMajorScale, mMinorCountScale);
}

#pragma mark Controls setters

void ResultsView::setXRange()
{
    QLocale locale = QLocale();

    mCurrentXMinEdit->blockSignals(true);
    mCurrentXMaxEdit->blockSignals(true);

    mCurrentXMinEdit->setText(locale.toString(mResultCurrentMinX,'f',0));
    mCurrentXMaxEdit->setText(locale.toString(mResultCurrentMaxX,'f',0));

    mCurrentXMinEdit->blockSignals(false);
    mCurrentXMaxEdit->blockSignals(false);
}

void ResultsView::setXSpin(const double value)
{
    mXSpin->blockSignals(true);
    mXSpin->setValue(value);
    mXSpin->blockSignals(false);
}

void ResultsView::setXSlider(const int value)
{
    mXSlider->blockSignals(true);
    mXSlider->setValue(value);
    mXSlider->blockSignals(false);
}

void ResultsView::setXIntervals()
{
    //mMinorScaleEdit->blockSignals(true);
    //mMajorScaleEdit->blockSignals(true);

    QLocale locale = QLocale();
    mMinorScaleEdit->setText(locale.toString(mMinorCountScale));
    mMajorScaleEdit->setText(locale.toString(mMajorScale));

    //mMinorScaleEdit->blockSignals(false);
    //mMajorScaleEdit->blockSignals(false);
}

#pragma mark Controls actions

void ResultsView::applyRuler(const double min, const double max)
{
    mResultCurrentMinX = min;
    mResultCurrentMaxX = max;

    setXRange();
    updateGraphsZoomX();
}

void ResultsView::applyStudyPeriod()
{
    if (isPostDistribGraph()) {
        if (mCurrentVariable == GraphViewResults::eTheta
           || mCurrentVariable == GraphViewResults::eTempo
           || mCurrentVariable == GraphViewResults::eActivity)
        {
            mResultCurrentMinX = mModel->mSettings.getTminFormated();
            mResultCurrentMaxX = mModel->mSettings.getTmaxFormated();
            mResultZoomX = (mResultMaxX - mResultMinX)/(mResultCurrentMaxX - mResultCurrentMinX);
        }
        else if ((mCurrentVariable == GraphViewResults::eSigma) ||
                (mCurrentVariable == GraphViewResults::eDuration) ||
                (mCurrentVariable == GraphViewResults::eVG) ||
                (mCurrentVariable == GraphViewResults::eAlpha))
        {
            mResultCurrentMinX = 0.;
            mResultCurrentMaxX = mCurrentVariableMaxX;
            mResultZoomX = (mResultMaxX - mResultMinX)/(mResultCurrentMaxX - mResultCurrentMinX);
        }
        
        Scale Xscale;
        Xscale.findOptimal(mResultCurrentMinX, mResultCurrentMaxX, 10);

        mMajorScale = Xscale.mark;
        mMinorCountScale = Xscale.tip;

        mRuler->setScaleDivision(Xscale);
        mRuler->setCurrent(mResultCurrentMinX, mResultCurrentMaxX);

        updateGraphsZoomX();

        setXRange();
        setXSlider(zoomToSlider(mResultZoomX));
        setXSpin(mResultZoomX);
        setXIntervals();
    }
}

void ResultsView::applyXRange()
{
    // --------------------------------------------------
    //  Find new current min & max (check range validity !)
    //  Update mResultZoomX
    // --------------------------------------------------
    QString minStr = mCurrentXMinEdit->text();
    bool minIsNumber = true;
    double min = locale().toDouble(&minStr, &minIsNumber);

    QString maxStr = mCurrentXMaxEdit->text();
    bool maxIsNumber = true;
    double max = locale().toDouble(&maxStr, &maxIsNumber);

    if (minIsNumber && maxIsNumber && ((min != mResultCurrentMinX) || (max != mResultCurrentMaxX))) {
        mResultCurrentMinX = std::max(min, mRuler->mMin);
        mResultCurrentMaxX = std::min(max, mRuler->mMax);

        mResultZoomX = (mResultMaxX - mResultMinX)/ (mResultCurrentMaxX - mResultCurrentMinX);

        mRuler->setCurrent(mResultCurrentMinX, mResultCurrentMaxX);

        updateGraphsZoomX();

        setXRange();
        setXSlider(zoomToSlider(mResultZoomX));
        setXSpin(mResultZoomX);
    }
}

void ResultsView::applyXSlider(int value)
{
    setXSpin(sliderToZoom(value));
    updateZoomX();
}

void ResultsView::applyXSpin(double value)
{
    setXSlider(zoomToSlider(value));
    updateZoomX();
}

void ResultsView::applyXIntervals()
{
    QString majorStr = mMajorScaleEdit->text();
    bool isMajorNumber = true;
    double majorNumber = locale().toDouble(&majorStr, &isMajorNumber);
    if (!isMajorNumber || majorNumber < 1)
        return;

    QString minorStr = mMinorScaleEdit->text();
    bool isMinorNumber = true;
    double minorNumber = locale().toDouble(&minorStr, &isMinorNumber);
    if (!isMinorNumber || minorNumber <= 1)
        return;

    mMajorScale = majorNumber;
    mMinorCountScale = (int) minorNumber;

    mRuler->setScaleDivision(mMajorScale, mMinorCountScale);

    QList<GraphViewResults*> graphs = currentGraphs(false);
    for (GraphViewResults*& graph : graphs) {
        graph->changeXScaleDivision(mMajorScale, mMinorCountScale);
    }

    QPair<GraphViewResults::Variable, GraphViewResults::TypeGraph> key(mCurrentVariable, mCurrentTypeGraph);
    mScales[key] = QPair<double, int>(mMajorScale, mMinorCountScale);
}

void ResultsView::applyYSlider(int value)
{
    mYSpin->blockSignals(true);
    mYSpin->setValue(value);
    mYSpin->blockSignals(false);

    updateGraphsHeight();
}

void ResultsView::applyYSpin(int value)
{
    mYSlider->blockSignals(true);
    mYSlider->setValue(value);
    mYSlider->blockSignals(false);

    updateGraphsHeight();
}

void ResultsView::applyFont()
{
    bool ok = false;
    const QFont& currentFont = font();
    QFont font(QFontDialog::getFont(&ok, currentFont, this));
    if (ok) {
        setFont(font);
        //updateGraphsFont(); TODO !! (to restore better...)
        mFontBut->setText(font.family() + ", " + QString::number(font.pointSizeF()));
    }
}

void ResultsView::applyThickness(int value)
{
    QList<GraphViewResults*> graphs = allGraphs();
    for (GraphViewResults*& graph : graphs) {
        graph->setGraphsThickness(value);
    }
}

void ResultsView::applyOpacity(int value)
{
    const int opValue = value * 10;
    QList<GraphViewResults*> graphs = allGraphs();
    for (GraphViewResults*& graph : graphs) {
        graph->setGraphsOpacity(opValue);
    }
}

void ResultsView::applyFFTLength()
{
    const int len = mFFTLenCombo->currentText().toInt();
    mModel->setFFTLength(len);
}

void ResultsView::applyBandwidth()
{
    const double bandwidth = mBandwidthSpin->value();
    mModel->setBandwidth(bandwidth);
}

void ResultsView::applyThreshold()
{
    const double hpd = locale().toDouble(mThresholdEdit->text());
    mModel->setThreshold(hpd);
}

void ResultsView::applyNextPage()
{
    if ((mCurrentPage + 1) * mGraphsPerPage < mMaximunNumberOfVisibleGraph) {
        ++mCurrentPage;
        updateControls();
        createGraphs();
        updateLayout();
    }
}

void ResultsView::applyPreviousPage()
{
    if (mCurrentPage > 0) {
        --mCurrentPage;
        updateControls();
        createGraphs();
        updateLayout();
    }
}

void ResultsView::applyGraphsPerPage(int graphsPerPage)
{
    mGraphsPerPage = graphsPerPage;
    mCurrentPage = 0;
    updateControls();
    createGraphs();
    updateLayout();
}

void ResultsView::showInfos(bool show)
{
    mTempoStatCheck->setChecked(show);
    mStatCheck->setChecked(show);
    
    QList<GraphViewResults*> graphs = allGraphs();
    for (GraphViewResults*& graph : graphs) {
        graph->showNumericalResults(show);
    }
    updateLayout();
}

#pragma mark Graph selection and export

void ResultsView::saveGraphData()
{
    QList<GraphViewResults*> graphs = currentGraphs(true);
    for (auto&& graph : graphs) {
        graph->saveGraphData();
    }
}

void ResultsView::resultsToClipboard()
{
    QString resultText;
    QList<GraphViewResults*> graphs = currentGraphs(true);
    for (auto&& graph : graphs) {
        resultText += graph->getTextAreaToPlainText();
    }
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(resultText);
}

void ResultsView::imageToClipboard()
{
    QList<GraphViewResults*> graphs = currentGraphs(true);

    if (!graphs.isEmpty()) {
        GraphViewResults* firstGraph = graphs.at(0);

        const int versionHeight = 20;
        short pr = short(AppSettings::mPixelRatio);

        QImage image(firstGraph->width() * pr, (graphs.size() * firstGraph->height() + versionHeight) * pr , QImage::Format_ARGB32_Premultiplied);

        image.setDevicePixelRatio(pr);
        image.fill(Qt::transparent);

        QPainter p;
        p.begin(&image);
        p.setRenderHint(QPainter::Antialiasing);

        QPoint ptStart (0, 0);
        for (auto&& graph : graphs) {
            graph->showSelectedRect(false);
            graph->render(&p, ptStart, QRegion(0, 0, graph->width(), graph->height()));
            ptStart = QPoint(0, ptStart.y() + graph->height());
            graph->showSelectedRect(true);
        }

        p.setPen(Qt::black);
        p.setBrush(Qt::white);
        p.fillRect(0, ptStart.y(), firstGraph->width(), versionHeight, Qt::white);
        p.drawText(0, ptStart.y(), firstGraph->width(), versionHeight,
                   Qt::AlignCenter,
                   qApp->applicationName() + " " + qApp->applicationVersion());


        p.end();

        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setImage(image);
    }
}

void ResultsView::saveAsImage()
{
    QList<GraphViewResults*> graphs = currentGraphs(true);
    if (graphs.isEmpty()) {
        return;
    }

    // --------------------------------------------------
    //  Ask for a file name and type (SVG or Image)
    // --------------------------------------------------
    QString fileName = QFileDialog::getSaveFileName(qApp->activeWindow(),
        tr("Save graph image as..."),
        MainWindow::getInstance()->getCurrentPath(),
        QObject::tr("Image (*.png);;Photo (*.jpg);;Scalable Vector Graphics (*.svg)"));

    if (!fileName.isEmpty()) {
        // --------------------------------------------------
        //  Get the file extension
        // --------------------------------------------------
        QFileInfo fileInfo;
        fileInfo = QFileInfo(fileName);
        QString fileExtension = fileInfo.suffix();
        bool asSvg = fileName.endsWith(".svg");


        const int heightText (2 * qApp->fontMetrics().height());
        const QString versionStr = qApp->applicationName() + " " + qApp->applicationVersion();

        // --- if png
        //const int versionHeight (20);
        if (!asSvg) {

            const short pr = short (AppSettings::mPixelRatio);

            QImage image (graphs.first()->width() * pr, (graphs.size() * graphs.first()->height() + heightText) * pr , QImage::Format_ARGB32_Premultiplied); //Format_ARGB32_Premultiplied //Format_ARGB32

            if (image.isNull() )
                qDebug()<< " image width = 0";

            image.setDevicePixelRatio(pr);
            image.fill(Qt::transparent);

            QPainter p;
            p.begin(&image);
            p.setRenderHint(QPainter::Antialiasing);
            p.setFont(qApp->font());

            QPoint ptStart (0, 0);
            for (auto &&graph : graphs) {
                graph->showSelectedRect(false);
                //GraphView::Rendering memoRendering= graph->getRendering();
                //graph->setRendering(GraphView::eHD);
                graph->render(&p, ptStart, QRegion(0, 0, graph->width(), graph->height()));
                ptStart = QPoint(0, ptStart.y() + graph->height());
                graph->showSelectedRect(true);
               // graph->setRendering(memoRendering);
            }
            p.setPen(Qt::black);
            p.setBrush(Qt::white);
            p.fillRect(0, ptStart.y(), graphs.first()->width(), heightText, Qt::white);
            p.drawText(0, ptStart.y(), graphs.first()->width(), heightText,
                       Qt::AlignCenter,
                       versionStr);
            p.end();

            if (fileExtension=="png") {
                image.save(fileName, "png");

            } else if (fileExtension == "jpg") {
                int imageQuality = AppSettings::mImageQuality;
                image.save(fileName, "jpg",imageQuality);

            } else if (fileExtension == "bmp") {
                image.save(fileName, "bmp");
            }

        } // not svg
        // if svg type
        else {
            //Rendering memoRendering= mRendering;
            const int wGraph = graphs.first()->width();
            const int hGraph = graphs.size() * graphs.first()->height();
            QRect rTotal ( 0, 0, wGraph, hGraph + heightText );
            // Set SVG Generator

            QSvgGenerator svgGen;
            svgGen.setFileName(fileName);
            //svgGen.setSize(rTotal.size());
            svgGen.setViewBox(rTotal);
            svgGen.setTitle(versionStr);
            svgGen.setDescription(fileName);

            QPainter painter;
            painter.begin(&svgGen);
            //font().wordSpacing();

            QPoint ptStart (0, 0);
            for (auto&& graph : graphs) {
                graph->showSelectedRect(false);
                 /* We can not have a svg graph in eSD Rendering Mode */
                //GraphView::Rendering memoRendering= graph->getRendering();
                //graph->setRendering(GraphView::eHD);
                graph->render(&painter, ptStart, QRegion(0, 0, graph->width(), graph->height()));
                ptStart = QPoint(0, ptStart.y() + graph->height());
                graph->showSelectedRect(true);
                //graph->setRendering(memoRendering);
            }
            painter.setPen(Qt::black);
            painter.drawText(0, ptStart.y(), wGraph, heightText,
                             Qt::AlignCenter,
                             versionStr);

            painter.end();

        }
    // end if not Empty filename
    }
}

/**
 * @brief ResultsView::exportResults export result into several files
 *
 */
void ResultsView::exportResults()
{
    if (mModel) {

        const QString csvSep = AppSettings::mCSVCellSeparator;
        const int precision = AppSettings::mPrecision;
        QLocale csvLocal = AppSettings::mCSVDecSeparator == "." ? QLocale::English : QLocale::French;

        csvLocal.setNumberOptions(QLocale::OmitGroupSeparator);

        const QString currentPath = MainWindow::getInstance()->getCurrentPath();
        const QString dirPath = QFileDialog::getSaveFileName(qApp->activeWindow(),
                                                        tr("Export to directory..."),
                                                        currentPath,
                                                       tr("Directory"));

        if (!dirPath.isEmpty()) {
            QDir dir(dirPath);
            if (dir.exists()) {
                /*if(QMessageBox::question(qApp->activeWindow(), tr("Are you sure?"), tr("This directory already exists and all its content will be deleted. Do you really want to replace it?")) == QMessageBox::No){
                    return;
                }*/
                dir.removeRecursively();
            }
            dir.mkpath(".");

            // copy tabs ------------------------------------------
            const QString version = qApp->applicationName() + " " + qApp->applicationVersion();
            const QString projectName = tr("Project filename : %1").arg(MainWindow::getInstance()->getNameProject()) + "<br>";

            QFile file(dirPath + "/Log_Model_Description.html");
            if (file.open(QFile::WriteOnly | QFile::Truncate)) {
                QTextStream output(&file);
                output<<"<!DOCTYPE html>"<< Qt::endl;
                output<<"<html>"<< Qt::endl;
                output<<"<body>"<< Qt::endl;

                output<<"<h2>"<< version << "</h2>" << Qt::endl;
                output<<"<h2>"<< projectName+ "</h2>" << Qt::endl;
                output<<"<hr>";
                output<<mModel->getModelLog();

                output<<"</body>"<< Qt::endl;
                output<<"</html>"<< Qt::endl;
            }
            file.close();

            file.setFileName(dirPath + "/Log_MCMC_Initialization.html");

            if (file.open(QFile::WriteOnly | QFile::Truncate)) {
                QTextStream output(&file);
                output<<"<!DOCTYPE html>"<< Qt::endl;
                output<<"<html>"<< Qt::endl;
                output<<"<body>"<< Qt::endl;

                output<<"<h2>"<< version << "</h2>" << Qt::endl;
                output<<"<h2>"<< projectName+ "</h2>" << Qt::endl;
                output<<"<hr>";
                output<<mModel->getMCMCLog();

                output<<"</body>"<< Qt::endl;
                output<<"</html>"<< Qt::endl;
            }
            file.close();

            file.setFileName(dirPath + "/Log_Posterior_Distrib_Stats.html");

            if (file.open(QFile::WriteOnly | QFile::Truncate)) {
                QTextStream output(&file);
                output<<"<!DOCTYPE html>"<< Qt::endl;
                output<<"<html>"<< Qt::endl;
                output<<"<body>"<< Qt::endl;

                output<<"<h2>"<< version << "</h2>" << Qt::endl;
                output<<"<h2>"<< projectName+ "</h2>" << Qt::endl;
                output<<"<hr>";
                output<<mModel->getResultsLog();

                output<<"</body>"<< Qt::endl;
                output<<"</html>"<< Qt::endl;
            }
            file.close();

            const QList<QStringList> stats = mModel->getStats(csvLocal, precision, true);
            saveCsvTo(stats, dirPath + "/Synthetic_Stats_Table.csv", csvSep, true);

            if (mModel->mPhases.size() > 0) {
                const QList<QStringList> phasesTraces = mModel->getPhasesTraces(csvLocal, false);
                saveCsvTo(phasesTraces, dirPath + "/Chain_all_Phases.csv", csvSep, false);

                for (int i=0; i<mModel->mPhases.size(); ++i) {
                    const QList<QStringList> phaseTrace = mModel->getPhaseTrace(i,csvLocal, false);
                    const QString name = mModel->mPhases.at(i)->mName.toLower().simplified().replace(" ", "_");
                    saveCsvTo(phaseTrace, dirPath + "/Chain_Phase_" + name + ".csv", csvSep, false);
                }
            }
            QList<QStringList> eventsTraces = mModel->getEventsTraces(csvLocal, false);
            saveCsvTo(eventsTraces, dirPath + "/Chain_all_Events.csv", csvSep, false);
        }
    }
}

void ResultsView::exportFullImage()
{
    //  define ScrollArea
    enum ScrollArrea{
        eScrollPhases = 0,
        eScrollEvents = 1,
        eScrollTempo = 2
    };

    //ScrollArrea witchScroll;
    bool printAxis = (mGraphHeight < GraphViewResults::mHeightForVisibleAxis);

    QWidget* curWid (nullptr);

    type_data max;

    if (mGraphListTab->currentIndex() == 0) {
        curWid = mEventsScrollArea->widget();
        curWid->setFont(mByEventsGraphs.at(0)->font());
        max = mByEventsGraphs.at(0)->getGraph()->maximumX();

    } else if(mGraphListTab->currentIndex() == 1) {
        curWid = mPhasesScrollArea->widget();
        curWid->setFont(mByPhasesGraphs.at(0)->font());
        max = mByPhasesGraphs.at(0)->getGraph()->maximumX();

    } else if(mGraphListTab->currentIndex() == 2) {
        curWid = mTempoScrollArea->widget();
        curWid->setFont(mByTempoGraphs.at(0)->font());
        max = mByTempoGraphs.at(0)->getGraph()->maximumX();

    } else
        return;

    // --------------------------------------------------------------------
    // Force rendering to HD for export
    //int rendering = mRenderCombo->currentIndex();
  //  updateRendering(1);

    AxisWidget* axisWidget = nullptr;
    QLabel* axisLegend = nullptr;
    QFontMetrics fm (font());
    int axeHeight (int (fm.ascent() * 2.2)); // equal MarginBottom()

    int legendHeight (int (fm.ascent() *2));
    if (printAxis) {
        curWid->setFixedHeight(curWid->height() + axeHeight + legendHeight );

        axisWidget = new AxisWidget(nullptr, curWid);
        axisWidget->mMarginLeft = mMarginLeft;
        axisWidget->mMarginRight = mMarginRight;
        axisWidget->setScaleDivision(mMajorScale, mMinorCountScale);

        if (mStatCheck->isChecked()) {
            axisWidget->setGeometry(0, curWid->height() - axeHeight, int (curWid->width()*2./3.), axeHeight);
            axisWidget->updateValues(int (curWid->width()*2./3. - axisWidget->mMarginLeft - axisWidget->mMarginRight), 50, mResultCurrentMinX, mResultCurrentMaxX);

        } else {
            axisWidget->setGeometry(0, curWid->height() - axeHeight, curWid->width(), axeHeight);
            axisWidget->updateValues(int (curWid->width() - axisWidget->mMarginLeft - axisWidget->mMarginRight), 50, mResultCurrentMinX, mResultCurrentMaxX);
        }

        axisWidget->mShowText = true;
        axisWidget->setAutoFillBackground(true);
        axisWidget->mShowSubs = true;
        axisWidget->mShowSubSubs = true;
        axisWidget->mShowArrow = true;
        axisWidget->mShowText = true;

        axisWidget->raise();
        axisWidget->setVisible(true);

        QString legend = "";
        if (mCurrentTypeGraph == GraphViewResults::ePostDistrib && ( mCurrentVariable == GraphViewResults::eTheta
                                                                     || mCurrentVariable == GraphViewResults::eTempo
                                                                     || mCurrentVariable == GraphViewResults::eActivity) )
            legend = DateUtils::getAppSettingsFormatStr();

        else if (mCurrentTypeGraph == GraphViewResults::eTrace || mCurrentTypeGraph == GraphViewResults::eAccept)
            legend = "Iterations";

        else if (mCurrentTypeGraph == GraphViewResults::ePostDistrib && mCurrentVariable == GraphViewResults::eDuration)
            legend = "Years";


        axisLegend = new QLabel(legend, curWid);

        axisLegend->setFont(font());
        QFontMetrics fm(font());
        if (mStatCheck->isChecked())
            axisLegend->setGeometry(fm.boundingRect(legend).width(), curWid->height() - axeHeight - legendHeight, int (curWid->width()*2./3. - 10), legendHeight);
        else
            axisLegend->setGeometry(int (curWid->width() - fm.boundingRect(legend).width() - mMarginRight), curWid->height() - axeHeight - legendHeight, fm.boundingRect(legend).width() , legendHeight);

        axisLegend->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        axisLegend->raise();
        axisLegend->setVisible(true);
    }

    QFileInfo fileInfo = saveWidgetAsImage(curWid,
                                           QRect(0, 0, curWid->width() , curWid->height()),
                                           tr("Save graph image as..."),
                                           MainWindow::getInstance()->getCurrentPath());

    // Delete additional widgets if necessary :
    if (printAxis) {
        if (axisWidget) {
            axisWidget->setParent(nullptr);
            delete axisWidget;
        }
        if (axisLegend) {
            axisLegend->setParent(nullptr);
            delete axisLegend;
        }
        curWid->setFixedHeight(curWid->height() - axeHeight - legendHeight);
    }


    // Revert to default display :

    if (fileInfo.isFile())
        MainWindow::getInstance()->setCurrentPath(fileInfo.dir().absolutePath());

    // Reset rendering back to its current value
   // updateRendering(rendering);

}

#pragma mark Chronocurve

bool ResultsView::isChronocurve() const
{
    return (this->modelChronocurve() != nullptr);
}

ModelChronocurve* ResultsView::modelChronocurve() const
{
    return dynamic_cast<ModelChronocurve*>(mModel);
}
