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

#ifndef GRAPHVIEWRESULTS_H
#define GRAPHVIEWRESULTS_H

#include "ProjectSettings.h"
#include "MCMCSettings.h"
#include "MCMCLoop.h"
#include "GraphView.h"

#include <QWidget>
#include <QList>
#include <QColor>
#include <QBrush>
#include <QTextEdit>
#include <QPointer>
#include <QEvent>

class Button;
//class QPropertyAnimation;

class MHVariable;
class MetropolisVariable;

class Overlay : public QWidget {
public:
    Overlay(QWidget * parent = nullptr) : QWidget{parent} {
       setAttribute(Qt::WA_TransparentForMouseEvents);
     //   setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::ToolTip | Qt::WindowStaysOnTopHint);
       setAttribute(Qt::WA_NoSystemBackground, true);
       setAttribute(Qt::WA_TranslucentBackground, true);
    }

protected:

    void paintEvent(QPaintEvent *) override {
        const QColor color (49, 112, 176, 40);
        QFont font (QFont().family(), 30, QFont::Medium, false);

        QPainter p (this);
        p.setFont(font);
        p.fillRect(rect(), color);
        p.setPen(color.darker());
        p.drawText(rect(), Qt::AlignCenter | Qt::TextWordWrap, QObject::tr("Selected"));
        p.end();
    }
};


class GraphViewResults: public QWidget
{
    Q_OBJECT
public:
    enum TypeGraph{
        ePostDistrib = 0,
        eTrace = 1,
        eAccept = 2,
        eCorrel = 3,
        eFunction = 4
    };
    enum Variable{
        eTheta = 0,
        eSigma = 1,
        eDuration = 2,
        eTempo = 3,
        eActivity = 4,
        eVG = 5,
        eG = 6,
        eGP = 7,
        eGS = 8,
        eAlpha = 9
  /*
        eInclination = 8,
        eDeclination = 8,
        eField = 8,
        eDepth = 8,
        eOther = 8*/
    };
    static int mHeightForVisibleAxis ;
    // member
protected:

    Overlay* mOverLaySelect;

    GraphView* mGraph;
    TypeGraph mCurrentTypeGraph;
    Variable mCurrentVariable;

    QString mTitle;

    QString mResultsText;

    QColor mItemColor;
    QString mItemTitle;

    bool mShowAllChains;
    QList<bool> mShowChainList;
    bool mShowCredibility;

    bool mShowCalib;
    bool mShowWiggle;
    bool mShowNumResults;
    bool mIsSelected;
    bool mShowSelectedRect;

    ProjectSettings mSettings;
    MCMCSettings mMCMCSettings;
    QList<ChainSpecs> mChains;

    QColor mMainColor;

    QTextEdit* mTextArea;

    qreal mMargin;
    qreal mLineH;
   // qreal mGraphLeft;
    int mTopShift;



    QFont mGraphFont;
    //-----

public:

    explicit GraphViewResults(QWidget *parent = nullptr);
    virtual ~GraphViewResults();

    virtual void mousePressEvent(QMouseEvent *event);

    void setSettings(const ProjectSettings& settings);
    void setMCMCSettings(const MCMCSettings& mcmc, const QList<ChainSpecs>& chains);

    void setMainColor(const QColor& color);
   // void toggle(const QRect& geometry); //useless
    void setTitle(const QString& title);

    void setMarginLeft (qreal &m);
    void setMarginRight (qreal &m);

   // void setRendering(GraphView::Rendering render);
    virtual void setGraphFont(const QFont& font);
    void setGraphsThickness(int value);
    void setGraphsOpacity(int value);

    void setItemColor(const QColor& itemColor);
    void setItemTitle(const QString& itemTitle);

    bool isSelected() const  { return mIsSelected;}
    void setSelected( const bool&  selected) {
            mIsSelected = selected;
    }

    void showSelectedRect(const bool & show) {
        mShowSelectedRect = show;
    }

    void setShowNumericalResults(const bool show);


    GraphView* getGraph() const { return mGraph; }
    Variable getCurrentVariable() const { return mCurrentVariable; }
    TypeGraph getCurrentType() const { return mCurrentTypeGraph; }
    
   // GraphView::Rendering getRendering() const  { return mGraph->getRendering(); }
    QString getResultsText() const {return mResultsText;}
    QString getTextAreaToHtml() const { return mTextArea->toHtml();}
    QString getTextAreaToPlainText() const { return mTextArea->toPlainText();}

    GraphCurve generateDensityCurve(const QMap<double, double> &data,
                                    const QString& name,
                                    const QColor& lineColor,
                                    const Qt::PenStyle penStyle = Qt::SolidLine,
                                    const QBrush& brush = Qt::NoBrush) const;

    GraphCurve generateHPDCurve(QMap<double, double>& data,
                                const QString& name,
                                const QColor& color) const;

    GraphCurve generateSectionCurve(const QPair<double, double>& section,
                                        const QString& name,
                                        const QColor& color) const;

    GraphCurve generateHorizontalLine(const double yValue,
                                      const QString& name,
                                      const QColor& color,
                                      const Qt::PenStyle penStyle = Qt::SolidLine) const;

    void generateTraceCurves(const QList<ChainSpecs>& chains,
                             MetropolisVariable* variable,
                             const QString& name = QString());

    void generateAcceptCurves(const QList<ChainSpecs>& chains,
                              MHVariable* variable);

    void generateCorrelCurves(const QList<ChainSpecs>& chains,
                              MHVariable* variable);

    // This method is used to recreate all curves in mGraph.
    // It is vitual because we want a different behavior in sub-classes (GraphViewDate, GraphViewEvent and GraphViewPhase)
    virtual void generateCurves(TypeGraph typeGraph, Variable variable);

    // This method is used to update visible existing curves in mGraph.
    // It is vitual because we want a different behavior in suclasses (GraphViewDate, GraphViewEvent and GraphViewPhase)
    virtual void updateCurvesToShow(bool showAllChains, const QList<bool>& showChainList, bool showCredibility, bool showCalib, bool showWiggle);

public slots:
    void setRange(type_data min, type_data max);
    void setCurrentX(type_data min, type_data max);

    void zoom(type_data min, type_data max);
    void showNumericalResults(const bool show);
    void setNumericalResults(const QString& resultsHTML, const QString& resultsText);

    void saveAsImage();
    void imageToClipboard();
    void resultsToClipboard();
    void saveGraphData() const; // must be accessible by ResultsView
    void changeXScaleDivision(const Scale &sc) {mGraph->changeXScaleDivision(sc); update();}
    void changeXScaleDivision(const double &major, const int &minor) {mGraph->changeXScaleDivision(major, minor); update();}

protected:

    // These methods are from QWidget and we want to modify their behavior
    virtual void paintEvent(QPaintEvent* e);
    virtual void resizeEvent(QResizeEvent* e);

    // This is not from QWidget : we create this function to update the layout from different places (eg: in resizeEvent()).
    // It is vitual because we want a different behavior in suclasses (GraphViewDate, GraphViewEvent and GraphViewPhase)
    virtual void updateLayout();

signals:
    void unfoldToggled(bool toggled);
    void visibilityChanged(bool visible);
    void selected();

};




class Filter : public QObject {
    QPointer<Overlay> m_overlay;
    QPointer<QWidget> m_overlayOn;
public:
    Filter(QObject * parent = nullptr) : QObject{parent} {}
protected:
    bool eventFilter(QObject * obj, QEvent * ev) override {
        //if (!obj->isWidgetType()) return false;
        auto w = static_cast<QWidget*>(obj);
        if (ev->type() == QEvent::MouseButtonPress) {
            if (!m_overlay) m_overlay = new Overlay(w->parentWidget());
            m_overlay->setGeometry(w->geometry());
            m_overlayOn = w;
            m_overlay->show();
        }
        else if (ev->type() == QEvent::Resize) {
            if (m_overlay && m_overlayOn == w)
                m_overlay->setGeometry(w->geometry());
        }
        return false;
    }
};


#endif
