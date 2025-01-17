#include "Project.h"
#include "MainWindow.h"
#include "Model.h"
#include "PluginManager.h"
#include "ProjectSettingsDialog.h"
#include "MCMCSettingsDialog.h"

#include "Event.h"
#include "EventKnown.h"
#include "EventDialog.h"
#include "EventConstraint.h"
#include "ConstraintDialog.h"

#include "Phase.h"
#include "PhaseConstraint.h"
#include "PhaseDialog.h"

#include "DateDialog.h"
#include "Date.h"
#include "../PluginAbstract.h"
#include "../PluginFormAbstract.h"
#include "TrashDialog.h"
#include "ImportDataView.h"

#include "ModelUtilities.h"
#include "QtUtilities.h"

#include "MCMCLoopMain.h"
#include "MCMCProgressDialog.h"

#include "SetProjectState.h"
#include "StateEvent.h"

#include <iostream>
#include <QtWidgets>
#include <QThread>
#include <QJsonObject>
#include <QFile>


// Temporary
//#include "Plugin14C.h"



Project::Project():
mName(tr("Chronomodel Project")),
mProjectFileDir(""),
mProjectFileName(QObject::tr("Untitled"))

{
    mState = emptyState();
    mLastSavedState = mState;
    
    mAutoSaveTimer = new QTimer(this);
    connect(mAutoSaveTimer, SIGNAL(timeout()), this, SLOT(save()));
    mAutoSaveTimer->start(3000);
    mModel = new Model();
    mRefreshResults =true;
}

Project::~Project()
{
    mAutoSaveTimer->stop();
}


#pragma mark Project State

void Project::initState(const QString& reason)
{
    QJsonObject state = emptyState();
    
    // Do no call pushProjectState here because we don't want to store this state in the UndoStack
    // This is called when closing a project or openning a new one,
    // so the undoStack has just been cleared and we want to keep it empty at project start!
    mState=state;
    sendUpdateState(state, reason, true);

}

QJsonObject Project::emptyState() const
{
    QJsonObject state;
    
    state[STATE_APP_VERSION] = qApp->applicationVersion();
    
    ProjectSettings projectSettings;
    QJsonObject settings = projectSettings.toJson();
    state[STATE_SETTINGS] = settings;
    
    MCMCSettings mcmcSettings;
    mcmcSettings.restoreDefault();
    QJsonObject mcmc = mcmcSettings.toJson();

    state[STATE_MCMC] = mcmc;
    
    QJsonArray events;
    state[STATE_EVENTS] = events;
    
    QJsonArray phases;
    state[STATE_PHASES] = phases;
    
    QJsonArray events_constraints;
    state[STATE_EVENTS_CONSTRAINTS] = events_constraints;
    
    QJsonArray phases_constraints;
    state[STATE_PHASES_CONSTRAINTS] = phases_constraints;
    
    QJsonArray dates_trash;
    state[STATE_DATES_TRASH] = dates_trash;
    
    QJsonArray events_trash;
    state[STATE_EVENTS_TRASH] = events_trash;
    
    return state;
}

QJsonObject Project::state() const
{
    return mState;
}

bool Project::pushProjectState(const QJsonObject& state, const QString& reason, bool notify, bool force)
{
    if(mState != state || force)
    {
        if(!mState.isEmpty()) mRefreshResults = checkRefreshResults(state,mState);
        else mRefreshResults=true;

        SetProjectState* command = new SetProjectState(this, mState, state, reason, notify);
        MainWindow::getInstance()->getUndoStack()->push(command);

        if(mRefreshResults) {
            emit projectDesignChanged(mRefreshResults);
        }
        return true;
    }
    return false;
}


void Project::sendUpdateState(const QJsonObject& state, const QString& reason, bool notify)
{
#ifdef DEBUG
    qDebug() << " +++  Sending : " << reason;
#endif
    StateEvent* event = new StateEvent(state, reason, notify);
    QCoreApplication::postEvent(this, event, Qt::NormalEventPriority);
}

bool Project::checkRefreshResults(const QJsonObject& stateNew,const QJsonObject& stateOld)
{
    bool refresh=false;

    if(stateOld.isEmpty())  {
        refresh=true;
        return refresh;
    }
    else {

        // Check phase design modification
        QJsonArray phaseNew = stateNew[STATE_PHASES].toArray();
        QJsonArray phaseOld = stateOld[STATE_PHASES].toArray();
        if( phaseNew.size()!=phaseOld.size()) {
            refresh=true;
            return refresh;
        }
        else {
            for(int i=0; i<phaseNew.size(); ++i){
               // Check name of Phases
               if(phaseNew[i].toObject()[STATE_NAME] != phaseOld[i].toObject()[STATE_NAME]) {
                   refresh=true;
                   return refresh;
               }
               // Check color of Phases
               QColor newPhaseColor=QColor(phaseNew[i].toObject()[STATE_COLOR_RED].toInt(),phaseNew[i].toObject()[STATE_COLOR_GREEN].toInt(),phaseNew[i].toObject()[STATE_COLOR_BLUE ].toInt());
               QColor oldPhaseColor=QColor(phaseOld[i].toObject()[STATE_COLOR_RED].toInt(),phaseOld[i].toObject()[STATE_COLOR_GREEN].toInt(),phaseOld[i].toObject()[STATE_COLOR_BLUE ].toInt());
               if(newPhaseColor != oldPhaseColor) {
                   refresh=true;
                   return refresh;
               }
            }



        }
         // Check Event and date design modification
        QJsonArray eventNew = stateNew[STATE_EVENTS].toArray();
        QJsonArray eventOld = stateOld[STATE_EVENTS].toArray();
        if( eventNew.size()!=eventOld.size()) {
            refresh=true;
            return refresh;
        }
        else {
            for(int i=0; i<eventNew.size(); ++i){
               // Check name of Event
               if(eventNew[i].toObject()[STATE_NAME] != eventOld[i].toObject()[STATE_NAME]) {
                   refresh=true;
                   return refresh;
               }
               // Check color of Event
               QColor newEventColor=QColor(eventNew[i].toObject()[STATE_COLOR_RED].toInt(),eventNew[i].toObject()[STATE_COLOR_GREEN].toInt(),eventNew[i].toObject()[STATE_COLOR_BLUE ].toInt());
               QColor oldEventColor=QColor(eventOld[i].toObject()[STATE_COLOR_RED].toInt(),eventOld[i].toObject()[STATE_COLOR_GREEN].toInt(),eventOld[i].toObject()[STATE_COLOR_BLUE ].toInt());
               if(newEventColor != oldEventColor) {
                   refresh=true;
                   return refresh;
               }
               // Check date inside Event


               QJsonArray datesNew = eventNew[i].toObject()[STATE_EVENT_DATES].toArray();
               QJsonArray datesOld = eventOld[i].toObject()[STATE_EVENT_DATES].toArray();
               if( datesNew.size()!=datesOld.size()) {
                   refresh=true;
                   return refresh;
               }

               else {
                   for(int j=0; j<datesNew.size(); ++j){
                        // Check name of Event
                        if(datesNew[j].toObject()[STATE_NAME] != datesOld[j].toObject()[STATE_NAME]) {
                            refresh=true;
                            return refresh;
                        }
                        // No color in date JSON
                        /*QColor newDateColor=QColor(eventNew[i].toObject()[STATE_COLOR_RED].toInt(),eventNew[i].toObject()[STATE_COLOR_GREEN].toInt(),eventNew[i].toObject()[STATE_COLOR_BLUE ].toInt());
                        QColor oldDateColor=QColor(eventOld[i].toObject()[STATE_COLOR_RED].toInt(),eventOld[i].toObject()[STATE_COLOR_GREEN].toInt(),eventOld[i].toObject()[STATE_COLOR_BLUE ].toInt());
                        if(newDateColor != oldDateColor) {
                            refresh=true;
                            return refresh;
                        }
                        */
                     }
                }
              // qDebug()<<"Check date";






            }

        }
      }

    return refresh;



}


bool Project::event(QEvent* e)
{
    if(e->type() == QEvent::User)
    {
        StateEvent* se = static_cast<StateEvent*>(e);
        if(se)
        {
            updateState(se->state(), se->reason(), se->notify());
        }
        return true;
    }
    else if(e->type() == 1001)
    {
#ifdef DEBUG
        qDebug() << "(---) Receiving events selection : adapt checked phases";
#endif
        emit selectedEventsChanged();
        return true;
    }
    else if(e->type() == 1002)
    {
#ifdef DEBUG
        qDebug() << "(---) Receiving phases selection : adapt selected events";
#endif
        emit selectedPhasesChanged();
        return true;
    }
    else
    {
        return QObject::event(e);
    }
}

void Project::updateState(const QJsonObject& state, const QString& reason, bool notify)
{
    //qDebug() << " ---  Receiving : " << reason;
    mState = state;
    if(notify)
    {
        /*QProgressDialog progress(qApp->activeWindow(), Qt::Sheet);
        progress.setLabelText(tr("Loading..."));
        progress.setRange(0, 0);
        progress.setModal(true);
        progress.setCancelButton(0);*/
        
        if(reason == PROJECT_LOADED_REASON)
        {
            // ------------------------------------------
            //  TODO : find a way to show progress by theading the loading process
            // ------------------------------------------
            
            /*progress.setWindowModality(Qt::WindowModal);
            progress.show();
            QThread::currentThread()->msleep(200);
            if(progress.wasCanceled())
            {
                
            }*/
        }
        else if(reason == NEW_PROJECT_REASON)
        {
            showStudyPeriodWarning();
        }
        emit projectStateChanged();
    }
}

void Project::sendEventsSelectionChanged()
{
#ifdef DEBUG
    qDebug() << "(+++) Sending events selection : use marked events";
#endif
    QEvent* e = new QEvent((QEvent::Type)1001);
    QCoreApplication::postEvent(this, e, Qt::NormalEventPriority);
}

void Project::sendPhasesSelectionChanged()
{
#ifdef DEBUG
    qDebug() << "(+++) Sending phases selection : use marked phases";
#endif
    QEvent* e = new QEvent((QEvent::Type)1002);
    QCoreApplication::postEvent(this, e, Qt::NormalEventPriority);
}



bool Project::isValid(){
    QJsonArray events = mState[STATE_EVENTS].toArray();
    for(int i=0; i<events.size(); ++i)
    {
        QJsonObject event = events[i].toObject();
        QJsonArray dates = event[STATE_EVENT_DATES].toArray();
        for(int j=0; j<dates.size(); ++j)
        {
            QJsonObject date = dates[j].toObject();
            if(!date[STATE_DATE_VALID].toBool())
                return false;
        }
    }
    return true;
}


#pragma mark Project File Management

bool Project::load(const QString& path)
{
    QFileInfo checkFile(path);
    if (!checkFile.exists() || !checkFile.isFile()) {
        QMessageBox message(QMessageBox::Critical,
                            tr("Error loading project file"),
                            tr("The project file could not be loaded.") + "\n" +
                            path  +
                            tr("Could not be find"),
                            QMessageBox::Ok,
                            qApp->activeWindow(),
                            Qt::Sheet);
        message.exec();
        return false;
    }
    QFile file(path);

    qDebug() << "in Project::load Loading project file : " << path;

    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QFileInfo info(path);
        MainWindow::getInstance()->setCurrentPath(info.absolutePath());
        
        mProjectFileDir = info.absolutePath();
        mProjectFileName = info.fileName();
        
        QByteArray saveData = file.readAll();
        
        QJsonParseError error;
        
        QJsonDocument jsonDoc(QJsonDocument::fromJson(saveData, &error));
        
        if(error.error !=  QJsonParseError::NoError)
        {
            QMessageBox message(QMessageBox::Critical,
                                tr("Error loading project file"),
                                tr("The project file could not be loaded.") + "\n" + 
                                tr("Error message") + " : " + error.errorString(),
                                QMessageBox::Ok,
                                qApp->activeWindow(),
                                Qt::Sheet);
            message.exec();
            return false;
        }
        else
        {
            mState = jsonDoc.object();
            
            //  Ask all plugins if dates are corrects.
            //  If not, it may be an incompatibility between plugins versions (new parameter added for example...)
            //  This function gives a chance to plugins to modify dates saved with old versions in order to use them with the new version.
            checkDatesCompatibility();
            
            //  Check if dates are valid on the current study period
            mState = checkValidDates(mState);
            
            if(!mState.contains(STATE_APP_VERSION))
                mState[STATE_APP_VERSION] = qApp->applicationVersion();
            
            mLastSavedState = mState;
            
            
            pushProjectState(mState, PROJECT_LOADED_REASON, true, true);
            
            file.close();
            
            
            // --------------------
            
            clearModel();
            
            QString dataPath = path + ".dat";
            //QFile dataFile(dataPath); // HL
            QFile dataFile;
            //QDir::setCurrent("/tmp");
            dataFile.setFileName(dataPath);
            
            //QFileInfo fi(dataFile);
            //dataFile.open(QIODevice::ReadOnly);
            //if(fi.isFile())

            if(false)//dataFile.exists())
            {

                qDebug() << "Project::load Loading model file.dat : " << dataPath << " size=" << dataFile.size();
      
                try{
                    mModel->setJson(mState);
                    mModel->fromJson(mState);

                }
                catch(QString error){
                    QMessageBox message(QMessageBox::Critical,
                                        tr("Error loading project"),
                                        tr("The project could not be loaded.") + "\n" +
                                        tr("Error message") + " : " + error,
                                        QMessageBox::Ok,
                                        qApp->activeWindow(),
                                        Qt::Sheet);
                    message.exec();
                    
                    clearModel();
                }
                
                
                try{
                    mModel->restoreFromFile(dataPath);
                    
                    emit mcmcFinished(mModel);
                }catch(QString error){
                    QMessageBox message(QMessageBox::Critical,
                                        tr("Error loading project MCMC results"),
                                        tr("The project MCMC results could not be loaded.") + "\n" +
                                        tr("Error message") + " : " + error,
                                        QMessageBox::Ok,
                                        qApp->activeWindow(),
                                        Qt::Sheet);
                    message.exec();
                }
            }
            
            // --------------------
            
            return true;
        }
    }
    return false;
}

bool Project::save(const QString& dialogTitle)
{
    QFileInfo info(mProjectFileDir + "/" + mProjectFileName);
    return info.exists() ? saveProjectToFile() : saveAs(dialogTitle);
}

bool Project::saveAs(const QString& dialogTitle)
{
    QString path = QFileDialog::getSaveFileName(qApp->activeWindow(),
                                                dialogTitle,
                                                MainWindow::getInstance()->getCurrentPath(),
                                                tr("Chronomodel Project (*.chr)"));
    if(!path.isEmpty())
    {
        QFileInfo info(path);
        MainWindow::getInstance()->setCurrentPath(info.absolutePath());
        
        mProjectFileDir = info.absolutePath();
        mProjectFileName = info.fileName();
        
        // We need to reset mLastSavedState because it corresponds
        // to the last saved state in the previous file.
        mLastSavedState = QJsonObject();
        
        return saveProjectToFile();
    }
    return false;
}

bool Project::askToSave(const QString& saveDialogTitle)
{
    // Check if modifs have been made
    if(mState == mLastSavedState)
        return true;
    
    // We have some modifications : ask to save :
    int result = QMessageBox::question(QApplication::activeWindow(),
                                       QApplication::applicationName(),
                                       tr("Do you want to save the current project ?"),
                                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    
    if(result == QMessageBox::Yes)
    {
        // return true if saving is done correcty
        return save(saveDialogTitle);
    }
    else if(result == QMessageBox::No)
    {
        // the user doesn't want to save : returning true to continue
        return true;
    }
    else if(result == QMessageBox::Cancel)
    {
        // the user canceled : return false to cancel any further operations
        return false;
    }
    return false;
}

bool Project::saveProjectToFile()
{
    if(mLastSavedState != mState)
    {
        QString path = mProjectFileDir + "/" + mProjectFileName;
        QFile file(path);
        if(file.open(QIODevice::ReadWrite | QIODevice::Text))
        {
#ifdef DEBUG
            qDebug() << "Project saved to : " << path;
#endif
            mLastSavedState = mState;
            
            QJsonDocument jsonDoc(mState);
            file.write(jsonDoc.toJson(QJsonDocument::Indented));
            file.resize(file.pos());
            file.close();
        }
        else
        {
            return false;
        }
    }
    else
    {
#ifdef DEBUG
        qDebug() << "Nothing new to save in project model";
#endif
    }
/*    if(!mModel->mChains.isEmpty()) // keep to the future version
    {
      //  qDebug() << "Saving project results";
        mModel->saveToFile(mProjectFileDir + "/" + mProjectFileName + ".dat");
    }
    else {
        QFileInfo checkFile(mProjectFileDir + "/" + mProjectFileName + ".dat");
        if (checkFile.exists() && checkFile.isFile()) {
            QFile(mProjectFileDir + "/" + mProjectFileName + ".dat").remove();
        }
         else {
            return true;
        }

    } */

    return true;
}

// --------------------------------------------------------------------
//     Project Settings
// --------------------------------------------------------------------
#pragma mark Settings
bool Project::setSettings(const ProjectSettings& settings)
{
    if(settings.mTmin >= settings.mTmax)
    {
        QMessageBox message(QMessageBox::Critical, tr("Inconsistent values"), tr("Start Date must be lower than End Date !"), QMessageBox::Ok, qApp->activeWindow(), Qt::Sheet);
        message.exec();
        return false;
    }
   /* else if(settings.mStep < 0.1)
    {
        QMessageBox message(QMessageBox::Critical, tr("Inconsistent values"), tr("Step must be >= 0.1 !"), QMessageBox::Ok, qApp->activeWindow(), Qt::Sheet);
        message.exec();
        return false;
    } */
    else
    {
        QJsonObject stateNext = mState;
        
        // Set the new srudy period in the new state
        stateNext[STATE_SETTINGS] = settings.toJson();
        
        // Check if dates are still valid on the new study period
        stateNext = checkValidDates(stateNext);
        
        //  Push the new state having a new study period with dates' "valid flag" updated!
        return pushProjectState(stateNext, tr("Settings updated"), true, true);
    }
}

void Project::setAppSettings(const AppSettings& settings)
{
    mAutoSaveTimer->setInterval(settings.mAutoSaveDelay * 1000);
    if(mAutoSaveTimer->isActive() && !settings.mAutoSave)
        mAutoSaveTimer->stop();
    else if(!mAutoSaveTimer->isActive() && settings.mAutoSave)
        mAutoSaveTimer->start();
}

/**
 * @brief Project::restoreMCMCSettings not used
 */
void Project::restoreMCMCSettings()
{
    MCMCSettings settings;
    settings.restoreDefault();

    QJsonObject stateNext = mState;
    stateNext[STATE_MCMC] = settings.toJson();
    pushProjectState(stateNext, tr("MCMC Settings restore default"), true);

}

void Project::mcmcSettings()
{
    MCMCSettingsDialog dialog(qApp->activeWindow());
    MCMCSettings settings = MCMCSettings::fromJson(mState[STATE_MCMC].toObject());
    //settings.fromJson(mState[STATE_MCMC].toObject());
    dialog.setSettings(settings);
    dialog.setModal(true);
    
    if(dialog.exec() == QDialog::Accepted)
    {
        MCMCSettings settings = dialog.getSettings();
        
        QJsonObject stateNext = mState;
        stateNext[STATE_MCMC] = settings.toJson();
        pushProjectState(stateNext, tr("MCMC Settings updated"), true);
    }
}
/**
 * @brief Project::resetMCMC Restore default samplinf methods on each ti and theta
 */
void Project::resetMCMC()
{
    QMessageBox message(QMessageBox::Warning,
                        tr("Reset MCMC methods"),
                        tr("All event's and data's MCMC methods will be reset to their default value.\nDo you really want to do this ?"),
                        QMessageBox::Yes | QMessageBox::No,
                        qApp->activeWindow(),
                        Qt::Sheet);
    if(message.exec() == QMessageBox::Yes)
    {
        QJsonObject stateNext = mState;
        QJsonArray events = mState[STATE_EVENTS].toArray();
        
        for(int i=0; i<events.size(); ++i)
        {
            QJsonObject event = events[i].toObject();
            event[STATE_EVENT_METHOD] = (int)Event::eDoubleExp;
            
            QJsonArray dates = event[STATE_EVENT_DATES].toArray();
            for(int j=0; j<dates.size(); ++j)
            {
                QJsonObject date = dates[j].toObject();
                
                try{
                    Date d = Date::fromJson(date);
                    if(!d.isNull())
                    {
                        date[STATE_DATE_METHOD] = (int)d.mPlugin->getDataMethod();
                        dates[j] = date;
                    }
                }
                catch(QString error){
                    QMessageBox message(QMessageBox::Critical,
                                        qApp->applicationName() + " " + qApp->applicationVersion(),
                                        tr("Error : ") + error,
                                        QMessageBox::Ok,
                                        qApp->activeWindow(),
                                        Qt::Sheet);
                    message.exec();
                }
            }
            event[STATE_EVENT_DATES] = dates;
            events[i] = event;
        }
        stateNext[STATE_EVENTS] = events;
        stateNext[STATE_MCMC_MIXING] = MCMC_MIXING_DEFAULT;
        pushProjectState(stateNext, tr("MCMC methods reset"), true);
    }
}

bool Project::studyPeriodIsValid()
{
    QJsonObject settings = mState[STATE_SETTINGS].toObject();
    int tmin = settings[STATE_SETTINGS_TMIN].toInt();
    int tmax = settings[STATE_SETTINGS_TMAX].toInt();
    if(tmin >= tmax)
    {
        showStudyPeriodWarning();
    }
    return (tmin < tmax);
}

void Project::showStudyPeriodWarning()
{
    // Display help message
    QMessageBox message(QMessageBox::Information,
                        tr("Study period definition required"),
                        tr("To start your new project, you first have to define a study period and click the \"Apply\" button."),
                        QMessageBox::Ok,
                        qApp->activeWindow(),
                        Qt::Sheet);
    message.exec();
}


// --------------------------------------------------------------------
//     Events
// --------------------------------------------------------------------
#pragma mark Events
int Project::getUnusedEventId(const QJsonArray& events)
{
    int id = -1;
    bool idIsFree = false;
    while(!idIsFree)
    {
        ++id;
        idIsFree = true;
        for(int i=0; i<events.size(); ++i)
        {
            QJsonObject event = events.at(i).toObject();
            if(event[STATE_ID].toInt() == id)
                idIsFree = false;
        }
    }
    return id;
}

void Project::createEvent()
{
    if(studyPeriodIsValid())
    {
        EventDialog dialog(qApp->activeWindow(), tr("New Event"), Qt::Sheet);
        if(dialog.exec() == QDialog::Accepted)
        {
            Event event;
            event.mInitName = dialog.getName();
            event.mInitColor= dialog.getColor();
            
            addEvent(event.toJson(), tr("Event created"));
        }
    }
}

void Project::createEventKnown()
{
    if(studyPeriodIsValid())
    {
        EventDialog dialog(qApp->activeWindow(), tr("New Bound"), Qt::Sheet);
        if(dialog.exec() == QDialog::Accepted)
        {
            EventKnown event;
            event.mInitName = dialog.getName();
            event.mInitColor= dialog.getColor();
            
            addEvent(event.toJson(), tr("Bound created"));
        }
    }
}

void Project::addEvent(QJsonObject event, const QString& reason)
{
    QJsonObject stateNext = mState;
    QJsonArray events = stateNext[STATE_EVENTS].toArray();
    
    event[STATE_ID] = getUnusedEventId(events);
    events.append(event);
    stateNext[STATE_EVENTS] = events;
    
    pushProjectState(stateNext, reason, true);
}


void Project::deleteSelectedEvents()
{
    QJsonObject stateNext = mState;
    
    QJsonArray events = mState[STATE_EVENTS].toArray();
    QJsonArray events_constraints = mState[STATE_EVENTS_CONSTRAINTS].toArray();
    QJsonArray events_trash = mState[STATE_EVENTS_TRASH].toArray();
    
    for(int i=events.size()-1; i>=0; --i)
    {
        QJsonObject event = events[i].toObject();
        if(event[STATE_IS_SELECTED].toBool())
        {
            int event_id = event[STATE_ID].toInt();
            for(int j=events_constraints.size()-1; j>=0; --j)
            {
                QJsonObject constraint = events_constraints[j].toObject();
                int bwd_id = constraint[STATE_CONSTRAINT_BWD_ID].toInt();
                int fwd_id = constraint[STATE_CONSTRAINT_FWD_ID].toInt();
                if(bwd_id == event_id || fwd_id == event_id)
                {
                    events_constraints.removeAt(j);
                }
            }
            events.removeAt(i);
            events_trash.append(event);
        }
    }
    stateNext[STATE_EVENTS] = events;
    stateNext[STATE_EVENTS_CONSTRAINTS] = events_constraints;
    stateNext[STATE_EVENTS_TRASH] = events_trash;
    
    pushProjectState(stateNext, tr("Event(s) deleted"), true);
    
    clearModel();
    MainWindow::getInstance() -> setResultsEnabled(false);
    MainWindow::getInstance() -> setLogEnabled(false);
}

void Project::deleteSelectedTrashedEvents(const QList<int>& ids)
{
    QJsonObject stateNext = mState;
    
    QJsonArray events_trash = mState[STATE_EVENTS_TRASH].toArray();
    
    for(int i=events_trash.size()-1; i>=0; --i)
    {
        QJsonObject event = events_trash[i].toObject();
        int id = event[STATE_ID].toInt();
        
        if(ids.contains(id))
        {
            events_trash.removeAt(i);
        }
    }
    stateNext[STATE_EVENTS_TRASH] = events_trash;
    
    pushProjectState(stateNext, tr("Trashed event(s) deleted"), true);
}

void Project::recycleEvents()
{
    TrashDialog dialog(TrashDialog::eEvent, qApp->activeWindow(), Qt::Sheet);
    if(dialog.exec() == QDialog::Accepted)
    {
        QList<int> indexes = dialog.getSelectedIndexes();
        qDebug() << indexes;
        
        QJsonObject stateNext = mState;
        QJsonArray events = mState[STATE_EVENTS].toArray();
        QJsonArray events_trash = mState[STATE_EVENTS_TRASH].toArray();
        
        for(int i=indexes.size()-1; i>=0; --i)
        {
            QJsonObject event = events_trash.takeAt(indexes[i]).toObject();
            event[STATE_ID] = getUnusedEventId(events);
            events.append(event);
        }
        stateNext[STATE_EVENTS] = events;
        stateNext[STATE_EVENTS_TRASH] = events_trash;
        
        pushProjectState(stateNext, tr("Event(s) restored"), true);
    }
}

void Project::updateEvent(const QJsonObject& event, const QString& reason)
{
    QJsonObject stateNext = mState;
    QJsonArray events = mState[STATE_EVENTS].toArray();
    for(int i=0; i<events.size(); ++i)
    {
        QJsonObject evt = events[i].toObject();
        if(evt[STATE_ID].toInt() == event[STATE_ID].toInt())
        {
            events[i] = event;
            break;
        }
    }
    stateNext[STATE_EVENTS] = events;
    pushProjectState(stateNext, reason, true);
}
/**
 * @brief Project::mergeEvents Offer to insert all the dates of an event inside an other event
 * @param eventFromId
 * @param eventToId
 */
void Project::mergeEvents(int eventFromId, int eventToId)
{
    QJsonObject stateNext = mState;
    QJsonArray events = mState[STATE_EVENTS].toArray();
    QJsonObject eventFrom;
    QJsonObject eventTo;
    int fromIndex = -1;
    int toIndex = -1;
    
    for(int i=events.size()-1; i>=0; --i)
    {
        QJsonObject evt = events.at(i).toObject();
        int id = evt[STATE_ID].toInt();
        if(id == eventFromId)
        {
            eventFrom = evt;
            fromIndex = i;
        }
        else if(id == eventToId)
        {
            eventTo = evt;
            toIndex = i;
        }
    }
    QJsonArray datesFrom = eventFrom[STATE_EVENT_DATES].toArray();
    QJsonArray datesTo = eventTo[STATE_EVENT_DATES].toArray();
    for(int i=0; i<datesFrom.size(); ++i){
        QJsonObject dateFrom = datesFrom.at(i).toObject();
        dateFrom[STATE_ID] = getUnusedDateId(datesTo);
        datesTo.append(dateFrom);
    }
    eventTo[STATE_EVENT_DATES] = datesTo;
    
    events[toIndex] = eventTo;
    events.removeAt(fromIndex);
    stateNext[STATE_EVENTS] = events;
    
    // Delete constraints around the disappearing event
    QJsonArray constraints = stateNext[STATE_EVENTS_CONSTRAINTS].toArray();
    for(int i=constraints.size()-1; i>=0; --i)
    {
        QJsonObject c = constraints.at(i).toObject();
        int fromId = c[STATE_CONSTRAINT_BWD_ID].toInt();
        int toId = c[STATE_CONSTRAINT_FWD_ID].toInt();
        if(eventFromId == fromId || eventFromId == toId)
        {
            constraints.removeAt(i);
        }
    }
    stateNext[STATE_EVENTS_CONSTRAINTS] = constraints;
    
    //qDebug() << events;
    
    pushProjectState(stateNext, tr("Events merged"), true);
}

#pragma mark Grouped actions on events
void Project::updateSelectedEventsColor(const QColor& color)
{
    QJsonObject stateNext = mState;
    QJsonArray events = mState[STATE_EVENTS].toArray();
    for(int i=0; i<events.size(); ++i)
    {
        QJsonObject evt = events[i].toObject();
        if(evt[STATE_IS_SELECTED].toBool())
        {
            evt[STATE_COLOR_RED] = color.red();
            evt[STATE_COLOR_GREEN] = color.green();
            evt[STATE_COLOR_BLUE] = color.blue();
            events[i] = evt;
        }
    }
    stateNext[STATE_EVENTS] = events;
    pushProjectState(stateNext, tr("Update selected events color"), true);
}

void Project::updateSelectedEventsMethod(Event::Method method)
{
    QJsonObject stateNext = mState;
    QJsonArray events = mState[STATE_EVENTS].toArray();
    for(int i=0; i<events.size(); ++i)
    {
        QJsonObject evt = events[i].toObject();
        if(evt[STATE_IS_SELECTED].toBool())
        {
            evt[STATE_EVENT_METHOD] = method;
            events[i] = evt;
        }
    }
    stateNext[STATE_EVENTS] = events;
    pushProjectState(stateNext, tr("Update selected events method"), true);
}

void Project::updateSelectedEventsDataMethod(Date::DataMethod method, const QString& pluginId)
{
    QJsonObject stateNext = mState;
    QJsonArray events = mState[STATE_EVENTS].toArray();
    for(int i=0; i<events.size(); ++i)
    {
        QJsonObject evt = events[i].toObject();
        if(evt[STATE_IS_SELECTED].toBool())
        {
            QJsonArray dates = evt[STATE_EVENT_DATES].toArray();
            for(int j=0; j<dates.size(); ++j)
            {
                QJsonObject date = dates[j].toObject();
                if(date[STATE_DATE_PLUGIN_ID].toString() == pluginId){
                    date[STATE_DATE_METHOD] = method;
                    dates[j] = date;
                }
            }
            evt[STATE_EVENT_DATES] = dates;
            events[i] = evt;
        }
    }
    stateNext[STATE_EVENTS] = events;
    pushProjectState(stateNext, tr("Update selected events method"), true);
}


// --------------------------------------------------------------------
//     Dates
// --------------------------------------------------------------------
#pragma mark Dates
int Project::getUnusedDateId(const QJsonArray& dates)
{
    int id = -1;
    bool idIsFree = false;
    while(!idIsFree)
    {
        ++id;
        idIsFree = true;
        for(int i=0; i<dates.size(); ++i)
        {
            QJsonObject date = dates.at(i).toObject();
            if(date[STATE_ID].toInt() == id)
                idIsFree = false;
        }
    }
    return id;
}

Date Project::createDateFromPlugin(PluginAbstract* plugin)
{
    Date date;
    if(plugin)
    {
        DateDialog dialog(qApp->activeWindow());
        PluginFormAbstract* form = plugin->getForm();
        dialog.setForm(form);
        dialog.setDataMethod(plugin->getDataMethod());
        
        if(dialog.exec() == QDialog::Accepted)
        {
            if(form->isValid())
            {
                date.mPlugin = plugin;
                date.mData = form->getData();
                
                date.mInitName = dialog.getName();
                date.mMethod = dialog.getMethod();
                date.mDeltaType = dialog.getDeltaType();
                date.mDeltaFixed = dialog.getDeltaFixed();
                date.mDeltaMin = dialog.getDeltaMin();
                date.mDeltaMax = dialog.getDeltaMax();
                date.mDeltaAverage = dialog.getDeltaAverage();
                date.mDeltaError = dialog.getDeltaError();
            }
            else
            {
                QMessageBox message(QMessageBox::Critical,
                                    tr("Invalid data"),
                                    form->mError,
                                    QMessageBox::Ok,
                                    qApp->activeWindow(),
                                    Qt::Sheet);
                message.exec();
            }
        }
    }
    return date;
}

void Project::addDate(int eventId, QJsonObject date)
{
    QJsonObject stateNext = mState;
    
    // Validate the date before adding it to the correct event and pushing the state
    QJsonObject settingsJson = stateNext[STATE_SETTINGS].toObject();
    ProjectSettings settings = ProjectSettings::fromJson(settingsJson);
    PluginAbstract* plugin = PluginManager::getPluginFromId(date[STATE_DATE_PLUGIN_ID].toString());
    bool valid = plugin->isDateValid(date[STATE_DATE_DATA].toObject(), settings);
    date[STATE_DATE_VALID] = valid;
    
    // Add the date
    QJsonArray events = mState[STATE_EVENTS].toArray();
    for(int i=0; i<events.size(); ++i)
    {
        QJsonObject evt = events[i].toObject();
        if(evt[STATE_ID].toInt() == eventId)
        {
            date[STATE_ID] = getUnusedDateId(evt[STATE_EVENT_DATES].toArray());
            QJsonArray dates = evt[STATE_EVENT_DATES].toArray();
            dates.append(date);
            
            evt[STATE_EVENT_DATES] = dates;
            events[i] = evt;
            stateNext[STATE_EVENTS] = events;
            pushProjectState(stateNext, tr("Date created"), true);
            break;
        }
    }
}

void Project::checkDatesCompatibility()
{
    QJsonObject state = mState;
    QJsonArray events = mState[STATE_EVENTS].toArray();
    
    for(int i=0; i<events.size(); ++i)
    {
        QJsonObject event = events[i].toObject();
        QJsonArray dates = event[STATE_EVENT_DATES].toArray();
        for(int j=0; j<dates.size(); ++j)
        {
            QJsonObject date = dates[j].toObject();
            
            // -----------------------------------------------------------
            //  Check the date compatibility with the plugin version
            // -----------------------------------------------------------
            if(date.find(STATE_DATE_SUB_DATES) == date.end())
                date[STATE_DATE_SUB_DATES] = QJsonArray();
            
            if(date.find(STATE_DATE_VALID) == date.end())
                date[STATE_DATE_VALID] = true;
            
            // etc...
            // Here, we could control if all date fields are present, and add them if not.
            
            // -----------------------------------------------------------
            //  Check the date compatibility with the plugin version
            //  Only the STATE_DATE_DATA is to be checked
            // -----------------------------------------------------------
            QString pluginId = date[STATE_DATE_PLUGIN_ID].toString();
            PluginAbstract* plugin = PluginManager::getPluginFromId(pluginId);
            date[STATE_DATE_DATA] = plugin->checkValuesCompatibility(date[STATE_DATE_DATA].toObject());
            
            // -----------------------------------------------------------
            //  Check subdates compatibility with the plugin version
            // -----------------------------------------------------------
            QJsonArray subdates = date[STATE_DATE_SUB_DATES].toArray();
            for(int k=0; k<subdates.size(); ++k){
                QJsonObject subdate = subdates[k].toObject();
                subdate[STATE_DATE_DATA] = plugin->checkValuesCompatibility(subdate[STATE_DATE_DATA].toObject());
                subdates[k] = subdate;
            }
            date[STATE_DATE_SUB_DATES] = subdates;
            // -----------------------------------------------------------
            
            dates[j] = date;
            event[STATE_EVENT_DATES] = dates;
            events[i] = event;
            state[STATE_EVENTS] = events;
        }
    }
    mState = state;
}

void Project::updateDate(int eventId, int dateIndex)
{
    QJsonObject state = mState;
    
    QJsonObject settingsJson = state[STATE_SETTINGS].toObject();
    ProjectSettings settings = ProjectSettings::fromJson(settingsJson);
    
    QJsonArray events = mState[STATE_EVENTS].toArray();
    
    for(int i=0; i<events.size(); ++i)
    {
        QJsonObject event = events[i].toObject();
        if(event[STATE_ID].toInt() == eventId)
        {
            QJsonArray dates = event[STATE_EVENT_DATES].toArray();
            if(dateIndex < dates.size())
            {
                QJsonObject date = dates[dateIndex].toObject();
                
                QString pluginId = date[STATE_DATE_PLUGIN_ID].toString();
                PluginAbstract* plugin = PluginManager::getPluginFromId(pluginId);
                
                DateDialog dialog(qApp->activeWindow(), Qt::Sheet);
                PluginFormAbstract* form = plugin->getForm();
                dialog.setForm(form);
                dialog.setDate(date);
                
                if(dialog.exec() == QDialog::Accepted)
                {
                    if(form->isValid())
                    {
                        date[STATE_DATE_DATA] = form->getData();
                        date[STATE_NAME] = dialog.getName();
                        date[STATE_DATE_METHOD] = dialog.getMethod();
                        
                        date[STATE_DATE_DELTA_TYPE] = dialog.getDeltaType();
                        date[STATE_DATE_DELTA_FIXED] = dialog.getDeltaFixed();
                        date[STATE_DATE_DELTA_MIN] = dialog.getDeltaMin();
                        date[STATE_DATE_DELTA_MAX] = dialog.getDeltaMax();
                        date[STATE_DATE_DELTA_AVERAGE] = dialog.getDeltaAverage();
                        date[STATE_DATE_DELTA_ERROR] = dialog.getDeltaError();
                        
                        PluginAbstract* plugin = PluginManager::getPluginFromId(date[STATE_DATE_PLUGIN_ID].toString());
                        bool valid = plugin->isDateValid(date[STATE_DATE_DATA].toObject(), settings);
                        date[STATE_DATE_VALID] = valid;
                        
                        dates[dateIndex] = date;
                        event[STATE_EVENT_DATES] = dates;
                        events[i] = event;
                        state[STATE_EVENTS] = events;
                        
                        pushProjectState(state, tr("Date updated"), true);
                        
                        //date->calibrate(mSettings.mTmin, mSettings.mTmax, mSettings.mStep);
                    }
                    else
                    {
                        QMessageBox message(QMessageBox::Critical,
                                            tr("Invalid data"),
                                            form->mError,
                                            QMessageBox::Ok,
                                            qApp->activeWindow(),
                                            Qt::Sheet);
                        message.exec();
                    }
                }
            }
            break;
        }
    }
}

void Project::deleteDates(int eventId, const QList<int>& dateIndexes)
{
    QJsonObject state = mState;
    
    QJsonArray events = state[STATE_EVENTS].toArray();
    for(int i=0; i<events.size(); ++i)
    {
        QJsonObject event = events[i].toObject();
        if(event[STATE_ID].toInt() == eventId)
        {
            QJsonArray dates_trash = state[STATE_DATES_TRASH].toArray();
            QJsonArray dates = event[STATE_EVENT_DATES].toArray();
            
            for(int j=dates.size()-1; j>=0; --j)
            {
                if(dateIndexes.contains(j))
                {
                    QJsonObject date = dates.takeAt(j).toObject();
                    dates_trash.append(date);
                }
            }
            event[STATE_EVENT_DATES] = dates;
            events[i] = event;
            state[STATE_EVENTS] = events;
            state[STATE_DATES_TRASH] = dates_trash;
            
            pushProjectState(state, QString::number(dateIndexes.size()) + tr(" date(s) deleted"), true);
            
            break;
        }
    }
    clearModel();
    MainWindow::getInstance() -> setResultsEnabled(false);
    MainWindow::getInstance() -> setLogEnabled(false); 
}

void Project::deleteSelectedTrashedDates(const QList<int>& ids)
{
    QJsonObject stateNext = mState;
    
    QJsonArray dates_trash = mState[STATE_DATES_TRASH].toArray();
    
    for(int i=dates_trash.size()-1; i>=0; --i)
    {
        QJsonObject date = dates_trash[i].toObject();
        int id = date[STATE_ID].toInt();
        
        if(ids.contains(id))
        {
            dates_trash.removeAt(i);
        }
    }
    stateNext[STATE_DATES_TRASH] = dates_trash;
    
    pushProjectState(stateNext, tr("Trashed data deleted"), true);
}

void Project::recycleDates(int eventId)
{
    TrashDialog dialog(TrashDialog::eDate, qApp->activeWindow());
    if(dialog.exec() == QDialog::Accepted)
    {
        QList<int> indexes = dialog.getSelectedIndexes();
        qDebug() << indexes;
        
        QJsonObject stateNext = mState;
        QJsonArray events = mState[STATE_EVENTS].toArray();
        QJsonArray dates_trash = mState[STATE_DATES_TRASH].toArray();
        
        for(int i=0; i<events.size(); ++i)
        {
            QJsonObject event = events[i].toObject();
            if(event[STATE_ID].toInt() == eventId)
            {
                QJsonArray dates = event[STATE_EVENT_DATES].toArray();
                for(int i=indexes.size()-1; i>=0; --i)
                {
                    QJsonObject date = dates_trash.takeAt(indexes[i]).toObject();
                    date[STATE_ID] = getUnusedDateId(dates);
                    dates.append(date);
                }
                event[STATE_EVENT_DATES] = dates;
                events[i] = event;
                stateNext[STATE_EVENTS] = events;
                stateNext[STATE_DATES_TRASH] = dates_trash;
                
                pushProjectState(stateNext, QString::number(indexes.size()) + tr(" date(s) restored"), true);
                
                break;
            }
        }
    }
}

QJsonObject Project::checkValidDates(const QJsonObject& stateToCheck)
{
    QJsonObject state = stateToCheck;
    
    QJsonObject settingsJson = state[STATE_SETTINGS].toObject();
    ProjectSettings settings = ProjectSettings::fromJson(settingsJson);
    
    QJsonArray events = state[STATE_EVENTS].toArray();
    for(int i=0; i<events.size(); ++i){
        QJsonObject event = events[i].toObject();
        QJsonArray dates = event[STATE_EVENT_DATES].toArray();
        for(int j=0; j<dates.size(); ++j){
            QJsonObject date = dates[j].toObject();
            
            PluginAbstract* plugin = PluginManager::getPluginFromId(date[STATE_DATE_PLUGIN_ID].toString());
            bool valid = plugin->isDateValid(date[STATE_DATE_DATA].toObject(), settings);
            date[STATE_DATE_VALID] = valid;
            
            dates[j] = date;
        }
        event[STATE_EVENT_DATES] = dates;
        events[i] = event;
    }
    state[STATE_EVENTS] = events;
    return state;
}

void Project::mergeDates(const int eventId, const QList<int>& dateIds)
{
    QJsonObject stateNext = mState;
    QJsonObject settingsJson = stateNext[STATE_SETTINGS].toObject();
    ProjectSettings settings = ProjectSettings::fromJson(settingsJson);
    QJsonArray events = mState[STATE_EVENTS].toArray();
    
    for(int i=0; i<events.size(); ++i)
    {
        QJsonObject event = events[i].toObject();
        if(event[STATE_ID].toInt() == eventId)
        {
            // All event dates :
            QJsonArray dates = event[STATE_EVENT_DATES].toArray();
            
            // event dates to be merged :
            QJsonArray datesToMerge;
            for(int j=0; j<dates.size(); ++j)
            {
                QJsonObject date = dates[j].toObject();
                if(dateIds.contains(date[STATE_ID].toInt())){
                    datesToMerge.push_back(date);
                }
            }
            
            // merge !
            if(datesToMerge.size() > 0){
                PluginAbstract* plugin = PluginManager::getPluginFromId(datesToMerge[0].toObject()[STATE_DATE_PLUGIN_ID].toString());
                if(plugin){
                    
                    // add the new combination
                    QJsonObject mergedDate = plugin->mergeDates(datesToMerge);
                    if(mergedDate.find("error") != mergedDate.end()){
                        QMessageBox message(QMessageBox::Critical,
                                            tr("Cannot combine"),
                                            mergedDate["error"].toString(),
                                            QMessageBox::Ok,
                                            qApp->activeWindow(),
                                            Qt::Sheet);
                        message.exec();
                    }else{
                        // remove merged dates
                        for(int j=dates.size()-1; j>=0; --j)
                        {
                            QJsonObject date = dates[j].toObject();
                            if(dateIds.contains(date[STATE_ID].toInt())){
                                dates.removeAt(j);
                            }
                        }
                        
                        // Validate the date before adding it to the correct event and pushing the state
                        bool valid = plugin->isDateValid(mergedDate[STATE_DATE_DATA].toObject(), settings);
                        mergedDate[STATE_DATE_VALID] = valid;
                        
                        dates.push_back(mergedDate);
                    }
                }
            }
            
            event[STATE_EVENT_DATES] = dates;
            events[i] = event;
            stateNext[STATE_EVENTS] = events;
            
            pushProjectState(stateNext, tr("Dates combined"), true);
            
            break;
        }
    }
}

void Project::splitDate(const int eventId, const int dateId)
{
    QJsonObject stateNext = mState;
    QJsonObject settingsJson = stateNext[STATE_SETTINGS].toObject();
    ProjectSettings settings = ProjectSettings::fromJson(settingsJson);
    QJsonArray events = mState[STATE_EVENTS].toArray();
    
    for(int i=0; i<events.size(); ++i)
    {
        QJsonObject event = events[i].toObject();
        if(event[STATE_ID].toInt() == eventId)
        {
            QJsonArray dates = event[STATE_EVENT_DATES].toArray();
            for(int j=0; j<dates.size(); ++j)
            {
                QJsonObject date = dates[j].toObject();
                if(date[STATE_ID].toInt() == dateId){
                    
                    // We have found the date to split !
                    QJsonArray subdates = date[STATE_DATE_SUB_DATES].toArray();
                    PluginAbstract* plugin = PluginManager::getPluginFromId(date[STATE_DATE_PLUGIN_ID].toString());
                    
                    for(int k=0; k<subdates.size(); ++k){
                        QJsonObject sd = subdates[k].toObject();
                        bool valid = plugin->isDateValid(sd[STATE_DATE_DATA].toObject(), settings);
                        sd[STATE_DATE_VALID] = valid;
                        dates.push_back(sd);
                    }
                    dates.removeAt(j);
                    break;
                }
            }
            event[STATE_EVENT_DATES] = dates;
        }
        events[i] = event;
    }
    stateNext[STATE_EVENTS] = events;
    pushProjectState(stateNext, tr("Dates splitted"), true);
}

#pragma mark Grouped actions on dates
void Project::updateAllDataInSelectedEvents(const QHash<QString, QVariant>& groupedAction)
{
    QJsonObject stateNext = mState;
    QJsonArray events = mState[STATE_EVENTS].toArray();
    
    for(int i=0; i<events.size(); ++i)
    {
        QJsonObject event = events[i].toObject();
        if((event[STATE_EVENT_TYPE].toInt() == Event::eDefault) &&  // Not a bound
           (event[STATE_IS_SELECTED].toBool())) // Event is selected
        {
            QJsonArray dates = event[STATE_EVENT_DATES].toArray();
            for(int j=0; j<dates.size(); ++j)
            {
                QJsonObject date = dates[j].toObject();
                if(date[STATE_DATE_PLUGIN_ID].toString() == groupedAction["pluginId"].toString())
                {
                    QJsonObject data = date[STATE_DATE_DATA].toObject();
                    data[groupedAction["valueKey"].toString()] = groupedAction["value"].toString();
                    date[STATE_DATE_DATA] = data;
                    dates[j] = date;
                }
            }
            event[STATE_EVENT_DATES] = dates;
            events[i] = event;
        }
    }
    stateNext[STATE_EVENTS] = events;
    
    pushProjectState(stateNext, tr("Grouped action applied : ") + groupedAction["title"].toString(), true);
}

#pragma mark Phases
void Project::createPhase()
{
    if(studyPeriodIsValid())
    {
        PhaseDialog dialog(qApp->activeWindow(), Qt::Sheet);
        if(dialog.exec() == QDialog::Accepted)
        {
            if(dialog.isValid())
            {
                QJsonObject phase = dialog.getPhase();
                
                QJsonObject stateNext = mState;
                QJsonArray phases = stateNext[STATE_PHASES].toArray();
                
                phase[STATE_ID] = getUnusedPhaseId(phases);
                phases.append(phase);
                stateNext[STATE_PHASES] = phases;
                
                pushProjectState(stateNext, tr("Phase created"), true);
            }
            else
            {
                QMessageBox message(QMessageBox::Critical,
                                    tr("Invalid value"),
                                    dialog.mError,
                                    QMessageBox::Ok,
                                    qApp->activeWindow(),
                                    Qt::Sheet);
                message.exec();
                return;
            }
        }
    }
}

void Project::updatePhase(const QJsonObject& phaseIn)
{
    PhaseDialog dialog(qApp->activeWindow(), Qt::Sheet);
    dialog.setPhase(phaseIn);
    if(dialog.exec() == QDialog::Accepted)
    {
        if(dialog.isValid())
        {
            QJsonObject phase = dialog.getPhase();
            
            QJsonObject stateNext = mState;
            QJsonArray phases = stateNext[STATE_PHASES].toArray();
            
            for(int i=0; i<phases.size(); ++i)
            {
                QJsonObject p = phases[i].toObject();
                if(p[STATE_ID].toInt() == phase[STATE_ID].toInt())
                {
                    phases[i] = phase;
                    break;
                }
            }
            stateNext[STATE_PHASES] = phases;
            pushProjectState(stateNext, tr("Phase updated"), true);
        }
        else
        {
            QMessageBox message(QMessageBox::Critical,
                                tr("Invalid value"),
                                dialog.mError,
                                QMessageBox::Ok,
                                qApp->activeWindow(),
                                Qt::Sheet);
            message.exec();
            return;
        }
    }
}

void Project::deleteSelectedPhases()
{
    QJsonObject stateNext = mState;
    
    QJsonArray phases = mState[STATE_PHASES].toArray();
    QJsonArray phases_constraints = mState[STATE_PHASES_CONSTRAINTS].toArray();
    QJsonArray events = mState[STATE_EVENTS].toArray();
    
    for(int i=phases.size()-1; i>=0; --i)
    {
        QJsonObject phase = phases[i].toObject();
        if(phase[STATE_IS_SELECTED].toBool())
        {
            int phase_id = phase[STATE_ID].toInt();
            for(int j=phases_constraints.size()-1; j>=0; --j)
            {
                QJsonObject constraint = phases_constraints[j].toObject();
                int bwd_id = constraint[STATE_CONSTRAINT_BWD_ID].toInt();
                int fwd_id = constraint[STATE_CONSTRAINT_FWD_ID].toInt();
                if(bwd_id == phase_id || fwd_id == phase_id)
                {
                    phases_constraints.removeAt(j);
                }
            }
            for(int j=0; j<events.size(); ++j)
            {
                QJsonObject event = events[j].toObject();
                QString idsStr = event[STATE_EVENT_PHASE_IDS].toString();
                QStringList ids = idsStr.split(",");
                ids.removeAll(QString::number(phase_id));
                event[STATE_EVENT_PHASE_IDS] = ids.join(",");
                events[j] = event;
            }
            phases.removeAt(i);
        }
    }
    stateNext[STATE_PHASES] = phases;
    stateNext[STATE_PHASES_CONSTRAINTS] = phases_constraints;
    stateNext[STATE_EVENTS] = events;
    
    pushProjectState(stateNext, tr("Phase(s) deleted"), true);
    
    clearModel();
    MainWindow::getInstance() -> setResultsEnabled(false);
    MainWindow::getInstance() -> setLogEnabled(false);
}

int Project::getUnusedPhaseId(const QJsonArray& phases)
{
    int id = -1;
    bool idIsFree = false;
    while(!idIsFree)
    {
        ++id;
        idIsFree = true;
        for(int i=0; i<phases.size(); ++i)
        {
            QJsonObject phase = phases[i].toObject();
            if(phase[STATE_ID].toInt() == id)
                idIsFree = false;
        }
    }
    return id;
}

void Project::mergePhases(int phaseFromId, int phaseToId)
{
    QJsonObject stateNext = mState;
    QJsonArray phases = mState[STATE_PHASES].toArray();
    QJsonArray events = mState[STATE_EVENTS].toArray();
    QJsonArray phases_constraints = mState[STATE_PHASES_CONSTRAINTS].toArray();
    
    // Delete constraints around the disappearing phase
    for(int j=phases_constraints.size()-1; j>=0; --j)
    {
        QJsonObject constraint = phases_constraints[j].toObject();
        int bwd_id = constraint[STATE_CONSTRAINT_BWD_ID].toInt();
        int fwd_id = constraint[STATE_CONSTRAINT_FWD_ID].toInt();
        if(bwd_id == phaseFromId || fwd_id == phaseFromId)
        {
            phases_constraints.removeAt(j);
        }
    }
    
    // Change phase id in events
    for(int j=0; j<events.size(); ++j)
    {
        QJsonObject event = events[j].toObject();
        QString idsStr = event[STATE_EVENT_PHASE_IDS].toString();
        QStringList ids = idsStr.split(",");
        if(ids.contains(QString::number(phaseFromId)))
        {
            for(int k=0; k<ids.size(); ++k)
            {
                if(ids[k] == QString::number(phaseFromId))
                    ids[k] = QString::number(phaseToId);
            }
        }
        event[STATE_EVENT_PHASE_IDS] = ids.join(",");
        events[j] = event;
    }
    
    // remove disappearing phase
    for(int i=phases.size()-1; i>=0; --i)
    {
        QJsonObject p = phases[i].toObject();
        int id = p[STATE_ID].toInt();
        if(id == phaseFromId)
        {
            phases.removeAt(i);
            break;
        }
    }
    
    stateNext[STATE_PHASES] = phases;
    stateNext[STATE_EVENTS] = events;
    stateNext[STATE_PHASES_CONSTRAINTS] = phases_constraints;
    
    pushProjectState(stateNext, tr("Phases merged"), true);
}

void Project::updatePhaseEvents(int phaseId, Qt::CheckState state)
{
    QJsonObject stateNext = mState;
    QJsonArray events = mState[STATE_EVENTS].toArray();
    
    for(int i=0; i<events.size(); ++i)
    {
        QJsonObject event = events[i].toObject();
        if(event[STATE_IS_SELECTED].toBool())
        {
            QString phaseIdsStr = event[STATE_EVENT_PHASE_IDS].toString();
            QStringList phaseIds = phaseIdsStr.isEmpty() ? QStringList() : phaseIdsStr.split(",");
            
            phaseIds.removeAll(QString::number(phaseId));
            if(state == Qt::Checked)
                phaseIds.append(QString::number(phaseId));
            
            event[STATE_EVENT_PHASE_IDS] = phaseIds.join(",");
            events[i] = event;
        }
    }
    stateNext[STATE_EVENTS] = events;
    pushProjectState(stateNext, tr("Phase's events updated"), true);
}

/*void Project::updatePhaseEyed(int phaseId, bool eyed)
{
    QJsonObject stateNext = mState;
    QJsonArray events = mState[STATE_EVENTS].toArray();
    QJsonArray phases = mState[STATE_PHASES].toArray();
    
    // Update eyed phase
    for(int i=0; i<phases.size(); ++i)
    {
        QJsonObject phase = phases[i].toObject();
        if(phase[STATE_ID].toInt() == phaseId)
        {
            phase[STATE_PHASE_EYED] = eyed;
            phases[i] = phase;
            break;
        }
    }
    
    // Update events greyed out
    for(int i=0; i<events.size(); ++i)
    {
        QJsonObject event = events[i].toObject();
        int eventId = event[STATE_ID].toInt();
        QString eventPhasesIdsStr = event[STATE_EVENT_PHASE_IDS].toString();
        QStringList eventPhasesIds = eventPhasesIdsStr.split(",");
        bool mustBeGreyedOut = true;
        
        for(int j=0; j<phases.size(); ++j)
        {
            QJsonObject phase = phases[j].toObject();
            if(phase[STATE_PHASE_EYED].toBool() && eventPhasesIds.contains(phase[STATE_ID].toString()))
                mustBeGreyedOut = false;
        }
        event[STATE_EVENT_GREYED_OUT] = mustBeGreyedOut;
        events[i] = event;
    }
    stateNext[STATE_EVENTS] = events;
    stateNext[STATE_PHASES] = phases;
    pushProjectState(stateNext, tr("Phase eyed updated"), true);
}*/

// --------------------------------------------------------------------
//     Event Constraints authorizations
// --------------------------------------------------------------------
/*#pragma mark Event Constraints authorizations

bool Project::isEventConstraintAllowed(Event* eventFrom, Event* eventTo, QString& message)
{
    if(eventFrom && eventTo && (eventFrom != eventTo))
    {
        // ------------------------------------------------------------
        //  Vérifier que la contrainte n'existe pas déjà, dans un sens ou dans l'autre
        // ------------------------------------------------------------
        for(int i=0; i<(int)mEventConstraints.size(); ++i)
        {
            if((mEventConstraints[i]->getEventTo() == eventTo &&
                mEventConstraints[i]->getEventFrom() == eventFrom))
            {
                message = tr("The constraint already exists");
                return false;
            }
            else if(mEventConstraints[i]->getEventTo() == eventFrom &&
                mEventConstraints[i]->getEventFrom() == eventTo)
            {
                message = tr("The constraint already exists in the other direction");
                return false;
            }
        }
        
        // ------------------------------------------------------------
        //  TODO : Vérifier que l'on ne créé pas de boucle
        // ------------------------------------------------------------
        //qDebug() << "Checking loops for " << QString::number(eventFrom->mId);
        QList<Event*> eventsFrom = getAllEventsFrom(eventFrom);
        for(int i=0; i<(int)eventsFrom.size(); ++i)
        {
            if(eventsFrom[i] == eventTo)
            {
                message = tr("You cannot create a loop");
                return false;
            }
        }
        
        // ------------------------------------------------------------
        //  TODO : Vérifier que la contrainte n'est pas redondante
        // ------------------------------------------------------------
        //qDebug() << "Checking duplicates for " << QString::number(eventFrom->mId);
        QList<Event*> eventsTo = getAllEventsTo(eventFrom);
        for(int i=0; i<(int)eventsFrom.size(); ++i)
        {
            QList<Event*> subEventsTo = getAllEventsTo(eventsFrom[i]);
            for(int j=0; j<(int)subEventsTo.size(); ++j)
            {
                if(std::find(eventsTo.begin(), eventsTo.end(), subEventsTo[j]) == eventsTo.end())
                {
                    eventsTo.push_back(subEventsTo[j]);
                }
            }
        }
        for(int i=0; i<(int)eventsTo.size(); ++i)
        {
            //qDebug() << eventsTo[i]->mId << " compared to " << eventTo->mId;
            if(eventsTo[i] == eventTo)
            {
                message = tr("This information already exists thanks to other constraints");
                return false;
            }
        }
        
        return true;
    }
    message = tr("Unvalid events for constraint creation");
    return false;
}

QList<Event*> Project::getAllEventsFrom(Event* event)
{
    QList<Event*> eventsFrom;
    for(int i=0; i<(int)event->mConstraintsBwd.size(); ++i)
    {
        Event* eventFrom = event->mConstraintsBwd[i]->getEventFrom();
        //qDebug() << "- from : " << eventFrom->mId;
        eventsFrom.push_back(eventFrom);
        
        QList<Event*> subEventsFrom = getAllEventsFrom(eventFrom);
        for(int i=0; i<(int)subEventsFrom.size(); ++i)
            eventsFrom.push_back(subEventsFrom[i]);
    }
    return eventsFrom;
}

QList<Event*> Project::getAllEventsTo(Event* event)
{
    QList<Event*> eventsTo;
    for(int i=0; i<(int)event->mConstraintsFwd.size(); ++i)
    {
        Event* eventTo = event->mConstraintsFwd[i]->getEventTo();
        //qDebug() << "- to : " << eventTo->mId;
        eventsTo.push_back(eventTo);
        
        QList<Event*> subEventsTo = getAllEventsTo(eventTo);
        for(int i=0; i<(int)subEventsTo.size(); ++i)
            eventsTo.push_back(subEventsTo[i]);
    }
    return eventsTo;
}*/









// --------------------------------------------------------------------
//     Phase Constraints authorizations
// --------------------------------------------------------------------
/*#pragma mark Phase Constraints authorizations

bool Project::isPhaseConstraintAllowed(Phase* phaseFrom, Phase* phaseTo)
{
    if(phaseFrom && phaseTo && (phaseFrom != phaseTo))
    {
        // ------------------------------------------------------------
        //  Vérifier que la contrainte n'existe pas déjà, dans un sens ou dans l'autre
        // ------------------------------------------------------------
        for(int i=0; i<(int)mPhaseConstraints.size(); ++i)
        {
            if((mPhaseConstraints[i]->getPhaseTo() == phaseTo &&
                mPhaseConstraints[i]->getPhaseFrom() == phaseFrom) ||
               (mPhaseConstraints[i]->getPhaseTo() == phaseFrom &&
                mPhaseConstraints[i]->getPhaseFrom() == phaseTo))
            {
                return false;
            }
        }
        
        // ------------------------------------------------------------
        //  Vérifier que les phases n'ont pas de faits communs
        // ------------------------------------------------------------
        for(int i=0; i<(int)phaseFrom->mEvents.size(); ++i)
        {
            for(int j=0; j<(int)phaseTo->mEvents.size(); ++j)
            {
                if(phaseFrom->mEvents[i] == phaseTo->mEvents[j])
                    return false;
            }
        }
        // ------------------------------------------------------------
        //  TODO : Vérifier que l'on ne créé pas de boucle
        // ------------------------------------------------------------
        //qDebug() << "Checking loops for " << QString::number(phaseFrom->mId);
        QList<Phase*> phasesFrom = getAllPhasesFrom(phaseFrom);
        for(int i=0; i<(int)phasesFrom.size(); ++i)
        {
            if(phasesFrom[i] == phaseTo)
                return false;
        }
        
        // ------------------------------------------------------------
        //  TODO : Vérifier que la contrainte n'est pas redondante
        // ------------------------------------------------------------
        //qDebug() << "Checking duplicates for " << QString::number(phaseFrom->mId);
        QList<Phase*> phasesTo = getAllPhasesTo(phaseFrom);
        for(int i=0; i<(int)phasesFrom.size(); ++i)
        {
            QList<Phase*> subPhasesTo = getAllPhasesTo(phasesFrom[i]);
            for(int j=0; j<(int)subPhasesTo.size(); ++j)
            {
                if(std::find(phasesTo.begin(), phasesTo.end(), subPhasesTo[j]) == phasesTo.end())
                {
                    phasesTo.push_back(subPhasesTo[j]);
                }
            }
        }
        for(int i=0; i<(int)phasesTo.size(); ++i)
        {
            //qDebug() << phasesTo[i]->mId << " compared to " << phaseTo->mId;
            if(phasesTo[i] == phaseTo)
                return false;
        }
        
        return true;
    }
    return false;
    return true;
}

QList<Phase*> Project::getAllPhasesFrom(Phase* phase)
{
    QList<Phase*> phasesFrom;
    for(int i=0; i<(int)phase->mConstraintsBwd.size(); ++i)
    {
        Phase* phaseFrom = phase->mConstraintsBwd[i]->getPhaseFrom();
        //qDebug() << "- from : " << phaseFrom->mId;
        phasesFrom.push_back(phaseFrom);
        
        QList<Phase*> subPhasesFrom = getAllPhasesFrom(phaseFrom);
        for(int i=0; i<(int)subPhasesFrom.size(); ++i)
            phasesFrom.push_back(subPhasesFrom[i]);
    }
    return phasesFrom;
}

QList<Phase*> Project::getAllPhasesTo(Phase* phase)
{
    QList<Phase*> phasesTo;
    for(int i=0; i<(int)phase->mConstraintsFwd.size(); ++i)
    {
        Phase* phaseTo = phase->mConstraintsFwd[i]->getPhaseTo();
        //qDebug() << "- to : " << phaseTo->mId;
        phasesTo.push_back(phaseTo);
        
        QList<Phase*> subPhasesTo = getAllPhasesTo(phaseTo);
        for(int i=0; i<(int)subPhasesTo.size(); ++i)
            phasesTo.push_back(subPhasesTo[i]);
    }
    return phasesTo;
}*/

#pragma mark Events constraints
bool Project::isEventConstraintAllowed(const QJsonObject& eventFrom, const QJsonObject& eventTo)
{
    // TODO
    /*qDebug() << "Project::isEventConstraintAllowed: " << eventFrom.value("name")<< eventFrom.value("id");
    qDebug() << "Project::isEventConstraintAllowed: " << eventTo.value("name")<< eventTo.value("id");
    if(!eventFrom.isEmpty() && !eventTo.isEmpty())
    {
        return true;
        qDebug() << "Project::isEventConstraintAllowed: " << eventFrom.keys();
        qDebug() << "Project::isEventConstraintAllowed: " << eventTo.keys();
    }
    return false;*/
    //-----------------
    //QJsonObject stateNext = mState;
    
    //QJsonArray events = mState[STATE_EVENTS].toArray();
    QJsonArray constraints = mState[STATE_EVENTS_CONSTRAINTS].toArray();
   
    int eventFromId = eventFrom[STATE_ID].toInt();
    int eventToId = eventTo[STATE_ID].toInt();
    bool ConstraintAllowed = true;
    for(int i=0; i<constraints.size(); ++i)
    {
        QJsonObject constraint = constraints[i].toObject();
        if(constraint[STATE_CONSTRAINT_BWD_ID] == eventFromId && constraint[STATE_CONSTRAINT_FWD_ID] == eventToId)
        {
            ConstraintAllowed = false;
            qDebug() << "Project::isEventConstraintAllowed: not Allowed " ;
        }
    }
    return ConstraintAllowed;
    
}

void Project::createEventConstraint(int eventFromId, int eventToId)
{
    QJsonObject stateNext = mState;
    
    QJsonArray events = mState[STATE_EVENTS].toArray();
    QJsonArray constraints = mState[STATE_EVENTS_CONSTRAINTS].toArray();
    
    QJsonObject eventFrom;
    QJsonObject eventTo;
    
    for(int i=0; i<events.size(); ++i)
    {
        QJsonObject event = events[i].toObject();
        if(event[STATE_ID].toInt() == eventFromId)
            eventFrom = event;
        else if(event[STATE_ID].toInt() == eventToId)
            eventTo = event;
    }
    
    if(isEventConstraintAllowed(eventFrom, eventTo))
    {
        EventConstraint c;
        c.mId = getUnusedEventConstraintId(constraints);
        c.mFromId = eventFrom[STATE_ID].toInt();
        c.mToId = eventTo[STATE_ID].toInt();
        QJsonObject constraint = c.toJson();
        constraints.append(constraint);
        stateNext[STATE_EVENTS_CONSTRAINTS] = constraints;
        
        pushProjectState(stateNext, tr("Event constraint created"), true);
    }
}

void Project::deleteEventConstraint(int constraintId)
{
    QJsonObject state = mState;
    QJsonArray constraints = mState[STATE_EVENTS_CONSTRAINTS].toArray();
    
    for(int i=0; i<constraints.size(); ++i)
    {
        QJsonObject c = constraints[i].toObject();
        if(c[STATE_ID].toInt() == constraintId)
        {
            constraints.removeAt(i);
            break;
        }
    }
    state[STATE_EVENTS_CONSTRAINTS] = constraints;
    pushProjectState(state, tr("Event constraint deleted"), true);
}

void Project::updateEventConstraint(int constraintId)
{
    QJsonObject stateNext = mState;
    QJsonArray constraints = mState[STATE_EVENTS_CONSTRAINTS].toArray();
    QJsonObject constraint;
    int index = -1;
    for(int i=0; i<constraints.size(); ++i)
    {
        QJsonObject c = constraints[i].toObject();
        if(c[STATE_ID].toInt() == constraintId)
        {
            constraint = c;
            index = i;
        }
    }
    if(index != -1)
    {
        ConstraintDialog dialog(qApp->activeWindow(), ConstraintDialog::eEvent, Qt::Sheet);
        dialog.setConstraint(constraint);
        if(dialog.exec() == QDialog::Accepted)
        {
            if(dialog.deleteRequested())
            {
                constraints.removeAt(index);
            }
            else
            {
                constraint = dialog.constraint();
                constraints[index] = constraint;
            }
            stateNext[STATE_EVENTS_CONSTRAINTS] = constraints;
            pushProjectState(stateNext, tr("Event constraint updated"), true);
        }
    }
}

int Project::getUnusedEventConstraintId(const QJsonArray& constraints)
{
    int id = -1;
    bool idIsFree = false;
    while(!idIsFree)
    {
        ++id;
        idIsFree = true;
        for(int i=0; i<constraints.size(); ++i)
        {
            QJsonObject constraint = constraints[i].toObject();
            if(constraint[STATE_ID].toInt() == id)
                idIsFree = false;
        }
    }
    return id;
}

#pragma mark Phases constraints
bool Project::isPhaseConstraintAllowed(const QJsonObject& phaseFrom, const QJsonObject& phaseTo)
{
    // TODO
    if(!phaseFrom.isEmpty() && !phaseTo.isEmpty())
    {
        return true;
    }
    return false;
}

void Project::createPhaseConstraint(int phaseFromId, int phaseToId)
{
    QJsonObject stateNext = mState;
    
    QJsonArray phases = mState[STATE_PHASES].toArray();
    QJsonArray constraints = mState[STATE_PHASES_CONSTRAINTS].toArray();
    
    QJsonObject phaseFrom;
    QJsonObject phaseTo;
    
    for(int i=0; i<phases.size(); ++i)
    {
        QJsonObject phase = phases[i].toObject();
        if(phase[STATE_ID].toInt() == phaseFromId)
            phaseFrom = phase;
        else if(phase[STATE_ID].toInt() == phaseToId)
            phaseTo = phase;
    }
    
    if(isPhaseConstraintAllowed(phaseFrom, phaseTo))
    {
        PhaseConstraint c;
        c.mId = getUnusedPhaseConstraintId(constraints);
        c.mFromId = phaseFrom[STATE_ID].toInt();
        c.mToId = phaseTo[STATE_ID].toInt();
        QJsonObject constraint = c.toJson();
        constraints.append(constraint);
        stateNext[STATE_PHASES_CONSTRAINTS] = constraints;
        
        pushProjectState(stateNext, tr("Phase constraint created"), true);
    }
}

void Project::deletePhaseConstraint(int constraintId)
{
    QJsonObject state = mState;
    QJsonArray constraints = mState[STATE_PHASES_CONSTRAINTS].toArray();
    
    for(int i=0; i<constraints.size(); ++i)
    {
        QJsonObject c = constraints[i].toObject();
        if(c[STATE_ID].toInt() == constraintId)
        {
            constraints.removeAt(i);
            break;
        }
    }
    state[STATE_PHASES_CONSTRAINTS] = constraints;
    pushProjectState(state, tr("Phase constraint deleted"), true);
}

void Project::updatePhaseConstraint(int constraintId)
{
    QJsonObject stateNext = mState;
    QJsonArray constraints = mState[STATE_PHASES_CONSTRAINTS].toArray();
    QJsonObject constraint;
    int index = -1;
    for(int i=0; i<constraints.size(); ++i)
    {
        QJsonObject c = constraints[i].toObject();
        if(c[STATE_ID].toInt() == constraintId)
        {
            constraint = c;
            index = i;
        }
    }
    if(index != -1)
    {
        ConstraintDialog dialog(qApp->activeWindow(), ConstraintDialog::ePhase, Qt::Sheet);
        dialog.setConstraint(constraint);
        if(dialog.exec() == QDialog::Accepted)
        {
            if(dialog.deleteRequested())
            {
                constraints.removeAt(index);
            }
            else
            {
                constraint = dialog.constraint();
                qDebug() << constraint;
                if(constraint[STATE_CONSTRAINT_GAMMA_TYPE].toInt() == PhaseConstraint::eGammaFixed &&
                   constraint[STATE_CONSTRAINT_GAMMA_FIXED].toDouble() == 0.)
                {
                    QMessageBox message(QMessageBox::Critical,
                                        tr("Invalid value"),
                                        tr("The fixed value must be positive!"),
                                        QMessageBox::Ok,
                                        qApp->activeWindow(),
                                        Qt::Sheet);
                    message.exec();
                    return;
                }
                else if(constraint[STATE_CONSTRAINT_GAMMA_TYPE].toInt() == PhaseConstraint::eGammaRange &&
                        constraint[STATE_CONSTRAINT_GAMMA_MIN].toDouble() >= constraint[STATE_CONSTRAINT_GAMMA_MAX].toDouble())
                {
                    QMessageBox message(QMessageBox::Critical,
                                        tr("Invalid values"),
                                        tr("Min must be lower than max!"),
                                        QMessageBox::Ok,
                                        qApp->activeWindow(),
                                        Qt::Sheet);
                    message.exec();
                    return;
                }
                
                constraints[index] = constraint;
            }
            stateNext[STATE_PHASES_CONSTRAINTS] = constraints;
            pushProjectState(stateNext, tr("Phase constraint updated"), true);
        }
    }
}

int Project::getUnusedPhaseConstraintId(const QJsonArray& constraints)
{
    int id = -1;
    bool idIsFree = false;
    while(!idIsFree)
    {
        ++id;
        idIsFree = true;
        for(int i=0; i<constraints.size(); ++i)
        {
            QJsonObject constraint = constraints[i].toObject();
            if(constraint[STATE_ID].toInt() == id)
                idIsFree = false;
        }
    }
    return id;
}


// -------
#pragma mark export
void Project::exportAsText()
{
    /*QString currentDir = MainWindow::getInstance()->getCurrentPath();
    
    QString path = QFileDialog::getExistingDirectory(qApp->activeWindow(), tr("Location"), currentDir);
    if(!path.isEmpty())
    {
        MainWindow::getInstance()->setCurrentPath(path);
        
        QString folderBaseName = mName.simplified().toLower().replace(" ", "_");
        QString folderName = folderBaseName;
        int index = 1;
        while(QFileInfo::exists(path + "/" + folderName))
        {
            folderName = folderBaseName + " (" + QString::number(index) + ")";
            ++index;
        }
        QDir dir(path);
        if(dir.mkdir(folderName))
        {
            dir = QDir(path + "/" + folderName);
            
            QString descFileName = "description.txt";
            QFile descFile(dir.absoluteFilePath(descFileName));
            if(descFile.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                QTextStream stream(&descFile);
                
                stream << "Project name : " << mName << endl;
                
                stream << endl << "****************************************" << endl;
                stream << "EVENTS" << endl;
                stream << "****************************************" << endl;
                
                for(int i=0; i<mEvents.size(); ++i)
                {
                    Event* event = mEvents[i];
                    stream << endl << "++++++++++++++++++++++++++++++++++++++++" << endl;
                    stream << "EVENT : " << endl;
                    stream << "- Name : " << event->mName << endl;
                    stream << "- Method : " << ModelUtilities::getEventMethodText(event->mMethod) << endl;
                    stream << "- Num data : " << QString::number(event->mDates.size()) << endl;
                    stream << "- Num phases : " << QString::number(event->mPhases.size()) << endl;
                    stream << "++++++++++++++++++++++++++++++++++++++++" << endl;
                    
                    for(int j=0; j<event->mDates.size(); ++j)
                    {
                        Date* date = (Date*) event->mDates[j];
                        stream << "DATA : " << endl;
                        stream << "- Name : " << date->mName << endl;
                        stream << "- Type : " << date->mPlugin->getName() << endl;
                        stream << "---------------" << endl;
                    }
                }
                
                stream << endl << "****************************************" << endl;
                stream << "PHASES" << endl;
                stream << "****************************************" << endl;
                
                for(int i=0; i<mPhases.size(); ++i)
                {
                    Phase* phase = mPhases[i];
                    stream << endl << "++++++++++++++++++++++++++++++++++++++++" << endl;
                    stream << "PHASE : " << endl;
                    stream << "- Name : " << phase->mName << endl;
                    stream << "- Num events : " << QString::number(phase->mEvents.size()) << endl;
                    stream << "++++++++++++++++++++++++++++++++++++++++" << endl;
                }
            }
        }
    }*/
    
    /*QMessageBox message(QMessageBox::Critical, tr("TODO"), tr("create readable textual representation of XML project file"), QMessageBox::Ok, qApp->activeWindow(), Qt::Sheet);
    message.exec();*/
}

// --------------------------------------------------------------------
//     Project Run
// --------------------------------------------------------------------
#pragma mark Run Project
void Project::run()
{
    // This is the occasion to clean EVERYTHING using the previous model before deleting it!
    // e.g. : clean the result view with any graphs, ...
    emit mcmcStarted();
    
    clearModel();
    
    //mModel = Model::fromJson(mState);
    mModel->setJson(mState);
    mModel->fromJson(mState);
    bool modelOk = false;
    try
    {
        modelOk = mModel->isValid();
    }
    catch(QString error)
    {
        QMessageBox message(QMessageBox::Warning,
                            tr("Your model is not valid"),
                            error,
                            QMessageBox::Ok,
                            qApp->activeWindow(),
                            Qt::Sheet);
        message.exec();
    }
    if(modelOk)
    {
        MCMCLoopMain loop(mModel);
        MCMCProgressDialog dialog(&loop, qApp->activeWindow(), Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::Sheet);
        if(dialog.startMCMC() == QDialog::Accepted)
        {
            if(loop.mAbortedReason.isEmpty())
            {
                //Memo of the init varaible state to show in Log view
                mModel->mLogMCMC = loop.getChainsLog() + loop.getInitLog();
                emit mcmcFinished(mModel);
            }
            else
            {
                if(loop.mAbortedReason != ABORTED_BY_USER)
                {
                    QMessageBox message(QMessageBox::Warning,
                                        tr("Error"),
                                        loop.mAbortedReason,
                                        QMessageBox::Ok,
                                        qApp->activeWindow(),
                                        Qt::Sheet);
                    message.exec();
                }
                clearModel();
            }
        }
        else
        {
            clearModel();
        }
    }
}


void Project::clearModel()
{
     mModel->clear();
     emit noResult();
}
