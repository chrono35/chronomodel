#include "MainWindow.h"
#include "Project.h"
#include "ProjectView.h"
#include "../PluginAbstract.h"
#include "AboutDialog.h"
#include "AppSettingsDialog.h"
#include <QtWidgets>


#pragma mark constructor / destructor
MainWindow::MainWindow(QWidget* aParent):QMainWindow(aParent)
{
    setWindowTitle("Chronomodel");
    
    QPalette tooltipPalette;
    tooltipPalette.setColor(QPalette::ToolTipBase, Qt::white);
    tooltipPalette.setColor(QPalette::ToolTipText, Qt::black);
    QToolTip::setPalette(tooltipPalette);
    QFont tooltipFont;
    tooltipFont.setItalic(true);
    tooltipFont.setPointSizeF(11.f);
    QToolTip::setFont(tooltipFont);
    
    mLastPath = QDir::homePath();
    
    mUndoStack = new QUndoStack();
    mUndoStack->setUndoLimit(1000);
    
    mProject = new Project();
    mProject->setAppSettings(mAppSettings);

    mProjectView = new ProjectView();
    setCentralWidget(mProjectView);

    mUndoView = new QUndoView(mUndoStack);
    mUndoView->setEmptyLabel(tr("Initial state"));
    mUndoDock = new QDockWidget(this);
    mUndoDock->setFixedWidth(250);
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
    setMinimumSize(1000, 700);
    
    connect(mProjectSaveAction, SIGNAL(triggered()), this, SLOT(saveProject()));
    connect(mProjectSaveAsAction, SIGNAL(triggered()), this, SLOT(saveProjectAs()));
    connect(mMCMCSettingsAction, SIGNAL(triggered()), mProject, SLOT(mcmcSettings()));
    connect(mProjectExportAction, SIGNAL(triggered()), mProject, SLOT(exportAsText()));
    connect(mRunAction, SIGNAL(triggered()), mProject, SLOT(run()));
    connect(mProject, SIGNAL(mcmcFinished(MCMCLoopMain&)), this, SLOT(mcmcFinished()));
    
    connect(mProject, SIGNAL(projectStateChanged()), mProjectView, SLOT(updateProject()));
    connect(mViewModelAction, SIGNAL(triggered()), mProjectView, SLOT(showModel()));
    connect(mViewResultsAction, SIGNAL(triggered()), mProjectView, SLOT(showResults()));
    connect(mViewLogAction, SIGNAL(triggered()), mProjectView, SLOT(showLog()));
    connect(mProject, SIGNAL(mcmcFinished(MCMCLoopMain&)), mProjectView, SLOT(updateLog(MCMCLoopMain&)));
    
    mProjectView->doProjectConnections(mProject);
    
    activateInterface(false);
}

MainWindow::~MainWindow()
{
    
}

#pragma mark Accessors
Project* MainWindow::getProject()
{
    return mProject;
}

AppSettings MainWindow::getAppSettings() const
{
    return mAppSettings;
}

QUndoStack* MainWindow::getUndoStack()
{
    return mUndoStack;
}

QString MainWindow::getCurrentPath()
{
    return mLastPath;
}

void MainWindow::setCurrentPath(const QString& path)
{
    mLastPath = path;
}

#pragma mark Actions & Menus
void MainWindow::createActions()
{
    mAppSettingsAction = new QAction(QIcon(":settings.png"), tr("Settings"), this);
    connect(mAppSettingsAction, SIGNAL(triggered()), this, SLOT(appSettings()));
    
    //-----------------------------------------------------------------
    // Project Actions
    //-----------------------------------------------------------------
    
    mNewProjectAction = new QAction(QIcon(":new.png"), tr("&New"), this);
    mNewProjectAction->setShortcuts(QKeySequence::New);
    mNewProjectAction->setStatusTip(tr("Create a new project"));
    connect(mNewProjectAction, SIGNAL(triggered()), this, SLOT(newProject()));
    
    mOpenProjectAction = new QAction(QIcon(":open.png"), tr("Open"), this);
    mOpenProjectAction->setShortcuts(QKeySequence::Open);
    mOpenProjectAction->setStatusTip(tr("Open an existing project"));
    connect(mOpenProjectAction, SIGNAL(triggered()), this, SLOT(openProject()));
    
    mCloseProjectAction = new QAction(tr("Close"), this);
    mCloseProjectAction->setShortcuts(QKeySequence::Close);
    mCloseProjectAction->setStatusTip(tr("Open an existing project"));
    connect(mCloseProjectAction, SIGNAL(triggered()), this, SLOT(closeProject()));
    
    mProjectSaveAction = new QAction(QIcon(":save.png"), tr("&Save"), this);
    mProjectSaveAction->setShortcuts(QKeySequence::Save);
    
    mProjectSaveAsAction = new QAction(QIcon(":save.png"), tr("Save as..."), this);
    
    mProjectExportAction = new QAction(QIcon(":export.png"), tr("Export"), this);
    mProjectExportAction->setVisible(false);
    
    mUndoAction = mUndoStack->createUndoAction(this);
    mUndoAction->setIcon(QIcon(":undo.png"));
    mUndoAction->setText(tr("Undo"));
    
    mRedoAction = mUndoStack->createRedoAction(this);
    mRedoAction->setIcon(QIcon(":redo.png"));
    
    mUndoViewAction = mUndoDock->toggleViewAction();
    mUndoViewAction->setText(tr("Show Undo Stack"));
    
    //-----------------------------------------------------------------
    // MCMC Actions
    //-----------------------------------------------------------------
    mMCMCSettingsAction = new QAction(QIcon(":settings.png"), tr("MCMC"), this);
    
    mRunAction = new QAction(QIcon(":run.png"), tr("Run"), this);
    //runAction->setIcon(qApp->style()->standardIcon(QStyle::SP_MediaPlay));
    mRunAction->setIconText(tr("Run"));
    mRunAction->setIconVisibleInMenu(true);
    mRunAction->setToolTip(tr("Run Model"));
    
    //-----------------------------------------------------------------
    // View Actions
    //-----------------------------------------------------------------
    mViewModelAction = new QAction(QIcon(":model.png"), tr("Model"), this);
    mViewModelAction->setCheckable(true);
    
    mViewResultsAction = new QAction(QIcon(":results.png"), tr("Results"), this);
    mViewResultsAction->setCheckable(true);
    mViewResultsAction->setEnabled(false);
    
    mViewLogAction = new QAction(QIcon(":results.png"), tr("Log"), this);
    mViewLogAction->setCheckable(true);
    
    mViewGroup = new QActionGroup(this);
    mViewGroup->addAction(mViewModelAction);
    mViewGroup->addAction(mViewResultsAction);
    mViewGroup->addAction(mViewLogAction);
    mViewModelAction->setChecked(true);
    
    //-----------------------------------------------------------------
    // Help/About Menu
    //-----------------------------------------------------------------
    mAboutAct = new QAction(QIcon(":light.png"), tr("About"), this);
    connect(mAboutAct, SIGNAL(triggered()), this, SLOT(about()));
    
    mAboutQtAct = new QAction(QIcon(":qt.png"), tr("About Qt"), this);
    connect(mAboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    
    mHelpAction = new QAction(QIcon(":help.png"), tr("Help"), this);
    mHelpAction->setCheckable(true);
    connect(mHelpAction, SIGNAL(toggled(bool)), this, SLOT(showHelp(bool)));
    
    mManualAction = new QAction(QIcon(":pdf.png"), tr("Manual"), this);
    connect(mManualAction, SIGNAL(triggered()), this, SLOT(openManual()));
    
    mWebsiteAction = new QAction(QIcon(":web.png"), tr("Website"), this);
    connect(mWebsiteAction, SIGNAL(triggered()), this, SLOT(openWebsite()));
}

void MainWindow::createMenus()
{
    //-----------------------------------------------------------------
    // Project menu
    //-----------------------------------------------------------------
    mProjectMenu = menuBar()->addMenu(tr("Project"));

    mProjectMenu->addAction(mAppSettingsAction);
    mProjectMenu->addAction(mNewProjectAction);
    mProjectMenu->addAction(mOpenProjectAction);
    mProjectMenu->addAction(mCloseProjectAction);

    mProjectMenu->addSeparator();

    mProjectMenu->addAction(mProjectSaveAction);
    mProjectMenu->addAction(mProjectSaveAsAction);
    
    mProjectMenu->addSeparator();

    mProjectMenu->addAction(mProjectExportAction);
    
    //-----------------------------------------------------------------
    // Edit menu
    //-----------------------------------------------------------------
    mEditMenu = menuBar()->addMenu(tr("Edit"));
    
    mEditMenu->addAction(mUndoAction);
    mEditMenu->addAction(mRedoAction);
    mEditMenu->addAction(mUndoViewAction);
    
    //-----------------------------------------------------------------
    // MCMC menu
    //-----------------------------------------------------------------
    mMCMCMenu = menuBar()->addMenu(tr("MCMC"));
    mMCMCMenu->addAction(mRunAction);
    mMCMCMenu->addAction(mMCMCSettingsAction);
    
    //-----------------------------------------------------------------
    // View menu
    //-----------------------------------------------------------------
    mViewMenu = menuBar()->addMenu(tr("View"));
    mViewMenu->addAction(mViewModelAction);
    mViewMenu->addAction(mViewResultsAction);
    mViewMenu->addAction(mViewLogAction);
    
    //-----------------------------------------------------------------
    // Help/About Menu
    //-----------------------------------------------------------------
    mHelpMenu = menuBar()->addMenu(tr("&Help"));
    mHelpMenu->addAction(mAboutAct);
    mHelpMenu->addAction(mAboutQtAct);
}

void MainWindow::createToolBars()
{
    //-----------------------------------------------------------------
    // Main ToolBar
    //-----------------------------------------------------------------
    QToolBar* toolBar = addToolBar("Main Tool Bar");
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->setMovable(false);
    toolBar->setAllowedAreas(Qt::TopToolBarArea);
    
    toolBar->addAction(mNewProjectAction);
    toolBar->addAction(mOpenProjectAction);
    toolBar->addAction(mProjectSaveAction);
    toolBar->addAction(mProjectExportAction);
    
    toolBar->addAction(mUndoAction);
    toolBar->addAction(mRedoAction);
    
    QWidget* separator3 = new QWidget(this);
    separator3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolBar->addWidget(separator3);
    
    toolBar->addAction(mViewModelAction);
    toolBar->addAction(mMCMCSettingsAction);
    toolBar->addAction(mRunAction);
    toolBar->addAction(mViewResultsAction);
    toolBar->addAction(mViewLogAction);
    
    QWidget* separator4 = new QWidget(this);
    separator4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolBar->addWidget(separator4);
    
    toolBar->addAction(mHelpAction);
    toolBar->addAction(mManualAction);
    toolBar->addAction(mWebsiteAction);
    /*toolBar->addAction(mAboutAct);
    toolBar->addAction(mAboutQtAct);*/
}


// -----------

#pragma mark Project Management

void MainWindow::newProject()
{
    // Ask to save the previous project.
    // Return true if the project doesn't need to be saved.
    // Returns true if the user saves the project or if the user doesn't want to save it.
    // Returns false if the user cancels.
    if(mProject->askToSave())
    {
        // Ask to save the new project.
        // Returns true only if a new file is created.
        // Note : at this point, the project state is still the previous project state.
        if(mProject->saveAs())
        {
            mUndoStack->clear();
            // Reset the project state and send a notification to update the views :
            mProject->initState();
            activateInterface(true);
        }
        
        mViewModelAction->trigger();
        //mProjectView->showModel();
        
        mViewResultsAction->setEnabled(false);
        updateWindowTitle();
    }
}

void MainWindow::openProject()
{
    QString path = QFileDialog::getOpenFileName(qApp->activeWindow(),
                                                tr("Open File"),
                                                getCurrentPath(),
                                                tr("Chronomodel Project (*.chr)"));
    
    if(!path.isEmpty())
    {
        if(mProject->askToSave())
        {
            QFileInfo info(path);
            setCurrentPath(info.absolutePath());
            
            mUndoStack->clear();
            mProject->load(path);
            activateInterface(true);
            updateWindowTitle();
        }
    }
}

void MainWindow::closeProject()
{
    if(mProject->askToSave())
    {
        mUndoStack->clear();
        
        mProject->initState();
        mProject->mProjectFileName = QString();
        
        mViewModelAction->trigger();
        //mProjectView->showModel();
        
        activateInterface(false);
        mViewResultsAction->setEnabled(false);
        
        updateWindowTitle();
    }
}

void MainWindow::saveProject()
{
    mProject->save();
    updateWindowTitle();
}

void MainWindow::saveProjectAs()
{
    mProject->saveAs();
    updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
    setWindowTitle(qApp->applicationName() + " " + qApp->applicationVersion() + (!(mProject->mProjectFileName.isEmpty()) ? QString(" - ") + mProject->mProjectFileName : ""));
}

#pragma mark Settings & About
void MainWindow::about()
{
    AboutDialog dialog;
    dialog.exec();
}

void MainWindow::appSettings()
{
    AppSettingsDialog dialog(qApp->activeWindow(), Qt::Sheet);
    dialog.setSettings(mAppSettings);
    if(dialog.exec() == QDialog::Accepted)
    {
        mAppSettings = dialog.getSettings();
        mProject->setAppSettings(mAppSettings);
    }
}

void MainWindow::openManual()
{
    QString path = qApp->applicationDirPath();
#ifdef Q_OS_MAC
    QDir dir(path);
    dir.cdUp();
    path = dir.absolutePath() + "/Resources";
#endif
    path += "/Chronomodel_User_Manual.pdf";
    QDesktopServices::openUrl(QUrl("file:///" + path, QUrl::TolerantMode));
}

void MainWindow::showHelp(bool show)
{
    mAppSettings.mShowHelp = show;
    mProjectView->showHelp(show);
}

void MainWindow::openWebsite()
{
    QDesktopServices::openUrl(QUrl("http://www.chronomodel.fr", QUrl::TolerantMode));
}

#pragma mark Events
void MainWindow::closeEvent(QCloseEvent* e)
{
    saveProject();
    writeSettings();
    e->accept();
}

void MainWindow::keyPressEvent(QKeyEvent* keyEvent)
{
    if(keyEvent->matches(QKeySequence::Undo))
    {
        mUndoStack->undo();
    }
    else if(keyEvent->matches(QKeySequence::Redo))
    {
        mUndoStack->redo();
    }
    QMainWindow::keyPressEvent(keyEvent);
}

#pragma mark Settings
void MainWindow::writeSettings()
{
    mProjectView->writeSettings();
    
    QSettings settings;
    settings.beginGroup("MainWindow");
    
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    
    settings.setValue("last_project_dir", mProject->mProjectFileDir);
    settings.setValue("last_project_filename", mProject->mProjectFileName);
    
    settings.beginGroup("AppSettings");
    settings.setValue(APP_SETTINGS_STR_AUTO_SAVE, mAppSettings.mAutoSave);
    settings.setValue(APP_SETTINGS_STR_AUTO_SAVE_DELAY_SEC, mAppSettings.mAutoSaveDelay);
    settings.setValue(APP_SETTINGS_STR_SHOW_HELP, mAppSettings.mShowHelp);
    settings.setValue(APP_SETTINGS_STR_CELL_SEP, mAppSettings.mCSVCellSeparator);
    settings.setValue(APP_SETTINGS_STR_DEC_SEP, mAppSettings.mCSVDecSeparator);
    settings.setValue(APP_SETTINGS_STR_OPEN_PROJ, mAppSettings.mOpenLastProjectAtLaunch);
    settings.endGroup();
    
    settings.endGroup();
    
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    
    resize(settings.value("size", QSize(400, 400)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    
    settings.beginGroup("AppSettings");
    mAppSettings.mAutoSave = settings.value(APP_SETTINGS_STR_AUTO_SAVE, APP_SETTINGS_DEFAULT_AUTO_SAVE).toBool();
    mAppSettings.mAutoSaveDelay = settings.value(APP_SETTINGS_STR_AUTO_SAVE_DELAY_SEC, APP_SETTINGS_DEFAULT_AUTO_SAVE_DELAY_SEC).toInt();
    mAppSettings.mShowHelp = settings.value(APP_SETTINGS_STR_SHOW_HELP, APP_SETTINGS_DEFAULT_SHOW_HELP).toBool();
    mAppSettings.mCSVCellSeparator = settings.value(APP_SETTINGS_STR_CELL_SEP, APP_SETTINGS_DEFAULT_CELL_SEP).toString();
    mAppSettings.mCSVDecSeparator = settings.value(APP_SETTINGS_STR_DEC_SEP, APP_SETTINGS_DEFAULT_DEC_SEP).toString();
    mAppSettings.mOpenLastProjectAtLaunch = settings.value(APP_SETTINGS_STR_OPEN_PROJ, APP_SETTINGS_DEFAULT_OPEN_PROJ).toBool();
    settings.endGroup();
    
    mProjectView->showHelp(mAppSettings.mShowHelp);
    mHelpAction->setChecked(mAppSettings.mShowHelp);
    
    QString dir = settings.value("last_project_dir", "").toString();
    QString filename = settings.value("last_project_filename", "").toString();
    
    QString path = dir + "/" + filename;
    QFileInfo fileInfo(path);
    qDebug() << "Loading project file : " << path;
    if(fileInfo.isFile())
    {
        mProject->load(path);
        activateInterface(true);
        updateWindowTitle();
    }
    
    settings.endGroup();
    
    mProjectView->readSettings();
}

void MainWindow::activateInterface(bool activate)
{
    mProjectView->setVisible(activate);
    mProjectSaveAction->setEnabled(activate);
    mProjectSaveAsAction->setEnabled(activate);
    mProjectExportAction->setEnabled(activate);
    mMCMCSettingsAction->setEnabled(activate);
    mRunAction->setEnabled(activate);
}

void MainWindow::mcmcFinished()
{
    mViewResultsAction->setEnabled(true);
    mViewResultsAction->trigger();
}
