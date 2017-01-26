#ifndef GraphViewPhase_H
#define GraphViewPhase_H

#include "GraphViewResults.h"

class Phase;


class GraphViewPhase: public GraphViewResults
{
    Q_OBJECT
public:
    explicit GraphViewPhase(QWidget *parent = 0);
    virtual ~GraphViewPhase();
    
    void setPhase(Phase* phase);
    void setGraphFont(const QFont& font);
    virtual void setButtonsVisible(const bool visible);
    
    GraphView* mDurationGraph;
    Phase* mPhase;
    
    void generateCurves(TypeGraph typeGraph, Variable variable);
    void updateCurvesToShow(bool showAllChains, const QList<bool>& showChainList, bool showCredibility, bool showCalib, bool showWiggle);


signals:
   // void durationDisplay(bool visible);

protected:
    void paintEvent(QPaintEvent* e);
    void resizeEvent(QResizeEvent* );
    void updateLayout();
    
protected slots:
    void showDuration(bool show);
    void saveGraphData() const; 
    
private:
    Button* mShowDuration;
};

#endif
