#include "EventKnown.h"
#include "StdUtilities.h"
#include "GraphView.h"
#include "Generator.h"
#include <QObject>


EventKnown::EventKnown():Event(),
mKnownType(eFixed),
mFixed(0),
mUniformStart(0),
mUniformEnd(0)
{
    mType = eKnown;
}

EventKnown::~EventKnown()
{

}
#pragma mark JSON

EventKnown EventKnown::fromJson(const QJsonObject& json)
{
    EventKnown event;
    
    event.mType = (Type)json[STATE_EVENT_TYPE].toInt();
    event.mId = json[STATE_ID].toInt();
    event.mInitName =  json[STATE_NAME].toString();
    event.mInitColor = QColor(json[STATE_COLOR_RED].toInt(),
                           json[STATE_COLOR_GREEN].toInt(),
                           json[STATE_COLOR_BLUE].toInt());
    event.mMethod = (Method)json[STATE_EVENT_METHOD].toInt();
    event.mItemX = json[STATE_ITEM_X].toDouble();
    event.mItemY = json[STATE_ITEM_Y].toDouble();
    event.mIsSelected = json[STATE_IS_SELECTED].toBool();
    event.mIsCurrent = json[STATE_IS_CURRENT].toBool();
    
    event.mKnownType = (EventKnown::KnownType)json[STATE_EVENT_KNOWN_TYPE].toInt();
    event.mFixed = json[STATE_EVENT_KNOWN_FIXED].toDouble();
    event.mUniformStart = json[STATE_EVENT_KNOWN_START].toDouble();
    event.mUniformEnd = json[STATE_EVENT_KNOWN_END].toDouble();
    
    QString eventIdsStr = json[STATE_EVENT_PHASE_IDS].toString();
    if(!eventIdsStr.isEmpty())
    {
        QStringList eventIds = eventIdsStr.split(",");
        for(int i=0; i<eventIds.size(); ++i)
            event.mPhasesIds.append(eventIds[i].toInt());
    }
    
    return event;
}

QJsonObject EventKnown::toJson() const
{
    QJsonObject event;

    event[STATE_EVENT_TYPE] = mType;
    event[STATE_ID] = mId;

    QColor mColor;
    if (mModelJson==NULL) {
        event[STATE_NAME] = mInitName;
        mColor= mInitColor;
    }
    else {
        event[STATE_NAME] = getName();
        mColor=getColor();
    }

    event[STATE_COLOR_RED] = mColor.red();
    event[STATE_COLOR_GREEN] = mColor.green();
    event[STATE_COLOR_BLUE] = mColor.blue();
    
    event[STATE_EVENT_METHOD] = mMethod;
    event[STATE_ITEM_X] = mItemX;
    event[STATE_ITEM_Y] = mItemY;
    event[STATE_IS_SELECTED] = mIsSelected;
    event[STATE_IS_CURRENT] = mIsCurrent;
    
    event[STATE_EVENT_KNOWN_TYPE] = mKnownType;
    event[STATE_EVENT_KNOWN_FIXED] = mFixed;
    event[STATE_EVENT_KNOWN_START] = mUniformStart;
    event[STATE_EVENT_KNOWN_END] = mUniformEnd;
    
    return event;
}

void EventKnown::setKnownType(KnownType type) {mKnownType = type;}
void EventKnown::setFixedValue(const double& value) {mFixed = value;}
void EventKnown::setUniformStart(const double& value) {mUniformStart = value;}
void EventKnown::setUniformEnd(const double& value) {mUniformEnd = value;}

EventKnown::KnownType EventKnown::knownType() const {return mKnownType;}
double EventKnown::fixedValue() const {return mFixed;}
double EventKnown::uniformStart() const {return mUniformStart;}
double EventKnown::uniformEnd() const {return mUniformEnd;}

void EventKnown::updateValues(double tmin, double tmax, double step)
{
    mValues.clear();
    switch(mKnownType)
    {
        case eFixed:
        {
            for(double t=tmin; t<=tmax; t+=step)
                mValues[t] = 0.f;
            mValues[mFixed] = 1.f;
            break;
        }
        case eUniform:
        {
            if(mUniformStart < mUniformEnd)
            {
                for(double t=tmin; t<=tmax; t+=step)
                {
                    double v = (t > mUniformStart && t <= mUniformEnd) ? 1 / (mUniformEnd - mUniformStart) : 0;
                    mValues[t] = v;
                }
            }
            break;
        }
        default:
            break;
    }
    if(mValues.size() == 0)
    {
        for(double t=tmin; t<=tmax; t+=step)
            mValues[t] = 0.f;
    }
}

void EventKnown::generateHistos(const QList<Chain>& chains, int fftLen, double hFactor, double tmin, double tmax)
{
    //QVector<double> subFullTrace = fullRunTrace(chains);
    // copy of MetropolisVariable.fullRunTrace(chains)
   /* QVector<double> subFullTrace(0);
    int shift = 0;
    for(int i=0; i<chains.size(); ++i)
    {
        const Chain& chain = chains[i];

        unsigned long burnAdaptSize = chain.mNumBurnIter + (chain.mBatchIndex * chain.mNumBatchIter);
        unsigned long traceSize = burnAdaptSize + chain.mNumRunIter / chain.mThinningInterval;

        unsigned long runSize = traceSize - burnAdaptSize;

        subFullTrace=mTrace.mid(shift + burnAdaptSize, traceSize - burnAdaptSize);

        shift += traceSize;
    }
    */
    mTheta.mHisto.clear();
    mTheta.mChainsHistos.clear();
    switch(mKnownType)
    {
        case eFixed:
        {
            mTheta.mHisto.insert(mFixed,1);
            for(int i=0; i<chains.size(); ++i)  {
                mTheta.mChainsHistos.append(mTheta.mHisto);
             }
            break;
        }
        case eUniform:
        {
            this->mTheta.generateHistos(chains, fftLen, hFactor, tmin, tmax);
            //troncate ousite mUniformStart < mUniformEnd

           /* auto end = mTheta.mHisto.cend();
            for (auto it = mTheta.mHisto.cbegin(); it != end; ++it)  {
                if(it->key()<mUniformStart || it->key()>mUniformEnd) (*it)[it->key()]=0;
            }

            for(int i=0; i<chains.size(); ++i)  {
                auto end = mTheta.mChainsHistos[i].cend();
                for (auto it = mTheta.mChainsHistos[i].cbegin(); it != end; ++it) {
                    if(it->key()<mUniformStart || it->key()>mUniformEnd) (*it)[it->key()]=0;
                }

            }*/
            break;
        }
        default:
            break;
    }
    /*mTheta.mHisto = generateHisto(subFullTrace, fftLen, hFactor, tmin, tmax);

    mChainsHistos.clear();
    if (mChainsHistos.isEmpty() ) {
        for(int i=0; i<chains.size(); ++i) {
            QVector<double> subTrace = runTraceForChain(chains, i);
            mChainsHistos.append(generateHisto(subTrace, fftLen, hFactor, tmin, tmax));
        }
    }*/
}

void EventKnown::updateTheta(double tmin, double tmax)
{
    switch(mKnownType)
    {
        case eFixed:
        {
            mTheta.tryUpdate(mFixed, 1);
            break;
        }
        case eUniform:
        {
            double min = getThetaMin(tmin);
            double max = getThetaMax(tmax);
            
            min = qMax(mUniformStart, min);
            max = qMin(mUniformEnd, max);
            
            double theta = min + Generator::randomUniform() * (max - min);
            mTheta.tryUpdate(theta, 1);
            break;
        }
        default:
            break;
    }
}

