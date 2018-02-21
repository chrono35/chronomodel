#ifndef PluginGaussRefView_H
#define PluginGaussRefView_H

#if USE_PLUGIN_GAUSS

#include "../GraphViewRefAbstract.h"

class PluginGauss;
class GraphView;
class QVBoxLayout;


class PluginGaussRefView: public GraphViewRefAbstract
{
    Q_OBJECT
public:
    explicit PluginGaussRefView(QWidget* parent = nullptr);
    virtual ~PluginGaussRefView();
    
    void setDate(const Date& d, const ProjectSettings& settings);
    
public slots:
    void zoomX(const double min, const double max);
    void setMarginRight(const int margin);
protected:
    void resizeEvent(QResizeEvent* e);

};

#endif
#endif


