#include "EventItem.h"
#include "Event.h"
#include "Phase.h"
#include "Date.h"
#include "Painting.h"
#include "DateItem.h"
#include "ProjectManager.h"
#include "Project.h"
#include "QtUtilities.h"
#include "Painting.h"
#include <QtWidgets>


EventItem::EventItem(EventsScene* scene, const QJsonObject& event, const QJsonObject& settings, QGraphicsItem* parent):AbstractItem(scene, parent)
{
    setEvent(event, settings);
}

EventItem::~EventItem()
{
    
}

#pragma mark Event Managment
QJsonObject& EventItem::getEvent()
{
    return mEvent;
}

void EventItem::setEvent(const QJsonObject& event, const QJsonObject& settings)
{
    mEvent = event;
    
    // ----------------------------------------------
    //  Update item position and selection
    // ----------------------------------------------
    setSelected(mEvent[STATE_EVENT_IS_SELECTED].toBool());
    setPos(mEvent[STATE_EVENT_ITEM_X].toDouble(),
           mEvent[STATE_EVENT_ITEM_Y].toDouble());
    
    // ----------------------------------------------
    //  Check if item should be greyed out
    // ----------------------------------------------
    updateGreyedOut();
    
    // ----------------------------------------------
    //  Delete Date Items
    // ----------------------------------------------
    QList<QGraphicsItem*> dateItems = childItems();
    for(int i=0; i<dateItems.size(); ++i)
    {
        mScene->removeItem(dateItems[i]);
        delete dateItems[i];
    }
    
    // ----------------------------------------------
    //  Re-create Date Items
    // ----------------------------------------------
    QJsonArray dates = mEvent[STATE_EVENT_DATES].toArray();
    for(int i=0; i<dates.size(); ++i)
    {
        QJsonObject date = dates[i].toObject();
        QColor color(mEvent[STATE_EVENT_RED].toInt(),
                     mEvent[STATE_EVENT_GREEN].toInt(),
                     mEvent[STATE_EVENT_BLUE].toInt());
        
        DateItem* dateItem = new DateItem((EventsScene*)mScene, date, color, settings);
        dateItem->setParentItem(this);
        dateItem->setGreyedOut(mGreyedOut);
        
        QPointF pos(0,
                    boundingRect().y() +
                    mTitleHeight +
                    mBorderWidth +
                    2*mEltsMargin +
                    i * (mEltsHeight + mEltsMargin));
        dateItem->setPos(pos);
        dateItem->setOriginalPos(pos);
    }
    
    // ----------------------------------------------
    //  Repaint based on mEvent
    // ----------------------------------------------
    update();
}

void EventItem::setGreyedOut(bool greyedOut)
{
    AbstractItem::setGreyedOut(greyedOut);
    
    QList<QGraphicsItem*> children = childItems();
    for(int i=0; i<children.size(); ++i)
    {
        ((DateItem*)children[i])->setGreyedOut(greyedOut);
    }
}

void EventItem::updateGreyedOut()
{
    mGreyedOut = true;
    QJsonObject state = ProjectManager::getProject()->state();
    QJsonArray phases = state[STATE_PHASES].toArray();
    QStringList selectedPhasesIds;
    for(int i=0; i<phases.size(); ++i)
    {
        QJsonObject phase = phases[i].toObject();
        bool isSelected = phase[STATE_PHASE_IS_SELECTED].toBool();
        if(isSelected)
            selectedPhasesIds.append(QString::number(phase[STATE_PHASE_ID].toInt()));
    }
    if(selectedPhasesIds.size() == 0)
    {
        mGreyedOut = false;
    }
    else
    {
        QString eventPhasesIdsStr = mEvent[STATE_EVENT_PHASE_IDS].toString();
        QStringList eventPhasesIds = eventPhasesIdsStr.split(",");
        for(int i=0; i<selectedPhasesIds.size(); ++i)
        {
            if(eventPhasesIds.contains(selectedPhasesIds[i]))
            {
                mGreyedOut = false;
                break;
            }
        }
    }
}

#pragma mark Events
void EventItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    mEvent[STATE_EVENT_ITEM_X] = pos().x();
    mEvent[STATE_EVENT_ITEM_Y] = pos().y();
    
    AbstractItem::mouseReleaseEvent(e);
}

void EventItem::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
    mEvent[STATE_EVENT_ITEM_X] = pos().x();
    mEvent[STATE_EVENT_ITEM_Y] = pos().y();
    
    AbstractItem::mouseMoveEvent(e);
}

void EventItem::dropEvent(QGraphicsSceneDragDropEvent* e)
{
    handleDrop(e);
}

void EventItem::handleDrop(QGraphicsSceneDragDropEvent* e)
{
    e->acceptProposedAction();
    Project* project = ProjectManager::getProject();
    QJsonObject event = mEvent;
    
    QJsonArray dates = event[STATE_EVENT_DATES].toArray();
    QList<Date> datesDragged = ((EventsScene*)mScene)->decodeDataDrop(e);
    for(int i=0; i<datesDragged.size(); ++i)
    {
        QJsonObject date = datesDragged[i].toJson();
        date[STATE_DATE_ID] = project->getUnusedDateId(dates);
        dates.append(date);
    }
    event[STATE_EVENT_DATES] = dates;
    
    project->updateEvent(event, QObject::tr("Dates added to event (CSV drag)"));
}


void EventItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    
    QRectF rect = boundingRect();
    
    QColor eventColor = QColor(mEvent[STATE_EVENT_RED].toInt(),
                               mEvent[STATE_EVENT_GREEN].toInt(),
                               mEvent[STATE_EVENT_BLUE].toInt());
    painter->setPen(Qt::NoPen);
    painter->setBrush(eventColor);
    painter->drawRect(rect);
    
    
    // Phases
    QJsonArray phases = getPhases();
    QRectF phasesRect(rect.x(), rect.y() + rect.height() - mPhasesHeight, rect.width(), mPhasesHeight);
    phasesRect.adjust(1, 1, -1, -1);
    
    int numPhases = (int)phases.size();
    float w = phasesRect.width()/numPhases;
    
    for(int i=0; i<numPhases; ++i)
    {
        QJsonObject phase = phases[i].toObject();
        QColor c(phase[STATE_PHASE_RED].toInt(),
                 phase[STATE_PHASE_GREEN].toInt(),
                 phase[STATE_PHASE_BLUE].toInt());
        painter->setPen(c);
        painter->setBrush(c);
        painter->drawRect(phasesRect.x() + i*w, phasesRect.y(), w, phasesRect.height());
    }
    
    if(numPhases == 0)
    {
        QFont font = qApp->font();
        font.setPointSizeF(pointSize(11));
        painter->setFont(font);
        painter->fillRect(phasesRect, QColor(0, 0, 0, 180));
        painter->setPen(QColor(200, 200, 200));
        painter->drawText(phasesRect, Qt::AlignCenter, tr("No Phase"));
    }
    
    painter->setPen(QColor(0, 0, 0));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(phasesRect);
    
    
    // Name
    QRectF tr(rect.x() + mBorderWidth + 2*mEltsMargin + mTitleHeight,
              rect.y() + mBorderWidth + mEltsMargin,
              rect.width() - 2*mBorderWidth - 2*(mTitleHeight + 2*mEltsMargin),
              mTitleHeight);
    
    QFont font = qApp->font();
    painter->setFont(font);
    QFontMetrics metrics(font);
    QString name = mEvent[STATE_EVENT_NAME].toString();
    name = metrics.elidedText(name, Qt::ElideRight, tr.width());
    
    QColor frontColor = getContrastedColor(eventColor);
    painter->setPen(frontColor);
    painter->drawText(tr, Qt::AlignCenter, name);
    
    
    if(mGreyedOut)
    {
        painter->setPen(Painting::greyedOut);
        painter->setBrush(Painting::greyedOut);
        painter->drawRect(boundingRect());
    }
    
    // Border
    painter->setBrush(Qt::NoBrush);
    if(mMergeable)
    {
        painter->setPen(QPen(Painting::mainColorLight, 3.f, Qt::DashLine));
        painter->drawRect(rect.adjusted(1, 1, -1, -1));
    }
    else if(isSelected())
    {
        painter->setPen(QPen(Painting::mainColorDark, 3.f));
        painter->drawRect(rect.adjusted(1, 1, -1, -1));
    }
    
    painter->restore();
}

QJsonArray EventItem::getPhases() const
{
    QJsonObject state = ProjectManager::getProject()->state();
    QJsonArray allPhases = state[STATE_PHASES].toArray();
    QJsonArray phases;
    QString eventPhaseIdsStr = mEvent[STATE_EVENT_PHASE_IDS].toString();
    QStringList eventPhaseIds = eventPhaseIdsStr.split(",");
    
    for(int i=0; i<allPhases.size(); ++i)
    {
        QJsonObject phase = allPhases[i].toObject();
        QString phaseId = QString::number(phase[STATE_PHASE_ID].toInt());
        if(eventPhaseIds.contains(phaseId))
            phases.append(phase);
    }
    return phases;
}

#pragma mark Geometry
QRectF EventItem::boundingRect() const
{
    qreal penWidth = 1;
    qreal w = 100.;
    
    float h = mTitleHeight + mPhasesHeight + 2*mBorderWidth + 2*mEltsMargin;

    QString name = mEvent[STATE_EVENT_NAME].toString();
    QJsonArray dates = mEvent[STATE_EVENT_DATES].toArray();
    
    QFont font = qApp->font();
    QFontMetrics metrics(font);
    w = metrics.width(name) + 2*mBorderWidth + 4*mEltsMargin + 2*mTitleHeight;
    
    int count = dates.size();
    
    if(count > 0)
        h += count * (mEltsHeight + mEltsMargin);
    else
        h += mEltsMargin + mEltsHeight;
    
    font.setPointSizeF(pointSize(11));
    metrics = QFontMetrics(font);
    for(int i=0; i<count; ++i)
    {
        QJsonObject date = dates[i].toObject();
        name = date[STATE_DATE_NAME].toString();
        int nw = metrics.width(name) + 2*mBorderWidth + 4*mEltsMargin;
        w = (nw > w) ? nw : w;
    }
    w = (w < 150) ? 150 : w;
    
    return QRectF(-(w+penWidth)/2, -(h+penWidth)/2, w + penWidth, h + penWidth);
}
