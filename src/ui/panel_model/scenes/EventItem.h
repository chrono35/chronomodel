#ifndef EventItem_H
#define EventItem_H

#include "AbstractItem.h"

class EventsScene;


class EventItem : public AbstractItem
{
public:
    EventItem(EventsScene* scene, const QJsonObject& event, const QJsonObject& settings, QGraphicsItem* parent = nullptr);
    virtual ~EventItem();

    virtual void setGreyedOut(const bool greyedOut);

    void setWithSelectedPhase(const bool selected) {mWithSelectedPhase = selected;}
    bool withSelectedPhase() { return mWithSelectedPhase;}

    QJsonObject& getEvent();
    virtual void setEvent(const QJsonObject& event, const QJsonObject& settings);
    
    virtual QRectF boundingRect() const;
    void handleDrop(QGraphicsSceneDragDropEvent* e);
    QJsonArray getPhases() const;
    
    virtual void updateItemPosition(const QPointF& pos);
    
    virtual void setDatesVisible(bool visible);
    void redrawEvent();
    
    bool withSelectedDate() const;
    
    void mousePressEvent(QGraphicsSceneMouseEvent* e);
    
protected:
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    virtual void dropEvent(QGraphicsSceneDragDropEvent* e);

    void updateGreyedOut();
    
    QSize mSize;
    QJsonObject mSettings;
    bool mWithSelectedPhase;
    bool mShowAllThumbs;
};

#endif
