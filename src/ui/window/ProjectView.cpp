#include "ProjectView.h"
#include "ModelView.h"
#include "ResultsView.h"
#include "Painting.h"
#include <QtWidgets>

#pragma mark Constructor / Destructor / Init
ProjectView::ProjectView(QWidget* parent, Qt::WindowFlags flags):QWidget(parent, flags)
{
    mModelView = new ModelView();
    mResultsView = new ResultsView();
    
    QPalette palette;
    palette.setColor(QPalette::Base, Qt::transparent);
    palette.setColor(QPalette::Text, Qt::black);
    
    
    mLogModelEdit = new QTextEdit();
    mLogModelEdit->setReadOnly(true);
    mLogModelEdit->setAcceptRichText(true);
    mLogModelEdit->setFrameStyle(QFrame::NoFrame);
    mLogModelEdit->setPalette(palette);
    
    mLogMCMCEdit = new QTextEdit();
    mLogMCMCEdit->setReadOnly(true);
    mLogMCMCEdit->setAcceptRichText(true);
    mLogMCMCEdit->setFrameStyle(QFrame::NoFrame);
    mLogMCMCEdit->setPalette(palette);
    
    mLogResultsEdit = new QTextEdit();
    mLogResultsEdit->setReadOnly(true);
    mLogResultsEdit->setAcceptRichText(true);
    mLogResultsEdit->setFrameStyle(QFrame::NoFrame);
    mLogResultsEdit->setPalette(palette);
    
    QFont font = mLogMCMCEdit->font();
    font.setPointSizeF(pointSize(11));
    
    mLogModelEdit->setFont(font);
    mLogMCMCEdit->setFont(font);
    mLogResultsEdit->setFont(font);
    
    mLogTabs = new QTabWidget();
    mLogTabs->addTab(mLogModelEdit,   tr("Model description"));
    mLogTabs->addTab(mLogMCMCEdit,    tr("MCMC initialization"));
    mLogTabs->addTab(mLogResultsEdit, tr("Posterior distrib. results"));
    mLogTabs->setContentsMargins(15, 15, 15, 15);
    
    mLogView = new QWidget();
    QVBoxLayout* logLayout = new QVBoxLayout();
    logLayout->addWidget(mLogTabs);
    mLogView->setLayout(logLayout);
    
    mStack = new QStackedWidget();
    mStack->addWidget(mModelView);
    mStack->addWidget(mResultsView);
    mStack->addWidget(mLogView);
    mStack->setCurrentIndex(0);
    
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mStack);
    setLayout(layout);
    
    connect(mResultsView, SIGNAL(resultsLogUpdated(const QString&)), this, SLOT(updateResultsLog(const QString&)));
}

ProjectView::~ProjectView()
{
    
}

void ProjectView::doProjectConnections(Project* project)
{
    mModelView   -> doProjectConnections(project);
    mResultsView -> doProjectConnections(project);
    //connect(*project, SIGNAL(designChanged(bool)), this, SLOT(changeDesign(bool)));
}

#pragma mark Interface
void ProjectView::resetInterface()
{
    showModel();
    mModelView   -> resetInterface();
    mResultsView -> clearResults();
}
void ProjectView::showHelp(bool show)
{
    mModelView->showHelp(show);
}

#pragma mark View Switch
void ProjectView::showModel()
{

    mStack->setCurrentIndex(0);
}

/**
 * @brief ProjectView::changeDesign solt connected to Project::projectDesignChange() in MainWindows
 * @param refresh
 */
void ProjectView::changeDesign(bool refresh)
{
  mRefreshResults = refresh;
}

void ProjectView::showResults()
{
    if(mRefreshResults)  {
        mResultsView->updateResults();
        mRefreshResults=false;
    }
    mStack->setCurrentIndex(1);
    // come from mViewResultsAction and  updateResults send repaint on mStack
}
void ProjectView::showLog()
{

    mStack->setCurrentIndex(2);
}

#pragma mark Update Model
/**
 * @brief Update All model views (Scenes, ...) after pushing state
 */
void ProjectView::updateProject()
{
    mModelView->updateProject();
}

#pragma mark Update Results

void ProjectView:: ApplySettings(Model* model)
{
    if(model)
    {
        mResultsView->updateResults(model);

        model->generateModelLog();
        mLogModelEdit->setText(model->getModelLog());

        mLogMCMCEdit->setText(model->getMCMCLog());

        model->generateResultsLog();
        updateResultsLog(model->getResultsLog());
    }
}

void ProjectView::updateResults(Model* model)
{
    if(model)
    {
        showResults();//false);
        
        mResultsView->updateResults(model);
        
        model->generateModelLog();
        mLogModelEdit->setText(model->getModelLog());
        
        mLogMCMCEdit->setText(model->getMCMCLog());
        
        model->generateResultsLog();
        mLogResultsEdit->setText(model->getResultsLog());
    }
}



void ProjectView::updateResultsLog(const QString& log)
{
    mLogResultsEdit->setText(log);
}


#pragma mark Read/Write settings
void ProjectView::writeSettings()
{
    mModelView->writeSettings();
}

void ProjectView::readSettings()
{
    mModelView->readSettings();
}
