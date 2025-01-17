#ifndef ModelUtilities_H
#define ModelUtilities_H

#include <QIcon>
#include <QString>
#include <QVector>
#include "Date.h"
#include "Event.h"
#include "Phase.h"


class ModelUtilities
{
public:
    static QString getEventMethodText(Event::Method method);
    static QString getDataMethodText(Date::DataMethod method);
    static QString getDeltaText(const Date& date);
    
    static Event::Method getEventMethodFromText(const QString& text);
    static Date::DataMethod getDataMethodFromText(const QString& text);
    
    static QVector<QVector<Event*> > getNextBranches(const QVector<Event*>& curBranch, Event* lastNode);
    static QVector<QVector<Event*> > getBranchesFromEvent(Event* start);
    static QVector<QVector<Event*> > getAllEventsBranches(const QList<Event*>& events);
    
    static QVector<QVector<Phase*> > getNextBranches(const QVector<Phase*>& curBranch, Phase* lastNode, const double gammaSum, const double maxLength);
    static QVector<QVector<Phase*> > getBranchesFromPhase(Phase* start, const double maxLength);
    static QVector<QVector<Phase*> > getAllPhasesBranches(const QList<Phase*>& events, const double maxLength);
    
    static QVector<Event*> sortEventsByLevel(const QList<Event*>& events);
    static QVector<Phase*> sortPhasesByLevel(const QList<Phase*>& phases);
    
    static QVector<Event*> unsortEvents(const QList<Event*>& events);
    
    static QString dateResultsText(Date* d);
    static QString eventResultsText(Event* e, bool withDates);
    static QString phaseResultsText(Phase* p);
    
    static QString dateResultsHTML(Date* d);
    static QString eventResultsHTML(Event* e, bool withDates);
    static QString phaseResultsHTML(Phase* p);
};

// These 2 global functions are used to sort events and phases lists in result view

bool sortEvents(Event* e1, Event* e2);
bool sortPhases(Phase* p1, Phase* p2);

#endif