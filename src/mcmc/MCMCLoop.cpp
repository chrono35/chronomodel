#include "MCMCLoop.h"
#include "Generator.h"
#include <QDebug>
#include <QTime>


MCMCLoop::MCMCLoop():
mChainIndex(0),
mState(eBurning)
{
    
}

MCMCLoop::~MCMCLoop()
{
    
}

void MCMCLoop::setSettings(const MCMCSettings& s)
{
    mChains.clear();
    for(unsigned int i=0; i<s.mNumChains; ++i)
    {
        Chain chain;
        //chain.mSeed = ;
        chain.mNumBurnIter = s.mNumBurnIter;
        chain.mBurnIterIndex = 0;
        chain.mMaxBatchs = s.mMaxBatches;
        chain.mNumBatchIter = s.mNumBatchIter;
        chain.mBatchIterIndex = 0;
        chain.mBatchIndex = 0;
        chain.mNumRunIter = s.mNumRunIter;
        chain.mRunIterIndex = 0;
        chain.mTotalIter = 0;
        chain.mThinningInterval = s.mThinningInterval;
        mChains.append(chain);
    }
}

const QList<Chain>& MCMCLoop::chains()
{
    return mChains;
}

const QString& MCMCLoop::getLog() const
{
    return mLog;
}

void MCMCLoop::run()
{
    mLog = QString();
    int timeDiff = 0;
    
    QTime startTotalTime = QTime::currentTime();
    
    mLog += "\n################################################\n";
    mLog += tr("MCMC START") + "\n";
    mLog += "################################################\n";
    
    //----------------------- Calibrating --------------------------------------
    
    mLog += "------------------------------------------------\n";
    mLog += tr("CALIBRATION") + "\n";
    mLog += "------------------------------------------------\n";
    
    emit stepChanged(tr("Calibrating data..."), 0, 0);
    
    QTime startCalibTime = QTime::currentTime();
    
    this->calibrate();
    
    QTime endCalibTime = QTime::currentTime();
    timeDiff = startCalibTime.msecsTo(endCalibTime);
    mLog += "=> Calib done in " + QString::number(timeDiff) + " ms\n";
    
    //----------------------- Chains --------------------------------------
    
    for(mChainIndex = 0; mChainIndex < mChains.size(); ++mChainIndex)
    {
        mLog += "================================================\n";
        mLog += tr("CHAIN") + " " + QString::number(mChainIndex + 1) + "\n";
        mLog += "================================================\n";
        
        QTime startChainTime = QTime::currentTime();
        
        Chain& chain = mChains[mChainIndex];
        chain.mSeed = Generator::createSeed();
        Generator::initGenerator(chain.mSeed);
        
        mLog += tr("Seed = ") + QString::number(chain.mSeed) + "\n";
        
        this->initVariablesForChain();
        
        //----------------------- Initializing --------------------------------------
        
        mLog += "------------------------------------------------\n";
        mLog += tr("INIT") + "\n";
        mLog += "------------------------------------------------\n";
        
        emit stepChanged("Chain " + QString::number(mChainIndex+1) + " : " + tr("Initializing MCMC"), 0, 0);
        
        QTime startInitTime = QTime::currentTime();
        
        this->initMCMC();
        
        QTime endInitTime = QTime::currentTime();
        timeDiff = startInitTime.msecsTo(endInitTime);
        mLog += "=> Init done in " + QString::number(timeDiff) + " ms\n";
        
        //----------------------- Burning --------------------------------------
        
        mLog += "------------------------------------------------\n";
        mLog += tr("BURN") + "\n";
        mLog += "------------------------------------------------\n";
        
        emit stepChanged("Chain " + QString::number(mChainIndex+1) + " : " + tr("Burning"), 0, chain.mNumBurnIter);
        mState = eBurning;
        
        QTime startBurnTime = QTime::currentTime();
        
        while(chain.mBurnIterIndex < chain.mNumBurnIter)
        {
            if(isInterruptionRequested())
                return;
            
            this->update();
            
            ++chain.mBurnIterIndex;
            ++chain.mTotalIter;
            
            emit stepProgressed(chain.mBurnIterIndex);
        }
        
        QTime endBurnTime = QTime::currentTime();
        timeDiff = startBurnTime.msecsTo(endBurnTime);
        mLog += "=> Burn done in " + QString::number(timeDiff) + " ms\n";
        
        //----------------------- Adapting --------------------------------------
        
        mLog += "------------------------------------------------\n";
        mLog += tr("ADAPT") + "\n";
        mLog += "------------------------------------------------\n";
        
        emit stepChanged("Chain " + QString::number(mChainIndex+1) + " : " + tr("Adapting"), 0, chain.mMaxBatchs * chain.mNumBatchIter);
        mState = eAdapting;
        
        QTime startAdaptTime = QTime::currentTime();
        
        while(chain.mBatchIndex * chain.mNumBatchIter < chain.mMaxBatchs * chain.mNumBatchIter)
        {
            chain.mBatchIterIndex = 0;
            while(chain.mBatchIterIndex < chain.mNumBatchIter)
            {
                if(isInterruptionRequested())
                    return;
                
                this->update();
                
                ++chain.mBatchIterIndex;
                ++chain.mTotalIter;
                
                emit stepProgressed(chain.mBatchIndex * chain.mNumBatchIter + chain.mBatchIterIndex);
            }
            ++chain.mBatchIndex;
            
            if(adapt())
            {
                mLog += "=> Adapt OK at batch " + QString::number(chain.mBatchIndex) + "/" + QString::number(chain.mMaxBatchs) + "\n";
                break;
            }
        }
        
        QTime endAdaptTime = QTime::currentTime();
        timeDiff = startAdaptTime.msecsTo(endAdaptTime);
        mLog += "=> Adapt done in " + QString::number(timeDiff) + " ms\n";
        
        //----------------------- Running --------------------------------------
        
        mLog += "------------------------------------------------\n";
        mLog += tr("ACQUIRE") + "\n";
        mLog += "------------------------------------------------\n";
        
        emit stepChanged("Chain " + QString::number(mChainIndex+1) + " : " + tr("Running"), 0, chain.mNumRunIter);
        mState = eRunning;
        
        QTime startRunTime = QTime::currentTime();
        
        while(chain.mRunIterIndex < chain.mNumRunIter)
        {
            if(isInterruptionRequested())
                return;
            
            this->update();
            
            ++chain.mRunIterIndex;
            ++chain.mTotalIter;
            
            emit stepProgressed(chain.mRunIterIndex);
        }
        
        QTime endRunTime = QTime::currentTime();
        timeDiff = startRunTime.msecsTo(endRunTime);
        mLog += "=> Acquire done in " + QString::number(timeDiff) + " ms\n";
        
        //-----------------------------------------------------------------------
        
        QTime endChainTime = QTime::currentTime();
        timeDiff = startChainTime.msecsTo(endChainTime);
        mLog += "=> Chain done in " + QString::number(timeDiff) + " ms\n";
    }
    
    mLog += "================================================\n\n";
    
    QTime endTotalTime = QTime::currentTime();
    timeDiff = startTotalTime.msecsTo(endTotalTime);
    mLog += "=> MCMC done in " + QString::number(timeDiff) + " ms\n";
    
    //-----------------------------------------------------------------------
    
    QTime startFinalizeTime = QTime::currentTime();
    
    this->finalize();

    QTime endFinalizeTime = QTime::currentTime();
    timeDiff = startFinalizeTime.msecsTo(endFinalizeTime);
    mLog += "=> Histos and results computed in " + QString::number(timeDiff) + " ms\n";
    
    mLog += "\n################################################\n";
    mLog += tr("MCMC END") + "\n";
    mLog += "################################################\n";
    
    //-----------------------------------------------------------------------
    
    qDebug() << mLog;
}

