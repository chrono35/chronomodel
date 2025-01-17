#include "Tabs.h"
#include "Painting.h"
#include <QtWidgets>


Tabs::Tabs(QWidget* parent):QWidget(parent),
mCurrentIndex(-1)
{
    mFont.setPointSizeF(pointSize(11));
}

Tabs::~Tabs()
{
    
}

void Tabs::addTab(const QString& name)
{
    mTabNames.append(name);
    if(mTabNames.size() == 1)
        mCurrentIndex = 0;
}

int Tabs::currentIndex() const
{
    return mCurrentIndex;
}

void Tabs::setTab(int i, bool notify)
{
    mCurrentIndex = i;
    if(notify)
        emit tabClicked(i);
    update();
}

void Tabs::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setFont(mFont);
    
    p.setPen(QColor(100, 100, 100));
    p.setBrush(QColor(200, 200, 200));
    
    for(int i=0; i<mTabNames.size(); ++i)
    {
        if(i != mCurrentIndex)
        {
            QRectF r = mTabRects[i];
            p.drawRect(r);
            p.drawText(r, Qt::AlignCenter, mTabNames[i]);
        }
    }
    
    if(mCurrentIndex != -1)
    {
        p.setPen(Painting::mainColorLight);
        p.setBrush(Qt::white);
        
        p.drawRect(mTabRects[mCurrentIndex].adjusted(0, 0, 0, 2));
        p.drawText(mTabRects[mCurrentIndex], Qt::AlignCenter, mTabNames[mCurrentIndex]);
    }
}

void Tabs::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);
    updateLayout();
}

void Tabs::mousePressEvent(QMouseEvent* e)
{
    for(int i=0; i<mTabNames.size(); ++i)
    {
        if(i != mCurrentIndex && mTabRects[i].contains(e->pos()))
        {
            setTab(i, true);
        }
    }
}

void Tabs::updateLayout()
{
    double m = 10.f;
    mTabRects.clear();
    double x = 1.f;
    double h = height()-1;
    QFontMetrics metrics(mFont);
    
    for(int i=0; i<mTabNames.size(); ++i)
    {
        double w = metrics.width(mTabNames[i]);
        mTabRects.append(QRectF(x, 1, 2*m + w, h));
        x += 2*m + w;
    }
    update();
}
