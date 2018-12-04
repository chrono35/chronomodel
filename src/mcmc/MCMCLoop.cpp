#include "MCMCLoop.h"
#include "Generator.h"
#include "QtUtilities.h"
#include <QDebug>
#include <QTime>


MCMCLoop::MCMCLoop():
mChainIndex(0),
mState(eBurning),
mProject(nullptr)
{
    
}

MCMCLoop::~MCMCLoop()
{
    mProject = nullptr;
}

void MCMCLoop::setMCMCSettings(const MCMCSettings& s)
{
    mChains.clear();
    for (int i=0; i<s.mNumChains; ++i) {
        ChainSpecs chain;
        
        if (i < s.mSeeds.size())
            chain.mSeed = s.mSeeds.at(i);
        else
            chain.mSeed = Generator::createSeed();
        
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
        chain.mMixingLevel = s.mMixingLevel;
        mChains.append(chain);
    }
}

const QList<ChainSpecs> &MCMCLoop::chains() const
{
    return mChains;
}

const QString& MCMCLoop::getChainsLog() const
{
    return mChainsLog;
}

const QString MCMCLoop::getMCMCSettingsLog() const
{
    QString log;
    int i (0);
    foreach (const ChainSpecs chain, mChains) {
            ++i;
            log += "<hr>";
            log += tr("Chain %1").arg(QString::number(i)) +"<br>";
            log += tr("Seed %1").arg(QString::number(chain.mSeed))+"<br>";
            log += tr("Number of burn-in iterations : %1").arg(QString::number(chain.mBurnIterIndex)) + "<br>";
            log += tr("Number of batches : %1 / %2").arg(QString::number(chain.mBatchIndex), QString::number(chain.mMaxBatchs)) + "<br>";
            log += tr("Number of iterations per batches : %1").arg(QString::number(chain.mNumBatchIter)) + "<br>";
            log += tr("Number of running iterations : %1").arg(QString::number(chain.mRunIterIndex)) + "<br>";
            log += tr("Thinning Interval : %1").arg(QString::number(chain.mThinningInterval)) + "<br>";
            log += tr("Total iterations : %1").arg(QString::number(chain.mTotalIter)) + "<br>";
            log += tr("Mixing level : %1").arg(QString::number(chain.mMixingLevel)) + "<br>";
     }

    return log;
}
const QString MCMCLoop::getInitLog() const
{
    const QString log = getMCMCSettingsLog() + mInitLog;
    return log;
}

void MCMCLoop::run()
{    
    QString mDate = QDateTime::currentDateTime().toString("dddd dd MMMM yyyy");
    QTime startTime = QTime::currentTime();

    QString log= "Start " + mDate+" -> " +startTime.toString("hh:mm:ss.zzz");
    

    //----------------------- Calibrating --------------------------------------
    
    emit stepChanged(tr("Calibrating data..."), 0, 0);
    
    mAbortedReason = this->calibrate();
    if (!mAbortedReason.isEmpty())
        return;

    

    //----------------------- Chains --------------------------------------
    
    QStringList seeds;
    
    mInitLog = QString();
    
    // initVariableForChain() reserve memory space
    initVariablesForChain();

    for (mChainIndex = 0; mChainIndex < mChains.size(); ++mChainIndex) {
        log += "<hr>";
        //log += line("Chain : " + QString::number(mChainIndex + 1) + "/" + QString::number(mChains.size()));

        ChainSpecs& chain = mChains[mChainIndex];
        Generator::initGenerator(chain.mSeed);
        
        log += line("Seed : " + QString::number(chain.mSeed));
        seeds << QString::number(chain.mSeed);      
        
        //----------------------- Initialising --------------------------------------
        
        if (isInterruptionRequested()) {
            mAbortedReason = ABORTED_BY_USER;
            return;
        }
        
        emit stepChanged(tr("Chain %1 / %2").arg(QString::number(mChainIndex+1), QString::number(mChains.size()))  + " : " + tr("Initialising MCMC"), 0, 0);
        
        //QTime startInitTime = QTime::currentTime();
        
        mAbortedReason = this->initMCMC();
        if(!mAbortedReason.isEmpty())
            return;
        
        
        /*QTime endInitTime = QTime::currentTime();
        timeDiff = startInitTime.msecsTo(endInitTime);
        
        log += "=> Init done in " + QString::number(timeDiff) + " ms\n";*/
        
        //----------------------- Burn-in --------------------------------------
        
        emit stepChanged(tr("Chain : %1 / %2").arg(QString::number(mChainIndex + 1), QString::number(mChains.size()))  + " : " + tr("Burn-in"), 0, chain.mNumBurnIter);
        mState = eBurning;
        
        //QTime startBurnTime = QTime::currentTime();
        
        while (chain.mBurnIterIndex < chain.mNumBurnIter) {
            if (isInterruptionRequested()) {
                mAbortedReason = ABORTED_BY_USER;
                return;
            }
            
            try {
                this->update();
            } catch (QString error) {
                mAbortedReason = error;
                return;
            }
            
            ++chain.mBurnIterIndex;
            ++chain.mTotalIter;
            
            emit stepProgressed(chain.mBurnIterIndex);
        }
        

        //----------------------- Adapting --------------------------------------
        
        emit stepChanged(tr("Chain %1 / %2").arg(QString::number(mChainIndex+1), QString::number(mChains.size()))  + " : " + tr("Adapting"), 0, chain.mMaxBatchs * chain.mNumBatchIter);
        mState = eAdapting;     

        while (chain.mBatchIndex * chain.mNumBatchIter < chain.mMaxBatchs * chain.mNumBatchIter) {
            if (isInterruptionRequested()) {
                mAbortedReason = ABORTED_BY_USER;
                return;
            }
            
            chain.mBatchIterIndex = 0;
            while (chain.mBatchIterIndex < chain.mNumBatchIter) {
                if (isInterruptionRequested()) {
                    mAbortedReason = ABORTED_BY_USER;
                    return;
                }

                try {
                    this->update();
                } catch (QString error) {
                    mAbortedReason = error;
                    return;
                }
                
                ++chain.mBatchIterIndex;
                ++chain.mTotalIter;
                
                emit stepProgressed(chain.mBatchIndex * chain.mNumBatchIter + chain.mBatchIterIndex);
            }
            ++chain.mBatchIndex;
            
            if(adapt())
                break;

        }
        //log += line("Adapt OK at batch : " + QString::number(chain.mBatchIndex) + "/" + QString::number(chain.mMaxBatchs));
        
       /* QTime endAdaptTime = QTime::currentTime();
        timeDiff = startAdaptTime.msecsTo(endAdaptTime);
        log += "=> Adapt done in " + QString::number(timeDiff) + " ms\n";*/
        
        //----------------------- Running --------------------------------------
        
        emit stepChanged(tr("Chain %1 / %2").arg(QString::number(mChainIndex+1), QString::number(mChains.size())) + " : " + tr("Running"), 0, chain.mNumRunIter);
        mState = eRunning;

        
        while (chain.mRunIterIndex < chain.mNumRunIter) {
            if (isInterruptionRequested()) {
                mAbortedReason = ABORTED_BY_USER;
                return;
            }

            try {
                this->update();
            } catch (QString error) {
                mAbortedReason = error;
                return;
            }
            
            ++chain.mRunIterIndex;
            ++chain.mTotalIter;

            emit stepProgressed(chain.mRunIterIndex);
        }
        
        /*QTime endRunTime = QTime::currentTime();
        timeDiff = startRunTime.msecsTo(endRunTime);
        log += "=> Acquire done in " + QString::number(timeDiff) + " ms\n";*/
        
        //-----------------------------------------------------------------------
        
       /* QTime endChainTime = QTime::currentTime();
        timeDiff = startChainTime.msecsTo(endChainTime);
        log += "=> Chain done in " + QString::number(timeDiff) + " ms\n";*/



    }
    
    log += line(tr("List of used chain seeds (to be copied for re-use in MCMC Settings) : ") + seeds.join(";"));
    
    
    //-----------------------------------------------------------------------

    emit stepChanged(tr("Computing posterior distributions and numerical results (HPD, credibility, ...)"), 0, 0);
    
    try {
        this->finalize();
    } catch (QString error) {
        mAbortedReason = error;
        return;
    }

    QTime endTime = QTime::currentTime();

    QTime timeDiff(0,0,0,1);
    timeDiff = timeDiff.addMSecs(startTime.elapsed()).addMSecs(-1);

    log += line(tr("Model computed") );
    log += line(tr("finish at %1").arg(endTime.toString("hh:mm:ss.zzz")) );
    log += line(tr("time elapsed %1 h %2 m %3 s %4 ms").arg(QString::number(timeDiff.hour()),
                                                            QString::number(timeDiff.minute()),
                                                            QString::number(timeDiff.second()),
                                                            QString::number(timeDiff.msec()) ));


    //-----------------------------------------------------------------------
    
    mChainsLog = log;
}

