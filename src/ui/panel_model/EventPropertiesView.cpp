 #include "EventPropertiesView.h"
#include "ColorPicker.h"
#include "Event.h"
#include "EventKnown.h"
#include "DatesList.h"
#include "Button.h"
//#include "RadioButton.h"
#include "GraphView.h"
#include "Painting.h"
#include "MainWindow.h"
#include "Project.h"
#include "../PluginAbstract.h"
#include "PluginManager.h"
#include "StdUtilities.h"
#include "QtUtilities.h"
#include "ModelUtilities.h"
#include "PluginOptionsDialog.h"
#include <QtWidgets>


EventPropertiesView::EventPropertiesView(QWidget* parent, Qt::WindowFlags flags):QWidget(parent, flags),
mToolbarH(100)
{
    minimumHeight =0;
    // ------------- commun with defautlt Event and Bound ----------
    mNameLab = new QLabel(tr("Name") + " :");
    
    mNameEdit = new QLineEdit();
    mNameEdit->setStyleSheet("QLineEdit { border-radius: 5px; }");
    mNameEdit->setAlignment(Qt::AlignHCenter);
    
    mColorLab = new QLabel(tr("Color") + " :");
    mColorPicker = new ColorPicker(Qt::black);
    
    mMethodLab = new QLabel(tr("Method") + " :");
    mMethodCombo = new QComboBox();
    
    mMethodCombo->addItem(ModelUtilities::getEventMethodText(Event::eDoubleExp));
    mMethodCombo->addItem(ModelUtilities::getEventMethodText(Event::eBoxMuller));
    mMethodCombo->addItem(ModelUtilities::getEventMethodText(Event::eMHAdaptGauss));
    
    connect(mNameEdit, SIGNAL(editingFinished()), this, SLOT(updateEventName()));
    connect(mColorPicker, SIGNAL(colorChanged(QColor)), this, SLOT(updateEventColor(QColor)));
    connect(mMethodCombo, SIGNAL(activated(int)), this, SLOT(updateEventMethod(int)));
    
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(6);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->addWidget(mNameLab, 0, 0);
    grid->addWidget(mNameEdit, 0, 1);
    grid->addWidget(mColorLab, 1, 0);
    grid->addWidget(mColorPicker, 1, 1);
    grid->addWidget(mMethodLab, 2, 0);
    grid->addWidget(mMethodCombo, 2, 1);
    
    QVBoxLayout* topLayout = new QVBoxLayout();
    topLayout->setContentsMargins(10, 6, 10, 6);
    topLayout->addLayout(grid);
    topLayout->addStretch();
    
    mTopView = new QWidget(this);
    mTopView->setLayout(topLayout);
    mTopView->setFixedHeight(mToolbarH);
    
    
    // Event default propreties Window mEventView
    mEventView = new QWidget(this);
    
    minimumHeight += mEventView->height();
    // -------------
    
    mDatesList = new DatesList(mEventView);
    connect(mDatesList, SIGNAL(calibRequested(const QJsonObject&)), this, SIGNAL(updateCalibRequested(const QJsonObject&)));
    
    // -------------

    const QList<PluginAbstract*>& plugins = PluginManager::getPlugins();

    for(int i=0; i<plugins.size(); ++i)
    {
        
        Button* button = new Button(plugins.at(i)->getName(), mEventView);
        button->setIcon(plugins.at(i)->getIcon());
        button->setFlatVertical();
        connect(button, SIGNAL(clicked()), this, SLOT(createDate()));
        
        minimumHeight+=button->height();
        
        
        if(plugins.at(i)->doesCalibration())
            mPluginButs1.append(button);
        else
            mPluginButs2.append(button);
    }
    
    mDeleteBut = new Button(tr("Delete"), mEventView);
    mDeleteBut->setIcon(QIcon(":delete.png"));
    mDeleteBut->setFlatVertical();
    connect(mDeleteBut, SIGNAL(clicked()), this, SLOT(deleteSelectedDates()));
    minimumHeight+=mDeleteBut->height();
    
    mRecycleBut = new Button(tr("Restore"), mEventView);
    mRecycleBut->setIcon(QIcon(":restore.png"));
    mRecycleBut->setFlatVertical();
    connect(mRecycleBut, SIGNAL(clicked()), this, SLOT(recycleDates()));

    // ---------------
    
    mCalibBut = new Button(tr("Calibrate"), mEventView);
    mCalibBut->setIcon(QIcon(":results_w.png"));
    mCalibBut->setFlatVertical();
    mCalibBut->setCheckable(true);
    
    mCombineBut = new Button(tr("Combine"), mEventView);
    mCombineBut->setFlatVertical();
    mCombineBut->setEnabled(false);

    mSplitBut = new Button(tr("Split"), mEventView);
    mSplitBut->setFlatVertical();
    mSplitBut->setEnabled(false);
    
    connect(mCalibBut, SIGNAL(clicked(bool)), this, SIGNAL(showCalibRequested(bool)));
    connect(mCombineBut, SIGNAL(clicked()), this, SLOT(sendCombineSelectedDates()));
    connect(mSplitBut, SIGNAL(clicked()), this, SLOT(sendSplitDate()));

    // --------------- Case of Event is a Bound -> Bound properties windows---------------------------
    
    mBoundView = new QWidget(this);
    
    mKnownFixedRadio   = new QRadioButton(tr("Fixed"), mBoundView);
    mKnownUniformRadio = new QRadioButton(tr("Uniform"), mBoundView);
    
    connect(mKnownFixedRadio, SIGNAL(clicked())  , this, SLOT(updateKnownType()));
    connect(mKnownUniformRadio, SIGNAL(clicked()), this, SLOT(updateKnownType()));
    
    mKnownFixedEdit = new QLineEdit(mBoundView);
    mKnownStartEdit = new QLineEdit(mBoundView);
    mKnownEndEdit   = new QLineEdit(mBoundView);
    
    QDoubleValidator* doubleValidator = new QDoubleValidator(this);
    doubleValidator->setDecimals(2);
    
    mKnownGraph = new GraphView(mBoundView);
    mKnownGraph->setMinimumHeight(250);
    
    mKnownGraph->setRendering(GraphView::eHD);
    
    mKnownGraph->showXAxisArrow(true);
    mKnownGraph->showXAxisTicks(true);
    mKnownGraph->showXAxisSubTicks(true);
    mKnownGraph->showXAxisValues(true);
    
    mKnownGraph->showYAxisArrow(true);
    mKnownGraph->showYAxisTicks(false);
    mKnownGraph->showYAxisSubTicks(false);
    mKnownGraph->showYAxisValues(false);
    
    mKnownGraph->setXAxisMode(GraphView::eMinMax);
    mKnownGraph->setYAxisMode(GraphView::eMinMax);
    
    connect(mDatesList, &DatesList::itemSelectionChanged, this, &EventPropertiesView::updateCombineAvailability);
    connect(mKnownFixedEdit, &QLineEdit::textEdited, this, &EventPropertiesView::updateKnownFixed);
    connect(mKnownStartEdit, &QLineEdit::textEdited, this, &EventPropertiesView::updateKnownUnifStart);
    connect(mKnownEndEdit, &QLineEdit::textEdited, this, &EventPropertiesView::updateKnownUnifEnd);
    
    QFormLayout* fixedLayout = new QFormLayout();
    fixedLayout->addRow(tr("Value"), mKnownFixedEdit);
    mFixedGroup = new QGroupBox();
    mFixedGroup->setLayout(fixedLayout);
    
    QFormLayout* uniformLayout = new QFormLayout();
    uniformLayout->addRow(tr("Start"), mKnownStartEdit);
    uniformLayout->addRow(tr("End"), mKnownEndEdit);
    mUniformGroup = new QGroupBox();
    mUniformGroup->setLayout(uniformLayout);
    
    QVBoxLayout* boundLayout = new QVBoxLayout();
    boundLayout->setContentsMargins(10, 6, 15, 6);
    boundLayout->setSpacing(10);
    boundLayout->addWidget(mKnownFixedRadio);
    boundLayout->addWidget(mFixedGroup);
    boundLayout->addWidget(mKnownUniformRadio);
    boundLayout->addWidget(mUniformGroup);
    boundLayout->addWidget(mKnownGraph);
    boundLayout->addStretch();
    mBoundView->setLayout(boundLayout);
    
    mEvent = QJsonObject();
    mTopView->setVisible(false);
    mEventView->setVisible(false);
    mBoundView->setVisible(false);
}

EventPropertiesView::~EventPropertiesView()
{
    
}

#pragma mark Event Managment
void EventPropertiesView::setEvent(const QJsonObject& event)
{
    mEvent = event;
    updateEvent();
}

void EventPropertiesView::updateEvent()
{
    qDebug()<<"EventPropertiesView::updateEvent";
    if (this->isVisible() == false)
        return;

    if (mEvent.isEmpty()) {
        mTopView->setVisible(false);
        mEventView->setVisible(false);
        mBoundView->setVisible(false);
        
    } else {
        Event::Type type = (Event::Type)mEvent.value(STATE_EVENT_TYPE).toInt();
        QString name = mEvent.value(STATE_NAME).toString();
        QColor color(mEvent.value(STATE_COLOR_RED).toInt(),
                     mEvent.value(STATE_COLOR_GREEN).toInt(),
                     mEvent.value(STATE_COLOR_BLUE).toInt());
        
        if (name != mNameEdit->text())
            mNameEdit->setText(name);
        mColorPicker->setColor(color);
        
        mMethodLab->setVisible(type == Event::eDefault);
        mMethodCombo->setVisible(type == Event::eDefault);
        
        mTopView->setVisible(true);
        mEventView->setVisible(type == Event::eDefault);
        mBoundView->setVisible(type == Event::eKnown);
        
        if (type == Event::eDefault) {
            mMethodCombo->setCurrentIndex(mEvent.value(STATE_EVENT_METHOD).toInt());
            mDatesList->setEvent(mEvent);
            
            QJsonArray dates = mEvent.value(STATE_EVENT_DATES).toArray();
            bool hasDates = (dates.size() > 0);
            
            mCalibBut->setEnabled(hasDates);
            mDeleteBut->setEnabled(hasDates);
            mRecycleBut->setEnabled(hasDates);
            
        } else if(type == Event::eKnown) {
            EventKnown::KnownType knownType = (EventKnown::KnownType)mEvent.value(STATE_EVENT_KNOWN_TYPE).toInt();
            
            mKnownFixedRadio   -> setChecked(knownType == EventKnown::eFixed);
            mKnownUniformRadio -> setChecked(knownType == EventKnown::eUniform);
            
            mKnownFixedEdit -> setText(QString::number(mEvent.value(STATE_EVENT_KNOWN_FIXED).toDouble()));
            mKnownStartEdit -> setText(QString::number(mEvent.value(STATE_EVENT_KNOWN_START).toDouble()));
            mKnownEndEdit   -> setText(QString::number(mEvent.value(STATE_EVENT_KNOWN_END).toDouble()));
            
            updateKnownControls();
            updateKnownGraph();
        }
    }
    updateLayout();
    update();
}

const QJsonObject& EventPropertiesView::getEvent() const
{
    return mEvent;
}

#pragma mark Event Properties
void EventPropertiesView::updateEventName()
{
    QJsonObject event = mEvent;
    event[STATE_NAME] = mNameEdit->text();
    MainWindow::getInstance()->getProject()->updateEvent(event, tr("Event name updated"));
}

void EventPropertiesView::updateEventColor(QColor color)
{
    QJsonObject event = mEvent;
    event[STATE_COLOR_RED] = color.red();
    event[STATE_COLOR_GREEN] = color.green();
    event[STATE_COLOR_BLUE] = color.blue();
    MainWindow::getInstance()->getProject()->updateEvent(event, tr("Event color updated"));
}

void EventPropertiesView::updateEventMethod(int index)
{
    QJsonObject event = mEvent;
    event[STATE_EVENT_METHOD] = index;
    MainWindow::getInstance()->getProject()->updateEvent(event, tr("Event method updated"));
}

#pragma mark Event Known Properties
void EventPropertiesView::updateKnownType()
{
    if((Event::Type)mEvent.value(STATE_EVENT_TYPE).toInt() == Event::eKnown)
    {
        EventKnown::KnownType type = EventKnown::eFixed;
        if(mKnownUniformRadio->isChecked())
            type = EventKnown::eUniform;
        
        if((EventKnown::KnownType)mEvent.value(STATE_EVENT_KNOWN_TYPE).toInt() != type)
        {
            QJsonObject event = mEvent;
            event[STATE_EVENT_KNOWN_TYPE] = type;
            MainWindow::getInstance()->getProject()->updateEvent(event, tr("Bound type updated"));
        }
    }
}

void EventPropertiesView::updateKnownFixed(const QString& text)
{
    QJsonObject event = mEvent;
    event[STATE_EVENT_KNOWN_FIXED] = round(text.toDouble());
    MainWindow::getInstance()->getProject()->updateEvent(event, tr("Bound fixed value updated"));
}
void EventPropertiesView::updateKnownUnifStart(const QString& text)
{
    QJsonObject event = mEvent;
    event[STATE_EVENT_KNOWN_START] = round(text.toDouble());
    MainWindow::getInstance()->getProject()->updateEvent(event, tr("Bound min updated"));
}

void EventPropertiesView::updateKnownUnifEnd(const QString& text)
{
    QJsonObject event = mEvent;
    event[STATE_EVENT_KNOWN_END] = round(text.toDouble());
    MainWindow::getInstance()->getProject()->updateEvent(event, tr("Bound max updated"));
}

/*void EventPropertiesView::loadKnownCsv()
{

}*/

void EventPropertiesView::updateKnownGraph()
{
    mKnownGraph->removeAllCurves();
    
    //if((Event::Type)mEvent[STATE_EVENT_TYPE].toInt() == Event::eKnown)
    //{
        Project* project = MainWindow::getInstance()->getProject();
        QJsonObject state = project->state();
        QJsonObject settings = state.value(STATE_SETTINGS).toObject();
        const double tmin = settings.value(STATE_SETTINGS_TMIN).toDouble();
        const double tmax = settings.value(STATE_SETTINGS_TMAX).toDouble();
        const double step = settings.value(STATE_SETTINGS_STEP).toDouble();
        EventKnown event = EventKnown::fromJson(mEvent);

        if(  ( (event.mKnownType==EventKnown::eFixed) && ((tmin>event.mFixed) || (event.mFixed>tmax)) )
          || ( (event.mKnownType==EventKnown::eUniform) && (event.mUniformStart>event.mUniformEnd)  )
          || ( (event.mKnownType==EventKnown::eUniform) && (event.mUniformStart>tmax)  )
          || ( (event.mKnownType==EventKnown::eUniform) && (event.mUniformEnd<tmin)  ))
        {

            return;
        }

        event.updateValues(tmin, tmax,step );
        
        mKnownGraph->setRangeX(tmin,tmax);
        
        //qDebug() << "EventPropertiesView::updateKnownGraph()"<<event.mValues.size();
        mKnownGraph->setCurrentX(tmin,tmax);
        
        double max = map_max_value(event.mValues);
        max = (max == 0) ? 1 : max;
        mKnownGraph->setRangeY(0, max);
        mKnownGraph->showYAxisValues(false);

        //---------------------

        GraphCurve curve;
        curve.mName = "Bound";
        curve.mBrush = Painting::mainColorLight;

        curve.mPen = QPen(Painting::mainColorLight, 2.f);
        
        curve.mIsHorizontalSections = true;
        qreal tLower;
        qreal tUpper;
        if(event.mKnownType == EventKnown::eFixed) {
            tLower = event.fixedValue();
            tUpper = tLower;

        }
        else {
            tLower = event.uniformStart();
            tUpper = event.uniformEnd();
        }

        curve.mSections.append(qMakePair(tLower,tUpper));
        mKnownGraph->addCurve(curve);
        //---------------------
    //}
}

void EventPropertiesView::updateKnownControls()
{
    mFixedGroup->setVisible(mKnownFixedRadio->isChecked());
    mUniformGroup->setVisible(mKnownUniformRadio->isChecked());
}

#pragma mark Event Data
void EventPropertiesView::createDate()
{
    if(!mEvent.isEmpty())
    {
        Button* but = dynamic_cast<Button*>(sender());
        if(but)
        {
            Project* project = MainWindow::getInstance()->getProject();
            const QList<PluginAbstract*>& plugins = PluginManager::getPlugins();
            
            for(int i=0; i<plugins.size(); ++i)
            {
                if(plugins.at(i)->getName() == but->text())
                {
                    Date date = project->createDateFromPlugin(plugins.at(i));
                    if(!date.isNull())
                        project->addDate(mEvent.value(STATE_ID).toInt(), date.toJson());
                }
            }
        }
    }
}

void EventPropertiesView::deleteSelectedDates()
{
    QList<QListWidgetItem*> items = mDatesList->selectedItems();
    QList<int> indexes;
    for(int i=0; i<items.size(); ++i)
        indexes.push_back(mDatesList->row(items[i]));
    
    MainWindow::getInstance()->getProject()->deleteDates(mEvent.value(STATE_ID).toInt(), indexes);
}

void EventPropertiesView::recycleDates()
{
    MainWindow::getInstance()->getProject()->recycleDates(mEvent.value(STATE_ID).toInt());
}

#pragma mark Merge / Split
void EventPropertiesView::updateCombineAvailability()
{
    bool mergeable = false;
    bool splittable = false;
    
    QJsonArray dates = mEvent.value(STATE_EVENT_DATES).toArray();
    QList<QListWidgetItem*> items = mDatesList->selectedItems();
    
    if (items.size() == 1) {
        // Split?
        int idx = mDatesList->row(items[0]);
        if (dates.size()>idx ) {
            QJsonObject date = dates.at(idx).toObject();
            if (date.value(STATE_DATE_SUB_DATES).toArray().size() > 0)
                splittable = true;
            
         }
    } else if (items.size() > 1 && dates.size() > 1) {
        // Combine?
        mergeable = true;
        PluginAbstract* plugin = 0;
        
        for (int i=0; i<items.size(); ++i) {
            int idx = mDatesList->row(items.at(i));
            if (idx < dates.size()) {
                QJsonObject date = dates.at(idx).toObject();
                // If selected date already has subdates, it cannot be combined :
                if (date.value(STATE_DATE_SUB_DATES).toArray().size() > 0) {
                    mergeable = false;
                    break;
                }
                // If selected dates have different plugins, they cannot be combined :
                PluginAbstract* plg = PluginManager::getPluginFromId(date.value(STATE_DATE_PLUGIN_ID).toString());
                if (plugin == 0)
                    plugin = plg;
                else if (plg != plugin) {
                    mergeable = false;
                    break;
                }
            }
        }
        // Dates are not combined yet and are from the same plugin.
        // We should now ask the plugin if they are combinable (they use the same ref curve for example...)
        if (mergeable && plugin != 0) {
            // This could be used instead to disable the "combine" button if dates cannot be combined.
            // We prefer letting the user combine them and get an error message explaining why they cannot be combined!
            // Check Plugin14C::mergeDates as example
            //mergeable = plugin->areDatesMergeable(dates);
            
            mergeable = true;
        }
    }
    mCombineBut->setEnabled(mergeable);
    mSplitBut->setEnabled(splittable);
}

void EventPropertiesView::sendCombineSelectedDates()
{
    QJsonArray dates = mEvent.value(STATE_EVENT_DATES).toArray();
    QList<QListWidgetItem*> items = mDatesList->selectedItems();
    QList<int> dateIds;
    
    for (int i=0; i<items.size(); ++i) {
        const int idx = mDatesList->row(items.at(i));
        if (idx < dates.size()) {
            QJsonObject date = dates.at(idx).toObject();
            dateIds.push_back(date.value(STATE_ID).toInt());
        }
    }
    emit combineDatesRequested(mEvent.value(STATE_ID).toInt(), dateIds);
}

void EventPropertiesView::sendSplitDate()
{
    QJsonArray dates = mEvent.value(STATE_EVENT_DATES).toArray();
    QList<QListWidgetItem*> items = mDatesList->selectedItems();
    if (items.size() > 0) {
        int idx = mDatesList->row(items[0]);
        if (idx < dates.size()) {
            QJsonObject date = dates.at(idx).toObject();
            int dateId = date.value(STATE_ID).toInt();
            emit splitDateRequested(mEvent.value(STATE_ID).toInt(), dateId);
        }
    }
}

#pragma mark Layout
void EventPropertiesView::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QWidget::paintEvent(e);
    QPainter p(this);
    p.fillRect(rect(), palette().color(QPalette::Background));
    
    if(mEvent.isEmpty()){
        QFont font = p.font();
        font.setBold(true);
        font.setPointSize(20);
        p.setFont(font);
        p.setPen(QColor(150, 150, 150));
        p.drawText(rect(), Qt::AlignCenter, tr("No event selected !"));
    }
    else{
        p.fillRect(QRect(0, 0, width(), mToolbarH), QColor(200, 200, 200));
    }
}

void EventPropertiesView::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);
    updateLayout();
}

void EventPropertiesView::updateLayout()
{
    mTopView->setGeometry(0, 0, width(), mToolbarH);

    int butPluginWidth = 80;
    int butPluginHeigth = 50;
    
    mEventView->setGeometry(0, mToolbarH, width(), height()-mToolbarH);
    mBoundView->setGeometry(0, mToolbarH, width(), height()-mToolbarH);
    
    QRect listRect(0, 0, mEventView->width() - butPluginWidth, mEventView->height() - butPluginHeigth);
    mDatesList->setGeometry(listRect);
    
    int x = listRect.width();
    int y = listRect.y();
    
    for(int i=0; i<mPluginButs1.size(); ++i)
    {
        mPluginButs1[i]->setGeometry(x, y, butPluginWidth, butPluginHeigth);
        y += butPluginHeigth;
    }
    
    for(int i=0; i<mPluginButs2.size(); ++i)
    {
        mPluginButs2[i]->setGeometry(x, y, butPluginWidth, butPluginHeigth);
        y += butPluginHeigth;
    }
    
    x = listRect.x();
    y = listRect.y() + listRect.height();
    int w = listRect.width() / 5;
    int h = butPluginHeigth;

    mCalibBut->setGeometry(x, y, w, h);
    mDeleteBut ->setGeometry(x + 1*w, y, w, h);
    mRecycleBut->setGeometry(x + 2*w, y, w, h);
    mCombineBut->setGeometry(x + 3*w, y, w, h);
    mSplitBut->setGeometry(x + 4*w, y, w, h);
}

void EventPropertiesView::setCalibChecked(bool checked)
{
    mCalibBut->setChecked(checked);
}

bool EventPropertiesView::hasEvent() const
{
    if(!mEvent.isEmpty()){
        Event::Type type = (Event::Type)mEvent[STATE_EVENT_TYPE].toInt();
        return (type == Event::eDefault);
    }
    return false;
}

bool EventPropertiesView::hasBound() const
{
    if(!mEvent.isEmpty()){
        Event::Type type = (Event::Type)mEvent[STATE_EVENT_TYPE].toInt();
        return (type == Event::eKnown);
    }
    return false;
}

bool EventPropertiesView::hasEventWithDates() const
{
    if(!mEvent.isEmpty()){
        Event::Type type = (Event::Type)mEvent[STATE_EVENT_TYPE].toInt();
        if(type == Event::eDefault){
            const QJsonArray dates = mEvent[STATE_EVENT_DATES].toArray();
            return (dates.size() > 0);
        }
    }
    return false;
}
