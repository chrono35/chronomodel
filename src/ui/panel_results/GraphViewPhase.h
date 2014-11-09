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
    
    void setVariablesToShow(bool showAlpha, bool showBeta, bool showTau);

private slots:
    void saveGraphData();
    
protected:
    void paintEvent(QPaintEvent* e);
    void refresh();
    
private:
    Phase* mPhase;
    bool mShowAlpha;
    bool mShowBeta;
    bool mShowTau;
};

#endif
