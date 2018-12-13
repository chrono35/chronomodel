/* ---------------------------------------------------------------------

Copyright or © or Copr. CNRS	2014 - 2018

Authors :
	Philippe LANOS
	Helori LANOS
 	Philippe DUFRESNE

This software is a computer program whose purpose is to
create chronological models of archeological data using Bayesian statistics.

This software is governed by the CeCILL V2.1 license under French law and
abiding by the rules of distribution of free software.  You can  use,
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability.

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or
data to be ensured and,  more generally, to use and operate it in the
same conditions as regards security.

The fact that you are presently reading this means that you have had
knowledge of the CeCILL V2.1 license and that you accept its terms.
--------------------------------------------------------------------- */

#ifndef MULTICALIBRATIONDRAWING_H
#define MULTICALIBRATIONDRAWING_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QScrollArea>

#include "GraphView.h"
#include "Marker.h"

class ColoredBar: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)

public:
    ColoredBar(QWidget *parent = nullptr);
    ~ColoredBar();
    void setColor(const QColor & color) {mColor = color;}
    QColor color() const {return mColor;}
static int mWidth;
protected:
    void paintEvent(QPaintEvent* e);

private:
    QColor mColor;

};

class MultiCalibrationDrawing: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int graphHeight READ graphHeight WRITE setGraphHeight)
public:
    MultiCalibrationDrawing(QWidget *parent = nullptr);
    ~MultiCalibrationDrawing();
    virtual QPixmap grab();

    void setGraphList(QList<GraphView*> &list);
    void setEventsColorList(QList<QColor> &colorList);
    QList<GraphView*> *getGraphList() {return &mListCalibGraph;}
    void updateLayout();
    void forceRefresh();
    void setGraphHeight(const int & height);
    int graphHeight() const {return mGraphHeight;}

    void hideMarker();
    void showMarker();

    QWidget* getGraphWidget() const {return mGraphWidget;}

private:

    QList<GraphView*> mListCalibGraph;
    QList<QColor> mListEventsColor;
    QList<ColoredBar*> mListBar;

    QScrollArea* mScrollArea;
    QWidget* mGraphWidget;

    Marker* mMarkerX;

    int mVerticalSpacer;

    int mGraphHeight;

    QFont mGraphFont;

    bool mMouseOverCurve;

protected:
  //  void paintEvent(QPaintEvent*);
    void mouseMoveEvent(QMouseEvent* e);
    void resizeEvent(QResizeEvent* e);

};

#endif // MULTICALIBRATIONDRAWING_H
