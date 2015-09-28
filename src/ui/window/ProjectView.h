#ifndef ProjectView_H
#define ProjectView_H

#include <QWidget>
#include "MCMCLoopMain.h"

class QStackedWidget;
class QTextEdit;
class QTabWidget;

class ModelView;
class ResultsView;
class Event;
class Project;


class ProjectView: public QWidget
{
    Q_OBJECT
public:
    ProjectView(QWidget* parent = 0, Qt::WindowFlags flags = 0);
    ~ProjectView();
    
    void doProjectConnections(Project* project);
    void resetInterface();
    
    void readSettings();
    void writeSettings();
    
public slots:
    void updateProject();
    void showModel();
    void showResults(bool updateModel = true);
    void showLog();
    void showHelp(bool show);
    
    void updateResults(Model*);
    void updateResultsLog(const QString& log);
    
private:
    QStackedWidget* mStack;
    ModelView* mModelView;
    ResultsView* mResultsView;
    QWidget* mLogView;
    QTabWidget* mLogTabs;
    QTextEdit* mLogModelEdit;
    QTextEdit* mLogMCMCEdit;
    QTextEdit* mLogResultsEdit;
};

#endif
