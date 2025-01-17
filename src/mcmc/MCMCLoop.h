#ifndef MCMCLOOP_H
#define MCMCLOOP_H

#include <QThread>
#include "MCMCSettings.h"

#define ABORTED_BY_USER "Aborted by user"


class MCMCLoop : public QThread
{
    Q_OBJECT
public:
    enum State
    {
        eBurning = 0,
        eAdapting = 1,
        eRunning = 2
    };
    
    MCMCLoop();
    virtual ~MCMCLoop();
    
    void setMCMCSettings(const MCMCSettings& settings);
    const QList<Chain>& chains();
    const QString& getChainsLog() const;
    const QString& getInitLog() const;
    
    void run();
    
signals:
    void stepChanged(QString title, int min, int max);
    void stepProgressed(int value);
    
protected:
    virtual QString calibrate() = 0;
    virtual void initVariablesForChain() = 0;
    virtual void initMCMC() = 0;
    virtual void update() = 0;
    virtual void finalize() = 0;
    virtual bool adapt() = 0;
    
protected:
    QList<Chain> mChains;
    int mChainIndex;
    State mState;
    QString mChainsLog;
    QString mInitLog;
    
public:
    QString mAbortedReason;
};

#endif
