#ifndef GRAPHVIEWPHASE_H
#define GRAPHVIEWPHASE_H

#include "GraphViewResults.h"

class Phase;


class GraphViewPhase: public GraphViewResults
{
    Q_OBJECT
public:
    explicit GraphViewPhase(QWidget *parent = nullptr);
    virtual ~GraphViewPhase();
    
    void setPhase(Phase* phase);
    void setGraphFont(const QFont& font);
    
    void generateCurves(TypeGraph typeGraph, Variable variable);
    void updateCurvesToShow(bool showAllChains, const QList<bool>& showChainList, bool showCredibility, bool showCalib, bool showWiggle);


protected:
    void paintEvent(QPaintEvent* e);
    void resizeEvent(QResizeEvent* );
    void updateLayout();
    
private:
    Phase* mPhase;

};

#endif
