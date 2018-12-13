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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Singleton.h"
#include "AppSettings.h"

#include <QMainWindow>

class QMenu;
class QAction;
class QActionGroup;
class QStackedWidget;
class QUndoStack;
class QUndoView;
class QDockWidget;
class ProjectView;
class Project;
class Event;
class Model;

class MainWindow : public QMainWindow, public Singleton<MainWindow>
{
    Q_OBJECT
    friend class Singleton<MainWindow>;

public:
    MainWindow(QWidget* aParent = nullptr);
    ~MainWindow();

    Project* getProject();
    QJsonObject getState() const;

    QUndoStack* getUndoStack();
    QString getCurrentPath() const;
    void setCurrentPath(const QString& path);

    void resetInterface();
    void activateInterface(bool activate);
    void setRunEnabled(bool enabled);
    void setLogEnabled(bool enabled);
    void setResultsEnabled(bool enabled);
    QString getNameProject() const;

    void readSettings(const QString& defaultFilePath);
    void writeSettings();
    void updateWindowTitle();
    void setFont(const QFont& font);

protected:
    void closeEvent(QCloseEvent* e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent* e) Q_DECL_OVERRIDE;
    void changeEvent(QEvent* event) Q_DECL_OVERRIDE;

    void connectProject();
    void disconnectProject();

private:
    void createActions();
    void createMenus();
    void createToolBars();

public slots:
    void newProject();
    void openProject();
    void insertProject();
    void closeProject();
    void saveProject();
    void saveProjectAs();
    void about();
    void appSettings();
    void setAppSettings();
    void setAppFilesSettings();
    void openManual();
    void openWebsite();
    void showHelp(bool);
    void setLanguage(QAction* action);
    void mcmcFinished(Model*);
    void noResult();
    void updateProject();

    void changeEventsColor();
    void changeEventsMethod();
    void changeDatesMethod();
    void selectedEventInSelectedPhases();
    void doGroupedAction();


private:
    QToolBar* mToolBar;

    QStackedWidget* mCentralStack;
    ProjectView* mProjectView;
    Project* mProject;
    QUndoStack* mUndoStack;
    QUndoView* mUndoView;
    QDockWidget* mUndoDock;

    QString mLastPath;

    QMenu* mProjectMenu;
    QMenu* mEditMenu;
    QMenu* mMCMCMenu;
    QMenu* mViewMenu;
    QMenu* mHelpMenu;
    QMenu* mActionsMenu;
    QMenu* mLanguageMenu;

    QAction* mAppSettingsAction;
    QAction* mAboutAct;
    QAction* mAboutQtAct;

    QActionGroup* mLangGroup;
    QAction* mTranslateEnglishAct;
    QAction* mTranslateFrenchAct;

    QAction* mNewProjectAction;
    QAction* mOpenProjectAction;
    QAction* mInsertProjectAction;
    QAction* mCloseProjectAction;

    QAction* mProjectSaveAction;
    QAction* mProjectSaveAsAction;
    QAction* mProjectExportAction;

    QAction* mMCMCSettingsAction;
    QAction* mRunAction;
    QAction* mResetMCMCAction;

    QActionGroup* mViewGroup;
    QAction* mViewModelAction;
    QAction* mViewResultsAction;
    QAction* mViewLogAction;

    QAction* mUndoAction;
    QAction* mRedoAction;
    QAction* mUndoViewAction;

    QAction* mSelectEventsAction;
    QAction* mEventsColorAction;
    QAction* mEventsMethodAction;
    QAction* mDatesMethodAction;
    QList<QAction*> mDatesActions;

    QAction* mHelpAction;
    QAction* mManualAction;
    QAction* mWebsiteAction;

private:
    Q_DISABLE_COPY(MainWindow)
};

#endif
