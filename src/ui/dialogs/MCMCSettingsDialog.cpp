#include "MCMCSettingsDialog.h"
#include "Button.h"
#include "Label.h"
#include "LineEdit.h"
#include "Painting.h"
#include "QtUtilities.h"
#include "HelpWidget.h"
#include <QtWidgets>


MCMCSettingsDialog::MCMCSettingsDialog(QWidget* parent, Qt::WindowFlags flags):
QDialog(parent, flags),
  mTotalWidth(600),
  mMargin(5),
  top(65), // y position of the colored box
  lineH(20),
  editW(100.),
  h (115.), // size of the colored box
  butW(80),
  butH(25)
{
    setWindowTitle(tr("MCMC Options"));
    QFont font (QApplication::font());
    mSeedsLab = new Label(tr("Seeds (separated by \";\")") + ": ", this);
    mSeedsEdit = new LineEdit(this);
    mSeedsLab->setFont(font);
    mSeedsEdit->setFont(font);

    mHelp = new HelpWidget(tr("About seeds : each MCMC chain is different from the others because it uses a different seed. By default, seeds are picked randomly. However, you can force the chains to use specific seeds by entering them below. By doing so, you can replicate exactly the same results using the same seeds."), this);
    mHelp->setLink("http://www.chronomodel.fr/Chronomodel_User_Manual.pdf#page=47"); // chapter 4.2 MCMC settings
    mHelp->setFont(font);

    mNumProcEdit = new LineEdit(this);
    mNumBurnEdit = new LineEdit(this);
    mNumProcEdit->setFont(font);
    mNumBurnEdit->setFont(font);

    mMaxBatchesEdit = new LineEdit(this);
    mNumIterEdit = new LineEdit(this);
    mDownSamplingEdit = new LineEdit(this);
    mMaxBatchesEdit->setFont(font);
    mNumIterEdit->setFont(font);
    mDownSamplingEdit->setFont(font);

    QIntValidator* positiveValidator = new QIntValidator(this);
    positiveValidator->setBottom(1);
    
    QIntValidator* chainsValidator = new QIntValidator(this);
    chainsValidator->setRange(1, 5);
    
    mIterPerBatchSpin = new QSpinBox(this);
    mIterPerBatchSpin->setRange(100, 10000);
    mIterPerBatchSpin->setSingleStep(100);
    mIterPerBatchSpin->setFont(font);
    
    mNumProcEdit->setValidator(chainsValidator);
    mNumProcEdit->setPlaceholderText(tr("From 1 to 5"));
    
    mNumIterEdit->setValidator(positiveValidator);
    mNumBurnEdit->setValidator(positiveValidator);
    mMaxBatchesEdit->setValidator(positiveValidator);
    mDownSamplingEdit->setValidator(positiveValidator);
    
    mNumBurnEdit->setAlignment(Qt::AlignCenter);
    mNumIterEdit->setAlignment(Qt::AlignCenter);
    mNumBurnEdit->setAlignment(Qt::AlignCenter);
    mMaxBatchesEdit->setAlignment(Qt::AlignCenter);
    mDownSamplingEdit->setAlignment(Qt::AlignCenter);
    
    mLabelLevel = new Label(tr("Mixing level"),this);
    mLevelEdit = new LineEdit(this);
    mLabelLevel->setFont(font);
    mLevelEdit->setFont(font);

    mOkBut = new Button(tr("OK"), this);
    mCancelBut = new Button(tr("Cancel"), this);
    
    //connect(mOkBut, &Button::clicked, this, &MCMCSettingsDialog::accept);
    connect(mOkBut, &Button::clicked, this, &MCMCSettingsDialog::inputControl);
    connect(this, &MCMCSettingsDialog::inputValided, this, &MCMCSettingsDialog::accept);

    connect(mCancelBut, &Button::clicked, this, &MCMCSettingsDialog::reject);
    

    w = mTotalWidth - 2.*mMargin;

    int fixedHeight = mHelp->heightForWidth(mTotalWidth - 2*mMargin)  + 3*mMargin + butH + lineH + h + top;

    setFixedSize(mTotalWidth, fixedHeight);
}

MCMCSettingsDialog::~MCMCSettingsDialog()
{

}

void MCMCSettingsDialog::setSettings(const MCMCSettings& settings)
{
    const QLocale mLoc=QLocale();
    mNumProcEdit->setText(mLoc.toString(settings.mNumChains));
    mNumIterEdit->setText(mLoc.toString(settings.mNumRunIter));
    mNumBurnEdit->setText(mLoc.toString(settings.mNumBurnIter));
    mMaxBatchesEdit->setText(mLoc.toString(settings.mMaxBatches));
    mIterPerBatchSpin->setValue(settings.mNumBatchIter);
    mDownSamplingEdit->setText(mLoc.toString(settings.mThinningInterval));
    mSeedsEdit->setText(intListToString(settings.mSeeds, ";"));
    
    mLevelEdit->setText(mLoc.toString(settings.mMixingLevel));
}

MCMCSettings MCMCSettingsDialog::getSettings()
{
    const QLocale mLoc = QLocale();
    MCMCSettings settings;
    const unsigned int UN = 1;
    settings.mNumChains = qMax(UN, mNumProcEdit->text().toUInt());

    settings.mNumBurnIter = qMax(UN, mNumBurnEdit->text().toUInt());

    settings.mMaxBatches = qMax(UN, mMaxBatchesEdit->text().toUInt());
    settings.mNumBatchIter = qMax(UN, (unsigned int) mIterPerBatchSpin->value());

    settings.mNumRunIter = qMax((unsigned int) 10, mNumIterEdit->text().toUInt());
    settings.mThinningInterval = qBound(UN, mDownSamplingEdit->text().toUInt(), (unsigned int)floor(settings.mNumRunIter/10) );
    
    settings.mMixingLevel = qBound(0.0001, mLoc.toDouble(mLevelEdit->text()),0.9999);
    
    settings.mSeeds = stringListToIntList(mSeedsEdit->text(), ";");
    
    return settings;
}

void MCMCSettingsDialog::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    
    int m = 5;
    qreal lineH = 20.;
    
    QPainter p(this);
    p.fillRect(rect(), QColor(220, 220, 220));
    
    p.setPen(QColor(50, 50, 50));
    QFont font = p.font();
    font.setWeight(QFont::Bold);
    p.setFont(font);
    
    p.drawText(0, 0, width(), 30, Qt::AlignCenter, tr("MCMC Settings"));
    
    font.setWeight(QFont::Normal);
    //font.setPointSizeF(pointSize(11));
    p.setFont(font);
    
    p.drawText(0, 40, width()/2, lineH, Qt::AlignVCenter | Qt::AlignRight, tr("Number of chains") + " :");
    
    p.setBrush(QColor(235, 115, 100));
    p.drawRect(mBurnRect);
    
    p.setBrush(QColor(250, 180, 90));
    p.drawRect(mAdaptRect);
    
    p.setBrush(QColor(130, 205, 110));
    p.drawRect(mAquireRect);
    
    p.setBrush(QColor(255, 255, 255, 100));
    p.drawRect(mBatch1Rect);
    p.drawRect(mBatchInterRect);
    p.drawRect(mBatchNRect);
    
    p.drawText(mBurnRect.adjusted(0, 0, 0, -mBurnRect.height() + lineH), Qt::AlignCenter, tr("1 - BURN"));
    p.drawText(mAdaptRect.adjusted(0, 0, 0, -mAdaptRect.height() + lineH), Qt::AlignCenter, tr("2 - ADAPT"));
    p.drawText(mAquireRect.adjusted(0, 0, 0, -mAquireRect.height() + lineH), Qt::AlignCenter, tr("3 - ACQUIRE"));
    
    p.drawText(mBurnRect.adjusted(0, 1*lineH, 0, -mBurnRect.height() + 2*lineH), Qt::AlignCenter, tr("Iterations") + " :");
    p.drawText(mAquireRect.adjusted(0, 1*lineH, 0, -mAquireRect.height() + 2*lineH), Qt::AlignCenter, tr("Iterations") + " :");
    p.drawText(mAquireRect.adjusted(0, 3*lineH, 0, -mAquireRect.height() + 4*lineH), Qt::AlignCenter, tr("Thinning interval") + " :");
    
    p.drawText(mBatch1Rect.adjusted(0, 0, 0, -mBatch1Rect.height() + lineH), Qt::AlignCenter, tr("BATCH 1"));
    p.drawText(mBatchInterRect, Qt::AlignCenter, "...");
    p.drawText(mBatchNRect, Qt::AlignCenter, tr("BATCH N"));
    
    p.drawText(mBatch1Rect.adjusted(0, 1*lineH, 0, -mBatch1Rect.height() + 2*lineH), Qt::AlignCenter, tr("Iterations") + " :");
    p.drawText(mAdaptRect.x(), mAdaptRect.y() + lineH + mBatch1Rect.height() + m, mAdaptRect.width()/2, lineH, Qt::AlignVCenter | Qt::AlignRight, tr("Max batches") + " :");
}

void MCMCSettingsDialog::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);
    updateLayout();
}

void MCMCSettingsDialog::updateLayout()
{
    mBurnRect = QRectF(mMargin, top, w * 0.2, h);
    mAdaptRect = QRectF(mMargin + mBurnRect.width(), top, w * 0.4, h);
    mAquireRect = QRectF(mMargin + mBurnRect.width() + mAdaptRect.width(), top, w * 0.4, h);

    mBatch1Rect = QRectF(mAdaptRect.x() + mMargin,
                         mAdaptRect.y() + lineH,
                         (mAdaptRect.width() - 4*mMargin) / 3,
                         mAdaptRect.height() - 2*lineH - 2*mMargin);
    mBatchInterRect = mBatch1Rect.adjusted(mBatch1Rect.width() + mMargin, 0, mBatch1Rect.width() + mMargin, 0);
    mBatchNRect = mBatch1Rect.adjusted(2*mBatch1Rect.width() + 2*mMargin, 0, 2*mBatch1Rect.width() + 2*mMargin, 0);
    
    mNumProcEdit->setGeometry(width()/2 + mMargin, 40, editW, lineH);
    mNumBurnEdit->setGeometry(mBurnRect.x() + (mBurnRect.width() - editW)/2, mBurnRect.y() + 2*lineH, editW, lineH);
    mNumIterEdit->setGeometry(mAquireRect.x() + (mAquireRect.width() - editW)/2, mAquireRect.y() + 2*lineH, editW, lineH);
    mDownSamplingEdit->setGeometry(mAquireRect.x() + (mAquireRect.width() - editW)/2, mAquireRect.y() + 4*lineH, editW, lineH);
    mIterPerBatchSpin->setGeometry(mBatch1Rect.x() + mMargin, mBatch1Rect.y() + 2*lineH, mBatch1Rect.width() - 2*mMargin, lineH);
    mMaxBatchesEdit->setGeometry(mAdaptRect.x() + mAdaptRect.width()/2 + mMargin, mAdaptRect.y() + mAdaptRect.height() - mMargin - lineH, editW, lineH);

    mHelp->setGeometry(mMargin,
                       height() - 3*mMargin - butH - lineH - mHelp->heightForWidth(width() - 2*mMargin),
                       width() - 2*mMargin,
                       mHelp->heightForWidth(width() - 2*mMargin));

    mSeedsLab->setGeometry(width()/2 - mMargin/2 - 300, height() - 2*mMargin - butH - lineH, 200, lineH);
    mSeedsEdit->setGeometry(width()/2 + mMargin/2-100, height() - 2*mMargin - butH - lineH, editW, lineH);

    mLabelLevel->setGeometry(width()/2 + mMargin/2+10, height() - 2*mMargin - butH - lineH, editW, lineH);
    mLevelEdit->setGeometry(width()/2 + mMargin/2+120, height() - 2*mMargin - butH - lineH, 50, lineH);
    
    mOkBut->setGeometry(width() - 2*mMargin - 2*butW, height() - mMargin - butH, butW, butH);
    mCancelBut->setGeometry(width() - mMargin - butW, height() - mMargin - butH, butW, butH);
}

void MCMCSettingsDialog::inputControl()
{
    bool isValided (true);
    bool ok(true);
    const QLocale mLoc = QLocale();
    QString errorMessage;

    MCMCSettings settings;

    settings.mNumChains = mNumProcEdit->text().toUInt(&ok);
    if (ok == false || settings.mNumChains < 1 ) {
        errorMessage = QObject::tr("The number of chain must be bigger than 0");
        isValided = false;
    }

    settings.mNumBurnIter = mNumBurnEdit->text().toUInt(&ok);
    if (ok == false || settings.mNumBurnIter < 1 ) {
        errorMessage = QObject::tr("The number of iteration in the burning must be bigger than 0");
        isValided = false;
    }

    settings.mMaxBatches = mMaxBatchesEdit->text().toUInt(&ok);
    if (ok == false || settings.mMaxBatches < 1 ) {
        errorMessage = QObject::tr("The number of the maximun batches in the adaptation must be bigger than 0");
        isValided = false;
    }

    settings.mNumBatchIter = (unsigned int) mIterPerBatchSpin->value();
    if (settings.mNumBatchIter < 1) {
        errorMessage = QObject::tr("The number of the iteration in one batch of the adaptation must be bigger than 0");
        isValided = false;
    }

    settings.mNumRunIter = mNumIterEdit->text().toUInt(&ok);
    if (ok == false || settings.mNumRunIter < 50) {
        errorMessage = QObject::tr("The number of the iteration in one run must be bigger than 50");
        isValided = false;
    }

    settings.mThinningInterval = mDownSamplingEdit->text().toUInt(&ok);
    if (ok == false || settings.mThinningInterval < 1 || settings.mThinningInterval > (unsigned int)floor(settings.mNumRunIter/40) ) {
        errorMessage = QObject::tr("The thinning interval in one run must be bigger than 1 and smaller than ")
                    + mLoc.toString((unsigned int)floor(settings.mNumRunIter/40));
        isValided = false;
     }


    settings.mMixingLevel = mLoc.toDouble(mLevelEdit->text(), &ok);
    if (ok == false || settings.mMixingLevel < 0.0001 || settings.mMixingLevel > 0.9999 ) {
            errorMessage = QObject::tr("The number of the iteration in one run must be bigger than ")
                        + mLoc.toString(0.0001) + QObject::tr( " and smaller than ")
                        + mLoc.toString(0.9999);
            isValided = false;
     }

    settings.mSeeds = stringListToIntList(mSeedsEdit->text(), ";");
    for (auto seed : settings.mSeeds)
        if (isnan(seed) || seed == 0) {
            errorMessage = QObject::tr("Each seed must be an integer, bigger than 0");
            isValided = false;
        }


    if (isValided)
        emit inputValided();

    else
        QMessageBox::warning(this, tr("Invalid input"),
                                   errorMessage,
                                   QMessageBox::Ok ,
                                   QMessageBox::Ok);

}
