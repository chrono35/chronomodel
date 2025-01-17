#include "MCMCProgressDialog.h"
#include "MCMCLoopMain.h"
#include <QtWidgets>


MCMCProgressDialog::MCMCProgressDialog(MCMCLoopMain* loop, QWidget* parent, Qt::WindowFlags flags):QDialog(parent, flags),
mLoop(loop)
{
    setWindowTitle(tr("MCMC in progress..."));
    
    // -----------
    
    mLabel1 = new QLabel();
    mLabel2 = new QLabel();
    
    mProgressBar1 = new QProgressBar();
    mProgressBar1->setMinimum(0);
    mProgressBar1->setMaximum(0);
    
    mProgressBar2 = new QProgressBar();
    mProgressBar2->setMinimum(0);
    mProgressBar2->setMaximum(0);
    
    // ----------
    
    QDialogButtonBox* buttonBox = new QDialogButtonBox();
    // mOKBut = buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    mCancelBut = buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
    // mOKBut->setEnabled(false);
    
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mCancelBut, SIGNAL(clicked()), this, SLOT(cancelMCMC()));
    
    // ----------
    
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(mLabel1);
    layout->addWidget(mProgressBar1);
    //layout->addWidget(mLabel2);
    //layout->addWidget(mProgressBar2);
    layout->addWidget(buttonBox);
    setLayout(layout);
    
    setMinimumWidth(700);
    
    // -----------
    
    //connect(mLoop, SIGNAL(messageSent(QString)), this, SLOT(addMessage(QString)));
    //connect(mLoop, SIGNAL(stepChanged(QString, int, int)), this, SLOT(setTitle(QString, int, int)));
    //connect(mLoop, SIGNAL(progressChanged(int)), this, SLOT(setProgress(int)));
    
    connect(mLoop, SIGNAL(finished()), this, SLOT(setFinishedState()));
    connect(mLoop, SIGNAL(finished()), this, SLOT(accept()));
    
    connect(mLoop, SIGNAL(stepChanged(const QString&, int, int)), this, SLOT(setTitle1(const QString&, int, int)));
    connect(mLoop, SIGNAL(stepProgressed(int)), this, SLOT(setProgress1(int)));
}

MCMCProgressDialog::~MCMCProgressDialog()
{
    mLoop->quit();
    //mLoop->wait();
}

int MCMCProgressDialog::startMCMC()
{
    mLoop->start();
    return exec();
}

void MCMCProgressDialog::cancelMCMC()
{
    mLoop->requestInterruption();
    //accept();
}

void MCMCProgressDialog::setTitle1(const QString& message, int minProgress, int maxProgress)
{
    mLabel1->setText(message);
    mProgressBar1->setMinimum(minProgress);
    mProgressBar1->setMaximum(maxProgress);
}

void MCMCProgressDialog::setProgress1(int value)
{
    mProgressBar1->setValue(value);
}

void MCMCProgressDialog::setTitle2(const QString& message, int minProgress, int maxProgress)
{
    mLabel2->setText(message);
    mProgressBar2->setMinimum(minProgress);
    mProgressBar2->setMaximum(maxProgress);
}

void MCMCProgressDialog::setProgress2(int value)
{
    mProgressBar2->setValue(value);
}

void MCMCProgressDialog::setFinishedState()
{
   // mOKBut->setEnabled(true);
    mCancelBut->setEnabled(false);
}

