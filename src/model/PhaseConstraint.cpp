#include "PhaseConstraint.h"
#include "Phase.h"
#include "Generator.h"


PhaseConstraint::PhaseConstraint():Constraint(),
mGamma(0.f),
mPhaseFrom(0),
mPhaseTo(0),
mGammaType(PhaseConstraint::eGammaUnknown),
mGammaFixed(1),
mGammaMin(0),
mGammaMax(1)
{
    
}

PhaseConstraint::PhaseConstraint(const PhaseConstraint& pc):Constraint()
{
    copyFrom(pc);
}
                
PhaseConstraint& PhaseConstraint::operator=(const PhaseConstraint& pc)
{
    copyFrom(pc);
    return *this;
}

void PhaseConstraint::copyFrom(const Constraint& c)
{
    Constraint::copyFrom(c);
    
    const PhaseConstraint& pc = ((PhaseConstraint&)c);
    
    mGamma = pc.mGamma;
    
    mPhaseFrom = pc.mPhaseFrom;
    mPhaseTo = pc.mPhaseTo;
    
    mGammaType = pc.mGammaType;
    mGammaFixed = pc.mGammaFixed;
    mGammaMin = pc.mGammaMin;
    mGammaMax = pc.mGammaMax;
}

PhaseConstraint::~PhaseConstraint()
{
    
}

PhaseConstraint PhaseConstraint::fromJson(const QJsonObject& json)
{
    PhaseConstraint c;
    c.mId = json[STATE_ID].toInt();
    c.mFromId = json[STATE_CONSTRAINT_BWD_ID].toInt();
    c.mToId = json[STATE_CONSTRAINT_FWD_ID].toInt();
    c.mGammaType = (PhaseConstraint::GammaType)json[STATE_CONSTRAINT_GAMMA_TYPE].toInt();
    c.mGammaFixed = json[STATE_CONSTRAINT_GAMMA_FIXED].toDouble();
    c.mGammaMin = json[STATE_CONSTRAINT_GAMMA_MIN].toDouble();
    c.mGammaMax = json[STATE_CONSTRAINT_GAMMA_MAX].toDouble();
    return c;
}

QJsonObject PhaseConstraint::toJson() const
{
    QJsonObject json = Constraint::toJson();
    json[STATE_ID] = mId;
    json[STATE_CONSTRAINT_BWD_ID] = mFromId;
    json[STATE_CONSTRAINT_FWD_ID] = mToId;
    json[STATE_CONSTRAINT_GAMMA_TYPE] = mGammaType;
    json[STATE_CONSTRAINT_GAMMA_FIXED] = mGammaFixed;
    json[STATE_CONSTRAINT_GAMMA_MIN] = mGammaMin;
    json[STATE_CONSTRAINT_GAMMA_MAX] = mGammaMax;
    return json;
}

void PhaseConstraint::initGamma()
{
    if(mGammaType == eGammaUnknown)
        mGamma = 0;
    else if(mGammaType == eGammaFixed && mGammaFixed != 0)
        mGamma = mGammaFixed;
    else if(mGammaType == eGammaRange && mGammaMax > mGammaMin)
        mGamma = mGammaMin;
}

void PhaseConstraint::updateGamma()
{
    if(mGammaType == eGammaUnknown)
        mGamma = 0;
    else if(mGammaType == eGammaFixed && mGammaFixed != 0)
        mGamma = mGammaFixed;
    else if(mGammaType == eGammaRange && mGammaMax > mGammaMin)
    {
        double max = qMin(mGammaMax, mPhaseTo->mAlpha.mX - mPhaseFrom->mBeta.mX);
        mGamma = Generator::randomUniform(mGammaMin, max);
    }
}

