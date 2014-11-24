#include "EventKnownItem.h"
#include "Phase.h"
#include "Event.h"
#include "Painting.h"
#include "QtUtilities.h"
#include "Painting.h"
#include "GraphView.h"
#include "EventKnown.h"
#include "StdUtilities.h"
#include "Project.h"
#include <QtWidgets>


EventKnownItem::EventKnownItem(EventsScene* eventsScene, const QJsonObject& event, const QJsonObject& settings, QGraphicsItem* parent):EventItem(eventsScene, event, settings, parent)
{
    mThumbH = 20;
    
    setEvent(event, settings);
}

EventKnownItem::~EventKnownItem()
{
    
}

void EventKnownItem::setEvent(const QJsonObject& event, const QJsonObject& settings)
{
    mData = event;
    
    // ----------------------------------------------
    //  Update item position and selection
    // ----------------------------------------------
    setSelected(mData[STATE_IS_SELECTED].toBool());
    setPos(mData[STATE_ITEM_X].toDouble(),
           mData[STATE_ITEM_Y].toDouble());
    
    // ----------------------------------------------
    //  Check if item should be greyed out
    // ----------------------------------------------
    //updateGreyedOut();
    
    // ----------------------------------------------
    //  Recreate thumb
    // ----------------------------------------------
    int tmin = settings[STATE_SETTINGS_TMIN].toDouble();
    int tmax = settings[STATE_SETTINGS_TMAX].toDouble();
    int step = settings[STATE_SETTINGS_STEP].toDouble();
    
    EventKnown bound = EventKnown::fromJson(event);
    bound.updateValues(tmin, tmax, step);
    
    /*QRectF rect = boundingRect();
    float side = 40.f;
    float top = 25.f;
    QRectF thumbRect(rect.x() + side, rect.y() + top + mEltsMargin + mTitleHeight, rect.width() - 2*side, mThumbH);*/
    
    GraphView* graph = new GraphView();
    //graph->setFixedSize(thumbRect.width(), thumbRect.height());
    graph->setFixedSize(200, 50);
    graph->setRangeX(tmin, tmax);
    graph->setRangeY(0, 1.1f);
    graph->showAxis(false);
    graph->showScrollBar(false);
    graph->showGrid(false);
    
    GraphCurve curve;
    curve.mData = normalize_map(bound.mValues);
    curve.mName = "Bound";
    curve.mPen.setColor(Painting::mainColorLight);
    curve.mPen.setWidthF(2.f);
    curve.mFillUnder = true;
    graph->addCurve(curve);
    
    mThumb = QPixmap(graph->size());
    graph->render(&mThumb);
    delete graph;
    
    // ----------------------------------------------
    //  Repaint based on mEvent
    // ----------------------------------------------
    update();
}

QRectF EventKnownItem::boundingRect() const
{
    QFont font = qApp->font();
    QFontMetrics metrics(font);
    QString name = mData[STATE_NAME].toString();
    
    qreal w = metrics.width(name) + 2 * (mBorderWidth + mEltsMargin);
    qreal h = mTitleHeight + mThumbH + mPhasesHeight + 2*mEltsMargin;
    
    w += 80.f;
    h += 50.f;
    
    w = qMax(w, 180.);
    
    return QRectF(-w/2, -h/2, w, h);
}

void EventKnownItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    QRectF rect = boundingRect();
    
    QColor eventColor = QColor(mData[STATE_COLOR_RED].toInt(),
                               mData[STATE_COLOR_GREEN].toInt(),
                               mData[STATE_COLOR_BLUE].toInt());
    
    painter->setPen(Qt::NoPen);
    painter->setBrush(eventColor);
    painter->drawEllipse(rect);
    
    if(isSelected())
    {
        painter->setPen(QPen(Painting::mainColorDark, 3.f));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(rect.adjusted(1, 1, -1, -1));
    }
    
    float side = 40.f;
    float top = 25.f;
    
    QRectF nameRect(rect.x() + side, rect.y() + top, rect.width() - 2*side, mTitleHeight);
    QRectF thumbRect(rect.x() + side, rect.y() + top + mEltsMargin + mTitleHeight, rect.width() - 2*side, mThumbH);
    QRectF phasesRect(rect.x() + side, rect.y() + top + 2*mEltsMargin + mTitleHeight + mThumbH, rect.width() - 2*side, mPhasesHeight);
    
    phasesRect.adjust(1, 1, -1, -1);
    
    // Name
    
    QFont font = qApp->font();
    painter->setFont(font);
    QFontMetrics metrics(font);
    QString name = mData[STATE_NAME].toString();
    name = metrics.elidedText(name, Qt::ElideRight, nameRect.width());
    
    QColor frontColor = getContrastedColor(eventColor);
    painter->setPen(frontColor);
    painter->drawText(nameRect, Qt::AlignCenter, name);
    
    // Thumb
    
    painter->drawPixmap(thumbRect, mThumb, mThumb.rect());
    
    // Phases
    
    QJsonArray phases = getPhases();
    int numPhases = (int)phases.size();
    float w = phasesRect.width()/numPhases;
    
    for(int i=0; i<numPhases; ++i)
    {
        QJsonObject phase = phases[i].toObject();
        QColor c(phase[STATE_COLOR_RED].toInt(),
                 phase[STATE_COLOR_GREEN].toInt(),
                 phase[STATE_COLOR_BLUE].toInt());
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
    
    if(mGreyedOut)
    {
        painter->setPen(Painting::greyedOut);
        painter->setBrush(Painting::greyedOut);
        painter->drawEllipse(boundingRect());
    }
}

void EventKnownItem::dropEvent(QGraphicsSceneDragDropEvent* e)
{
    e->ignore();
}

QRectF EventKnownItem::toggleRect() const
{
    return QRectF();
}
