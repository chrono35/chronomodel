#include "DateDialog.h"
#include "../PluginFormAbstract.h"
#include "Collapsible.h"
#include "RadioButton.h"
#include "Button.h"
#include "Label.h"
#include "HelpWidget.h"
#include "LineEdit.h"
#include "Painting.h"
#include "ModelUtilities.h"
#include "PluginManager.h"
#include "../PluginAbstract.h"
#include <QtWidgets>


DateDialog::DateDialog(QWidget* parent, Qt::WindowFlags flags):QDialog(parent, flags),
mForm(0),
mWidth(600),
mMargin(5),
mLineH(20),
mButW(80),
mButH(25)
{
    setWindowTitle(tr("Create / Modify Data"));
    
    // -----------
    
    mNameLab = new Label(tr("Name") + " :", this);
    
    mNameEdit = new LineEdit(this);
    mNameEdit->setText("<New Date>");
    mNameEdit->selectAll();
    mNameEdit->setFocus();
    
    mAdvancedWidget = new QWidget();
    
    // ----------
    
    mMethodCombo = new QComboBox(mAdvancedWidget);
    mMethodCombo->addItem(ModelUtilities::getDataMethodText(Date::eMHIndependant));
    mMethodCombo->addItem(ModelUtilities::getDataMethodText(Date::eInversion));
    mMethodCombo->addItem(ModelUtilities::getDataMethodText(Date::eMHSymGaussAdapt));
    
    mComboH = mMethodCombo->sizeHint().height();
    
    // ----------
    
    mDeltaFixedRadio = new RadioButton(tr("Wiggle Matching : Fixed"), mAdvancedWidget);
    mDeltaRangeRadio = new RadioButton(tr("Wiggle Matching : Range"), mAdvancedWidget);
    mDeltaGaussRadio = new RadioButton(tr("Wiggle Matching : Gaussian"), mAdvancedWidget);
    
    mMethodLab = new Label(tr("Method") + " :", mAdvancedWidget);
    mDeltaHelp = new HelpWidget(tr("Wiggle Sign : \"+\" if data ≤ event, \"-\" if data ≥ event"), mAdvancedWidget);
    mDeltaFixedLab = new Label(tr("Value") + " :", mAdvancedWidget);
    mDeltaMinLab = new Label(tr("Min") + " :", mAdvancedWidget);
    mDeltaMaxLab = new Label(tr("Max") + " :", mAdvancedWidget);
    mDeltaAverageLab = new Label(tr("Average") + " :", mAdvancedWidget);
    mDeltaErrorLab = new Label(tr("Error") + " :", mAdvancedWidget);
    
    mDeltaFixedEdit = new LineEdit(mAdvancedWidget);
    mDeltaMinEdit = new LineEdit(mAdvancedWidget);
    mDeltaMaxEdit = new LineEdit(mAdvancedWidget);
    mDeltaAverageEdit = new LineEdit(mAdvancedWidget);
    mDeltaErrorEdit = new LineEdit(mAdvancedWidget);
    
    // ----------
    
    mAdvanced = new Collapsible(tr("Advanced"), this);
    mAdvanced->setWidget(mAdvancedWidget, 11*mMargin + 8*mLineH + mComboH + mDeltaHelp->heightForWidth(mWidth - 4*mMargin));
    
    connect(mAdvanced, SIGNAL(collapsing(int)), this, SLOT(adaptSize()));
    
    // ----------
    
    mOkBut = new Button(tr("OK"), this);
    mCancelBut = new Button(tr("Cancel"), this);
    
    connect(mOkBut, SIGNAL(clicked()), this, SLOT(accept()));
    connect(mCancelBut, SIGNAL(clicked()), this, SLOT(reject()));
    
    // ----------
    
    mDeltaFixedRadio->setChecked(true);
    mDeltaFixedEdit->setText(QString::number(0));
    
    adaptSize();
}

DateDialog::~DateDialog()
{
    
}

void DateDialog::setForm(PluginFormAbstract* form)
{
    if(mForm)
    {
        mForm->setVisible(false);
        mForm->setParent(0);
    }
    if(form)
    {
        mForm = form;
        mForm->setParent(this);
        mForm->setVisible(true);
        
        // Check if wiggle is allowed by plugin
        
        PluginAbstract* plugin = form->mPlugin;
        if(plugin->wiggleAllowed())
        {
            mAdvanced->setWidget(mAdvancedWidget, 11*mMargin + 8*mLineH + mComboH + mDeltaHelp->heightForWidth(mWidth - 4*mMargin));
        }
        else
        {
            mAdvanced->setWidget(mAdvancedWidget, 2*mMargin + mComboH);
        }
        
        
        // Disable methods forbidden by plugin
        
        const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(mMethodCombo->model());
        for(int i=0; i<mMethodCombo->count(); ++i)
        {
            QStandardItem* item = model->item(i);
            bool allowed = plugin->allowedDataMethods().contains((Date::DataMethod)i);
            
            item->setFlags(!allowed ? item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled)
                           : Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            // visually disable by greying out - works only if combobox has been painted already and palette returns the wanted color
            item->setData(!allowed ? mMethodCombo->palette().color(QPalette::Disabled, QPalette::Text)
                          : QVariant(), // clear item data in order to use default color
                          Qt::TextColorRole);
        }
        
        adaptSize();
    }
}

void DateDialog::setDataMethod(Date::DataMethod method)
{
    mMethodCombo->setCurrentIndex((int)method);
}

void DateDialog::setDate(const QJsonObject& date)
{
    mNameEdit->setText(date[STATE_NAME].toString());
    mMethodCombo->setCurrentIndex(date[STATE_DATE_METHOD].toInt());
    
    Date::DeltaType deltaType = (Date::DeltaType)date[STATE_DATE_DELTA_TYPE].toInt();
    
    mDeltaFixedRadio->setChecked(deltaType == Date::eDeltaFixed);
    mDeltaRangeRadio->setChecked(deltaType == Date::eDeltaRange);
    mDeltaGaussRadio->setChecked(deltaType == Date::eDeltaGaussian);
    
    mDeltaFixedEdit->setText(QString::number(date[STATE_DATE_DELTA_FIXED].toDouble()));
    mDeltaMinEdit->setText(QString::number(date[STATE_DATE_DELTA_MIN].toDouble()));
    mDeltaMaxEdit->setText(QString::number(date[STATE_DATE_DELTA_MAX].toDouble()));
    mDeltaAverageEdit->setText(QString::number(date[STATE_DATE_DELTA_AVERAGE].toDouble()));
    mDeltaErrorEdit->setText(QString::number(date[STATE_DATE_DELTA_ERROR].toDouble()));
    
    mNameEdit->selectAll();
    mNameEdit->setFocus();
    
    if(mForm)
    {
        mForm->setData(date[STATE_DATE_DATA].toObject());
    }
}

QString DateDialog::getName() const {return mNameEdit->text();}
double DateDialog::getDeltaFixed() const {return mDeltaFixedEdit->text().toDouble();}
double DateDialog::getDeltaMin() const {return mDeltaMinEdit->text().toDouble();}
double DateDialog::getDeltaMax() const {return mDeltaMaxEdit->text().toDouble();}
double DateDialog::getDeltaAverage() const {return mDeltaAverageEdit->text().toDouble();}
double DateDialog::getDeltaError() const {return mDeltaErrorEdit->text().toDouble();}

Date::DataMethod DateDialog::getMethod() const
{
    Date::DataMethod method = Date::eMHIndependant;
    if(mMethodCombo->currentIndex() == 1)
        method = Date::eInversion;
    else if(mMethodCombo->currentIndex() == 2)
        method = Date::eMHSymGaussAdapt;
    return method;
}

Date::DeltaType DateDialog::getDeltaType() const
{
    Date::DeltaType type = Date::eDeltaFixed;
    if(mDeltaRangeRadio->isChecked())
        type = Date::eDeltaRange;
    else if(mDeltaGaussRadio->isChecked())
        type = Date::eDeltaGaussian;
    return type;
}

void DateDialog::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    
    QPainter p(this);
    p.fillRect(rect(), QColor(180, 180, 180));
}

void DateDialog::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);
    updateLayout();
}

void DateDialog::updateLayout()
{
    int m = mMargin;
    int w = width();
    int h = height();
    int w1 = 100;
    int w2 = w - 3*m - w1;
    
    mNameLab->setGeometry(m, m, w1, mLineH);
    mNameEdit->setGeometry(2*m + w1, m, w2, mLineH);
    
    if(mForm)
    {
        mForm->setGeometry(m, 2*m + mLineH, w - 2*m, mForm->height());
        mAdvanced->setGeometry(m, 2*m + mLineH + mForm->height() + m, w - 2*m, mAdvanced->height());
    }
    
    mOkBut->setGeometry(w - 2*m - 2*mButW, h - m - mButH, mButW, mButH);
    mCancelBut->setGeometry(w - m - mButW, h - m - mButH, mButW, mButH);
    
    w2 = w - 5*m - w1;
    
    int i = 1;
    mMethodLab->setGeometry(m, m, w1, mComboH);
    mDeltaHelp->setGeometry(m, 2*m + mComboH, width() - 4*m, mDeltaHelp->heightForWidth(width() - 4*m));
    mDeltaFixedLab->setGeometry(m, 3*m + mComboH + mDeltaHelp->height() + i * (m + mLineH), w1, mLineH); ++i; ++i;
    mDeltaMinLab->setGeometry(m, 3*m + mComboH + mDeltaHelp->height() + i * (m + mLineH), w1, mLineH); ++i;
    mDeltaMaxLab->setGeometry(m, 3*m + mComboH + mDeltaHelp->height() + i * (m + mLineH), w1, mLineH); ++i; ++i;
    mDeltaAverageLab->setGeometry(m, 3*m + mComboH + mDeltaHelp->height() + i * (m + mLineH), w1, mLineH); ++i;
    mDeltaErrorLab->setGeometry(m, 3*m + mComboH + mDeltaHelp->height() + i * (m + mLineH), w1, mLineH); ++i;
    
    i = 0;
    mMethodCombo->setGeometry(2*m + w1, m, w2, mComboH); ++i;
    mDeltaFixedRadio->setGeometry(m, 3*m + mComboH + mDeltaHelp->height(), w - 4*m, mLineH);
    mDeltaFixedEdit->setGeometry(2*m + w1, 3*m + mComboH + mDeltaHelp->height() + i * (m + mLineH), w2, mLineH); ++i;
    mDeltaRangeRadio->setGeometry(m, 3*m + mComboH + mDeltaHelp->height() + i * (m + mLineH), w - 4*m, mLineH); ++i;
    mDeltaMinEdit->setGeometry(2*m + w1, 3*m + mComboH + mDeltaHelp->height() + i * (m + mLineH), w2, mLineH); ++i;
    mDeltaMaxEdit->setGeometry(2*m + w1, 3*m + mComboH + mDeltaHelp->height() + i * (m + mLineH), w2, mLineH); ++i;
    mDeltaGaussRadio->setGeometry(m, 3*m + mComboH + mDeltaHelp->height() + i * (m + mLineH), w - 4*m, mLineH); ++i;
    mDeltaAverageEdit->setGeometry(2*m + w1, 3*m + mComboH + mDeltaHelp->height() + i * (m + mLineH), w2, mLineH); ++i;
    mDeltaErrorEdit->setGeometry(2*m + w1, 3*m + mComboH + mDeltaHelp->height() + i * (m + mLineH), w2, mLineH); ++i;
}

void DateDialog::adaptSize()
{
    QSize s(mWidth, 4*mMargin + 1*mLineH + mButH + mAdvanced->height());
    if(mForm)
    {
        s.setHeight(s.height() + mMargin + mForm->height());
    }
    resize(s);
    setFixedHeight(s.height());
}
