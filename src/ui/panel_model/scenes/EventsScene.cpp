#include "EventsScene.h"
#include "Event.h"
#include "EventKnown.h"
#include "EventConstraint.h"
#include "EventItem.h"
#include "EventKnownItem.h"
#include "DateItem.h"
#include "ArrowItem.h"
#include "ArrowTmpItem.h"
#include "SetProjectState.h"
#include "MainWindow.h"
#include "Project.h"
#include "HelpWidget.h"
#include "QtUtilities.h"
#include <QtWidgets>
#include <QProgressDialog>


EventsScene::EventsScene(QGraphicsView* view, QObject* parent):AbstractScene(view, parent)
{
    mHelpView = new HelpWidget(view);
    mHelpView->setLink("http://www.chronomodel.fr/Chronomodel_User_Manual.pdf#page=9");
    mHelpTimer = new QTimer(this);
    
    mDatesAnimTimer = new QTimeLine(100);
    mDatesAnimTimer->setFrameRange(0, 2);
    
    mDatesAnim = new QGraphicsItemAnimation();
    mDatesAnim->setTimeLine(mDatesAnimTimer);
    
    connect(this, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
    connect(mHelpTimer, SIGNAL(timeout()), this, SLOT(updateHelp()));
    
    mHelpTimer->start(200);
}

EventsScene::~EventsScene()
{
    
}

#pragma mark Actions
void EventsScene::deleteSelectedItems()
{
    MainWindow::getInstance()->getProject()->deleteSelectedEvents();
}

bool EventsScene::constraintAllowed(AbstractItem* itemFrom, AbstractItem* itemTo)
{
    QJsonArray constraints = MainWindow::getInstance()->getProject()->mState.value(STATE_EVENTS_CONSTRAINTS).toArray();
    
    QJsonObject eventFrom = ((EventItem*)itemFrom)->getEvent();
    QJsonObject eventTo = ((EventItem*)itemTo)->getEvent();
    
    int eventFromId = eventFrom.value(STATE_ID).toInt();
    int eventToId = eventTo.value(STATE_ID).toInt();
    
    bool constraintAllowed = true;
    
    for(int i=0; i<constraints.size(); ++i)
    {
        QJsonObject constraint = constraints.at(i).toObject();
        // If the constraint already exist, impossible to create another identical one.
        if(constraint.value(STATE_CONSTRAINT_BWD_ID).toInt() == eventFromId && constraint.value(STATE_CONSTRAINT_FWD_ID).toInt() == eventToId)
        {
            constraintAllowed = false;
            qDebug() << "EventsScene::constraintAllowed: not Allowed " ;
        }
        // Impossible to have 2 constraints with oposite directions, between 2 events.
        else if(constraint.value(STATE_CONSTRAINT_BWD_ID).toInt() == eventToId && constraint.value(STATE_CONSTRAINT_FWD_ID).toInt() == eventFromId)
        {
            constraintAllowed = false;
            qDebug() << "EventsScene::constraintAllowed: not Allowed Inversion" ;
        }
        
    }
    if(constraintAllowed && constraintIsCircular(constraints, eventFromId, eventToId))
    {
        constraintAllowed = false;
        qDebug() << "EventsScene::constraintAllowed: not Allowed Circular" ;
    }
    return constraintAllowed;
}

void EventsScene::createConstraint(AbstractItem* itemFrom, AbstractItem* itemTo)
{
    QJsonObject eventFrom = ((EventItem*)itemFrom)->getEvent();
    QJsonObject eventTo = ((EventItem*)itemTo)->getEvent();
    
    MainWindow::getInstance()->getProject()->createEventConstraint(eventFrom[STATE_ID].toInt(),
                                                        eventTo[STATE_ID].toInt());
}

void EventsScene::mergeItems(AbstractItem* itemFrom, AbstractItem* itemTo)
{
    QJsonObject eventFrom = ((EventItem*)itemFrom)->getEvent();
    QJsonObject eventTo = ((EventItem*)itemTo)->getEvent();
    
    MainWindow::getInstance()->getProject()->mergeEvents(eventFrom[STATE_ID].toInt(),
                                              eventTo[STATE_ID].toInt());
}

void EventsScene::updateGreyedOutEvents(const QMap<int, bool>& eyedPhases)
{
    //qDebug() << "-> Update greyed out events";
    
    // If no phases is eyed, then no event must be greyed out!
    bool noEyedPhases = true;
    QMapIterator<int, bool> iter(eyedPhases);
    while(iter.hasNext())
    {
        iter.next();
        if(iter.value())
        {
            noEyedPhases = false;
            break;
        }
    }
    
    // ----------------------------------------------------
    //  Grey out events
    // ----------------------------------------------------
    for(int i=0; i<mItems.size(); ++i)
    {
        EventItem* item = (EventItem*)mItems[i];
        if(noEyedPhases)
        {
            item->setGreyedOut(false);
        }
        else
        {
            QString eventPhasesIdsStr = item->mData.value(STATE_EVENT_PHASE_IDS).toString();
            bool mustBeGreyedOut = true;
            if(!eventPhasesIdsStr.isEmpty())
            {
                QStringList eventPhasesIds = eventPhasesIdsStr.split(",");
                for(int j=0; j<eventPhasesIds.size(); ++j)
                {
                    if(eyedPhases[eventPhasesIds.at(j).toInt()])
                    {
                        mustBeGreyedOut = false;
                    }
                }
            }
            item->setGreyedOut(mustBeGreyedOut);
        }
    }
    
    // ----------------------------------------------------
    //  Grey out constraints
    // ----------------------------------------------------
    for(int i=0; i<mConstraintItems.size(); ++i)
    {
        ArrowItem* item = (ArrowItem*)mConstraintItems[i];
        if(noEyedPhases)
        {
            item->setGreyedOut(false);
        }
        else
        {
            int eventFromId = item->mData.value(STATE_CONSTRAINT_BWD_ID).toInt();
            int eventToId = item->mData.value(STATE_CONSTRAINT_FWD_ID).toInt();
            
            bool eventFromIsOk = false;
            bool eventToIsOk = false;
            
            const QJsonObject state = MainWindow::getInstance()->getProject()->state();
            QJsonArray events = state.value(STATE_EVENTS).toArray();
            
            for(int i=0; i<events.size(); ++i)
            {
                QJsonObject event = events.at(i).toObject();
                
                if(event.value(STATE_ID).toInt() == eventFromId)
                {
                    const QString eventFromPhasesIdsStr = event.value(STATE_EVENT_PHASE_IDS).toString();
                    QList<int> eventFromPhasesIds = stringListToIntList(eventFromPhasesIdsStr);
                    for(int j=0; j<eventFromPhasesIds.size(); ++j)
                    {
                        if(eyedPhases[eventFromPhasesIds.at(j)])
                            eventFromIsOk = true;
                    }
                }
                else if(event.value(STATE_ID).toInt() == eventToId)
                {
                    const QString eventToPhasesIdsStr = event.value(STATE_EVENT_PHASE_IDS).toString();
                    QList<int> eventToPhasesIds = stringListToIntList(eventToPhasesIdsStr);
                    for(int j=0; j<eventToPhasesIds.size(); ++j)
                    {
                        if(eyedPhases[eventToPhasesIds.at(j)])
                            eventToIsOk = true;
                    }
                }
            }
            item->setGreyedOut(!eventFromIsOk || !eventToIsOk);
        }
    }
}

#pragma mark Help Bubble
void EventsScene::updateHelp()
{
    QString text;
    QList<QGraphicsItem*> selected = selectedItems();
    
    if(mItems.size() == 0)
    {
        text = tr("Define a study period on the right panel, apply it, and start creating your model by clicking on \"New Event...\".");
            mHelpView->setLink("http://www.chronomodel.fr/Chronomodel_User_Manual.pdf#page=24"); // chapter
    }
    else if(selected.count() == 0)
    {
        text = tr("Select an event or a bound by clicking on it.");
        if(mConstraintItems.size() != 0)
            text += tr("\nYou can also edit constraints by double clicking on the arrow");
            mHelpView->setLink("http://www.chronomodel.fr/Chronomodel_User_Manual.pdf#page=24"); // Chapter
    }
    else if(selected.count() == 1)
    {
        bool isBound = (dynamic_cast<EventKnownItem*>(selected[0]) != 0);
        
        if(mAltIsDown)
        {
            text = tr("Mouve your mouse and click on another element to create a constraint.");
                mHelpView->setLink("http://www.chronomodel.fr/Chronomodel_User_Manual.pdf#page=24");
        }
        else if(mShiftIsDown && !isBound)
        {
            text = tr("Drag the event onto another one to merge them together.");
                mHelpView->setLink("http://www.chronomodel.fr/Chronomodel_User_Manual.pdf#page=24");
        }
        else
        {
            text = tr("You have selected an element. You can now:\n- Edit its properties from the right panel.\n- Create a constraint by holding the \"Alt\" key down and clicking on another element.");
                mHelpView->setLink("http://www.chronomodel.fr/Chronomodel_User_Manual.pdf#page=24");
            if(!isBound)
                text += tr("\n- Merge it with another element by holding the \"Shift\" key down and dragging the selected element onto another one.\n- Delete it with the button on the left.");
        }
    }
    else
    {
        text = tr("You have selected multiple elements. You can move them together or delete them (all constraints linked to them will also be deleted).");
    }
    
    mHelpView->setText(text);
    mHelpView->setGeometry(mHelpView->x(),
                           mHelpView->y(),
                           mHelpView->width(),
                           mHelpView->heightForWidth(mHelpView->width()));
}

HelpWidget* EventsScene::getHelpView()
{
    return mHelpView;
}

void EventsScene::showHelp(bool show)
{
    mHelpView->setVisible(show);
}

#pragma mark Project Update
void EventsScene::sendUpdateProject(const QString& reason, bool notify, bool storeUndoCommand)
{
    Project* project = MainWindow::getInstance()->getProject();
    
    QJsonObject statePrev = project->state();
    QJsonObject stateNext = statePrev;
    
    QJsonArray events;
    for(int i=0; i<mItems.size(); ++i)
    {
        EventItem* item = (EventItem*)mItems[i];
        events.append(item->getEvent());
    }
    stateNext[STATE_EVENTS] = events;
    
    if(statePrev != stateNext)
    {
        if(storeUndoCommand)
            MainWindow::getInstance()->getProject()->pushProjectState(stateNext, reason, notify);
        else
            MainWindow::getInstance()->getProject()->sendUpdateState(stateNext, reason, notify);
    }
}

void EventsScene::updateProject()
{

#ifdef DEBUG
   qDebug()<<"EventsScene::updateProject() begin";
   QTime startTime = QTime::currentTime();
#endif
    QJsonObject state = MainWindow::getInstance()->getProject()->state();
    QJsonArray eventsInNewState = state.value(STATE_EVENTS).toArray();
    QJsonArray constraints = state.value(STATE_EVENTS_CONSTRAINTS).toArray();
    QJsonObject settings = state.value(STATE_SETTINGS).toObject();

    QProgressDialog* progress = 0;
    bool displayProgress = false;
    if(mItems.size() != eventsInNewState.size()) {
        displayProgress = true;
        progress = new QProgressDialog("Create / Update event items","Wait" , 1, eventsInNewState.size(),qApp->activeWindow());
        progress->setWindowModality(Qt::WindowModal);
        progress->setCancelButton(0);
    }

    QList<int> events_ids_inNewState;
    for(int i=0; i<eventsInNewState.size(); ++i)
        events_ids_inNewState << eventsInNewState.at(i).toObject().value(STATE_ID).toInt();
    
    QList<int> constraints_ids;
    for(int i=0; i<constraints.size(); ++i)
        constraints_ids << constraints.at(i).toObject().value(STATE_ID).toInt();
    
    // ------------------------------------------------------
    //  Update settings
    // ------------------------------------------------------
    bool settingsChanged = false;
    ProjectSettings s = ProjectSettings::fromJson(settings);
    if(mSettings != s)
    {
        settingsChanged = true;
        mSettings = s;

    }
    
    mUpdatingItems = true;

    //progress->show();
    // ------------------------------------------------------
    //  Delete items not in current state
    // ------------------------------------------------------
    bool hasDeleted = false;
    for(int i=mItems.size()-1; i>=0; --i)
    {
        EventItem* eventItem = (EventItem*)mItems[i];
        QJsonObject& event = eventItem->getEvent();
        
        if(!events_ids_inNewState.contains(event.value(STATE_ID).toInt()))
        {
            if(event.value(STATE_EVENT_TYPE).toInt() == Event::eDefault)
            {
                QList<QGraphicsItem*> dateItems = eventItem->childItems();
                for(int j=0; j<dateItems.size(); ++j)
                {
                    removeItem(dateItems.at(j));
                    delete dateItems[j];
                }
            }
#ifdef DEBUG
            qDebug() << "Event deleted : " << event[STATE_ID].toInt();
#endif
            mItems.removeAt(i);
            hasDeleted = true;
            
            // This does not break the code but is commented to match PhasesScene implementation
            //removeItem(eventItem);
            
            eventItem->deleteLater();
        }
    }
    
    bool hasCreated = false;
   // ------------------------------------------------------
    //  Create / Update event items
    // ------------------------------------------------------
    int i = 0;
    if(displayProgress) progress->setLabelText(tr("Create / Update event items"));
    for(QJsonArray::const_iterator citer = eventsInNewState.constBegin(); citer != eventsInNewState.constEnd(); ++citer)
    {
        const QJsonObject event = (*citer).toObject();
        ++i;
        if(displayProgress) progress->setValue(i);

        bool itemUnkown = true;
        for(QList<AbstractItem*>::const_iterator cIterOld = mItems.cbegin(); cIterOld != mItems.cend() && itemUnkown; ++cIterOld) {
            EventItem* oldItem = (EventItem*)(*cIterOld);

            const QJsonObject OldItemEvent = oldItem->getEvent();

            if( OldItemEvent.value(STATE_ID).toInt() == event.value(STATE_ID).toInt()) {

                itemUnkown = false;
                if(event != OldItemEvent || settingsChanged) {
                    // UPDATE ITEM
#ifdef DEBUG
                    qDebug() << "Event updated : id = " << event.value(STATE_ID).toInt()<< event.value(STATE_NAME).toString();
#endif
                    oldItem->setEvent(event, settings);
                }
             }

            oldItem = 0;
        }
        if(itemUnkown) {
            // CREATE ITEM
                Event::Type type = (Event::Type)event.value(STATE_EVENT_TYPE).toInt();
                EventItem* newItem = 0;
                if(type == Event::eDefault)
                    newItem = new EventItem(this, event, settings);
                else //if(type == Event::eKnown)
                    newItem = new EventKnownItem(this, event, settings);

                mItems.append(newItem);
                addItem(newItem);


                // Usefull to add some dates to last created Event!
                clearSelection();
                newItem->setSelected(true);

                // Note : setting an event in (0, 0) tells the scene that this item is new!
                // Thus the scene will move it randomly around the currently viewed center point.
                QPointF pos = newItem->pos();
                if(pos.isNull()) {
                    QList<QGraphicsView*> gviews = views();
                    if(gviews.size() > 0)
                    {
                        QGraphicsView* gview = gviews[0];
                        QPointF pt = gview->mapToScene(gview->width()/2, gview->height()/2);
                        int posDelta = 100;
                        newItem->setPos(pt.x() + rand() % posDelta - posDelta/2,
                                          pt.y() + rand() % posDelta - posDelta/2);
                    }

                newItem = 0;
                hasCreated = true;
    #ifdef DEBUG
                qDebug() << "Event created : id = " << event.value(STATE_ID).toInt() << event.value(STATE_NAME).toString()<<", type : " << type;
    #endif
            }

        }

    }
    // ------------------------------------------------------
    //  Delete constraints not in current state
    // ------------------------------------------------------
    for(int i=mConstraintItems.size()-1; i>=0; --i)
    {
        ArrowItem* constraintItem = mConstraintItems[i];
        QJsonObject& constraint = constraintItem->data();
        
        if(!constraints_ids.contains(constraint.value(STATE_ID).toInt()))
        {
#ifdef DEBUG
            qDebug() << "Event Constraint deleted : " << constraint.value(STATE_ID).toInt();
#endif
            removeItem(constraintItem);
            mConstraintItems.removeOne(constraintItem);
            delete constraintItem;
        }
    }
    
    // ------------------------------------------------------
    //  Create / Update constraint items
    // ------------------------------------------------------

    if(displayProgress) progress->setLabelText(tr("Create / Update constraint items"));

    for(int i=0; i<constraints.size(); ++i)
    {
        const QJsonObject constraint = constraints.at(i).toObject();
        if(displayProgress) progress->setValue(i);
        bool itemExists = false;
        for(int j=0; j<mConstraintItems.size(); ++j)
        {
            QJsonObject constraintItem = mConstraintItems.at(j)->data();
            if(constraintItem.value(STATE_ID).toInt() == constraint.value(STATE_ID).toInt())
            {
                itemExists = true;
                if(constraint != constraintItem)
                {
                    // UPDATE ITEM
#ifdef DEBUG
                    qDebug() << "Constraint updated : id = " << constraint.value(STATE_ID).toInt();
#endif
                    mConstraintItems.at(j)->setData(constraint);
                }
            }
        }
        if(!itemExists)
        {
            // CREATE ITEM
            ArrowItem* constraintItem = new ArrowItem(this, ArrowItem::eEvent, constraint);
            mConstraintItems.append(constraintItem);
            addItem(constraintItem);
#ifdef DEBUG
            //qDebug() << "Constraint created : id = " << constraint[STATE_ID].toInt();
#endif
        }
    }
    
    mUpdatingItems = false;
    
    // Deleting an item that was selected involves changing the selection (and updating properties view)
    // Nothing has been triggered so far because of the mUpdatingItems flag, so we need to trigger it now!
    // As well, creating an item changes the selection because we want the newly created item to be selected.
    if(hasDeleted || hasCreated)
    {
        updateSelection(true);
    }
    
    adjustSceneRect();
    adaptItemsForZoom(mZoom);

   if(displayProgress) {
       delete progress;
       progress = 0;
   }
 #ifdef DEBUG

    QTime timeDiff(0,0,0,1);
    timeDiff = timeDiff.addMSecs(startTime.elapsed()).addMSecs(-1);

    qDebug()<<"EventsScene::updateProject() finish at " + timeDiff.toString("hh:mm:ss.zzz");
 #endif
}

void EventsScene::clean()
{
    // ------------------------------------------------------
    //  Delete all items
    // ------------------------------------------------------
    for(int i=mItems.size()-1; i>=0; --i)
    {
        EventItem* eventItem = (EventItem*)mItems[i];
        QJsonObject& event = eventItem->getEvent();
        
        if(event.value(STATE_EVENT_TYPE).toInt() == Event::eDefault)
        {
            QList<QGraphicsItem*> dateItems = eventItem->childItems();
            for(int j=0; j<dateItems.size(); ++j)
            {
                removeItem(dateItems[j]);
                delete dateItems[j];
            }
        }
#ifdef DEBUG
        //qDebug() << "Event deleted : " << event[STATE_ID].toInt();
#endif
        mItems.removeAt(i);
        
        // This does not break the code but is commented to match PhasesScene implementation
        //removeItem(eventItem);
        
        eventItem->deleteLater();
    }
    
    // ------------------------------------------------------
    //  Delete all constraints
    // ------------------------------------------------------
    for(int i=mConstraintItems.size()-1; i>=0; --i)
    {
        ArrowItem* constraintItem = mConstraintItems[i];
#ifdef DEBUG
        //QJsonObject& constraint = constraintItem->data();
        //qDebug() << "Event Constraint deleted : " << constraint.value(STATE_ID).toInt();
#endif
        removeItem(constraintItem);
        mConstraintItems.removeOne(constraintItem);
        delete constraintItem;
    }
    
    // ------------------------------------------------------
    //  Reset scene rect
    // ------------------------------------------------------
    //setSceneRect(QRectF(-100, -100, 200, 200));
    update(sceneRect());
}


#pragma mark Selection & Current
void EventsScene::updateSelection(bool sendNotification, bool force)
{
    if(!mUpdatingItems)
    {
        bool modified = false;
        
        //static int counter = 0;
        //qDebug() << "---------- selection " << counter;
        //counter++;
        
        for(int i=0; i<mItems.size(); ++i)
        {
            EventItem* item = (EventItem*)mItems[i];
            QJsonObject& event = item->getEvent();
            bool selected = mItems[i]->isSelected();
            
            if(event[STATE_IS_SELECTED].toBool() != selected)
            {
                event[STATE_IS_SELECTED] = selected;
                modified = true;
            }
            if(event[STATE_IS_CURRENT].toBool())
            {
                event[STATE_IS_CURRENT] = false;
                modified = true;
            }
            //qDebug() << event[STATE_ID].toInt() << event[STATE_IS_SELECTED].toBool();
        }
        QJsonObject event;
        EventItem* curItem = (EventItem*)currentItem();
        if(curItem)
        {
            QJsonObject& evt = curItem->getEvent();
            if(!evt[STATE_IS_CURRENT].toBool())
            {
                evt[STATE_IS_CURRENT] = true;
                event = evt;
                modified = true;
            }
        }
        if(modified || force)
        {
            emit MainWindow::getInstance()->getProject()->currentEventChanged(event);
            if(sendNotification)
            {
                sendUpdateProject(tr("events selection : no undo, no view update!"), false, false);
                MainWindow::getInstance()->getProject()->sendEventsSelectionChanged();
            }
        }
    }
}

void EventsScene::updateSelectedEventsFromPhases()
{
    // Do not send "selection updated" each time an item is selected in this function!
    // Do it all at once at the end.
    mUpdatingItems = true;
    
    QJsonObject state = MainWindow::getInstance()->getProject()->state();
    QJsonArray phases = state[STATE_PHASES].toArray();
    
    for(int i=0; i<mItems.size(); ++i)
    {
        EventItem* item = (EventItem*)mItems[i];
        bool mustBeSelected = false;
        QList<int> eventPhasesIds = stringListToIntList(item->mData[STATE_EVENT_PHASE_IDS].toString());
        
        for(int i=0; i<phases.size(); ++i)
        {
            QJsonObject phase = phases[i].toObject();
            int phaseId = phase[STATE_ID].toInt();
            
            if(eventPhasesIds.contains(phaseId))
            {
                if(phase[STATE_IS_SELECTED].toBool())
                    mustBeSelected = true;
            }
        }
        item->setSelected(mustBeSelected);
    }
    mUpdatingItems = false;
    updateSelection();
}

void EventsScene::adaptItemsForZoom(double prop)
{
    mZoom = prop;
    for(int i=0; i<mItems.size(); ++i)
    {
        EventItem* item = dynamic_cast<EventItem*>(mItems[i]);
        item->setDatesVisible(mZoom > 0.6);
    }
}

void EventsScene::centerOnEvent(int eventId)
{
    for(int i=0; i<mItems.size(); ++i)
    {
        EventItem* item = dynamic_cast<EventItem*>(mItems[i]);
        QJsonObject& event = item->getEvent();
        if(event[STATE_ID].toInt() == eventId && views().size() > 0)
        {
            views()[0]->centerOn(item);
            clearSelection();
            item->setSelected(true);
            break;
        }
    }
}

#pragma mark Utilities
AbstractItem* EventsScene::currentItem()
{
    QList<QGraphicsItem*> selItems = selectedItems();
    //qDebug() << "selected items : " << selItems.size();
    
    if(!selItems.isEmpty())
    {
        EventItem* evtItem = dynamic_cast<EventItem*>(selItems.first());
        return evtItem;
    }
    else
    {
        return 0;
    }
}



AbstractItem* EventsScene::collidingItem(QGraphicsItem* item)
{
    for(int i=0; i<mItems.size(); ++i)
    {
        bool isBound = (dynamic_cast<EventKnownItem*>(mItems[i]) != 0);
        if(item != mItems[i] && !isBound && item->collidesWithItem(mItems[i]))
            return mItems[i];
    }
    return 0;
}

#pragma mark Dates Items
void EventsScene::dateMoved(DateItem* dateItem, QGraphicsSceneMouseEvent* e)
{
    Q_UNUSED(e);
    if(dateItem)
    {
        static EventItem* lastEntered = 0;
        EventItem* hoveredEventItem = (EventItem*)collidingItem(dateItem);
        EventItem* prevEventItem = (EventItem*)dateItem->parentItem();
        
        if(hoveredEventItem != lastEntered)
        {
            if(lastEntered)
                lastEntered->setMergeable(false);
            lastEntered = hoveredEventItem;
            
            if(hoveredEventItem && prevEventItem && (hoveredEventItem != prevEventItem))
            {
                const QJsonObject& event = hoveredEventItem->getEvent();
                if(event[STATE_EVENT_TYPE].toInt() == Event::eDefault)
                    hoveredEventItem->setMergeable(true);
            }
        }
    }
}

void EventsScene::dateReleased(DateItem* dateItem, QGraphicsSceneMouseEvent* e)
{
    Q_UNUSED(e);
    if(dateItem)
    {
        EventItem* hoveredEventItem = dynamic_cast<EventItem*>(collidingItem(dateItem));
        EventItem* prevEventItem = dynamic_cast<EventItem*>(dateItem->parentItem());
        
        if(hoveredEventItem && prevEventItem && (hoveredEventItem != prevEventItem))
        {
            QJsonObject prevEvent = prevEventItem->getEvent();
            QJsonObject nextEvent = hoveredEventItem->getEvent();
            QJsonObject dateToRemove = dateItem->date();
            QJsonObject dateToAdd = dateItem->date();

            if(nextEvent.value(STATE_EVENT_TYPE).toInt() == Event::eDefault)
            {
                // Move the date to another event :
                
                Project* project = MainWindow::getInstance()->getProject();
                QJsonObject state = project->state();
                QJsonArray events = state.value(STATE_EVENTS).toArray();
                bool isRemove = false;
                bool isAdd = false;
                for(int i=0; !(isRemove && isAdd) && (i<events.size()) ; ++i)
                {
                    QJsonObject event = events.at(i).toObject();
                    
                    // remove dateToRemove from previous event :
                    if(event.value(STATE_ID).toInt() == prevEvent.value(STATE_ID).toInt())
                    {
                        QJsonArray dates = event.value(STATE_EVENT_DATES).toArray();
                        for(int j=0; j<dates.size(); ++j)
                        {
                            if(dates.at(j).toObject() == dateToRemove)
                            {
                                dates.removeAt(j);
                                isRemove=true;
                                break;
                            }
                        }
                        event[STATE_EVENT_DATES] = dates;
                    }
                    // add dateToAdd to next event :
                    else if(event.value(STATE_ID).toInt() == nextEvent.value(STATE_ID).toInt())
                    {
                        QJsonArray dates = event.value(STATE_EVENT_DATES).toArray();
                        dateToAdd[STATE_ID] = project->getUnusedDateId(dates);
                        dates.append(dateToAdd);
                        event[STATE_EVENT_DATES] = dates;
                        isAdd=true;
                    }
                    events[i] = event;
                }
                state[STATE_EVENTS] = events;
                project->pushProjectState(state, tr("Date moved to event"), true);
                
                hoveredEventItem->setMergeable(false);
                return;
            }
        }
        
        mDatesAnim->setItem(dateItem);
        mDatesAnim->setPosAt(0, dateItem->pos());
        mDatesAnim->setPosAt(1, dateItem->mOriginalPos);
        mDatesAnimTimer->start();
    }
}

// ----------------------------------------------------------------------------------------
//  Event Items Events
// ----------------------------------------------------------------------------------------
#pragma mark Item mouse events
void EventsScene::itemDoubleClicked(AbstractItem* item, QGraphicsSceneMouseEvent* e)
{
    AbstractScene::itemDoubleClicked(item, e);
    emit eventDoubleClicked();
}

void EventsScene::constraintDoubleClicked(ArrowItem* item, QGraphicsSceneMouseEvent* e)
{
    Q_UNUSED(e);
    Q_UNUSED(item);
    
    /*QMessageBox message(QMessageBox::Question, tr("Delete constraint"), tr("Do you really want to delete this constraint?"), QMessageBox::Yes | QMessageBox::No, qApp->activeWindow(), Qt::Sheet);
    if(message.exec() == QMessageBox::Yes)
    {
        MainWindow::getInstance()->getProject()->deleteEventConstraint(item->data()[STATE_ID].toInt());
        qDebug() << "TODO : delete constraint";
    }*/
    
    //Project* project = MainWindow::getInstance()->getProject();
    //project->updateEventConstraint(item->constraint()[STATE_ID].toInt());
}

void EventsScene::constraintClicked(ArrowItem* item, QGraphicsSceneMouseEvent* e)
{
    Q_UNUSED(e);
    Q_UNUSED(item);
    
    QMessageBox message(QMessageBox::Question, tr("Delete constraint"), tr("Do you really want to delete this constraint?"), QMessageBox::Yes | QMessageBox::No, qApp->activeWindow(), Qt::Sheet);
    if(message.exec() == QMessageBox::Yes)
    {
        MainWindow::getInstance()->getProject()->deleteEventConstraint(item->data().value(STATE_ID).toInt());
       // qDebug() << "TODO : delete constraint";
    }
    
    //Project* project = MainWindow::getInstance()->getProject();
    //project->updateEventConstraint(item->constraint()[STATE_ID].toInt());
}


// -----------------------------------------------------------
//  The following function are about drag & drop
// -----------------------------------------------------------
#pragma mark Drag & Drop
void EventsScene::dragMoveEvent(QGraphicsSceneDragDropEvent* e)
{
    for(int i=0; i<mItems.size(); ++i)
    {
        QRectF r = mItems.at(i)->boundingRect();
        r.translate(mItems.at(i)->scenePos());
        if(r.contains(e->scenePos()))
        {
            QGraphicsScene::dragMoveEvent(e);
            return;
        }
    }
    e->accept();
}

void EventsScene::dropEvent(QGraphicsSceneDragDropEvent* e)
{
    // ------------------------------------------------------------------
    //  Check if data have been dropped on an existing event.
    //  If so, QGraphicsScene::dropEvent(e) will pass the event to the corresponding item
    // ------------------------------------------------------------------
    for(int i=0; i<mItems.size(); ++i)
    {
        QRectF r = mItems.at(i)->boundingRect();
        r.translate(mItems.at(i)->scenePos());
        if(r.contains(e->scenePos()))
        {
            QGraphicsScene::dropEvent(e);
            return;
        }
    }
    
    // ------------------------------------------------------------------
    //  The data have been dropped on the scene background,
    //  so create one event per data!
    // ------------------------------------------------------------------
    e->accept();
    
    Project* project = MainWindow::getInstance()->getProject();
    QJsonObject state = project->state();
    QJsonArray events = state.value(STATE_EVENTS).toArray();
    
    QList<Date> dates = decodeDataDrop(e);
    
    // Create one event per data
    int deltaY = 100;
    for(int i=0; i<dates.size(); ++i)
    {
        Event event;
        event.mName = dates.at(i).mName;
        event.mId = project->getUnusedEventId(events);
        dates[i].mId = 0;
        event.mDates.append(dates.at(i));
        event.mItemX = e->scenePos().x();
        event.mItemY = e->scenePos().y() + i * deltaY;
        event.mColor = randomColor();
        events.append(event.toJson());
    }
    state[STATE_EVENTS] = events;
    project->pushProjectState(state, tr("Dates added (CSV drag)"), true);
}

QList<Date> EventsScene::decodeDataDrop(QGraphicsSceneDragDropEvent* e)
{
    const QMimeData* mimeData = e->mimeData();

    QByteArray encodedData = mimeData->data("application/chronomodel.import.data");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    
   // QList<QStringList> failed;
    QList<int> acceptedRows;
    QList<int> rejectedRows;
    QList<Date> dates;

    while(!stream.atEnd())
    {
        QString itemStr;
        stream >> itemStr;
        const AppSettings settings = MainWindow::getInstance()->getAppSettings();
        const QString csvSep = settings.mCSVCellSeparator;
        QStringList dataStr = itemStr.split(csvSep);
        
        // Remove first column corresponding to csvRow
        const int csvRow = dataStr.takeFirst().toInt();
        
        const QLocale csvLocal = settings.mCSVDecSeparator == "." ? QLocale::English : QLocale::French;
        Date date = Date::fromCSV(dataStr, csvLocal);

        if(!date.isNull())
        {
            dates << date;
            acceptedRows.append(csvRow);
        }
        else
        {
            //failed.append(dataStr);
            rejectedRows.append(csvRow);
        }
    }
    emit csvDataLineDropAccepted(acceptedRows); //connected to slot ImportDataView::removeCsvRows
    emit csvDataLineDropRejected(rejectedRows);

    return dates;
}

