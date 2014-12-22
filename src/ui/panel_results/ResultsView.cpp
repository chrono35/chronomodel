#include "ResultsView.h"
#include "GraphView.h"
#include "GraphViewDate.h"
#include "GraphViewEvent.h"
#include "GraphViewPhase.h"
#include "Tabs.h"
#include "Ruler.h"
#include "ZoomControls.h"
#include "Marker.h"
#include "ScrollCompressor.h"

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

#include <QtWidgets>
#include <iostream>


ResultsView::ResultsView(QWidget* parent, Qt::WindowFlags flags):QWidget(parent, flags),
mModel(0),
mMargin(5),
mOptionsW(200),
mLineH(15),
mGraphLeft(130),
mRulerH(40),
mTabsH(30),
mGraphsH(130),
mHasPhases(false)
{
    mTabs = new Tabs(this);
    mTabs->addTab(tr("Posterior distrib."));
    mTabs->addTab(tr("History plots"));
    mTabs->addTab(tr("Acceptation rate"));
    mTabs->addTab(tr("Autocorrelation"));
    
    connect(mTabs, SIGNAL(tabClicked(int)), this, SLOT(changeTab(int)));
    
    // -------------
    
    mRuler = new Ruler(this);
    mRuler->showControls(false);
    
    mStack = new QStackedWidget(this);
    
    mEventsScrollArea = new QScrollArea();
    mEventsScrollArea->setMouseTracking(true);
    mStack->addWidget(mEventsScrollArea);
    
    mPhasesScrollArea = new QScrollArea();
    mPhasesScrollArea->setMouseTracking(true);
    mStack->addWidget(mPhasesScrollArea);
    
    mTimer = new QTimer();
    mTimer->setSingleShot(true);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(updateScrollHeights()));
    
    mMarker = new Marker(this);
    
    setMouseTracking(true);
    mStack->setMouseTracking(true);
    
    connect(mRuler, SIGNAL(zoomChanged(float, float)), this, SLOT(setGraphZoom(float, float)));
    
    // ----------
    
    mByPhasesBut = new Button(tr("By phases"), this);
    mByPhasesBut->setCheckable(true);
    mByPhasesBut->setChecked(true);
    mByPhasesBut->setAutoExclusive(true);
    mByPhasesBut->setFlatHorizontal();
    
    mByEventsBut = new Button(tr("By events"), this);
    mByEventsBut->setCheckable(true);
    mByEventsBut->setChecked(false);
    mByEventsBut->setAutoExclusive(true);
    mByEventsBut->setFlatHorizontal();
    
    connect(mByPhasesBut, SIGNAL(toggled(bool)), this, SLOT(showByPhases(bool)));
    connect(mByEventsBut, SIGNAL(toggled(bool)), this, SLOT(showByEvents(bool)));
    
    // ----------
    
    mZoomWidget = new QWidget();
    mZoomWidget->setFixedHeight(mRulerH);
    
    mZoomInBut = new Button(mZoomWidget);
    mZoomInBut->setIcon(QIcon(":zoom_plus.png"));
    mZoomInBut->setFlatHorizontal();
    
    mZoomOutBut = new Button(mZoomWidget);
    mZoomOutBut->setIcon(QIcon(":zoom_minus.png"));
    mZoomOutBut->setFlatHorizontal();
    
    mZoomDefaultBut = new Button(mZoomWidget);
    mZoomDefaultBut->setIcon(QIcon(":zoom_default.png"));
    mZoomDefaultBut->setFlatHorizontal();
    
    connect(mZoomInBut, SIGNAL(clicked()), mRuler, SLOT(zoomIn()));
    connect(mZoomOutBut, SIGNAL(clicked()), mRuler, SLOT(zoomOut()));
    connect(mZoomDefaultBut, SIGNAL(clicked()), mRuler, SLOT(zoomDefault()));
    
    // -------------------------
    
    mDisplayTitle = new Label(tr("Display options"));
    mDisplayTitle->setIsTitle(true);
    
    mCompressor = new ScrollCompressor(this);
    mCompressor->setVertical(false);
    mCompressor->showText(tr("Scale"), true);
    
    mUnfoldBut = new Button(tr("Unfold"));
    mUnfoldBut->setCheckable(true);
    mUnfoldBut->setFlatHorizontal();
    mUnfoldBut->setIcon(QIcon(":picture_save.png"));
    mUnfoldBut->setFixedHeight(50);
    
    mInfosBut = new Button(tr("Results"));
    mInfosBut->setCheckable(true);
    mInfosBut->setFlatHorizontal();
    mInfosBut->setIcon(QIcon(":picture_save.png"));
    mInfosBut->setFixedHeight(50);
    
    mExportImgBut = new Button(tr("Save"));
    mExportImgBut->setFlatHorizontal();
    mExportImgBut->setIcon(QIcon(":picture_save.png"));
    mExportImgBut->setFixedHeight(50);
    
    mDisplayGroup = new QWidget();
    QHBoxLayout* displayButsLayout = new QHBoxLayout();
    displayButsLayout->setContentsMargins(0, 0, 0, 0);
    displayButsLayout->setSpacing(0);
    displayButsLayout->addWidget(mUnfoldBut);
    displayButsLayout->addWidget(mInfosBut);
    displayButsLayout->addWidget(mExportImgBut);
    QVBoxLayout* displayLayout = new QVBoxLayout();
    displayLayout->setContentsMargins(0, 0, 0, 0);
    displayLayout->setSpacing(0);
    displayLayout->addWidget(mCompressor);
    displayLayout->addWidget(mZoomWidget);
    displayLayout->addLayout(displayButsLayout);
    mDisplayGroup->setLayout(displayLayout);
    
    connect(mCompressor, SIGNAL(valueChanged(float)), this, SLOT(compress(float)));
    connect(mUnfoldBut, SIGNAL(toggled(bool)), this, SLOT(unfoldResults(bool)));
    connect(mInfosBut, SIGNAL(toggled(bool)), this, SLOT(showInfos(bool)));
    connect(mExportImgBut, SIGNAL(clicked()), this, SLOT(exportFullImage()));
    
    // -----------
    
    mChainsTitle = new Label(tr("MCMC Chains"));
    mChainsTitle->setIsTitle(true);
    mChainsGroup = new QWidget();
    mAllChainsCheck = new CheckBox(tr("Chains concatenation"), mChainsGroup);
    mAllChainsCheck->setChecked(true);
    mChainsGroup->setFixedHeight(2*mMargin + mLineH);
    
    connect(mAllChainsCheck, SIGNAL(clicked()), this, SLOT(updateGraphs()));
    
    // -----------
    
    mDataTitle = new Label(tr("Results options"));
    mDataTitle->setIsTitle(true);
    mDataGroup = new QWidget();
    
    mDataThetaRadio = new RadioButton(tr("Calendar dates"), mDataGroup);
    mDataSigmaRadio = new RadioButton(tr("Individual variances"), mDataGroup);
    mDataCalibCheck = new CheckBox(tr("Distrib. of calib. dates"), mDataGroup);
    mWiggleCheck = new CheckBox(tr("Wiggle unshifted"), mDataGroup);
    mDataThetaRadio->setChecked(true);
    mDataCalibCheck->setChecked(true);
    
    connect(mDataThetaRadio, SIGNAL(clicked()), this, SLOT(updateGraphs()));
    connect(mDataCalibCheck, SIGNAL(clicked()), this, SLOT(updateGraphs()));
    connect(mWiggleCheck, SIGNAL(clicked()), this, SLOT(updateGraphs()));
    connect(mDataSigmaRadio, SIGNAL(clicked()), this, SLOT(updateGraphs()));
    
    // -----------
    
    mPostDistOptsTitle = new Label(tr("Post. distrib. options"));
    mPostDistOptsTitle->setIsTitle(true);
    mPostDistGroup = new QWidget();
    
    mHPDCheck = new CheckBox(tr("Show credibility"), mPostDistGroup);
    mHPDCheck->setChecked(true);
    mThreshLab = new Label(tr("HPD / Credibility (%)") + " :", mPostDistGroup);
    mThreshLab->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mHPDEdit = new LineEdit(mPostDistGroup);
    mHPDEdit->setText("95");
    
    connect(mHPDEdit, SIGNAL(textChanged(const QString&)), this, SLOT(generateHPD()));
    connect(mHPDCheck, SIGNAL(clicked()), this, SLOT(updateGraphs()));
    connect(mHPDEdit, SIGNAL(textChanged(const QString&)), this, SLOT(updateGraphs()));
    
    mRawCheck = new CheckBox(tr("Raw results"), mPostDistGroup);
    mRawCheck->setChecked(false);
    
    connect(mRawCheck, SIGNAL(clicked()), this, SLOT(updateGraphs()));
    
    mFFTLenLab = new Label(tr("FFT length") + " :", mPostDistGroup);
    mFFTLenLab->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mFFTLenCombo = new QComboBox(mPostDistGroup);
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
    mFFTLenCombo->setCurrentText("1024");
    
    mComboH = mFFTLenCombo->sizeHint().height();
    mTabsH = mComboH + 2*mMargin;
    
    connect(mFFTLenCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFFTLength()));
    
    mHFactorLab = new Label(tr("Bandwidth factor") + " :", mPostDistGroup);
    mHFactorLab->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mHFactorEdit = new LineEdit(mPostDistGroup);
    mHFactorEdit->setText("1");
    
    connect(mHFactorEdit, SIGNAL(returnPressed()), this, SLOT(updateHFactor()));
    
    // -------------------------
    
    mOptionsWidget = new QWidget(this);
    
    QVBoxLayout* optionsLayout = new QVBoxLayout();
    optionsLayout->setContentsMargins(0, 0, 0, 0);
    optionsLayout->setSpacing(0);
    optionsLayout->addWidget(mDisplayTitle);
    optionsLayout->addWidget(mDisplayGroup);
    optionsLayout->addWidget(mChainsTitle);
    optionsLayout->addWidget(mChainsGroup);
    optionsLayout->addWidget(mDataTitle);
    optionsLayout->addWidget(mDataGroup);
    optionsLayout->addWidget(mPostDistOptsTitle);
    optionsLayout->addWidget(mPostDistGroup);
    optionsLayout->addStretch();
    mOptionsWidget->setLayout(optionsLayout);
    
    // -------------------------
    
    mMarker->raise();
}

ResultsView::~ResultsView()
{
    
}

void ResultsView::doProjectConnections(Project* project)
{
    connect(project, SIGNAL(mcmcStarted()), this, SLOT(clearResults()));
    connect(project, SIGNAL(mcmcFinished(MCMCLoopMain&)), this, SLOT(updateResults(MCMCLoopMain&)));
}

void ResultsView::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.fillRect(width() - mOptionsW, 0, mOptionsW, height(), QColor(220, 220, 220));
}

void ResultsView::mouseMoveEvent(QMouseEvent* e)
{
    int x = e->pos().x() - 5;
    x = (x >= mGraphLeft) ? x : mGraphLeft;
    x = (x <= width() - mOptionsW) ? x : width() - mOptionsW;
    mMarker->setGeometry(x, mMarker->pos().y(), mMarker->width(), mMarker->height());
}

void ResultsView::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);
    updateLayout();
}

QList<QRect> ResultsView::getGeometries(const QList<GraphViewResults*>& graphs, bool open, bool byPhases)
{
    QList<QRect> rects;
    int y = 0;
    int h = mGraphsH;
    int sbe = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    
    for(int i=0; i<graphs.size(); ++i)
    {
        QRect rect = graphs[i]->geometry();
        rect.setY(y);
        rect.setWidth(width() - mOptionsW - sbe);
        
        if(byPhases)
        {
            GraphViewPhase* graph = dynamic_cast<GraphViewPhase*>(graphs[i]);
            rect.setHeight((graph || open) ? h : 0);
        }
        else
        {
            GraphViewEvent* graph = dynamic_cast<GraphViewEvent*>(graphs[i]);
            rect.setHeight((graph || open) ? h : 0);
        }
        y += rect.height();
        rects.append(rect);
    }
    return rects;
}

void ResultsView::updateLayout()
{
    int m = mMargin;
    int sbe = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    int dx = mLineH + m;
    int graphYAxis = 50;
    
    mByPhasesBut->setGeometry(0, 0, mGraphLeft/2, mRulerH);
    mByEventsBut->setGeometry(mGraphLeft/2, 0, mGraphLeft/2, mRulerH);
    
    mTabs->setGeometry(mGraphLeft + graphYAxis, 0, width() - mGraphLeft - mOptionsW - sbe - graphYAxis, mTabsH);
    mRuler->setGeometry(mGraphLeft + graphYAxis, mTabsH, width() - mGraphLeft - mOptionsW - sbe - graphYAxis, mRulerH);
    mStack->setGeometry(0, mTabsH + mRulerH, width() - mOptionsW, height() - mRulerH - mTabsH);
    mMarker->setGeometry(mMarker->pos().x(), mTabsH + sbe, mMarker->thickness(), height() - sbe - mTabsH);
    
    if(QWidget* wid = mEventsScrollArea->widget())
    {
        QList<QRect> geometries = getGeometries(mByEventsGraphs, mUnfoldBut->isChecked(), false);
        int h = 0;
        for(int i=0; i<mByEventsGraphs.size(); ++i)
        {
            mByEventsGraphs[i]->setGeometry(geometries[i]);
            h += geometries[i].height();
        }
        wid->setFixedSize(width() - sbe - mOptionsW, h);
        //qDebug() << "Graph events viewport : " << wid->geometry();
    }
    
    if(QWidget* wid = mPhasesScrollArea->widget())
    {
        QList<QRect> geometries = getGeometries(mByPhasesGraphs, mUnfoldBut->isChecked(), true);
        int h = 0;
        for(int i=0; i<mByPhasesGraphs.size(); ++i)
        {
            mByPhasesGraphs[i]->setGeometry(geometries[i]);
            h += geometries[i].height();
        }
        wid->setFixedSize(width() - sbe - mOptionsW, h);
        //qDebug() << "Graph phases viewport : " << wid->geometry();
    }
    
    mOptionsWidget->setGeometry(width() - mOptionsW, 0, mOptionsW, height());
    
    float zw = mOptionsW / 3;
    float zh = mRulerH;
    mZoomInBut->setGeometry(0, 0, zw, zh);
    mZoomDefaultBut->setGeometry(zw, 0, zw, zh);
    mZoomOutBut->setGeometry(2*zw, 0, zw, zh);
    
    int numChains = mCheckChainChecks.size();
    if(mTabs->currentIndex() == 0)
    {
        mChainsGroup->setFixedHeight(m + (numChains+1) * (mLineH + m));
        mAllChainsCheck->setGeometry(m, m, mChainsGroup->width()-2*m, mLineH);
        for(int i=0; i<numChains; ++i)
        {
            QRect geometry(m, m + (i+1) * (mLineH + m), mChainsGroup->width()-2*m, mLineH);
            mCheckChainChecks[i]->setGeometry(geometry);
            mChainRadios[i]->setGeometry(geometry);
        }
    }
    else
    {
        mChainsGroup->setFixedHeight(m + numChains * (mLineH + m));
        for(int i=0; i<numChains; ++i)
        {
            QRect geometry(m, m + i * (mLineH + m), mChainsGroup->width()-2*m, mLineH);
            mCheckChainChecks[i]->setGeometry(geometry);
            mChainRadios[i]->setGeometry(geometry);
        }
    }
    
    int y = m;
    mDataThetaRadio->setGeometry(m, y, mDataGroup->width() - 2*m, mLineH);
    if(mTabs->currentIndex() == 0)
    {
        mDataCalibCheck->setGeometry(m + dx, y += (m + mLineH), mDataGroup->width() - 2*m - dx, mLineH);
        mWiggleCheck->setGeometry(m + dx, y += (m + mLineH), mDataGroup->width() - 2*m - dx, mLineH);
    }
    mDataSigmaRadio->setGeometry(m, y += (m + mLineH), mDataGroup->width()-2*m, mLineH);
    mDataGroup->setFixedHeight(y += (m + mLineH));
    
    y = m;
    int sw = (mPostDistGroup->width() - 3*m) * 0.5;
    int w1 = (mPostDistGroup->width() - 3*m) * 0.7;
    int w2 = (mPostDistGroup->width() - 3*m) * 0.3;
    mHPDCheck->setGeometry(m, y, mPostDistGroup->width() - 2*m, mLineH);
    mThreshLab->setGeometry(m, y += (m + mLineH), w1, mLineH);
    mHPDEdit->setGeometry(2*m + w1, y, w2, mLineH);
    mRawCheck->setGeometry(m, y += (m + mLineH), mPostDistGroup->width() - 2*m, mLineH);
    mFFTLenLab->setGeometry(m, y += (m + mLineH), sw, mComboH);
    mFFTLenCombo->setGeometry(2*m + sw, y, sw, mComboH);
    mHFactorLab->setGeometry(m, y += (m + mComboH), w1, mLineH);
    mHFactorEdit->setGeometry(2*m + w1, y, w2, mLineH);
    mPostDistGroup->setFixedHeight(y += (m + mLineH));
    
    update();
}

void ResultsView::generateHPD()
{
    if(mModel)
    {
        mModel->generateCredibilityAndHPD(mChains, mHPDEdit->text().toInt());
    }
}

void ResultsView::updateFFTLength()
{
    if(mModel)
    {
        int len = mFFTLenCombo->currentText().toInt();
        float hFactor = mHFactorEdit->text().toFloat();
        
        mModel->generatePosteriorDensities(mChains, len, hFactor);
        mModel->generateNumericalResults(mChains);
        mModel->generateCredibilityAndHPD(mChains, mHPDEdit->text().toInt());
        
        updateGraphs();
    }
}

void ResultsView::updateHFactor()
{
    if(mModel)
    {
        int len = mFFTLenCombo->currentText().toInt();
        float hFactor = mHFactorEdit->text().toFloat();
        if(!(hFactor > 0 && hFactor <= 100))
        {
            hFactor = 1;
            mHFactorEdit->setText("1");
        }
        
        mModel->generatePosteriorDensities(mChains, len, hFactor);
        mModel->generateNumericalResults(mChains);
        mModel->generateCredibilityAndHPD(mChains, mHPDEdit->text().toInt());
        
        updateGraphs();
    }
}

void ResultsView::updateGraphs()
{
    updateRulerAreas();
    
    ProjectSettings s = mSettings;
    MCMCSettings mcmc = mMCMCSettings;
    
    GraphViewResults::Variable variable;
    if(mDataThetaRadio->isChecked()) variable = GraphViewResults::eTheta;
    else if(mDataSigmaRadio->isChecked()) variable = GraphViewResults::eSigma;
    
    GraphViewResults::Result result;
    if(mTabs->currentIndex() == 0) result = GraphViewResults::eHisto;
    else if(mTabs->currentIndex() == 1) result = GraphViewResults::eTrace;
    else if(mTabs->currentIndex() == 2) result = GraphViewResults::eAccept;
    else if(mTabs->currentIndex() == 3) result = GraphViewResults::eCorrel;
    
    bool showAllChains = mAllChainsCheck->isChecked();
    QList<bool> showChainList;
    if(mTabs->currentIndex() == 0)
    {
        for(int i=0; i<mCheckChainChecks.size(); ++i)
            showChainList.append(mCheckChainChecks[i]->isChecked());
    }
    else
    {
        for(int i=0; i<mChainRadios.size(); ++i)
            showChainList.append(mChainRadios[i]->isChecked());
    }
    bool showHpd = mHPDCheck->isChecked();
    int hdpThreshold = mHPDEdit->text().toInt();
    
    bool showRaw = mRawCheck->isChecked();
    
    bool showCalib = mDataCalibCheck->isChecked();
    bool showWiggle = mWiggleCheck->isChecked();
    
    // ---------------------------
    
    for(int i=0; i<mByPhasesGraphs.size(); ++i)
    {
        mByPhasesGraphs[i]->setResultToShow(result, variable, showAllChains, showChainList, showHpd, hdpThreshold, showCalib, showWiggle, showRaw);
    }
    for(int i=0; i<mByEventsGraphs.size(); ++i)
    {
        mByEventsGraphs[i]->setResultToShow(result, variable, showAllChains, showChainList, showHpd, hdpThreshold, showCalib, showWiggle, showRaw);
    }
    update();
}

void ResultsView::updateRulerAreas()
{
    ProjectSettings s = mSettings;
    MCMCSettings mcmc = mMCMCSettings;
    
    if(mTabs->currentIndex() == 0)
    {
        int min = s.mTmin;
        int max = s.mTmax;
        mRuler->clearAreas();
        mRuler->setRange(min, max);
        
        if(mDataThetaRadio->isChecked())
            mRuler->setRange(min, max);
        else if(mDataSigmaRadio->isChecked())
            mRuler->setRange(0, max - min);
    }
    else if(mTabs->currentIndex() == 3)
    {
        mRuler->clearAreas();
        mRuler->setRange(0, 100);
    }
    else
    {
        int curChainIdx = -1;
        for(int i=0; i<mChainRadios.size(); ++i)
            if(mChainRadios[i]->isChecked())
                curChainIdx = i;
        
        if(curChainIdx != -1)
        {
            int min = 0;
            int max = mChains[curChainIdx].mNumBurnIter + mChains[curChainIdx].mBatchIndex * mChains[curChainIdx].mNumBatchIter + mChains[curChainIdx].mNumRunIter;
            mRuler->setRange(min, max);
            mRuler->clearAreas();
            
            mRuler->addArea(0,
                            mChains[curChainIdx].mNumBurnIter,
                            QColor(235, 115, 100));
            
            mRuler->addArea(mChains[curChainIdx].mNumBurnIter,
                            mChains[curChainIdx].mNumBurnIter + mChains[curChainIdx].mBatchIndex * mChains[curChainIdx].mNumBatchIter,
                            QColor(250, 180, 90));
            
            mRuler->addArea(mChains[curChainIdx].mNumBurnIter + mChains[curChainIdx].mBatchIndex * mChains[curChainIdx].mNumBatchIter,
                            mChains[curChainIdx].mNumBurnIter + mChains[curChainIdx].mBatchIndex * mChains[curChainIdx].mNumBatchIter + mChains[curChainIdx].mNumRunIter,
                            QColor(130, 205, 110));
        }
    }
}

void ResultsView::clearResults()
{
    mByEventsBut->setVisible(false);
    mByPhasesBut->setVisible(false);
    
    for(int i=mCheckChainChecks.size()-1; i>=0; --i)
    {
        CheckBox* check = mCheckChainChecks.takeAt(i);
        disconnect(check, SIGNAL(clicked()), this, SLOT(updateGraphs()));
        check->setParent(0);
        delete check;
    }
    mCheckChainChecks.clear();
    
    for(int i=mChainRadios.size()-1; i>=0; --i)
    {
        RadioButton* but = mChainRadios.takeAt(i);
        disconnect(but, SIGNAL(clicked()), this, SLOT(updateGraphs()));
        but->setParent(0);
        delete but;
    }
    mChainRadios.clear();
    
    for(int i=0; i<mByEventsGraphs.size(); ++i)
    {
        mByEventsGraphs[i]->setParent(0);
        delete mByEventsGraphs[i];
    }
    mByEventsGraphs.clear();
    
    for(int i=0; i<mByPhasesGraphs.size(); ++i)
    {
        mByPhasesGraphs[i]->setParent(0);
        delete mByPhasesGraphs[i];
    }
    mByPhasesGraphs.clear();
    
    QWidget* eventsWidget = mEventsScrollArea->takeWidget();
    if(eventsWidget)
        delete eventsWidget;
    
    QWidget* phasesWidget = mPhasesScrollArea->takeWidget();
    if(phasesWidget)
        delete phasesWidget;
}

void ResultsView::updateResults(MCMCLoopMain& loop)
{
    clearResults();
    mChains = loop.chains();
    mModel = loop.mModel;
    mSettings = mModel->mSettings;
    mMCMCSettings = mModel->mMCMCSettings;
    mFFTLenCombo->setCurrentText("1024");
    
    if(!mModel)
        return;
    
    //mRuler->setRange(model->mSettings.mTmin, model->mSettings.mTmax);
    
    mHasPhases = (mModel->mPhases.size() > 0);
    
    mByEventsBut->setVisible(mHasPhases);
    mByPhasesBut->setVisible(mHasPhases);
    
    for(int i=0; i<mChains.size(); ++i)
    {
        CheckBox* check = new CheckBox(tr("Chain") + " " + QString::number(i+1), mChainsGroup);
        connect(check, SIGNAL(clicked()), this, SLOT(updateGraphs()));
        check->setVisible(true);
        mCheckChainChecks.append(check);
        
        RadioButton* radio = new RadioButton(tr("Chain") + " " + QString::number(i+1), mChainsGroup);
        connect(radio, SIGNAL(clicked()), this, SLOT(updateGraphs()));
        radio->setVisible(true);
        if(i == 0)
            radio->setChecked(true);
        mChainRadios.append(radio);
    }
    
    // ----------------------------------------------------
    //  Generate HPD (will then be updated only when HPD value changed)
    // ----------------------------------------------------
    generateHPD();
    
    // ----------------------------------------------------
    //  Phases View
    // ----------------------------------------------------
    
    QWidget* phasesWidget = new QWidget();
    phasesWidget->setMouseTracking(true);
    
    for(int p=0; p<(int)mModel->mPhases.size(); ++p)
    {
        Phase* phase = mModel->mPhases[p];
        GraphViewPhase* graphPhase = new GraphViewPhase(phasesWidget);
        graphPhase->setSettings(mModel->mSettings);
        graphPhase->setMCMCSettings(mModel->mMCMCSettings, mChains);
        graphPhase->setPhase(phase);
        mByPhasesGraphs.append(graphPhase);
        
        for(int i=0; i<(int)phase->mEvents.size(); ++i)
        {
            Event* event = phase->mEvents[i];
            GraphViewEvent* graphEvent = new GraphViewEvent(phasesWidget);
            graphEvent->setSettings(mModel->mSettings);
            graphEvent->setMCMCSettings(mModel->mMCMCSettings, mChains);
            graphEvent->setEvent(event);
            mByPhasesGraphs.append(graphEvent);
            
            for(int j=0; j<(int)event->mDates.size(); ++j)
            {
                Date& date = event->mDates[j];
                GraphViewDate* graphDate = new GraphViewDate(phasesWidget);
                graphDate->setSettings(mModel->mSettings);
                graphDate->setMCMCSettings(mModel->mMCMCSettings, mChains);
                graphDate->setDate(&date);
                graphDate->setColor(event->mColor);
                mByPhasesGraphs.append(graphDate);
            }
        }
    }
    mPhasesScrollArea->setWidget(phasesWidget);
    
    // ----------------------------------------------------
    //  Events View
    // ----------------------------------------------------
    
    QWidget* eventsWidget = new QWidget();
    eventsWidget->setMouseTracking(true);
    
    for(int i=0; i<(int)mModel->mEvents.size(); ++i)
    {
        Event* event = mModel->mEvents[i];
        GraphViewEvent* graphEvent = new GraphViewEvent(eventsWidget);
        graphEvent->setSettings(mModel->mSettings);
        graphEvent->setMCMCSettings(mModel->mMCMCSettings, mChains);
        graphEvent->setEvent(event);
        mByEventsGraphs.append(graphEvent);
        
        for(int j=0; j<(int)event->mDates.size(); ++j)
        {
            Date& date = event->mDates[j];
            GraphViewDate* graphDate = new GraphViewDate(eventsWidget);
            graphDate->setSettings(mModel->mSettings);
            graphDate->setMCMCSettings(mModel->mMCMCSettings, mChains);
            graphDate->setDate(&date);
            graphDate->setColor(event->mColor);
            mByEventsGraphs.append(graphDate);
        }
    }
    mEventsScrollArea->setWidget(eventsWidget);
    
    
    if(mHasPhases && mByPhasesBut->isChecked())
        showByPhases(true);
    else
        showByEvents(true);
    
    changeTab(0);
    
    // Done by changeTab :
    //updateLayout();
    //updateGraphs();
}

void ResultsView::unfoldResults(bool open)
{
    QList<QRect> geometries = getGeometries(mByEventsGraphs, open, false);
    for(int i=0; i<mByEventsGraphs.size(); ++i)
        mByEventsGraphs[i]->toggle(geometries[i]);

    geometries = getGeometries(mByPhasesGraphs, open, true);
    for(int i=0; i<mByPhasesGraphs.size(); ++i)
        mByPhasesGraphs[i]->toggle(geometries[i]);
    
    //updateScrollHeights();
    
    if(open)
    {
        updateScrollHeights();
    }
    else
    {
        mTimer->start(200);
    }
}

void ResultsView::updateScrollHeights()
{
    if(QWidget* wid = mEventsScrollArea->widget())
    {
        QList<QRect> geometries = getGeometries(mByEventsGraphs, mUnfoldBut->isChecked(), false);
        int h = 0;
        for(int i=0; i<geometries.size(); ++i)
            h += geometries[i].height();
        wid->setFixedHeight(h);
        //qDebug() << "Graph events viewport : " << wid->geometry();
    }
    if(QWidget* wid = mPhasesScrollArea->widget())
    {
        QList<QRect> geometries = getGeometries(mByPhasesGraphs, mUnfoldBut->isChecked(), true);
        int h = 0;
        for(int i=0; i<geometries.size(); ++i)
            h += geometries[i].height();
        wid->setFixedHeight(h);
        //qDebug() << "Graph phases viewport : " << wid->geometry();
    }
}

void ResultsView::showInfos(bool show)
{
    for(int i=0; i<mByEventsGraphs.size(); ++i)
        mByEventsGraphs[i]->showNumericalResults(show);
    
    for(int i=0; i<mByPhasesGraphs.size(); ++i)
        mByPhasesGraphs[i]->showNumericalResults(show);
}

void ResultsView::exportFullImage()
{
    QWidget* curWid = (mStack->currentWidget() == mPhasesScrollArea) ? mPhasesScrollArea->widget() : mEventsScrollArea->widget();
    
    QRect r(mGraphLeft, 0, curWid->width() - mGraphLeft, curWid->height());
    QFileInfo fileInfo = saveWidgetAsImage(curWid, r,
                                           tr("Save graph image as..."),
                                           MainWindow::getInstance()->getCurrentPath());
    if(fileInfo.isFile())
        MainWindow::getInstance()->setCurrentPath(fileInfo.dir().absolutePath());
}

void ResultsView::compress(float prop)
{
    float min = 20;
    float max = 200;
    mGraphsH = min + prop * (max - min);
    updateLayout();
}

void ResultsView::setGraphZoom(float min, float max)
{
    for(int i=0; i<mByPhasesGraphs.size(); ++i)
        mByPhasesGraphs[i]->zoom(min, max);
        
    for(int i=0; i<mByEventsGraphs.size(); ++i)
        mByEventsGraphs[i]->zoom(min, max);
}

void ResultsView::showByPhases(bool)
{
    mStack->setCurrentWidget(mPhasesScrollArea);
}

void ResultsView::showByEvents(bool)
{
    mStack->setCurrentWidget(mEventsScrollArea);
}

void ResultsView::changeTab(int index)
{
    mHPDCheck->setVisible(index == 0);
    mHPDEdit->setVisible(index == 0);
    mRawCheck->setVisible(index == 0);
    mFFTLenLab->setVisible(index == 0);
    mFFTLenCombo->setVisible(index == 0);
    mAllChainsCheck->setVisible(index == 0);
    mDataCalibCheck->setVisible(index == 0);
    mWiggleCheck->setVisible(index == 0);
    
    if(index == 0)
    {
        for(int i=0; i<mCheckChainChecks.size(); ++i)
            mCheckChainChecks[i]->setVisible(true);
        
        for(int i=0; i<mChainRadios.size(); ++i)
            mChainRadios[i]->setVisible(false);
    }
    else
    {
        for(int i=0; i<mCheckChainChecks.size(); ++i)
            mCheckChainChecks[i]->setVisible(false);
        
        for(int i=0; i<mChainRadios.size(); ++i)
            mChainRadios[i]->setVisible(true);
    }
    
    updateLayout();
    updateGraphs();
}
