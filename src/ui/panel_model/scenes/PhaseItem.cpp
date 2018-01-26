#include "PhaseItem.h"
#include "PhasesScene.h"
#include "Event.h"
#include "Date.h"
#include "Painting.h"
#include "QtUtilities.h"
#include "MainWindow.h"
#include "Project.h"
#include "ArrowTmpItem.h"
#include <QtWidgets>


PhaseItem::PhaseItem(AbstractScene* scene, const QJsonObject& phase, QGraphicsItem* parent):AbstractItem(scene, parent),
mControlsVisible(false),
mControlsEnabled(false),
mAtLeastOneEventSelected(false)
{
    mBorderWidth = 10.;
    mEltsHeight = 15.;
    setPhase(phase);
    inPix = new QPixmap(":insert_event.png");
    exPix = new QPixmap(":extract_event.png");
}

PhaseItem::~PhaseItem()
{
    
}

QJsonObject& PhaseItem::getPhase()
{
    return mData;
}

void PhaseItem::setPhase(const QJsonObject& phase)
{
    Q_ASSERT(&phase);
    mData = phase;
    
    setSelected(mData.value(STATE_IS_SELECTED).toBool() || mData.value(STATE_IS_CURRENT).toBool() );
    setPos(mData.value(STATE_ITEM_X).toDouble(),
           mData.value(STATE_ITEM_Y).toDouble());
    
    // ----------------------------------------------------
    //  Calculate item size
    // ----------------------------------------------------
    const qreal w = 150.;
    qreal h = mTitleHeight + 2*mBorderWidth + 2*mEltsMargin;
    
    const QJsonArray events = getEvents();
    if (events.size() > 0)
        h += events.size() * (mEltsHeight + mEltsMargin) - mEltsMargin;
    
    const QString tauStr = getTauString();
    if (!tauStr.isEmpty())
        h += mEltsMargin + mEltsHeight;

    mSize = QSize(w, h);
    
    update();
}


void PhaseItem::setControlsEnabled(const bool enabled)
{
    mControlsEnabled = enabled;
}

void PhaseItem::setControlsVisible(const bool visible)
{
    mControlsVisible = visible;
}

// Mouse events

void PhaseItem::hoverEnterEvent(QGraphicsSceneHoverEvent* e)
{
    setControlsEnabled(true);
    AbstractItem::hoverEnterEvent(e);
}

void PhaseItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* e)
{
    setControlsEnabled(false);
    AbstractItem::hoverLeaveEvent(e);
}

void PhaseItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
  if (mControlsVisible) {

        if (insertRect().contains(e->pos())) {
            //qDebug() << "PhaseItem::mousePressEvent-> insertRect clicked";
            e->accept();
            mScene->getProject()->updatePhaseEvents(mData.value(STATE_ID).toInt(), Project::InsertEventsToPhase);
            return;

        } else if (mAtLeastOneEventSelected && extractRect().contains(e->pos())) {
            //qDebug() << "PhaseItem::mousePressEvent-> extractRect clicked";
            e->accept();
            mScene->getProject()->updatePhaseEvents(mData.value(STATE_ID).toInt(), Project::ExtractEventsFromPhase);
            return;
        }

    }

  // overwrite AbstractItem::mousePressEvent(e);

    PhasesScene* itemScene = dynamic_cast<PhasesScene*>(mScene);

    if (itemScene->selectedItems().size()<2) {
        if (!itemScene->itemClicked(this, e)) {
            setZValue(2.);
            QGraphicsItem::mousePressEvent(e);

        } else
            itemScene->mTempArrow->setFrom(pos().x(), pos().y());
    }

    QGraphicsObject::mousePressEvent(e);

}

void PhaseItem::updateItemPosition(const QPointF& pos)
{
    mData[STATE_ITEM_X] = pos.x();
    mData[STATE_ITEM_Y] = pos.y();
}

QRectF PhaseItem::boundingRect() const
{
    return QRectF(-mSize.width()/2, -mSize.height()/2, mSize.width(), mSize.height());
}


void PhaseItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    // QPainter::Antialiasing is effective on native shape = circle, square, line

    painter->setRenderHints(painter->renderHints() | QPainter::SmoothPixmapTransform | QPainter::Antialiasing );
    
    QRectF rect = boundingRect();
    int rounded (15);
    
    const QColor phaseColor = QColor(mData.value(STATE_COLOR_RED).toInt(),
                               mData.value(STATE_COLOR_GREEN).toInt(),
                               mData.value(STATE_COLOR_BLUE).toInt());
    const QColor fontColor = getContrastedColor(phaseColor);
    QFont font = qApp->font();
    QFontMetrics fm (font);

    // Draw then container
    painter->setPen(Qt::NoPen);
    painter->setBrush(phaseColor);
    painter->drawRoundedRect(rect, rounded, rounded);

    //
    // Events
    QRectF r(rect.x() + mBorderWidth + mEltsMargin,
             rect.y() + mBorderWidth + mEltsMargin + mTitleHeight + mEltsMargin,
             rect.width() - 2 * (mEltsMargin + mBorderWidth),
             mEltsHeight);

    const QJsonArray events = getEvents();
    double dy = mEltsMargin + mEltsHeight;

    const bool showAlldata = mScene->showAllThumbs();
    mAtLeastOneEventSelected = false;
    for (int i=0; i<events.size(); ++i) {

        const QJsonObject event = events.at(i).toObject();
        const QColor eventColor(event.value(STATE_COLOR_RED).toInt(),
                          event.value(STATE_COLOR_GREEN).toInt(),
                          event.value(STATE_COLOR_BLUE).toInt());
        const bool isSelected = ( event.value(STATE_IS_SELECTED).toBool() || event.value(STATE_IS_CURRENT).toBool() );

        if (i > 0)
            r.adjust(0, dy, 0, dy);

        painter->setPen(getContrastedColor(eventColor));

        const QString eventName = fm.elidedText(event.value(STATE_NAME).toString(), Qt::ElideRight, r.width());
        // magnify and highlight selected events
        if (isSelected) {
            mAtLeastOneEventSelected = true;

            painter->setPen(QPen(fontColor, 3.));
            r.adjust(1, 1, -1, -1);
            painter->drawRoundedRect(r, 1, 1);
            painter->fillRect(r, eventColor);
            painter->setPen(QPen(getContrastedColor(eventColor), 1));

            painter->drawText(r, Qt::AlignCenter, eventName);
            r.adjust(-1, -1, +1, +1);
        }
        else if (isSelected || showAlldata) {
                painter->fillRect(r, eventColor);
                painter->drawText(r, Qt::AlignCenter, eventName);

        } else {
            painter->setOpacity(0.2);
            painter->fillRect(r, eventColor);
            painter->drawText(r, Qt::AlignCenter, eventName);
            painter->setOpacity(1);
        }

    }



  //
    if (mControlsVisible && mControlsEnabled) {
        // insert button
        const QRectF inRect = insertRect();
        painter->drawRect(inRect);
        painter->fillRect(inRect, Qt::black);

        qreal sx = inRect.width()/inPix->width();
        qreal sy =  inRect.height()/inPix->height();
        QMatrix mx = QMatrix();
        mx.scale(sx, sy);

        const QPixmap inPix2 = inPix->transformed(mx, Qt::SmoothTransformation);

        painter->drawPixmap(inRect.x(), inRect.y(), inPix2);

        // extract button
        const QRectF exRect = extractRect();
        // we suppose, it is the same matrix
        const QPixmap exPix2 = exPix->transformed(mx, Qt::SmoothTransformation);
        painter->drawRect(exRect);
        painter->fillRect(exRect, Qt::black);

        if (mAtLeastOneEventSelected) {
            painter->drawPixmap(exRect, exPix2, exPix2.rect());

        } else {
            painter->setOpacity(0.5);
            painter->drawPixmap(exRect, exPix2, exPix2.rect());
            painter->setOpacity(1);
        }

    } else {
        // Name
        const QRectF tr(rect.x() + mBorderWidth ,
                  rect.y() + mBorderWidth + mEltsMargin,
                  rect.width() - 2*mBorderWidth ,
                  mTitleHeight);

        painter->setFont(font);
        QString name = mData.value(STATE_NAME).toString();
        name = fm.elidedText(name, Qt::ElideRight, tr.width());
        painter->setPen(fontColor);
        painter->drawText(tr, Qt::AlignCenter, name);
    }

    // Change font
    font.setPointSizeF(pointSize(11.f));
    painter->setFont(font);
    
    // Type (duration tau)
    const QString tauStr = getTauString();
    if (!tauStr.isEmpty()) {
        const QRectF tpr(rect.x() + mBorderWidth + mEltsMargin,
                   rect.y() + rect.height() - mBorderWidth - mEltsMargin - mEltsHeight,
                   rect.width() - 2*mBorderWidth - 2*mEltsMargin,
                   mEltsHeight);
        
        painter->setPen(Qt::black);
        painter->setBrush(Qt::white);
        painter->drawRect(tpr);
        painter->drawText(tpr, Qt::AlignCenter, tauStr);
    }
    

    // Border
    painter->setBrush(Qt::NoBrush);
    if (isSelected()) {
        painter->setPen(QPen(Qt::white, 5.f));
        painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1), rounded, rounded);
        
        painter->setPen(QPen(Qt::red, 3.f));
        painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1), rounded, rounded);
    }
}

QJsonArray PhaseItem::getEvents() const
{
    QString phaseId = QString::number(mData.value(STATE_ID).toInt());
    QJsonObject state = MainWindow::getInstance()->getProject()->state();
    QJsonArray allEvents = state.value(STATE_EVENTS).toArray();
    QJsonArray events;
    for (int i=0; i<allEvents.size(); ++i) {
        QJsonObject event = allEvents.at(i).toObject();
        QString phasesIdsStr = event.value(STATE_EVENT_PHASE_IDS).toString();
        QStringList phasesIds = phasesIdsStr.split(",");
        if (phasesIds.contains(phaseId))
            events.append(event);
    }
    return events;
}

QString PhaseItem::getTauString() const
{
    QString tauStr;
    Phase::TauType type = (Phase::TauType)mData.value(STATE_PHASE_TAU_TYPE).toInt();
    if (type == Phase::eTauFixed)
        tauStr += tr("Duration ≤ %1").arg(QString::number(mData.value(STATE_PHASE_TAU_FIXED).toDouble()));

    return tauStr;
}

QRectF PhaseItem::checkRect() const
{
    QRectF rect = boundingRect();
    QRectF r(rect.x() + mBorderWidth + mEltsMargin,
             rect.y() + mBorderWidth + mEltsMargin,
             mTitleHeight,
             mTitleHeight);
    return r;
}


QRectF PhaseItem::extractRect() const
{
    QRectF rect = boundingRect();
    QRectF r(rect.x() + mBorderWidth + mEltsMargin + mEltsMargin + mTitleHeight,
             rect.y() + mBorderWidth + mEltsMargin,
             mTitleHeight,
             mTitleHeight);
    return r;
}

QRectF PhaseItem::insertRect() const
{
    QRectF rect = boundingRect();
    QRectF r(rect.x() + mBorderWidth + mEltsMargin,
             rect.y() + mBorderWidth + mEltsMargin,
             mTitleHeight,
             mTitleHeight);
    return r;
}
