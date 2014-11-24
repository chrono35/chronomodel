#ifndef AbstractScene_H
#define AbstractScene_H

#include <QGraphicsScene>

class AbstractItem;
class ArrowItem;
class ArrowTmpItem;


class AbstractScene: public QGraphicsScene
{
    Q_OBJECT
public:
    AbstractScene(QGraphicsView* view, QObject* parent = 0);
    ~AbstractScene();

public:
    virtual void constraintDoubleClicked(ArrowItem* item, QGraphicsSceneMouseEvent* e) = 0;
    
    virtual bool itemClicked(AbstractItem* eventItem, QGraphicsSceneMouseEvent* e);
    virtual void itemDoubleClicked(AbstractItem* eventItem, QGraphicsSceneMouseEvent* e);
    virtual void itemEntered(AbstractItem* eventItem, QGraphicsSceneHoverEvent* e);
    virtual void itemLeaved(AbstractItem* item, QGraphicsSceneHoverEvent* e);
    virtual void itemMoved(AbstractItem* item, QPointF newPos, bool merging);
    virtual void itemReleased(AbstractItem* item, QGraphicsSceneMouseEvent* e);
    
    virtual void sendUpdateProject(const QString& reason, bool notify, bool async) = 0;
    
    void updateConstraintsPos(AbstractItem* movedItem, const QPointF& newPos);
    
protected:
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* e);
    
    virtual void keyPressEvent(QKeyEvent* keyEvent);
    virtual void keyReleaseEvent(QKeyEvent* keyEvent);
    
    virtual AbstractItem* currentItem() = 0;
    virtual AbstractItem* collidingItem(QGraphicsItem* item) = 0;
    
    virtual void deleteSelectedItems() = 0;
    virtual void createConstraint(AbstractItem* itemFrom, AbstractItem* itemTo) = 0;
    virtual void mergeItems(AbstractItem* itemFrom, AbstractItem* itemTo) = 0;
    
    

protected:
    QGraphicsView* mView;
    QList<AbstractItem*> mItems;
    ArrowTmpItem* mTempArrow;
    QList<ArrowItem*> mConstraintItems;
    
    bool mDrawingArrow;
    bool mUpdatingItems;
    bool mAltIsDown;
    bool mShiftIsDown;
};

#endif
