#ifndef PhaseItem_H
#define PhaseItem_H

#include "AbstractItem.h"
#include "PhasesScene.h"
#include "Phase.h"


class PhaseItem : public AbstractItem
{
    Q_OBJECT
// members
public:
    //Qt::CheckState mState;
    //bool mEyeActivated;
    bool mControlsVisible;
    bool mControlsEnabled;
    bool mAtLeastOneEventSelected;
    QSize mSize;
 void redrawPhase();

//functions
protected:
    QRectF boundingRect() const;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void mousePressEvent(QGraphicsSceneMouseEvent* e);

    void hoverEnterEvent(QGraphicsSceneHoverEvent* e);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* e);

    // size and position of the buttons
    QRectF checkRect() const;
    QRectF insertRect() const;
    QRectF extractRect() const;

    QJsonArray getEvents() const;
    QString getTauString() const;
    // the two pixmaps of insert_Event and extract Event
    QPixmap* inPix;
    QPixmap* exPix;

public:
    PhaseItem(AbstractScene* scene, const QJsonObject& phase, QGraphicsItem* parent = nullptr);
    virtual ~PhaseItem();
    
    QJsonObject& getPhase();
    void setPhase(const QJsonObject& phase);
    
    virtual void updateItemPosition(const QPointF& pos);
    
    void setControlsVisible(const bool visible);
    void setControlsEnabled(const bool enabled);


};

#endif
