#include "ProjectView.h"
#include "ModelView.h"
#include "ResultsView.h"
#include "Painting.h"
#include "AppSettings.h"

#include <QtWidgets>

// Constructor / Destructor / Init
ProjectView::ProjectView(QWidget* parent, Qt::WindowFlags flags):QWidget(parent, flags)
{
    /* find screen definition */
    int numScreen (QApplication::desktop()->screenNumber(this));
    QScreen *screen = QApplication::screens().at(numScreen);

    //qreal mm_per_cm = 10;
    const qreal cm_per_in = 2.54;
    /*
    qDebug()<<"ProjectView()"<< numScreen << QApplication::desktop()->screenGeometry(numScreen) << QApplication::desktop()->availableGeometry(numScreen)<< width();
    qDebug()<<"ProjectView()"<< numScreen << QApplication::desktop()->screenGeometry(numScreen) << QApplication::desktop()->availableGeometry(numScreen)<< QApplication::desktop()->width();
    qDebug()<<"ProjectView() screen width"<< width() / screen->physicalDotsPerInchX() * cm_per_in;
    qDebug()<<"ProjectView() screen height"<< height() / screen->physicalDotsPerInchY() * cm_per_in;
    */
    int unitX = int(screen->physicalDotsPerInchX() / cm_per_in);
    AppSettings::setWidthUnit( unitX);

    int unitY = int(screen->physicalDotsPerInchY() / cm_per_in);
    AppSettings::setHeigthUnit( unitY);


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

    mLogTabs = new Tabs(this);
    mLogTabs->addTab(mLogModelEdit,   tr("Model Description"));
    mLogTabs->addTab(mLogMCMCEdit,    tr("MCMC Initialisation"));
    mLogTabs->addTab(mLogResultsEdit, tr("Posterior Distrib. Stats"));

    connect(mLogTabs, &Tabs::tabClicked, this, &ProjectView::showLogTab);

    mStack = new QStackedWidget();
    mStack->addWidget(mModelView);
    mStack->addWidget(mResultsView);
    mStack->addWidget(mLogTabs);
    mStack->setCurrentIndex(0);
    
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mStack);
    setLayout(layout);
    
    connect(mResultsView, &ResultsView::resultsLogUpdated, this, &ProjectView::updateResultsLog);

    //setAppSettingsFont();
    mLogTabs->setTab(2, false);
    mLogTabs->showWidget(2);




}

ProjectView::~ProjectView()
{
    
}

void ProjectView::resizeEvent(QResizeEvent* e)
{
    (void) e;
    /* find screen definition */
    int numScreen (QApplication::desktop()->screenNumber(this));
    QScreen *screen = QApplication::screens().at(numScreen);

    //qreal mm_per_cm = 10;
    qreal cm_per_in = 2.54;

        qDebug()<<"ProjectView::resizeEvent()"<< numScreen << QApplication::desktop()->screenGeometry(numScreen) << QApplication::desktop()->availableGeometry(numScreen)<< width();
 qDebug()<<"ProjectView::resizeEvent()"<< numScreen << QApplication::desktop()->screenGeometry(numScreen) << QApplication::desktop()->availableGeometry(numScreen)<< QApplication::desktop()->width();
            qDebug()<<"ProjectView::resizeEvent() screen setWidthUnit"<< screen->physicalDotsPerInchX() / cm_per_in;
            qDebug()<<"ProjectView::resizeEvent() screen setHeigthUnit"<< screen->physicalDotsPerInchY() / cm_per_in;

            int unitX = int(screen->physicalDotsPerInchX() / cm_per_in);
            AppSettings::setWidthUnit( unitX);

            int unitY = int(screen->physicalDotsPerInchY() / cm_per_in);
            AppSettings::setHeigthUnit( unitY);



   const int logTabHusefull (height() - mLogTabs->tabHeight() - AppSettings::heigthUnit());

    mLogModelEdit->resize( width() - AppSettings::widthUnit(), logTabHusefull );
    mLogMCMCEdit->resize( width() - AppSettings::widthUnit(), logTabHusefull );
    mLogResultsEdit->resize( width() - AppSettings::widthUnit() , logTabHusefull );



}

void ProjectView::doProjectConnections(Project* project)
{
    mModelView   -> setProject(project);
    mResultsView -> doProjectConnections(project);
}

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

void ProjectView::showModel()
{
    mStack->setCurrentIndex(0);
}

/**
 * @brief ProjectView::changeDesign slot connected to Project::projectDesignChange() in MainWindows
 * @param refresh
 */
void ProjectView::changeDesign(bool refresh)
{
    (void) refresh;
    mRefreshResults = true;
}

void ProjectView::setAppSettings()
{
   // setFont(AppSettings::font());

    mModelView->applyAppSettings();
    mResultsView->applyAppSettings();

  /*  mLogTabs->setFont(AppSettings::font());
    mLogModelEdit->setFontFamily(AppSettings::font().family());
    mLogModelEdit->setFontPointSize(AppSettings::font().pointSizeF());

    mLogMCMCEdit->setFontFamily(AppSettings::font().family());
    mLogMCMCEdit->setFontPointSize(AppSettings::font().pointSizeF());

   // mLogResultsEdit->setFontFamily(AppSettings::font().family());
    mLogResultsEdit->setFontPointSize(AppSettings::font().pointSizeF()*2);
#ifdef Q_OS_MAC
    const QFont font (AppSettings::font());
    QString styleSh = "QLineEdit { border-radius: 5px; font: "+ QString::number(font.pointSize()) + "px ;font-family: "+font.family() + ";}";
    mLogResultsEdit->setStyleSheet(styleSh);
#else
    mLogResultsEdit->setFont(AppSettings::font());
#endif

    const int logTabHusefull (height() - mLogTabs->tabHeight() - AppSettings::heigthUnit());

    mLogModelEdit->resize( width() -  AppSettings::widthUnit(), logTabHusefull );
    mLogMCMCEdit->resize( width() - AppSettings::widthUnit(), logTabHusefull );
    mLogResultsEdit->resize( width() -AppSettings::widthUnit(), logTabHusefull );
  */

}

void ProjectView::showResults()
{
    mResultsView->clearResults();
    mResultsView->updateModel(); // update Design e.g. Name and color //updateResults() is call inside
    mRefreshResults = false;

    mStack->setCurrentIndex(1);
    // come from mViewResultsAction and  updateResults send repaint on mStack
}


void ProjectView::showLog()
{
    mResultsView->mModel->generateResultsLog();
    updateResultsLog(mResultsView->mModel->getResultsLog());
    mStack->setCurrentIndex(2);
}

/**
 * @brief Update All model views (Scenes, ...) after pushing state
 */
void ProjectView::updateProject()
{
    mModelView->updateProject();
}

void ProjectView::createProject()
{
    mModelView->createProject();
}

void ProjectView::newPeriod()
{
    mModelView->modifyPeriod();
}

void ProjectView:: applyFilesSettings(Model* model)
{
    // Rebuild all calibration curve

    QJsonObject state = mModelView->getProject()->state();
    ProjectSettings s = ProjectSettings::fromJson(state.value(STATE_SETTINGS).toObject());
    bool calibrate = mModelView->findCalibrateMissing();
    if (calibrate)
        mModelView->calibrateAll(s);

    applySettings(model);
}

void ProjectView::applySettings(Model* model)
{
    setAppSettings();
    if (model) {

        mResultsView->updateFormatSetting(model);

        // force to regenerate the densities
        mResultsView->initResults(model);

        model->generateModelLog();
        mLogModelEdit->setText(model->getModelLog());

        mLogMCMCEdit->setText(model->getMCMCLog());

        model->generateResultsLog();
        updateResultsLog(model->getResultsLog());
    }
}

void ProjectView::updateMultiCalibration()
{
    mModelView->updateMultiCalibration();
}

void ProjectView::updateResults(Model* model)
{
    if (model) {
        mResultsView->updateResults(model);

        model->generateModelLog();
        mLogModelEdit->setText(model->getModelLog());

        mLogMCMCEdit->setText(model->getMCMCLog());

        model->generateResultsLog();
        mLogResultsEdit->setText(model->getResultsLog());

        mStack->setCurrentIndex(1);
    }
}

void ProjectView::initResults(Model* model)
{
    qDebug()<<"ProjectView::initResults()";
    if (model) {
        mResultsView->clearResults();
        
        mResultsView->initResults(model);
        mRefreshResults = true;
        mResultsView->update();

        model->generateModelLog();
        mLogModelEdit->setText(model->getModelLog());

        mLogMCMCEdit->setText(model->getMCMCLog());

        model->generateResultsLog();
        mLogResultsEdit->setText(model->getResultsLog());

       // showResults();
       mStack->setCurrentIndex(1);
    }
    
}

void ProjectView::updateResultsLog(const QString& log)
{
#ifdef Q_OS_MAC
    const QFont font (qApp->font());
    QString styleSh = "QLineEdit { border-radius: 5px; font: "+ QString::number(font.pointSize()) + "px ;font-family: "+font.family() + ";}";
    mLogResultsEdit->setStyleSheet(styleSh);
#endif
    mLogResultsEdit->setHtml(log);
}

void ProjectView::showLogTab(const int &i)
{
    mLogTabs->showWidget(i);
}

//  Read/Write settings
void ProjectView::writeSettings()
{
    mModelView->writeSettings();
}

void ProjectView::readSettings()
{
    mModelView->readSettings();
}
