﻿#include "Functions.h"
#include "Generator.h"
#include "StdUtilities.h"
#include "DateUtils.h"
#include <QDebug>
#include <QProgressDialog>
#include <QApplication>
#include <set>
#include <map>
#include <QTime>

// -----------------------------------------------------------------
//  sumP = Sum (pi)
//  sum = Sum (pi * xi)
//  sum2 = Sum (pi * xi^2)
// -----------------------------------------------------------------

/**
 * @brief Product a FunctionAnalysis from a QMap
 * @todo Handle empty function case and null density case (pi = 0)
*/
FunctionAnalysis analyseFunction(const QMap<double, double>& aFunction)
{
    FunctionAnalysis result;
    if (aFunction.isEmpty()) {
        result.max = 0.f;
        result.mode = 0.f;
        result.mean = 0.f;
        result.stddev = -1.f;
        qDebug() << "WARNING : in analyseFunction() aFunction isEmpty !! ";
        return result;
    }

    double max = 0.;
    double mode = 0.;
    double sum = 0.;
    double sum2 = 0.;
    double sumP = 0.;

    float prevY = 0;
    QList<float> uniformXValues;

    QMap<double,double>::const_iterator citer = aFunction.cbegin();
    for (;citer != aFunction.cend(); ++citer) {
        const double x = citer.key();
        const double y = citer.value();

        sumP += y;
        sum += y * x;
        sum2 += y * x * x;

        if (max <= y) {
            max = y;
            if (prevY == y) {
                uniformXValues.append(x);
                int middleIndex = floor(uniformXValues.size()/2);
                mode = uniformXValues.at(middleIndex);
            } else {
                uniformXValues.clear();
                mode = x;
            }
        }
        prevY = y;
    }

    //FunctionAnalysis result;
    result.max = (float) max;
    result.mode = (float) mode;
    result.mean = (float) 0.f;
    result.stddev = (float) 0.f;

    if (sumP != 0) {
        result.mean = sum / sumP;
        float variance = (sum2 / sumP) - pow(result.mean, 2);

        if (variance < 0) {
            qDebug() << "WARNING : in analyseFunction() negative variance found : " << variance<<" return 0";
            variance = -variance;
        }

        result.stddev = sqrt(variance);
    }

    return result;
}
FunctionAnalysis analyseFunction(const QMap<float, float>& aFunction)
{
    FunctionAnalysis result;
    if (aFunction.isEmpty()) {
        result.max = 0.f;
        result.mode = 0.f;
        result.mean = 0.f;
        result.stddev = -1.f;
        qDebug() << "WARNING : in analyseFunction() aFunction isEmpty !! ";
        return result;
    }
    
    float max = 0.;
    float mode = 0.;
    float sum = 0.;
    float sum2 = 0.;
    float sumP = 0.;
    
    float prevY = 0;
    QList<float> uniformXValues;
    
    QMap<float,float>::const_iterator citer = aFunction.cbegin();
    for (;citer != aFunction.cend(); ++citer) {
        const float x = citer.key();
        const float y = citer.value();
        
        sumP += y;
        sum += y * x;
        sum2 += y * x * x;
        
        if (max <= y) {
            max = y;
            if (prevY == y) {
                uniformXValues.append(x);
                int middleIndex = floor(uniformXValues.size()/2);
                mode = uniformXValues.at(middleIndex);
            } else {
                uniformXValues.clear();
                mode = x;
            }
        }
        prevY = y;
    }
    
    //FunctionAnalysis result;
    result.max = max;
    result.mode = mode;
    result.mean = 0.f;
    result.stddev = 0.f;
    
    if (sumP != 0) {
        result.mean = sum / sumP;
        float variance = (sum2 / sumP) - pow(result.mean, 2);
        
        if (variance < 0) {
            qDebug() << "WARNING : in analyseFunction() negative variance found : " << variance<<" return 0";
            variance = -variance;
        }
        
        result.stddev = sqrt(variance);
    }
    
    return result;
}

float dataStd(const QVector<float>& data)
{
    // Work with double precision here because sum2 might be big !
    
    const float s = sum<float>(data);
    const float s2 = sum2<float>(data);
    const float mean = s / data.size();
    const float variance = s2 / data.size() - mean * mean;
    
    if (variance < 0) {
        qDebug() << "WARNING : in dataStd() negative variance found : " << variance<<" return 0";
        return 0.f;
    }
    return (float)sqrt(variance);
}

double shrinkageUniform(const double so2)
{
    //double u = Generator::randomUniform();
    const double u = Generator::randomUniform(0,1);
    return (so2 * (1. - u) / u);
}

/**
 * @brief Return a text from a FunctionAnalysis
 * @see FunctionAnalysis
 */
QString functionAnalysisToString(const FunctionAnalysis& analysis)
{
    QString result;

    if (analysis.stddev<0.)
        result = QObject::tr("No data");
    else {
        result += "MAP : " + DateUtils::dateToString(analysis.mode) + "   ";
        result += "Mean : " + DateUtils::dateToString(analysis.mean) + "   ";
        result += "Std deviation : " +DateUtils::dateToString(analysis.stddev);
    }
    return result;
}

/**
 * @brief Return a text with the value of th Quartiles Q1, Q2 and Q3
 * @see DensityAnalysis
 */
QString densityAnalysisToString(const DensityAnalysis& analysis, const QString& nl)
{
    QString result (QObject::tr("No data"));
    if (analysis.analysis.stddev>=0.) {
        result = functionAnalysisToString(analysis.analysis) + nl;
        result += "Q1 : " + DateUtils::dateToString(analysis.quartiles.Q1) + "   ";
        result += "Q2 (Median) : " + DateUtils::dateToString(analysis.quartiles.Q2) + "   ";
        result += "Q3 : " + DateUtils::dateToString(analysis.quartiles.Q3);
    }
    return result;
}


Quartiles quartilesForTrace(const QVector<float>& trace)
{
    Quartiles quartiles;
    const int n = trace.size();
    if (n<5) {
        quartiles.Q1 = 0.;
        quartiles.Q2 = 0.;
        quartiles.Q3 = 0.;
        return quartiles;
    }

    QVector<float> sorted (trace);
    std::sort(sorted.begin(),sorted.end());
    
    const int q1index = (int) ceil((float)n * 0.25f);
    const int q3index = (int) ceil((float)n * 0.75f);
    
    quartiles.Q1 = sorted.at(q1index);
    quartiles.Q3 = sorted.at(q3index);
    
    if (n % 2 == 0) {
        const int q2indexLow = n / 2;
        const int q2indexUp = q2indexLow + 1;
        
        quartiles.Q2 = sorted.at(q2indexLow) + (sorted.at(q2indexUp) - sorted.at(q2indexLow)) / 2.f;
    } else {
        const int q2index = (int)ceil((float)n * 0.5f);
        quartiles.Q2 = sorted.at(q2index);
    }
    return quartiles;
}

Quartiles quartilesForRepartition(const QVector<float>& repartition, const float tmin, const float step)
{
    Quartiles quartiles;
    if (repartition.size()<5) {
        quartiles.Q1 = 0.;
        quartiles.Q2 = 0.;
        quartiles.Q3 = 0.;
        return quartiles;
    }
    const float q1index = vector_interpolate_idx_for_value(0.25, repartition);
    const float q2index = vector_interpolate_idx_for_value(0.5, repartition);
    const float q3index = vector_interpolate_idx_for_value(0.75, repartition);
    
    quartiles.Q1 = tmin + q1index * step;
    quartiles.Q2 = tmin + q2index * step;
    quartiles.Q3 = tmin + q3index * step;
    
    return quartiles;
}

QPair<float, float> credibilityForTrace(const QVector<float>& trace, float thresh, float& exactThresholdResult,const  QString description)
{
    QPair<float, float> credibility(0.f,0.f);
    exactThresholdResult = 0.f;
    const int n = trace.size();
    if (thresh > 0 && n > 0) {
        float threshold = inRange(0.0f, thresh, 100.0f);
        QVector<float> sorted (trace);
        std::sort(sorted.begin(),sorted.end());
        
        const int numToRemove = (int)floor((float)n * (1.f - threshold / 100.f));
        exactThresholdResult = ((float)n - (float)numToRemove) / (float)n;
        
        float lmin = 0.f;
        int foundJ = 0;

        for (int j=0; j<=numToRemove; ++j) {
            const float l = sorted.at((n - 1) - numToRemove + j) - sorted.at(j);
            if ((lmin == 0.f) || (l < lmin)) {
                foundJ = j;
                lmin = l;
            }
        }
        credibility.first = sorted.at(foundJ);
        credibility.second = sorted.at((n - 1) - numToRemove + foundJ);
    }

    if (credibility.first == credibility.second) {
        //It means : there is only on value
        return QPair<float, float>();
    }
    else return credibility;
}

/**
 * @brief timeRangeFromTraces
 * @param trace1
 * @param trace2
 * @param thresh
 * @param description  compute type 7 R quantile
 * @return
 */
QPair<float, float> timeRangeFromTraces(const QVector<float>& trace1, const QVector<float>& trace2, const float thresh, const QString description)
{
    QPair<float, float> range(- INFINITY, +INFINITY);
#ifdef DEBUG
    QTime startTime (QTime::currentTime());
#endif
    // limit of precision, to accelerate the calculus
    const float epsilonStep = 0.1f/100.f;

    // if thresh is equal 0 then return an QPair=(-INFINITY,+INFINITY)

    const int n = trace1.size();

    if ( (thresh > 0) && (n > 0) && (trace2.size() == n) ) {

        const float gamma = 1.f - thresh/100.f;

        float dMin = INFINITY;

        std::vector<float> traceAlpha (trace1.toStdVector());
        std::vector<float> traceBeta (trace2.size());

        // 1 - map with relation Beta to Alpha
        std::multimap<float,float> betaAlpha;
        for(int i=0; i<trace1.size(); ++i)
            betaAlpha.insert(std::pair<float,float>(trace2.at(i),trace1.at(i)) );

        std::copy(trace2.begin(),trace2.end(),traceBeta.begin());

        // keep the beta trace in the same position of the Alpha, so we need to sort them with there values of alpha
        std::sort(traceBeta.begin(),traceBeta.end(),[&betaAlpha](const float i, const float j){ return betaAlpha.find(i)->second < betaAlpha.find(j)->second  ;} );

        std::sort(traceAlpha.begin(),traceAlpha.end());

        //if (nTarget>= n)
        //    return QPair<float,float>(traceAlpha.at(0),*std::max_element(traceBeta.cbegin(),traceBeta.cend()));

        std::vector<float> betaUpper(n);

        // 2- loop on Epsilon to look for a and b with the smallest length
        for (float epsilon = 0.f; epsilon <= gamma; ) {

            // original calcul according to the article const float ha( (traceAlpha.size()-1)*epsilon +1 );
            // We must decrease of 1 because the array begin at 0
            const float ha( (traceAlpha.size()-1)*epsilon);

            const int haInf( floor(ha) );
            const int haSup( ceil(ha) );

            const float a = traceAlpha.at(haInf) + ( (ha-haInf)*(traceAlpha.at(haSup)-traceAlpha.at(haInf)) );

            // 3 - copy only value of beta with alpha greater than a(epsilon)
            const int alphaIdx = ha==haInf ? haInf:haSup;

            const int remainingElemt =  n - alphaIdx;
            betaUpper.resize(remainingElemt);   // allocate space

            // traceBeta is sorted with the value alpha join
            auto it = std::copy( traceBeta.begin()+ alphaIdx, traceBeta.end(), betaUpper.begin() );

            const int betaUpperSize = std::distance(betaUpper.begin(),it);

            betaUpper.resize(betaUpperSize);  // shrink container to new size

            /*  std::nth_element has O(N) complexity,
             *  whereas std::sort has O(Nlog(N)).
             *  here we don't need complete sorting of the range, so it's advantageous to use it.
             *
             * std::nth_element(betaUpper.begin(), betaUpper.begin() + nTarget-1, betaUpper.end());
             *
             * in the future with C++17
             * std::experimental::parallel::nth_element(par,betaUpper.begin(), betaUpper.begin() + nTarget, betaUpper.end());
             *
             */

            // 4- We sort all the array
            std::sort(betaUpper.begin(), betaUpper.end());

           // 5 - Calcul b
            const float bEpsilon( (1.-gamma)/(1.-epsilon) );
            // original calcul according to the article const float hb( (betaUpper.size()-1)*bEpsilon +1 );
            // We must decrease of 1 because the array begin at 0
            const float hb( (betaUpper.size()-1)*bEpsilon);
            const int hbInf( floor(hb) );
            const int hbSup( ceil(hb) );

            const float b = betaUpper.at(hbInf) + ( (hb-hbInf)*(betaUpper.at(hbSup)-betaUpper.at(hbInf)) );

            // 6 - keep the shortest length

            if ((b-a) < dMin) {
                dMin = b - a;
                range.first = a;
                range.second = b;



            }

            epsilon += epsilonStep;
        }
    }


#ifdef DEBUG
    qDebug()<<description;
    QTime timeDiff(0,0,0,1);
    timeDiff = timeDiff.addMSecs(startTime.elapsed()).addMSecs(-1);
    qDebug()<<"timeRangeFromTraces ->time elapsed = "<<timeDiff.hour()<<"h "<<QString::number(timeDiff.minute())<<"m "<<QString::number(timeDiff.second())<<"s "<<QString::number(timeDiff.msec())<<"ms" ;
#endif
    return range;
}


QPair<float, float> timeRangeFromTraces_old(const QVector<float>& trace1, const QVector<float>& trace2, const float thresh, const QString description)
{
    QPair<float, float> range(- INFINITY, +INFINITY);
#ifdef DEBUG
    QTime startTime (QTime::currentTime());
#endif
    // limit of precision, to accelerate the calculus
    const float perCentStep = 0.01f;
    // if thresh is equal 0 then return an QPair=(-INFINITY,+INFINITY)

    const int n = trace1.size();
    if ( (thresh > 0) && (n > 0) && (trace2.size() == n) ) {

        const int nTarget = (const int)(ceil((float)n * thresh/100.f));
        const int nGamma = n - nTarget;

        float dMin = INFINITY;

        std::vector<float> traceAlpha (trace1.toStdVector());
        std::vector<float> traceBeta (trace2.size());

        // map with relation Beta to Alpha
        std::multimap<float,float> betaAlpha;
        for(int i=0; i<trace1.size(); ++i)
            betaAlpha.insert(std::pair<float,float>(trace2.at(i),trace1.at(i)) );//std::pair<char,int>('a',100)

        std::copy(trace2.begin(),trace2.end(),traceBeta.begin());

        // keep the beta trace in the same position of the Alpha, so we need to sort them with there values of alpha
        std::sort(traceBeta.begin(),traceBeta.end(),[&betaAlpha](const float i, const float j){ return betaAlpha.find(i)->second <betaAlpha.find(j)->second  ;} );

        std::sort(traceAlpha.begin(),traceAlpha.end());

        if (nTarget>= n)
            return QPair<float,float>(traceAlpha.at(0),*std::max_element(traceBeta.cbegin(),traceBeta.cend()));

        const int epsilonStep = qMax(1, (int)floor(n*perCentStep));

        std::vector<float> betaUpper(n);

        for (int nEpsilon=0; nEpsilon<nGamma; ) {

            const float a = traceAlpha.at(nEpsilon);

            // copy only value of beta with alpha greater than a(epsilon)

            const int remainingElemt =  n - nEpsilon;
            betaUpper.resize(remainingElemt);   // allocate space

            auto it = std::copy( traceBeta.begin()+ nEpsilon+1, traceBeta.end(), betaUpper.begin() );

            const int betaUpperSize = std::distance(betaUpper.begin(),it);

            betaUpper.resize(betaUpperSize);  // shrink container to new size

            // if there is Beta value under a, we could have less than nTarget-1 elements, so it's finish
            if (betaUpperSize<(nTarget-1))
                 break;

            /*  std::nth_element has O(N) complexity,
             *  whereas std::sort has O(Nlog(N)).
             *  here we don't need complete sorting of the range, so it's advantageous to use it.
             */

            std::nth_element(betaUpper.begin(), betaUpper.begin() + nTarget-1, betaUpper.end());

// in the future with C++17
//std::experimental::parallel::nth_element(par,betaUpper.begin(), betaUpper.begin() + nTarget, betaUpper.end());

            // Remember ->  m*(n-nGamma)/(n-nEpsilon) = nTarget
            const float b  = betaUpper.at(nTarget-1);

            // keep the shortest length
            float aInterpolate(0);
            if ( (nEpsilon-epsilonStep+1) < traceAlpha.size() ){
                float h = ((traceAlpha.size()-1)*nEpsilon/n)+1;
                aInterpolate = traceAlpha.at((int)floor(h)-1) + ( (h-floor(h))*(traceAlpha.at((int)floor(h)+1-1)-traceAlpha.at((int)floor(h)-1)) );
                // aInterpolate = interpolate(thresh/100.f, (float)(n-nEpsilon-epsilonStep)/n, (float)((n-nEpsilon)/n), traceAlpha.at(nEpsilon-epsilonStep+1), a);

            } else aInterpolate =  a;

            float bInterpolate(0) ;
            if ( (nTarget-1) < betaUpper.size() ) {
                float h = ((betaUpper.size()-1)*thresh/100.f)+1;
                bInterpolate = betaUpper.at((int)floor(h)-1) + ( (h-floor(h))*(betaUpper.at((int)floor(h)+1-1)-betaUpper.at((int)floor(h)-1)) );
                //   bInterpolate = interpolate(thresh/100.f, (float)(nTarget)/n, (float)((nTarget-1)/n), b, betaUpper.at(nTarget-1-1));

            } else bInterpolate = b;

            if ((b-a) < dMin) {
                dMin = b - a;
                range.first = a;//Interpolate;
                range.second = b;//Interpolate;

                // compute type 7 R quantile
              /*
                const float ha = n*(thresh/100);
                const int floorHa = qMin((int)nEpsilon,(int)(traceAlpha.size()-1-epsilonStep));//(int)floor(ha);

                const double a7 = traceAlpha.at(floorHa)+((ha-floor(ha))*(traceAlpha.at(floorHa+epsilonStep)-traceAlpha.at(floorHa)));

               const float hb = n*(thresh/100);//(n-1)*(nEpsilon/n)+1;
               const int floorHb = qMin(nTarget-1,(int)(betaUpper.size()-1-epsilonStep));//(int)floor(hb);

               const double b7 = betaUpper.at(floorHb)+((hb-floor(hb))*(betaUpper.at(floorHb+epsilonStep)-betaUpper.at(floorHb)));
               range.first = a7;
               range.second = b7;*/
            }

            nEpsilon += epsilonStep;
        }
    }


#ifdef DEBUG
    qDebug()<<description;
    QTime timeDiff(0,0,0,1);
    timeDiff = timeDiff.addMSecs(startTime.elapsed()).addMSecs(-1);
    qDebug()<<"timeRangeFromTraces ->time elapsed = "<<timeDiff.hour()<<"h "<<QString::number(timeDiff.minute())<<"m "<<QString::number(timeDiff.second())<<"s "<<QString::number(timeDiff.msec())<<"ms" ;
#endif
    return range;
}

QPair<float, float> transitionRangeFromTraces(const QVector<float>& trace1, const QVector<float>& trace2, const float thresh, const QString description)
{
    return timeRangeFromTraces(trace1, trace2, thresh, description);
}


/**
 * @brief gapRangeFromTraces find the gap between two traces, if there is no solution corresponding to the threshold, we return a QPair=(-INFINITY,+INFINITY)
 * @param traceBeta QVector of double corresponding to the first trace
 * @param traceAlpha QVector of double corresponding to the second trace
 * @param thresh Threshold to obtain
 * @param description a simple text
 * @return
 */
QPair<float, float> gapRangeFromTraces(const QVector<float>& traceEnd, const QVector<float>& traceBegin, const float thresh, const QString description)
{
#ifdef DEBUG
    QTime startTime = QTime::currentTime();
#endif

    QPair<float, float> range = QPair<float, float>(- INFINITY, + INFINITY);

    // limit of precision, to accelerate the calculus
    const float epsilonStep = 0.1f/100.f;

    // if thresh is equal 0 then return an QPair=(-INFINITY,+INFINITY)

    const int n = traceBegin.size();

    if ( (thresh > 0) && (n > 0) && (traceEnd.size() == n) ) {

        const double gamma = 1. - thresh/100.;

        float dMax(0.f);

        std::vector<float> traceBeta (traceEnd.toStdVector());

        std::vector<float> traceAlpha (traceBegin.size());
        std::copy(traceBegin.begin(),traceBegin.end(),traceAlpha.begin());

        // 1 - map with relation Alpha to Beta
        std::multimap<float,float> alphaBeta;
        for(int i=0; i<traceBegin.size(); ++i)
            alphaBeta.insert(std::pair<float,float>(traceAlpha.at(i),traceBeta.at(i)) );

        // keep the beta trace in the same position of the Alpha, so we need to sort them with there values of alpha
        std::sort(traceAlpha.begin(),traceAlpha.end(),[&alphaBeta](const float i, const float j){ return alphaBeta.find(i)->second < alphaBeta.find(j)->second  ;} );

        std::sort(traceBeta.begin(),traceBeta.end());

        std::vector<float> alphaUnder(n);

        // 2- loop on Epsilon to look for a and b with the smallest length

        for (float epsilon = 0.f; epsilon <= gamma; ) {

            const double aEpsilon = 1. - epsilon;

            // Linear intertpolation according to R quantile( type=7)
            // We must decrease of 1 because the array begin at 0
            const double ha( (double)(traceBeta.size()-1) * aEpsilon);

            const int haInf( floor(ha) );
            const int haSup( ceil(ha) );

            const float a = traceBeta.at(haInf) + ( (ha-(double)haInf)*(traceBeta.at(haSup)-traceBeta.at(haInf)) );

            // 3 - copy only value of beta with alpha smaller than a(epsilon)!
            const int alphaIdx ( ha == haInf ? haInf : haSup );//( ha == haSup ? haSup : haInf );//

            const int remainingElemt ( alphaIdx );
            alphaUnder.resize(remainingElemt);   // allocate space

            // traceAlpha is sorted with the value alpha join
            auto it = std::copy( traceAlpha.begin(), traceAlpha.begin() + alphaIdx, alphaUnder.begin() );

            const int alphaUnderSize = std::distance(alphaUnder.begin(),it);

            alphaUnder.resize(alphaUnderSize);  // shrink container to new size

            // 4- We sort all the array
            std::sort(alphaUnder.begin(), alphaUnder.end());

           // 5 - Calcul b
            const double bEpsilon( (gamma-epsilon)/(1.-epsilon) );

            // Linear intertpolation like in R quantile( type=7)

            const double hb( (double)(alphaUnder.size()-1)* bEpsilon );
            const int hbInf( floor(hb) );
            const int hbSup( ceil(hb) );

            if (hbSup >= alphaUnder.size())
                break;

            const float b = alphaUnder.at(hbInf) + ( (hb-(float)hbInf)*(alphaUnder.at(hbSup)-alphaUnder.at(hbInf)) );

            // 6 - keep the longest length

            if ((b-a) > dMax) {
                dMax = b - a;
                range.first = a;
                range.second = b;
            }
            epsilon += epsilonStep;
        }
    }

#ifdef DEBUG
    qDebug()<<description;
    QTime timeDiff(0,0,0,1);
    timeDiff = timeDiff.addMSecs(startTime.elapsed()).addMSecs(-1);
    qDebug()<<"gapRangeFromTraces ->time elapsed = "<<timeDiff.hour()<<"h "<<QString::number(timeDiff.minute())<<"m "<<QString::number(timeDiff.second())<<"s "<<QString::number(timeDiff.msec())<<"ms" ;
#endif

    return range;
}

QPair<float, float> gapRangeFromTraces_old(const QVector<float>& traceBeta, const QVector<float>& traceAlpha, const float thresh, const QString description)
{
    QPair<float, float> range = QPair<float, float>(- INFINITY, + INFINITY);
#ifdef DEBUG
    QTime startTime = QTime::currentTime();
#endif
    // limit of precision, to accelerate the calculus
    const float perCentStep = 0.01f;

    const int n = traceBeta.size();
    if ( (thresh > 0) && (n > 0) && (traceAlpha.size() == n) ) {

        const int nTarget = (const int) ceil( (float)n * thresh/100.);
        const int nGamma = n - nTarget;

        float dMax = 0.0;

        // make couple beta vs alpha in a std::map, it's a sorted container with ">"
        std::multimap<float,float> mapPair;

        QVector<float>::const_iterator ctB = traceBeta.cbegin();
        QVector<float>::const_iterator ctA = traceAlpha.cbegin();

        while (ctB != traceBeta.cend() ) {
            mapPair.insert(std::pair<float,float>(*ctB,*ctA));
            ++ctA;
            ++ctB;
        }

        // we suppose there is never the several time the same value inside traceBeta or inside traceAlpha
        // so we can just shift the iterator
        std::multimap<float,float>::const_reverse_iterator i_shift = mapPair.rbegin();

        std::vector<float> alphaUnder;
        alphaUnder.reserve(n);
        std::multimap<float,float>::const_reverse_iterator iMapTmp = i_shift;


        const int epsilonStep = qMax(1, (int)floor(nGamma*perCentStep));

        for (int nEpsilon = 0; (nEpsilon <= nGamma ) && (i_shift != mapPair.rend()); ) {

            // We use a reverse Iterator so the first is the last value in the QMap
            const float a = (*i_shift).first;//a=beta(i)=a(epsilon);

            // find position of beta egual a(epsilon)
            iMapTmp = i_shift;
            alphaUnder.clear();
            while (iMapTmp != mapPair.rend()) {
                alphaUnder.push_back((*iMapTmp).second);
                ++iMapTmp;
            }
            //std::sort(alphaUnder.begin(),alphaUnder.end());

            const int j = nGamma - nEpsilon;
            std::nth_element(alphaUnder.begin(), alphaUnder.begin() + j, alphaUnder.end());

            const float b = alphaUnder.at(j); //b=alpha(j)
            // keep the longest length
            if ((b-a) > dMax) {
                dMax = b-a;
                range.first = a;
                range.second = b;
            }

            nEpsilon = nEpsilon + epsilonStep;
            if (nEpsilon<=nGamma)
                std::advance(i_shift,epsilonStep);// reverse_iterator

        }

    }

#ifdef DEBUG
    qDebug()<<description;
    QTime timeDiff(0,0,0,1);
    timeDiff = timeDiff.addMSecs(startTime.elapsed()).addMSecs(-1);
    qDebug()<<"gapRangeFromTraces ->time elapsed = "<<timeDiff.hour()<<"h "<<QString::number(timeDiff.minute())<<"m "<<QString::number(timeDiff.second())<<"s "<<QString::number(timeDiff.msec())<<"ms" ;
#endif
    return range;
}



QString intervalText(const QPair<float, QPair<float, float> >& interval, FormatFunc formatFunc)
{
    const QLocale locale;
    if (formatFunc){
        return "[" + formatFunc(interval.second.first) + "; " + formatFunc(interval.second.second) + "] (" + locale.toString(interval.first, 'f', 1) + "%)";
    }
    else {
        return "[" + DateUtils::dateToString(interval.second.first) + "; " + DateUtils::dateToString(interval.second.second) + "] (" + locale.toString(interval.first, 'f', 1) + "%)";
    }
}

QString getHPDText(const QMap<float, float>& hpd, float thresh, const QString& unit, FormatFunc formatFunc)
{
    if(hpd.isEmpty() ) return "";

    QList<QPair<float, QPair<float, float> > > intervals = intervalsForHpd(hpd, thresh);
    QStringList results;
    for(int i=0; i<intervals.size(); ++i) {
        results << intervalText(intervals.at(i), formatFunc);
    }
    QString result = results.join(", ");
    if(!unit.isEmpty()) {
        result += " " + unit;
    }
    return result;
}
/**
 * @brief Extract intervals (QPair of date) and calcul the area corresponding, from a HPD QMap maded before
 */
QList<QPair<float, QPair<float, float> > > intervalsForHpd(const QMap<float, float>& hpd, float thresh)
{
    QList<QPair<float, QPair<float, float> >> intervals;

    if(hpd.isEmpty()) return intervals;

    QMapIterator<float, float> it(hpd);
    bool inInterval = false;
    float lastKeyInInter = 0.;
    QPair<float, float> curInterval;
    
    float areaTot= map_area(hpd);
    float lastValueInInter = 0.;
    
    float areaCur = 0;
    it.toFront();
    while(it.hasNext()) {
        it.next();
        
        if(it.value() != 0 && !inInterval) {
            inInterval = true;
            curInterval.first = it.key();
            lastKeyInInter = it.key();
            areaCur = 0.; // start, not inside
        } else if(inInterval) {
            if((it.value() == 0) ) {
                inInterval = false;
                curInterval.second = lastKeyInInter;
                
                QPair<float, QPair<float, float> > inter;
                inter.first = thresh * areaCur / areaTot;
                inter.second = curInterval;
                intervals.append(inter);
                
                areaCur = 0;

            } else {
                areaCur += (lastValueInInter+it.value())/2 * (it.key()-lastKeyInInter);
             
                lastKeyInInter = it.key();
                lastValueInInter = it.value();
                
            }
        }
    }
    
    if (inInterval) { // Correction to close unclosed interval
       
        curInterval.second = lastKeyInInter;
        areaCur += (lastValueInInter+it.value())/2 * (it.key()-lastKeyInInter);
        QPair<float, QPair<float, float> > inter;
        inter.first = thresh * areaCur / areaTot;
        inter.second = curInterval;

        intervals.append(inter);
    }
    
    return intervals;
}
