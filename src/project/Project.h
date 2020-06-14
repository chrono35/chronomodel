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

#ifndef PROJECT_H
#define PROJECT_H

#include "AppSettings.h"
#include "ProjectSettings.h"
#include "MCMCSettings.h"
#include "MCMCLoopMain.h"
#include "Model.h"
#include "StateKeys.h"
#include "CalibrationCurve.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QJsonObject>


#define PROJECT_LOADED_REASON "Project Loaded"
#define NEW_PROJECT_REASON "New Project"
#define INSERT_PROJECT_REASON "Insert Project"
#define CLOSE_PROJECT_REASON "Close Project"
#define PROJECT_SETTINGS_UPDATED_REASON "Settings updated"
#define MCMC_SETTINGS_UPDATED_REASON "MCMC Settings updated"
#define MCMC_SETTINGS_RESTORE_DEFAULT_REASON "MCMC Settings restore default"
#define MCMC_METHODE_RESET_REASON "MCMC methods reset"
#define CHRONOCURVE_SETTINGS_UPDATED_REASON "Chronocurve Settings updated"
#define DATE_MOVE_TO_EVENT_REASON "Date moved to event"
#define NEW_EVEN_BY_CSV_DRAG_REASON "New Event by CSV drag"


class Date;
class Event;
class EventKnown;
class Phase;
class EventConstraint;
class PhaseConstraint;
class PluginAbstract;


class Project: public QObject
{
    Q_OBJECT
public:
    Project();
    virtual ~Project();

    enum ActionOnModel {
        InsertEventsToPhase,
        ExtractEventsFromPhase,
    };

    void initState(const QString& reason);

    // This is the function to call when the project state is to be modified.
    // An undo command is created and a StateEvent is fired asynchronously.
    // The project state will be modified and the view notified (if required)
    bool pushProjectState(const QJsonObject& state, const QString& reason, bool notify, bool force = false);
    void checkStateModification(const QJsonObject& stateNew,const QJsonObject& stateOld);
    bool structureIsChanged();
    bool designIsChanged();
    // Sends a StateEvent asynchronously.
    // Called by "SetProjectState" undo/redo commands.
    void sendUpdateState(const QJsonObject& state, const QString& reason, bool notify);

    // Event handler for events of type "StateEvent".
    // Updates the project state by calling updateState() and send a notification (if required).
    bool event(QEvent* e);

    // Update the project state directly.
    // This is not async! so be careful when calling this from views with notify = true
    void updateState(const QJsonObject& state, const QString& reason, bool notify);

    // Special events for selection... too bad!
    void sendEventsSelectionChanged();

    static QJsonObject emptyState();
    QJsonObject state() const;
    QJsonObject* state_ptr();


    bool load(const QString& path);
    bool saveAs(const QString& dialogTitle);
    bool askToSave(const QString& saveDialogTitle);
    bool saveProjectToFile();

    bool recenterProject();
    bool insert(const QString& path);

    /**
     * @brief setNoResults : set to disable the saving the file *.res
     * @param noResults
     */
    void setNoResults( const bool& noResults) { mNoResults = noResults;}
    bool withResults() {return !mNoResults;}

    bool setSettings(const ProjectSettings& settings);

    bool studyPeriodIsValid();
    void showStudyPeriodWarning();

    QJsonArray getInvalidDates();

    // ---------------------------
    void restoreMCMCSettings();

    void addEvent(QJsonObject event, const QString& reason);
    int getUnusedEventId(const QJsonArray& events);
    void updateEvent(const QJsonObject& event, const QString& reason);
    void mergeEvents(int eventFromId, int eventToId);
    void deleteSelectedTrashedEvents(const QList<int>& ids);

    Date createDateFromPlugin(PluginAbstract* plugin);
    Date createDateFromData(const QString& pluginName, const QStringList& dataStr);
    void addDate(int eventId, QJsonObject date);
    int getUnusedDateId(const QJsonArray& dates) const;
    void updateDate(int eventId, int dateId);
    void deleteDates(int eventId, const QList<int>& dateIndexes);
    void recycleDates(int eventId);
    void deleteSelectedTrashedDates(const QList<int>& ids);
    QJsonObject checkDatesCompatibility(QJsonObject state, bool &isCorrected);
    QJsonObject checkValidDates(const QJsonObject& state);

    void unselectedAllInState();
    void updateSelectedEventsColor(const QColor& color);
    void updateSelectedEventsMethod(Event::Method);
    void updateSelectedEventsDataMethod(Date::DataMethod method, const QString& pluginId);
    void updateAllDataInSelectedEvents(const QHash<QString, QVariant>& groupedAction);
    void selectedEventsFromSelectedPhases();

    void updatePhase(const QJsonObject& phaseIn);
    int getUnusedPhaseId(const QJsonArray& phases);
    void mergePhases(int phaseFromId, int phaseToId);
    void updatePhaseEvents(const int phaseId, ActionOnModel action);
    QJsonObject getPhasesWithId(const int id);

    void createEventConstraint(const int idFrom, const int idTo);
    void deleteEventConstraint(int constraintId);
    bool isEventConstraintAllowed(const QJsonObject& eventFrom, const QJsonObject& eventTo);
    void updateEventConstraint(int constraintId);
    int getUnusedEventConstraintId(const QJsonArray& constraints);

    void createPhaseConstraint(int phaseFromId, int phaseToId);
    void deletePhaseConstraint(int constraintId);
    bool isPhaseConstraintAllowed(const QJsonObject& phaseFrom, const QJsonObject& phaseTo);
    void updatePhaseConstraint(const int constraintId);
    int getUnusedPhaseConstraintId(const QJsonArray& constraints);

    void clearModel();
    void createEvent(qreal x, qreal y);
    void createEventKnown(qreal x, qreal y);
    void createPhase(qreal x, qreal y);

public slots:
    bool save();

    void mcmcSettings();
    void resetMCMC();
    void run();
    void exportAsText();
    void runChronocurve();

    void setAppSettings();

    void deleteSelectedEvents();
    void recycleEvents();


    void deleteSelectedPhases();

    void combineDates(const int eventId, const QList<int>& dateIds);
    void splitDate(const int eventId, const int dateId);

signals:
    void noResult();
    void projectStateChanged();
    void currentEventChanged(const QJsonObject& event);

    void eyedPhasesModified(const QMap<int, bool>& eyedPhases);

    void mcmcStarted();
    void mcmcFinished(Model* model);

    void projectStructureChanged(bool structureChanged);
    void projectDesignChanged(bool designIsChanged);
    void projectItemsIsMoved(bool itemsIsMoved);



public:
    QJsonObject mState;
    QJsonObject mLastSavedState;

    QString mName;
   // QString mProjectFileDir;
    //QString mProjectFileName;

    Model* mModel;

    QMap<QString, CalibrationCurve> mCalibCurves;
    QTimer* mAutoSaveTimer;

private :
    // used to define scene modification
    bool mDesignIsChanged;
    bool mStructureIsChanged;
    bool mItemsIsMoved;
    QSet<QString> mReasonChangeStructure;
    QSet<QString> mReasonChangeDesign;
    QSet<QString> mReasonChangePosition;

    bool mNoResults;
};

#endif
