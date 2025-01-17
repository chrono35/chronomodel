#ifndef DatesListItemDelegate_H
#define DatesListItemDelegate_H

#include <QItemDelegate>
#include <QtWidgets>
#include "PluginManager.h"
#include "../PluginAbstract.h"
#include "Painting.h"


class DatesListItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    inline DatesListItemDelegate(QObject* parent = 0):QItemDelegate(parent){}
    
    inline QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex&) const
    {
        QFont font = option.font;
        font.setPointSizeF(pointSize(11));
        QFontMetrics metrics(font);
        
        /*int mm = 2;
        int mh = metrics.height();
        int butH = 20;
        return QSize(option.rect.width(), 4*mh + butH + 6*mm);*/
        
        int mm = 2;
        int mh = metrics.height();
        return QSize(option.rect.width(), 4*mh + 5*mm);
    }
    
    inline void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        int mm = 2;
        int x = option.rect.x();
        int y = option.rect.y();
        int w = option.rect.width();
        int h = option.rect.height();
        int iconW = 30;
        int iconS = 20;
        //int butH = 20;
        //int butW = 60;
        
        painter->setRenderHint(QPainter::Antialiasing);
        
        if(option.state & QStyle::State_Selected)
        {
            //painter->fillRect(option.rect, option.palette.highlight());
            painter->fillRect(option.rect, QColor(230, 230, 230));
        }
        QString dateName = index.model()->data(index, 0x0101).toString();
        QString dateDesc = index.model()->data(index, 0x0103).toString();
        QString pluginId = index.model()->data(index, 0x0102).toString();
        QString delta = index.model()->data(index, 0x0105).toString();
        QString dateMethodStr = index.model()->data(index, 0x0106).toString();
        bool isValid = index.model()->data(index, 0x0107).toBool();
        bool isCombined = index.model()->data(index, 0x0108).toBool();
        
        
        PluginAbstract* plugin = PluginManager::getPluginFromId(pluginId);
        
        if(plugin)
        {
            int im = 5;
            int is = (h - 3*im)/2;
            int ix = x + w - im - is;
            int iy = y + im;
            
            painter->save();
            QFont font = option.font;
            font.setPointSizeF(pointSize(16));
            painter->setFont(font);
            if(!isValid){
                painter->setBrush(Qt::red);
                painter->setPen(Qt::red);
                painter->drawEllipse(ix, iy, is, is);
                painter->setPen(Qt::white);
                painter->drawText(ix, iy, is, is, Qt::AlignCenter, "X");
            }
            if(isCombined){
                painter->setBrush(Painting::mainColorDark);
                painter->setPen(Painting::mainColorDark);
                painter->drawEllipse(ix, iy + is + im, is, is);
                painter->setPen(Qt::white);
                painter->drawText(ix, iy + is + im, is, is, Qt::AlignCenter, "C");
            }
            painter->restore();
            
            QIcon icon = plugin->getIcon();
            QPixmap pixmap = icon.pixmap(iconS, iconS);
            painter->drawPixmap(x + (iconW - iconS)/2, y + (h - iconS)/2, iconS, iconS, pixmap, 0, 0, pixmap.width(), pixmap.height());
            
            font = option.font;
            font.setPointSizeF(pointSize(11));
            painter->setFont(font);
            QFontMetrics metrics(font);
            
            int mh = metrics.height();
            
            painter->setPen(Qt::black);
            painter->drawText(x + iconW, y + mm, w - iconW, mh, Qt::AlignLeft | Qt::AlignVCenter, dateName);
            
            painter->setPen(QColor(120, 120, 120));
            painter->drawText(x + iconW, y + 2*mm + mh, w - iconW, mh, Qt::AlignLeft | Qt::AlignVCenter, tr("Type") + " : " + plugin->getName() + " | " + tr("Method") + " : " + dateMethodStr);
            painter->drawText(x + iconW, y + 3*mm + 2*mh, w - iconW, mh, Qt::AlignLeft | Qt::AlignVCenter, dateDesc);
            
            painter->setPen(Painting::mainColorLight);
            painter->drawText(x + iconW, y + 4*mm + 3*mh, w - iconW, mh, Qt::AlignLeft | Qt::AlignVCenter, delta);
            
            painter->setPen(QColor(200, 200, 200));
            painter->drawLine(x, y + h, x + w, y + h);
            
            // ------
            
            /*QRect updateRect(x + iconW, y + h - mm - butH, butW, butH);
            QRect calibRect(x + iconW + 2*mm + butW, y + h - mm - butH, butW, butH);
            
            painter->setPen(QColor(150, 150, 150));
            painter->setBrush(QColor(240, 240, 240));
            painter->drawRect(updateRect);
            painter->drawRect(calibRect);
            
            painter->setPen(QColor(40, 40, 40));
            painter->drawText(updateRect, Qt::AlignCenter, tr("Modify"));
            painter->drawText(calibRect, Qt::AlignCenter, tr("Calibrate"));*/
        }
    }
};

#endif