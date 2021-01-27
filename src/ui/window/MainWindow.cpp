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

#include "MainWindow.h"
#include "Project.h"
#include "ProjectView.h"
#include "PluginAbstract.h"
#include "AboutDialog.h"
#include "AppSettingsDialog.h"
#include "PluginManager.h"
#include "ModelUtilities.h"
#include "SwitchAction.h"

#include <QtWidgets>
#include <QLocale>
#include <QFont>

// Constructor / Destructor
MainWindow::MainWindow(QWidget* aParent):QMainWindow(aParent)
{
   // setWindowTitle("ChronoModel");
#ifdef DEBUG
    setWindowTitle(qApp->applicationName() + " " + qApp->applicationVersion() + " DEBUG Mode ");
#else
    setWindowTitle(qApp->applicationName() + " " + qApp->applicationVersion() );
#endif

    QPalette tooltipPalette;
    tooltipPalette.setColor(QPalette::ToolTipBase, Qt::white);
    tooltipPalette.setColor(QPalette::ToolTipText, Qt::black);
    QToolTip::setPalette(tooltipPalette);
    QFont tooltipFont(font());
    tooltipFont.setItalic(true);


    QToolTip::setFont(tooltipFont);

    mLastPath = QDir::homePath();

    mProject = nullptr;

    /* Creation of ResultsView and ModelView */
    mProjectView = new ProjectView();
    setCentralWidget(mProjectView);

    mUndoStack = new QUndoStack();
    mUndoStack->setUndoLimit(1000);

    mUndoView = new QUndoView(mUndoStack);
    mUndoView->setEmptyLabel(tr("Initial state"));
    mUndoDock = new QDockWidget(this);
    //mUndoDock->setFixedWidth(250);
    mUndoDock->setWidget(mUndoView);
    mUndoDock->setAllowedAreas(Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, mUndoDock);
    mUndoDock->setVisible(false);

    createActions();
    createMenus();
    createToolBars();

    statusBar()->showMessage(tr("Ready"));
    //setUnifiedTitleAndToolBarOnMac(true);
    setWindowIcon(QIcon(":chronomodel.png"));

    resize(AppSettings::mLastSize);

    connect(mProjectSaveAction, static_cast<void (QAction::*)(bool)> (&QAction::triggered), this, &MainWindow::saveProject);
    connect(mProjectSaveAsAction, static_cast<void (QAction::*)(bool)> (&QAction::triggered), this, &MainWindow::saveProjectAs);

    connect(mViewModelAction, static_cast<void (QAction::*)(bool)> (&QAction::triggered), mProjectView, &ProjectView::showModel );
    connect(mViewLogAction, static_cast<void (QAction::*)(bool)> (&QAction::triggered), mProjectView, &ProjectView::showLog);
    connect(mViewResultsAction, static_cast<void (QAction::*)(bool)> (&QAction::triggered), mProjectView, &ProjectView::showResults);

    QLocale newLoc(QLocale::system());
    AppSettings::mLanguage = newLoc.language();
    AppSettings::mCountry = newLoc.country();
    newLoc.setNumberOptions(QLocale::OmitGroupSeparator);
    QLocale::setDefault(newLoc);

    if (newLoc.decimalPoint()==',') {
        AppSettings::mCSVCellSeparator=";";
        AppSettings::mCSVDecSeparator=",";
    } else {
        AppSettings::mCSVCellSeparator=",";
        AppSettings::mCSVDecSeparator=".";
    }

    activateInterface(false);
}

MainWindow::~MainWindow()
{

}

// Accessors
Project* MainWindow::getProject()
{
    return mProject;
}

QJsonObject MainWindow::getState() const
{
    return mProject->mState;
}
QString MainWindow::getNameProject() const
{
    return mProject->mName;
}


QUndoStack* MainWindow::getUndoStack()
{
    return mUndoStack;
}

QString MainWindow::getCurrentPath() const
{
    return mLastPath;
}

void MainWindow::setCurrentPath(const QString& path)
{
    mLastPath = path;
}

// Actions & Menus
void MainWindow::createActions()
{
    //QWhatsThis::createAction();

    mAppSettingsAction = new QAction(QIcon(":settings.png"), tr("Settings"), this);
    connect(mAppSettingsAction, &QAction::triggered, this, &MainWindow::appSettings);

    //-----------------------------------------------------------------
    // Project Actions
    //-----------------------------------------------------------------

    mNewProjectAction = new QAction(QIcon(":new_p.png"), tr("&New"), this);
    mNewProjectAction->setShortcuts(QKeySequence::New);
    mNewProjectAction->setStatusTip(tr("Create a new project"));
    mNewProjectAction->setToolTip(tr("New project"));
    mNewProjectAction->setWhatsThis(tr("What's This? :Create a new project"));

    connect(mNewProjectAction, &QAction::triggered, this, &MainWindow::newProject);

    mOpenProjectAction = new QAction(QIcon(":open_p.png"), tr("Open"), this);
    mOpenProjectAction->setShortcuts(QKeySequence::Open);
    mOpenProjectAction->setStatusTip(tr("Open an existing project"));
    connect(mOpenProjectAction, &QAction::triggered, this, &MainWindow::openProject);

    mInsertProjectAction = new QAction(QIcon(":open_p.png"), tr("Insert"), this);
    mInsertProjectAction->setStatusTip(tr("Insert an existing project"));
    connect(mInsertProjectAction, &QAction::triggered, this, &MainWindow::insertProject);

    mCloseProjectAction = new QAction(tr("Close"), this);
    mCloseProjectAction->setShortcuts(QKeySequence::Close);
    mCloseProjectAction->setStatusTip(tr("Open an existing project"));
    connect(mCloseProjectAction, &QAction::triggered, this, &MainWindow::closeProject);

    mProjectSaveAction = new QAction(QIcon(":save_p.png"), tr("&Save"), this);
    mProjectSaveAction->setShortcuts(QKeySequence::Save);
    mProjectSaveAction->setStatusTip(tr("Save the current project in the same place with the same name"));

    mProjectSaveAsAction = new QAction(QIcon(":save_p.png"), tr("Save as..."), this);
    mProjectSaveAsAction->setStatusTip(tr("Change the current project on an other name or on an other place"));

    mProjectExportAction = new QAction(QIcon(":export.png"), tr("Export"), this);
    mProjectExportAction->setVisible(false);

    mUndoAction = mUndoStack->createUndoAction(this);
    mUndoAction->setShortcuts(QKeySequence::Undo);
    mUndoAction->setIcon(QIcon(":undo_p.png"));
    mUndoAction->setText(tr("Undo"));
    mUndoAction->setToolTip(tr("Undo"));

    mRedoAction = mUndoStack->createRedoAction(this);
    mRedoAction->setShortcuts(QKeySequence::Redo);
    mRedoAction->setIcon(QIcon(":redo_p.png"));
    mRedoAction->setText(tr("Redo"));
    mRedoAction->setToolTip(tr("Redo"));

    mUndoViewAction = mUndoDock->toggleViewAction();
    mUndoViewAction->setText(tr("Show Undo Stack"));

    //-----------------------------------------------------------------
    // MCMC Actions
    //-----------------------------------------------------------------
    mMCMCSettingsAction = new QAction(QIcon(":settings_p.png"), tr("MCMC"), this);
    mMCMCSettingsAction->setToolTip(tr("Change MCMC Settings"));

    mRunAction = new QAction(QIcon(":run_p.png"), tr("Run"), this);
    //runAction->setIcon(qApp->style()->standardIcon(QStyle::SP_MediaPlay));
    mRunAction->setIconText(tr("Run"));
    mRunAction->setIconVisibleInMenu(true);
    mRunAction->setToolTip(tr("Run Model"));

    mResetMCMCAction = new QAction(tr("Reset Events and Data methods"), this);

    //-----------------------------------------------------------------
    // View Actions
    //-----------------------------------------------------------------
    mChronocurveAction = new SwitchAction(this);
    mChronocurveAction->setCheckable(true);
    mChronocurveAction->setChecked(false);

    mViewModelAction = new QAction(QIcon(":model_p.png"), tr("Model"), this);
    mViewModelAction->setCheckable(true);

    mViewResultsAction = new QAction(QIcon(":results_p.png"), tr("Results"), this);
    mViewResultsAction->setCheckable(true);
    mViewResultsAction->setEnabled(false);

    mViewLogAction = new QAction(QIcon(":log_p.png"), tr("Log"), this);
    mViewLogAction->setCheckable(true);
    mViewLogAction->setEnabled(false);

    mViewGroup = new QActionGroup(this);
    mViewGroup->addAction(mViewModelAction);
    mViewGroup->addAction(mViewResultsAction);
    mViewGroup->addAction(mViewLogAction);
    mViewModelAction->setChecked(true);

    //-----------------------------------------------------------------
    //  Dates Actions (Plugins specific)
    //-----------------------------------------------------------------
    const QList<PluginAbstract*>& plugins = PluginManager::getPlugins();
    for (int i=0; i<plugins.size(); ++i) {
        QList<QHash<QString, QVariant>> groupedActions = plugins.at(i)->getGroupedActions();
        for (int j=0; j<groupedActions.size(); ++j) {
            QAction* act = new QAction(groupedActions[j]["title"].toString(), this);
            act->setData(QVariant(groupedActions[j]));
            connect(act, &QAction::triggered, this, &MainWindow::doGroupedAction);
            mDatesActions.append(act);
        }
    }

    //-----------------------------------------------------------------
    //  Grouped actions
    //-----------------------------------------------------------------
    mEventsColorAction = new QAction(tr("Selected Events: Change Colour"), this);
    connect(mEventsColorAction, &QAction::triggered, this, &MainWindow::changeEventsColor);

    mEventsMethodAction = new QAction(tr("Selected Events: Change Method"), this);
    connect(mEventsMethodAction, &QAction::triggered, this, &MainWindow::changeEventsMethod);

    mDatesMethodAction = new QAction(tr("Selected Events: Change Data Method"), this);
    connect(mDatesMethodAction, &QAction::triggered, this, &MainWindow::changeDatesMethod);

    mSelectEventsAction = new QAction(tr("Select All Events of the Selected Phases"), this);
    connect(mSelectEventsAction, &QAction::triggered, this, &MainWindow::selectedEventInSelectedPhases);

    //-----------------------------------------------------------------
    // Help/About Menu
    //-----------------------------------------------------------------
    mAboutAct = new QAction(QIcon(":light.png"), tr("About"), this);
    connect(mAboutAct, &QAction::triggered, this, &MainWindow::about);

    mAboutQtAct = new QAction(QIcon(":qt.png"), tr("About Qt"), this);
    connect(mAboutQtAct, &QAction::triggered, qApp, QApplication::aboutQt);

    mHelpAction = new QAction(QIcon(":help_p.png"), tr("Help"), this);
    mHelpAction->setCheckable(true);
    connect(mHelpAction, &QAction::triggered, this, &MainWindow::showHelp);

    mManualAction = new QAction(QIcon(":pdf_p.png"), tr("Manual Online"), this);
    connect(mManualAction, &QAction::triggered, this, &MainWindow::openManual);

    mWebsiteAction = new QAction(QIcon(":web_p.png"), tr("Website"), this);
    connect(mWebsiteAction, &QAction::triggered, this, &MainWindow::openWebsite);

    //-----------------------------------------------------------------
    // Translation Menu
    //-----------------------------------------------------------------
    mTranslateEnglishAct = new QAction(tr("English"), this);
    mTranslateEnglishAct->setCheckable(true);
    mTranslateEnglishAct->setData("en");

    mTranslateFrenchAct = new QAction(tr("French"), this);
    mTranslateFrenchAct->setCheckable(true);
    mTranslateFrenchAct->setData("fr");

    mLangGroup = new QActionGroup(this);
    mLangGroup->addAction(mTranslateEnglishAct);
    mLangGroup->addAction(mTranslateFrenchAct);
    mTranslateEnglishAct->setChecked(true);
    connect(mLangGroup, &QActionGroup::triggered, this, &MainWindow::setLanguage);
}

void MainWindow::createMenus()
{
    //-----------------------------------------------------------------
    // Project menu
    //-----------------------------------------------------------------
    const  QFont ft (font());
    mProjectMenu = menuBar()->addMenu(tr("File"));
    mProjectMenu->addAction(mAppSettingsAction);
    mProjectMenu->addAction(mNewProjectAction);
    mProjectMenu->addAction(mOpenProjectAction);
    mProjectMenu->addAction(mInsertProjectAction);
    mProjectMenu->addAction(mCloseProjectAction);

    mProjectMenu->addSeparator();

    mProjectMenu->addAction(mProjectSaveAction);
    mProjectMenu->addAction(mProjectSaveAsAction);

    mProjectMenu->addSeparator();

    mProjectMenu->addAction(mProjectExportAction);
    mProjectMenu->setFont(ft);

    //-----------------------------------------------------------------
    // Edit menu
    //-----------------------------------------------------------------
    mEditMenu = menuBar()->addMenu(tr("Edit"));

    mEditMenu->addAction(mUndoAction);
    mEditMenu->addAction(mRedoAction);
    mEditMenu->setFont(ft);

    //-----------------------------------------------------------------
    // MCMC menu
    //-----------------------------------------------------------------
    mMCMCMenu = menuBar()->addMenu(tr("MCMC"));
    mMCMCMenu->addAction(mRunAction);
    mMCMCMenu->addAction(mMCMCSettingsAction);
    mMCMCMenu->addSeparator();
    mMCMCMenu->addAction(mResetMCMCAction);

    //-----------------------------------------------------------------
    // View menu
    //-----------------------------------------------------------------
    mViewMenu = menuBar()->addMenu(tr("View"));
    mViewMenu->addAction(mViewModelAction);
    mViewMenu->addAction(mViewResultsAction);
    mViewMenu->addAction(mViewLogAction);

    //-----------------------------------------------------------------
    // Help/About Menu this menu depend of the system. On MacOs it's in Chronomodel menu
    //-----------------------------------------------------------------
    mHelpMenu = menuBar()->addMenu(tr("About"));
    mHelpMenu->menuAction()->setShortcut(Qt::Key_Question);
    mHelpMenu->addAction(mAboutAct);
    mHelpMenu->addAction(mAboutQtAct);

    //-----------------------------------------------------------------
    // Grouped Actions Menu
    //-----------------------------------------------------------------
    mActionsMenu = menuBar()->addMenu(tr("Actions"));
    mActionsMenu->addAction(mSelectEventsAction);
    mActionsMenu->addSeparator();
    mActionsMenu->addAction(mEventsColorAction);
    mActionsMenu->addAction(mEventsMethodAction);
    mActionsMenu->addAction(mDatesMethodAction);
    mActionsMenu->addSeparator();

    for (int i=0; i<mDatesActions.size(); ++i)
        mActionsMenu->addAction(mDatesActions[i]);

}
/**
 * @brief MainWindow::createToolBars
 * Create the ToolBar with the icons, under the application menu
 */
void MainWindow::createToolBars()
{
    //-----------------------------------------------------------------
    // Main ToolBar
    //-----------------------------------------------------------------


    mToolBar = addToolBar("Main Tool Bar"); /* all types of tool button */
    //QString sty = palette().text().color().name(); // find the color defined by the system/theme
   // mToolBar->setStyleSheet("QToolButton { color :"+ sty +" ;}");

    mToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);//ToolButtonTextUnderIcon); // offer to write the text under the icon
    mToolBar->setMovable(false);
    mToolBar->setAllowedAreas(Qt::TopToolBarArea);

    mToolBar->addAction(mNewProjectAction);
    mToolBar->addAction(mOpenProjectAction);
    mToolBar->addAction(mProjectSaveAction);
    mToolBar->addAction(mProjectExportAction);

    mToolBar->addAction(mUndoAction);
    mToolBar->addAction(mRedoAction);

    QWidget* separator3 = new QWidget(this);
    separator3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mToolBar->addWidget(separator3);

    mToolBar->addAction(mChronocurveAction);
    mToolBar->addAction(mViewModelAction);
    mToolBar->addAction(mMCMCSettingsAction);
    mToolBar->addAction(mRunAction);
    mToolBar->addAction(mViewResultsAction);
    mToolBar->addAction(mViewLogAction);

    QWidget* separator4 = new QWidget(this);
    separator4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mToolBar->addWidget(separator4);

    mToolBar->addAction(mHelpAction);
    mToolBar->addAction(mManualAction);
    mToolBar->addAction(mWebsiteAction);
    /* toolBar->addAction(mAboutAct);
    toolBar->addAction(mAboutQtAct); */
    mToolBar->setFont(qApp->font()); // must be after all addAction

}


// -----------

void MainWindow::newProject()
{
    // Ask to save the previous project.
    // Return true if the project doesn't need to be saved.
    // Returns true if the user saves the project or if the user doesn't want to save it.
    // Returns false if the user cancels.
    bool yesCreate= false;

    if ((mProject == nullptr) || (mProject->askToSave(tr("Save current project as...") )))
        yesCreate= true;

    if (yesCreate)
    {
        Project* newProject = new Project();
         // just update mAutoSaveTimer to avoid open the save() dialog box
        newProject-> mAutoSaveTimer->stop();

        /* Ask to save the new project.
         * Returns true only if a new file is created.
         * Note : at this point, the project state is still the previous project state.*/
        if (newProject->saveAs(tr("Save new project as..."))) {
            //mUndoStack->clear();

            // resetInterface Disconnect also the scene
            resetInterface();

            activateInterface(true);

            /* Reset the project state and the MCMC Setting to the default value
             * and then send a notification to update the views : send desabled */

            newProject->initState(NEW_PROJECT_REASON);// emit showStudyPeriodWarning();

            delete mProject;

            mProject = newProject;

            // Create project connections
            connectProject();

            // Apply app settings to the project
            mProject->setAppSettings();

            // Send the project to the views
            mProjectView->setProject(mProject);

            // Ask for the a Study Period (open dialog)
            mProjectView->newPeriod();

            // Open the Model View
            mViewModelAction->trigger();

            // Disable the Result View
            mViewResultsAction->setEnabled(false);

            updateWindowTitle();

            mUndoStack->clear();

        } else {
            delete newProject;
        }
    }
}

void MainWindow::openProject()
{
    const QString currentPath = getCurrentPath();
    QString path = QFileDialog::getOpenFileName(this,
                                                      tr("Open File"),
                                                      currentPath,
                                                      tr("Chronomodel Project (*.chr)"));

    if (!path.isEmpty()) {

        if (mProject) {
            mProject->askToSave(tr("Save current project as..."));

            disconnectProject();

            //resetInterface(): clear mEventsScene and mPhasesScene, set mProject = nullptr
            resetInterface();

            delete mProject;
        }
        statusBar()->showMessage(tr("Loading project : %1").arg(path));
        // assign new project
        mProject = new Project();
        connectProject();

        //setAppSettings(): just update mAutoSaveTimer
        mProject->setAppSettings();
        const QFileInfo info(path);
        setCurrentPath(info.absolutePath());


        // look MainWindows::readSetting()
        if (mProject->load(path)) {
            activateInterface(true);
            updateWindowTitle();
        // Create mEventsScene and mPhasesScenes

            mProjectView->setProject(mProject);

            mProject->pushProjectState(mProject->mState, PROJECT_LOADED_REASON, true, true);
           /* if (! mProject->mModel->mChains.isEmpty())
                emit mProject->mcmcFinished(mProject->mModel);*/
         }

        mUndoStack->clear();
        statusBar()->showMessage(tr("Ready"));
    }

}


void MainWindow::insertProject()
{
    const QString currentPath = getCurrentPath();
    QString path = QFileDialog::getOpenFileName(this,
                                                      tr("Insert File"),
                                                      currentPath,
                                                      tr("Chronomodel Project (*.chr)"));

    if (!path.isEmpty()) {

        statusBar()->showMessage(tr("Insert project : %1").arg(path));

        const QFileInfo info(path);
        setCurrentPath(info.absolutePath());


        // look MainWindows::readSetting()
        if (mProject->insert(path)) {

        // Create mEventsScene and mPhasesScenes
            mProjectView->updateProject();

         }

        //mUndoStack->clear();
        statusBar()->showMessage(tr("Ready"));
    }

}

void MainWindow::connectProject()
{
    connect(mProject, &Project::noResult, this, &MainWindow::noResult);
    connect(mProject, &Project::mcmcFinished, this, &MainWindow::mcmcFinished);
    connect(mProject, &Project::projectStateChanged, this, &MainWindow::updateProject);
    connect(mProject, &Project::projectStructureChanged, this, &MainWindow::noResult);

    connect(mChronocurveAction, &QAction::toggled, this, &MainWindow::toggleChronocurve);
    connect(mMCMCSettingsAction, &QAction::triggered, mProject, &Project::mcmcSettings);
    connect(mResetMCMCAction, &QAction::triggered, mProject, &Project::resetMCMC);
    connect(mProjectExportAction, &QAction::triggered, mProject, &Project::exportAsText);
    connect(mRunAction, &QAction::triggered, mProject, &Project::run);
}

void MainWindow::disconnectProject()
{
    disconnect(mProject, &Project::noResult, this, &MainWindow::noResult);
    disconnect(mProject, &Project::mcmcFinished, this, &MainWindow::mcmcFinished);
    disconnect(mProject, &Project::projectStateChanged, this, &MainWindow::updateProject);
    disconnect(mProject, &Project::projectStructureChanged, this, &MainWindow::noResult);

    disconnect(mMCMCSettingsAction, &QAction::triggered, mProject, &Project::mcmcSettings);
    disconnect(mResetMCMCAction, &QAction::triggered, mProject, &Project::resetMCMC);
    disconnect(mProjectExportAction, &QAction::triggered, mProject, &Project::exportAsText);
    disconnect(mRunAction, &QAction::triggered, mProject, &Project::run);

    connect(mChronocurveAction, &QAction::triggered, this, &MainWindow::toggleChronocurve);
}

void MainWindow::closeProject()
{
   if (mProject) {
        if ( mProject->askToSave(tr("Save current project as...")) == true)
             mProject->saveProjectToFile();

        mUndoStack->clear();

        mProject->initState(CLOSE_PROJECT_REASON);
        mProject->mLastSavedState = mProject->mState;//emptyState();
        AppSettings::mLastDir = QString();

        // Go back to model tab :
        mViewModelAction->trigger();
        mProject->clearModel();
        disconnectProject();

        resetInterface();

        activateInterface(false);
        mViewResultsAction->setEnabled(false);

        updateWindowTitle();
        delete mProject;
        mProject = nullptr;

   } else // if there is no project, we suppose it means to close the programm
       QApplication::exit(0);
}

void MainWindow::saveProject()
{
    mProject->save();
    updateWindowTitle();
}

void MainWindow::saveProjectAs()
{
    mProject->saveAs(tr("Save current project as..."));
    updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
#ifdef DEBUG
    setWindowTitle(qApp->applicationName() + " " + qApp->applicationVersion() + " DEBUG Mode "+ (AppSettings::mLastFile.isEmpty() ?  "" : QString(" - ") + AppSettings::mLastFile));
#else
    setWindowTitle(qApp->applicationName() + " " + qApp->applicationVersion() + (AppSettings::mLastFile.isEmpty() ?  "" : QString(" - ") + AppSettings::mLastFile));
#endif
}

void MainWindow::updateProject()
{
    mUndoAction->setText(tr("Undo"));
    mUndoAction->setToolTip(tr("Undo") + " : " + mUndoStack->undoText());
    mUndoAction->setStatusTip(tr("Click to go back to the previous action") + " : " + mUndoStack->undoText());

    mRedoAction->setText(tr("Redo"));
    mRedoAction->setToolTip(tr("Redo") + " : " + mUndoStack->redoText());
    mRedoAction->setStatusTip(tr("Click to redo the last action") + " : " + mUndoStack->redoText());

    mRunAction->setEnabled(true);
    mProjectView->updateProject();

    mChronocurveAction->setChecked(mProject->isChronocurve());
}

void MainWindow::toggleChronocurve(bool checked)
{
    mProjectView->toggleChronocurve(checked);
}

// Settings & About
void MainWindow::about()
{
    AboutDialog dialog(qApp->activeWindow());
    dialog.exec();
}

void MainWindow::appSettings()
{
    AppSettingsDialog dialog(qApp->activeWindow());

    dialog.setSettings();
    connect(&dialog, &AppSettingsDialog::settingsChanged, this, &MainWindow::setAppSettings);
    connect(&dialog, &AppSettingsDialog::settingsFilesChanged, this, &MainWindow::setAppFilesSettings);
    dialog.exec();

}

void MainWindow::setAppFilesSettings()
{
    QLocale::Language newLanguage = AppSettings::mLanguage;
    QLocale::Country newCountry= AppSettings::mCountry;

    QLocale newLoc = QLocale(newLanguage,newCountry);
    newLoc.setNumberOptions(QLocale::OmitGroupSeparator);
    QLocale::setDefault(newLoc);
    //statusBar()->showMessage(tr("Language") + " : " + QLocale::languageToString(QLocale().language()));

    QFont tooltipFont(font());
    tooltipFont.setItalic(true);

    QToolTip::setFont(tooltipFont);

    if (mProject) {
        mProject->setAppSettings();
        mProjectView->applyFilesSettings(mProject->mModel);
    }
    writeSettings();
}

void MainWindow::setAppSettings()
{
    QLocale::Language newLanguage = AppSettings::mLanguage;
    QLocale::Country newCountry= AppSettings::mCountry;

    QLocale newLoc = QLocale(newLanguage, newCountry);
    newLoc.setNumberOptions(QLocale::OmitGroupSeparator);
    QLocale::setDefault(newLoc);

    setFont(qApp->font());

    QFont tooltipFont(font());
    tooltipFont.setItalic(true);

    QToolTip::setFont(tooltipFont);

    if (mProject) {
        mProject->setAppSettings();
        mProjectView->updateMultiCalibration();
        mProjectView->applySettings(mProject->mModel);

    }
    writeSettings();
}

void MainWindow::openManual()
{
    QDesktopServices::openUrl(QUrl("https://chronomodel.com/storage/medias/3_chronomodel_user_manual.pdf", QUrl::TolerantMode));


 /*   QString path = qApp->applicationDirPath();
#ifdef Q_OS_MAC
    QDir dir(path);
    dir.cdUp();
    path = dir.absolutePath() + "/Resources";
#endif
    path += "/Chronomodel_User_Manual.pdf";
    QDesktopServices::openUrl(QUrl("file:///" + path, QUrl::TolerantMode));
  */
}

void MainWindow::showHelp(bool show)
{
   /* if (show)
        QWhatsThis::enterWhatsThisMode();
    else
        QWhatsThis::leaveWhatsThisMode();*/
    AppSettings::mShowHelp = show;
    mProjectView->showHelp(show);
}

void MainWindow::openWebsite()
{
    QDesktopServices::openUrl(QUrl("http://chronomodel.com", QUrl::TolerantMode));
}

void MainWindow::setFont(const QFont &font)
{
    mToolBar->setFont(font);
    //mCentralStack->setFont(font);
    mProjectView->setFont(font);
    //mProject->setFont(font);
    //mUndoStack->setFont(font);
    mUndoView->setFont(font);
    mUndoDock->setFont(font);
}

// Language
void MainWindow::setLanguage(QAction* action)
{
    QString lang = action->data().toString();
    QLocale locale = QLocale(lang);
    QLocale::setDefault(locale);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + locale.name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qApp->installTranslator(&qtTranslator);

    QTranslator translator;
    if (translator.load(locale, ":/Chronomodel", "_")) {
        qDebug() << "-> Locale set to : " << QLocale::languageToString(locale.language()) << "(" << locale.name() << ")";
        qApp->installTranslator(&translator);
    }
}

// Grouped Actions
void MainWindow::selectedEventInSelectedPhases() {
    if (mProject)
        mProject->selectedEventsFromSelectedPhases();
}


void MainWindow::changeEventsColor()
{
    if (!mProject)
        return;

    const QColor color = QColorDialog::getColor(Qt::blue, qApp->activeWindow(), tr("Change Selected Events Colour"));
    if (color.isValid() && mProject)
        mProject->updateSelectedEventsColor(color);

}
void MainWindow::changeEventsMethod()
{
    if (!mProject)
        return;

    QStringList opts;
    opts.append(ModelUtilities::getEventMethodText(Event::eMHAdaptGauss));
    opts.append(ModelUtilities::getEventMethodText(Event::eBoxMuller));
    opts.append(ModelUtilities::getEventMethodText(Event::eDoubleExp));

    bool ok;
    QString methodStr = QInputDialog::getItem(qApp->activeWindow(),
                                          tr("Change Events Method"),
                                          tr("Change Selected Events MCMC Method") + " :",
                                          opts, 0, false, &ok);
    if (ok && !methodStr.isEmpty()) {
        Event::Method method = ModelUtilities::getEventMethodFromText(methodStr);
        mProject->updateSelectedEventsMethod(method);
    }
}

void MainWindow::changeDatesMethod()
{
    if (!mProject)
        return;

    QStringList opts;
    const QList<PluginAbstract*>& plugins = PluginManager::getPlugins();
    for (int i=0; i<plugins.size(); ++i)
        opts.append(plugins[i]->getName());

    bool ok;
    QString pluginName = QInputDialog::getItem(qApp->activeWindow(),
                                             tr("Change Data Method"),
                                             tr("For what type of data do you want to change the method ?"),
                                             opts, 0, false, &ok);
    if (ok) {
        opts.clear();
        opts.append(ModelUtilities::getDataMethodText(Date::eMHSymetric));
        opts.append(ModelUtilities::getDataMethodText(Date::eInversion));
        opts.append(ModelUtilities::getDataMethodText(Date::eMHSymGaussAdapt));

        QString methodStr = QInputDialog::getItem(qApp->activeWindow(),
                                                  tr("Change Data Method"),
                                                  tr("Change MCMC method of data in selected events") + " :",
                                                  opts, 0, false, &ok);
        if (ok && !methodStr.isEmpty()) {
            Date::DataMethod method = ModelUtilities::getDataMethodFromText(methodStr);
            PluginAbstract* plugin =PluginManager::getPluginFromName(pluginName);
            QString pluginId = plugin->getId();
            mProject->updateSelectedEventsDataMethod(method, pluginId);
        }
    }
}
void MainWindow::doGroupedAction()
{
    if (!mProject)
        return;

    QAction* act = qobject_cast<QAction*>(sender());
    QVariant groupedActionVariant = act->data();
    QHash<QString, QVariant> groupedAction = groupedActionVariant.toHash();

    if (groupedAction["inputType"] == "combo") {
        bool ok;
        QString curve = QInputDialog::getItem(qApp->activeWindow(),
                                               groupedAction["title"].toString(),
                                               groupedAction["label"].toString(),
                                               groupedAction["items"].toStringList(),
                                               0, false, &ok);
        if (ok && !curve.isEmpty()) {
            groupedAction["value"] = curve;
            mProject->updateAllDataInSelectedEvents(groupedAction);
        }
    }
}

// Events
/**
 * @todo Fix app close event called twice when updating with Qt >= 5.6
 */
void MainWindow::closeEvent(QCloseEvent* e)
{
    if (mProject) {
        QMessageBox message(QMessageBox::Question,
                            QApplication::applicationName(),
                            tr("Do you really want to quit ChronoModel ?"),
                            QMessageBox::Yes | QMessageBox::No,
                            qApp->activeWindow());

        if (message.exec() == QMessageBox::Yes) {
            if (mProject->askToSave(tr("Save project before quitting?")) == true) {
                writeSettings();
                e->accept();

                // This is a temporary Qt bug fix (should be corrected by Qt 5.6 when released)
                // The close event is called twice on Mac when closing with "cmd + Q" key or with the "Quit Chronomodel" menu.
                QCoreApplication::exit(0);
            } else
                e->ignore();

        } else
            e->ignore();
    } else {
        e->accept();

        // This is a temporary Qt bug fix (should be corrected by Qt 5.6 when released)
        // The close event is called twice on Mac when closing with "cmd + Q" key or with the "Quit Chronomodel" menu.
        QCoreApplication::exit(0);
    }

}

void MainWindow::keyPressEvent(QKeyEvent* keyEvent)
{
    if (keyEvent->matches(QKeySequence::Undo))
        mUndoStack->undo();

    else if (keyEvent->matches(QKeySequence::Redo))
        mUndoStack->redo();

    QMainWindow::keyPressEvent(keyEvent);
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        // TODO : set every text that needs translation !!!
        qDebug() << "--> MainWindow language updated";
        mNewProjectAction->setText(tr("&New"));
    } else
        QMainWindow::changeEvent(event);

}

// Settings
void MainWindow::writeSettings()
{
    mProjectView->writeSettings();
    AppSettings::mLastSize = size();
    AppSettings::mLastPosition = pos();

    AppSettings::writeSettings();

}

void MainWindow::readSettings(const QString& defaultFilePath)
{
 /*   QSettings settings;
    settings.beginGroup("MainWindow");

    resize(settings.value("size", QSize(400, 400)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());

    settings.beginGroup("AppSettings");
    mAppSettings.mLanguage = (QLocale::Language) settings.value(APP_SETTINGS_STR_LANGUAGE, QLocale::system().language()).toInt();
    mAppSettings.mCountry = (QLocale::Country) settings.value(APP_SETTINGS_STR_COUNTRY, QLocale::system().language()).toInt();
    QFont f;
    QString fam = settings.value(APP_SETTINGS_STR_FONT_FAMILY, APP_SETTINGS_DEFAULT_FONT_FAMILY).toString();
    qreal pointF = settings.value(APP_SETTINGS_STR_FONT_SIZE, APP_SETTINGS_DEFAULT_FONT_SIZE).toDouble();
    f.setFamily(fam);
    f.setPointSizeF(pointF);
    AppSettings::setFont(f);
    mAppSettings.mAutoSave = settings.value(APP_SETTINGS_STR_AUTO_SAVE, APP_SETTINGS_DEFAULT_AUTO_SAVE).toBool();
    mAppSettings.mAutoSaveDelay = settings.value(APP_SETTINGS_STR_AUTO_SAVE_DELAY_SEC, APP_SETTINGS_DEFAULT_AUTO_SAVE_DELAY_SEC).toInt();
    mAppSettings.mShowHelp = settings.value(APP_SETTINGS_STR_SHOW_HELP, APP_SETTINGS_DEFAULT_SHOW_HELP).toBool();
    mAppSettings.mCSVCellSeparator = settings.value(APP_SETTINGS_STR_CELL_SEP, APP_SETTINGS_DEFAULT_CELL_SEP).toString();
    mAppSettings.mCSVDecSeparator = settings.value(APP_SETTINGS_STR_DEC_SEP, APP_SETTINGS_DEFAULT_DEC_SEP).toString();
    mAppSettings.mOpenLastProjectAtLaunch = settings.value(APP_SETTINGS_STR_OPEN_PROJ, APP_SETTINGS_DEFAULT_OPEN_PROJ).toBool();
    mAppSettings.mPixelRatio = settings.value(APP_SETTINGS_STR_PIXELRATIO, APP_SETTINGS_DEFAULT_PIXELRATIO).toInt();
    mAppSettings.mDpm = settings.value(APP_SETTINGS_STR_DPM, APP_SETTINGS_DEFAULT_DPM).toInt();
    mAppSettings.mImageQuality = settings.value(APP_SETTINGS_STR_IMAGE_QUALITY, APP_SETTINGS_DEFAULT_IMAGE_QUALITY).toInt();
    mAppSettings.mFormatDate = (DateUtils::FormatDate)settings.value(APP_SETTINGS_STR_FORMATDATE, APP_SETTINGS_DEFAULT_FORMATDATE).toInt();
    mAppSettings.mPrecision = settings.value(APP_SETTINGS_STR_PRECISION, APP_SETTINGS_DEFAULT_PRECISION).toInt();
    mAppSettings.mNbSheet = settings.value(APP_SETTINGS_STR_SHEET, APP_SETTINGS_DEFAULT_SHEET).toInt();
    settings.endGroup();

    //settings.endGroup();
*/

    move( AppSettings::mLastPosition);
    if (AppSettings::mLastSize.width() >50)
        resize( AppSettings::mLastSize);
    else
        resize( QSize(400, 400));

    mProjectView->showHelp(AppSettings::mShowHelp);
    mHelpAction->setChecked(AppSettings::mShowHelp);

    bool fileOpened = false;
    if (!defaultFilePath.isEmpty()) {
        QFileInfo fileInfo(defaultFilePath);
        if (fileInfo.isFile()) {
            if (!mProject)
                mProject = new Project();

            else if (mProject->mModel) {
                mProject->mModel->clear();
                mProjectView->resetInterface();
            }
            if (mProject->load(defaultFilePath)) {
                activateInterface(true);
                updateWindowTitle();
                connectProject();

                mProject->setAppSettings();

                mProjectView->setProject(mProject);

                mProject->pushProjectState(mProject->mState, PROJECT_LOADED_REASON, true, true);
                // to do, it'is done in project load
                if (! mProject->mModel->mChains.isEmpty()) {
                    mViewLogAction -> setEnabled(true);
                    mViewResultsAction -> setEnabled(true);
                    mViewResultsAction -> setChecked(true); // Just check the Result Button after computation and mResultsView is show after

                    mProject->mModel->updateFormatSettings();
                 }

                fileOpened = true;
            }
        }
    }

    if (!fileOpened && AppSettings::mOpenLastProjectAtLaunch) {
        const QString dir = AppSettings::mLastDir;
        const QString filename = AppSettings::mLastFile;
        const QString path = dir + "/" + filename;
        QFileInfo fileInfo(path);

        // look MainWindows::openProject
        if (fileInfo.isFile()) {
            mProject = new Project();
            if (mProject->load(path)) {
                activateInterface(true);
                updateWindowTitle();
                connectProject();

                mProject->setAppSettings();
                mProjectView->setProject(mProject);

                mProject->pushProjectState(mProject->mState, PROJECT_LOADED_REASON, true, true);
                // to do, it'is done in project load
                if (! mProject->mModel->mChains.isEmpty()) {
                    // pushProjectState find mStructurelsChanged on true and emit NoResult()
                //    mProject->mStructureIsChanged = false;
                //    mProject->setNoResults(false);
                    mViewLogAction -> setEnabled(true);
                    mViewResultsAction -> setEnabled(true);
                    mViewResultsAction -> setChecked(true); // Just check the Result Button after computation and mResultsView is show after

                    mProject->mModel->updateFormatSettings();
                 }
            }
        }
    }

    setAppSettings();
    mProjectView->readSettings();

    if (mProject && (! mProject->mModel->mChains.isEmpty()) )
    {
        mProject->mModel->updateDesignFromJson();
        mProjectView->showResults();
   }
}

void MainWindow::resetInterface()
{
    mProjectView->resetInterface();
}

void MainWindow::activateInterface(bool activate)
{
    mProjectView->setVisible(activate);

    mProjectSaveAction->setEnabled(activate);
    mProjectSaveAsAction->setEnabled(activate);
    mProjectExportAction->setEnabled(activate);

    mChronocurveAction->setEnabled(activate);
    mViewModelAction->setEnabled(activate);
    mMCMCSettingsAction->setEnabled(activate);
    mResetMCMCAction->setEnabled(activate);

    mSelectEventsAction->setEnabled(activate);
    mEventsColorAction->setEnabled(activate);
    mEventsMethodAction->setEnabled(activate);
    mDatesMethodAction->setEnabled(activate);
    for (auto act : mDatesActions)
        act->setEnabled(activate);

    // Les actions suivantes doivent être désactivées si on ferme le projet.
    // Par contre, elles ne doivent pas être ré-activée dès l'ouverture d'un projet
    mRunAction->setEnabled(activate);

    if (!activate) {
        mViewResultsAction->setEnabled(activate);
        mViewLogAction->setEnabled(activate);
    }

    //  int largeurEcran = QApplication::desktop()->width();
    //  int hauteurEcran = QApplication::desktop()->height();

    int numScreen (QApplication::desktop()->screenNumber(this));
    QScreen *screen;
    if (numScreen>0) {
        screen = QApplication::screens().at(numScreen);
    } else {
        screen =  QGuiApplication::primaryScreen();
        numScreen = 0;
    }

    //qreal mm_per_cm = 10;
    qreal cm_per_in = 2.54;

    qDebug()<<"MainWindow::activateInterface numScreen = "<< numScreen <<  QGuiApplication::screens().at(numScreen);
    qDebug()<<"MainWindow::activateInterface this >>screenGeometry"<< numScreen << QApplication::desktop()->screenGeometry(this) << QApplication::desktop()->availableGeometry(this)<< QApplication::desktop()->width();
    qDebug()<<"MainWindow::activateInterface screen width"<< width() / screen->physicalDotsPerInchX() * cm_per_in;
    qDebug()<<"MainWindow::activateInterface screen height"<< height() / screen->physicalDotsPerInchY() * cm_per_in;

}

void MainWindow::setRunEnabled(bool enabled)
{
    mRunAction->setEnabled(enabled);
}

void MainWindow::setResultsEnabled(bool enabled)
{
    mViewResultsAction->setEnabled(enabled);
}

void MainWindow::setLogEnabled(bool enabled)
{
    mViewLogAction->setEnabled(enabled);
}

void MainWindow::mcmcFinished(Model* model)
{
    Q_ASSERT(model);

    // Set Results and Log tabs enabled
    mViewLogAction->setEnabled(true);
    mViewResultsAction->setEnabled(true);

    // Should be elsewhere ?
    mProject->setNoResults(false); // set to be able to save the file *.res
    model->updateFormatSettings();

    // Just check the Result Button (the view will be shown by ProjectView::initResults below)
    mViewResultsAction->setChecked(true);

    // Tell the views to update
    mProjectView->initResults(model);
}

void MainWindow::noResult()
{
     mViewLogAction -> setEnabled(false);
     mViewResultsAction -> setEnabled(false);
     mViewResultsAction -> setChecked(false);

     mViewModelAction->trigger();
     mProject->setNoResults(true); // set to disable the saving the file *.res
}
