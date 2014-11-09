#include "MCMCLoopMain.h"

#include "Model.h"
#include "EventKnown.h"
#include "Functions.h"
#include "Generator.h"
#include "StdUtilities.h"
#include "Date.h"
#include "../PluginAbstract.h"

#include <vector>
#include <cmath>
#include <iostream>
#include <random>
#include <QDebug>
#include <QMessageBox>
#include <QApplication>
#include <QTime>


MCMCLoopMain::MCMCLoopMain(Model* model):MCMCLoop(),
mModel(model)
{
    setSettings(mModel->mMCMCSettings);
}

MCMCLoopMain::~MCMCLoopMain()
{

}

void MCMCLoopMain::calibrate()
{
    if(mModel)
    {
        QList<Event>& events = mModel->mEvents;
        double tmin = mModel->mSettings.mTmin;
        double tmax = mModel->mSettings.mTmax;
        double step = mModel->mSettings.mStep;
        
        //----------------- Calibrate measures --------------------------------------
        
        QList<Date*> dates;
        for(int i=0; i<events.size(); ++i)
        {
            int num_dates = (int)events[i].mDates.size();
            for(int j=0; j<num_dates; ++j)
            {
                Date* date = &events[i].mDates[j];
                dates.push_back(date);
            }
        }
        
        emit stepChanged(tr("Calibrating..."), 0, dates.size());
        
        for(int i=0; i<dates.size(); ++i)
        {
            QTime startTime = QTime::currentTime();
            
            dates[i]->calibrate(tmin, tmax, step);
            
            emit stepProgressed(i);
            
            QTime endTime = QTime::currentTime();
            int timeDiff = startTime.msecsTo(endTime);
            mLog += "Data \"" + dates[i]->mName + "\" (" + dates[i]->mPlugin->getName() + ") calibrated in " + QString::number(timeDiff) + " ms\n";
        }
    }
}

void MCMCLoopMain::initVariablesForChain()
{
    Chain& chain = mChains[mChainIndex];
    QList<Event>& events = mModel->mEvents;
    
    for(int i=0; i<events.size(); ++i)
    {
        Event& event = events[i];
        event.mTheta.mLastAccepts.clear();
        event.mTheta.mLastAcceptsLength = chain.mNumBatchIter;
        
        for(int j=0; j<event.mDates.size(); ++j)
        {
            Date& date = event.mDates[j];
            date.mTheta.mLastAccepts.clear();
            date.mTheta.mLastAcceptsLength = chain.mNumBatchIter;
            date.mSigma.mLastAccepts.clear();
            date.mSigma.mLastAcceptsLength = chain.mNumBatchIter;
            date.mDelta.mLastAccepts.clear();
            date.mDelta.mLastAcceptsLength = chain.mNumBatchIter;
        }
    }
}

void MCMCLoopMain::initMCMC()
{
    QList<Event>& events = mModel->mEvents;
    QList<Phase>& phases = mModel->mPhases;
    double tmin = mModel->mSettings.mTmin;
    double tmax = mModel->mSettings.mTmax;
    
    // ----------------------------------------------------------------
    //  Thetas des Mesures
    //  mTheta = mode max de g(theta i) = fonction de calibration
    //  mSigmaThetaMH = variance de G(theta i) = vraissemblance,
    //  (différent de g(theta i) = fonction de calibration)
    // ----------------------------------------------------------------
    int numDates = 0;
    for(int i=0; i<events.size(); ++i)
        numDates += events[i].mDates.size();
    
    emit stepChanged(tr("Initializing dates..."), 0, numDates);

    for(int i=0; i<events.size(); ++i)
    {
        for(int j=0; j<events[i].mDates.size(); ++j)
        {
            Date& date = events[i].mDates[j];
            
            // TODO Init mieux que ça!
            date.updateDelta();
            
            FunctionAnalysis data = analyseFunction(date.mCalibration);
            
            date.mTheta.mX = data.mode;
            date.mTheta.mSigmaMH = sqrt(data.variance);
            
            emit stepProgressed(i);
        }
    }
    
    // ----------------------------------------------------------------
    //  Thetas des Faits
    //  mTheta = moyenne des thetas des dates.
    //  mS02 = moyenne harmonique de variances de G(theta i).
    //  Choix du shrinkage uniforme : 1
    // ----------------------------------------------------------------
    emit stepChanged(tr("Initializing events..."), 0, events.size());
    for(int i=0; i<events.size(); ++i)
    {
        if(events[i].type() == Event::eKnown)
        {
            EventKnown& ek = (EventKnown&)events[i];
            switch (ek.knownType())
            {
                case EventKnown::eFixed:
                    events[i].mTheta.mX = ek.fixedValue();
                    break;
                case EventKnown::eUniform:
                    events[i].mTheta.mX = ek.uniformStart() + (ek.uniformEnd() - ek.uniformStart())/2;
                    break;
                case EventKnown::eGauss:
                    events[i].mTheta.mX = ek.gaussMeasure();
                    break;
                default:
                    break;
            }
        }
        else
        {
            double theta_sum = 0;
            double s02_sum = 0;
            for(int j=0; j<events[i].mDates.size(); ++j)
            {
                theta_sum += events[i].mDates[j].mTheta.mX;
                
                // SO2 est la moyenne harmonique des variances sur les dates calibrées a posteriori
                // On doit donc utiliser "sigma(calib)" (variance de la date calibrée), qui a déjà servi à initialiser mTheta.mSigmaMH
                double sigmaCalib = events[i].mDates[j].mTheta.mSigmaMH;
                s02_sum += 1 / (sigmaCalib * sigmaCalib);
            }
            
            events[i].mS02 = events[i].mDates.size() / s02_sum;
            events[i].mAShrinkage = 1.;
            
            // Moyenne des theta i
            events[i].mTheta.mX = theta_sum / events[i].mDates.size();
            
            // Si la moyenne n'est pas dans le support du fait, on prend la moyenne des theta i qui s'y trouvent
            /*if(events[i].mTheta.mX < events[i].mTmin || events[i].mTheta.mX > events[i].mTmax)
             {
             theta_sum = 0;
             int counter = 0;
             for(unsigned int j=0; j<events[i].mDates.size(); ++j)
             {
             double thetaDate = events[i].mDates[j].mTheta.mX;
             if(thetaDate >= events[i].mTmin && thetaDate <= events[i].mTmax)
             {
             theta_sum += thetaDate;
             ++counter;
             }
             }
             if(counter == 0)
             {
             // Aucun theta i dans le support du fait ! => on prend le centre du support
             events[i].mTheta.mX = (events[i].mTmax - events[i].mTmin) / 2;
             }
             else
             {
             // Moyenne des theta i se trouvant dans le support
             events[i].mTheta.mX = theta_sum / counter;
             }
             }*/
            
            // Init sigmaMH de theta f
            events[i].mTheta.mSigmaMH = sqrt(events[i].mS02);
        }
        
        emit stepProgressed(i);
    }
    
    // ----------------------------------------------------------------
    //  Theta des Phases
    // ----------------------------------------------------------------
    emit stepChanged(tr("Initializing phases..."), 0, phases.size());
    for(int i=0; i<phases.size(); ++i)
    {
        phases[i].mAlpha.mX = Generator::randomUniform(tmin, phases[i].getMinThetaEvents());
        phases[i].mBeta.mX = Generator::randomUniform(phases[i].getMaxThetaEvents(), tmax);
        
        emit stepProgressed(i);
    }
    
    // ----------------------------------------------------------------
    // Vérifier thetas des Faits et alpha beta TODO !!!!!!
    // ----------------------------------------------------------------
    bool verif = false;
    while(!verif)
    {
        verif = true;
        for(int i=0; i<events.size(); ++i)
        {
            Event& event = events[i];
            
            double thetaMin = event.getThetaMin(tmin);
            double thetaMax = event.getThetaMax(tmax);
            
            if(event.mTheta.mX <= thetaMin || event.mTheta.mX >= thetaMax)
            {
                verif = false;
                double theta = thetaMin + (thetaMin + thetaMax) / 2; // random_uniform mieux pour s'en sortir !!!
                event.mTheta.mX = theta;
            }
        }
    }
    
    // ----------------------------------------------------------------
    //  Variance des Mesures
    // ----------------------------------------------------------------
    emit stepChanged(tr("Initializing dates variances..."), 0, numDates);
    
    for(int i=0; i<events.size(); ++i)
    {
        Event& event = events[i];
        for(int j=0; j<event.mDates.size(); ++j)
        {
            Date& date = event.mDates[j];
            
            double diff = abs(date.mTheta.mX - event.mTheta.mX);
            if(diff != 0)
                date.mSigma.mX = diff;
            else
                date.mSigma.mX = date.mTheta.mSigmaMH;
            
            date.mSigma.mSigmaMH = 1.;
            
            emit stepProgressed(i);
        }
    }
    // ----------------------------------------------------------------
    
    for(int i=0; i<events.size(); ++i)
    {
        Event& event = events[i];
        
        mLog += ">> Event : " + event.mName + "\n";
        mLog += "  - theta (value) : " + QString::number(event.mTheta.mX) + "\n";
        mLog += "  - theta (sigma MH) : " + QString::number(event.mTheta.mSigmaMH) + "\n";
        mLog += "  - SO2 : " + QString::number(event.mS02) + "\n";
        mLog += "  - AShrinkage : " + QString::number(event.mAShrinkage) + "\n";
        mLog += "---------------\n";
        
        for(int j=0; j<event.mDates.size(); ++j)
        {
            Date& date = event.mDates[j];
            
            mLog += " > Data : " + date.mName + "\n";
            mLog += "  - theta (value) : " + QString::number(date.mTheta.mX) + "\n";
            mLog += "  - theta (sigma MH) : " + QString::number(date.mTheta.mSigmaMH) + "\n";
            mLog += "  - sigma (value) : " + QString::number(date.mSigma.mX) + "\n";
            mLog += "  - sigma (sigma MH) : " + QString::number(date.mSigma.mSigmaMH) + "\n";
            mLog += "  - delta (value) : " + QString::number(date.mDelta.mX) + "\n";
            mLog += "  - delta (sigma MH) : " + QString::number(date.mDelta.mSigmaMH) + "\n";
            mLog += "--------\n";
        }
    }
    
    for(int i=0; i<phases.size(); ++i)
    {
        Phase& phase = phases[i];
        
        mLog += "---------------\n";
        mLog += ">> Phase : " + phase.mName + "\n";
        mLog += " - alpha : " + QString::number(phase.mAlpha.mX) + "\n";
        mLog += " - beta : " + QString::number(phase.mBeta.mX) + "\n";
        mLog += " - tau : " + QString::number(phase.mTau.mX) + "\n";
    }
}

void MCMCLoopMain::update()
{
    QList<Event>& events = mModel->mEvents;
    QList<Phase>& phases = mModel->mPhases;
    double t_min = mModel->mSettings.mTmin;
    double t_max = mModel->mSettings.mTmax;

    Chain& chain = mChains[mChainIndex];
    
    bool doMemo = (mState != eRunning || (chain.mRunIterIndex % chain.mThinningInterval == 0));
    
    //--------------------- Update Dates -----------------------------------------
    
    for(int i=0; i<events.size(); ++i)
    {
        Event& event = events[i];
        for(int j=0; j<event.mDates.size(); ++j)
        {
            Date& date = events[i].mDates[j];
            
            date.updateDelta();
            date.updateTheta(t_min, t_max, event);
            date.updateSigma(event);
            
            if(doMemo)
            {
                date.mTheta.memo();
                date.mSigma.memo();
                date.mDelta.memo();
                
                date.mTheta.saveCurrentAcceptRate();
                date.mSigma.saveCurrentAcceptRate();
            }
        }
    }

    //--------------------- Update Events -----------------------------------------

    for(int i=0; i<events.size(); ++i)
    {
        Event& event = events[i];
        
        event.updateTheta(t_min, t_max);
        if(doMemo)
        {
            event.mTheta.memo();
            event.mTheta.saveCurrentAcceptRate();
        }
    }

    //--------------------- Update Phases -----------------------------------------

    for(int i=0; i<phases.size(); ++i)
    {
        phases[i].update();
        if(doMemo)
            phases[i].memoAll();
    }
}

bool MCMCLoopMain::adapt()
{
    Chain& chain = mChains[mChainIndex];
    QList<Event>& events = mModel->mEvents;
    
    const double taux_min = 42.;           // taux_min minimal rate of acceptation=42
    const double taux_max = 46.;           // taux_max maximal rate of acceptation=46
    
    bool allOK = true;
    
    //--------------------- Adapt -----------------------------------------
    
    for(int i=0; i<events.size(); ++i)
    {
        Event& event = events[i];
        float delta = (chain.mBatchIndex < 10000) ? 0.01f : (1 / sqrtf(chain.mBatchIndex));
        
        for(int j=0; j<event.mDates.size(); ++j)
        {
            Date& date = event.mDates[j];
            
            //--------------------- Adapt Sigma MH de Theta i -----------------------------------------
            
            if(date.mMethod == Date::eMHSymGaussAdapt)
            {
                float taux = 100.f * date.mTheta.getCurrentAcceptRate();
                if(taux <= taux_min || taux >= taux_max)
                {
                    allOK = false;
                    float sign = (taux <= taux_min) ? -1.f : 1.f;
                    date.mTheta.mSigmaMH *= powf(10.f, sign * delta);
                }
            }
            
            //--------------------- Adapt Sigma MH de Sigma i -----------------------------------------
            
            float taux = 100.f * date.mSigma.getCurrentAcceptRate();
            if(taux <= taux_min || taux >= taux_max)
            {
                allOK = false;
                float sign = (taux <= taux_min) ? -1.f : 1.f;
                date.mSigma.mSigmaMH *= powf(10.f, sign * delta);
            }
        }
        
        //--------------------- Adapt Theta MH de Theta f -----------------------------------------
        
        if(event.mMethod == Event::eMHAdaptGauss)
        {
            float taux = 100.f * event.mTheta.getCurrentAcceptRate();
            if(taux <= taux_min || taux >= taux_max)
            {
                allOK = false;
                float sign = (taux <= taux_min) ? -1.f : 1.f;
                event.mTheta.mSigmaMH *= powf(10.f, sign * delta);
            }
        }
    }
    return allOK;
}

void MCMCLoopMain::finalize()
{
    float tmin = mModel->mSettings.mTmin;
    float tmax = mModel->mSettings.mTmax;
    
    QList<Event>& events = mModel->mEvents;
    QList<Phase>& phases = mModel->mPhases;
    
    for(int i=0; i<events.size(); ++i)
    {
        Event& event = events[i];
        
        event.mTheta.generateFullHisto(mChains, tmin, tmax);
        event.mTheta.generateHistos(mChains, tmin, tmax);
        
        FunctionAnalysis data = analyseFunction(event.mTheta.fullHisto());
        event.mTheta.mHistoMode = data.mode;
        event.mTheta.mHistoMean = data.mean;
        event.mTheta.mHistoVariance = data.variance;
        
        for(int j=0; j<event.mDates.size(); ++j)
        {
            Date& date = event.mDates[j];
            
            date.mTheta.generateFullHisto(mChains, tmin, tmax);
            date.mSigma.generateFullHisto(mChains, 0, tmax - tmin);
            //date.mDelta.generateFullHisto(mChains, tmin, tmax);
            
            date.mTheta.generateHistos(mChains, tmin, tmax);
            date.mSigma.generateHistos(mChains, 0, tmax - tmin);
            //date.mDelta.generateHistos(mChains, tmin, tmax);
            
            FunctionAnalysis data = analyseFunction(date.mTheta.fullHisto());
            date.mTheta.mHistoMode = data.mode;
            date.mTheta.mHistoMean = data.mean;
            date.mTheta.mHistoVariance = data.variance;
        }
    }
    
    for(int i=0; i<phases.size(); ++i)
    {
        Phase& phase = phases[i];
        
        phase.mAlpha.generateFullHisto(mChains, tmin, tmax);
        phase.mBeta.generateFullHisto(mChains, tmin, tmax);
        phase.mTau.generateFullHisto(mChains, tmin, tmax);
        
        phase.mAlpha.generateHistos(mChains, tmin, tmax);
        phase.mBeta.generateHistos(mChains, tmin, tmax);
        phase.mTau.generateHistos(mChains, tmin, tmax);
    }
}


