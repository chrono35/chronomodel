
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QMap>
#include <QVector>
#include <cmath>
#include "StdUtilities.h"
typedef double type_data;

struct FunctionAnalysis{
    type_data max = (type_data)0.;
    type_data mode = (type_data)0.;
    type_data mean = (type_data)0.;
    type_data stddev = (type_data)0.;
};

struct Quartiles{
    type_data Q1 = (type_data)0.;
    type_data Q2 = (type_data)0.;
    type_data Q3 = (type_data)0.;
};

struct DensityAnalysis
{
    Quartiles quartiles;
    FunctionAnalysis analysis;
};

FunctionAnalysis analyseFunction(const QMap<type_data, type_data>& aFunction);

QString functionAnalysisToString(const FunctionAnalysis& analysis, const bool forCSV = false);
QString densityAnalysisToString(const DensityAnalysis& analysis, const QString& nl = "<br>", const bool forCSV = false);

// Standard Deviation of a vector of data
type_data dataStd(const QVector<type_data> &data);

double shrinkageUniform(const double so2);

Quartiles quartilesForTrace(const QVector<type_data>& trace);

Quartiles quartilesForRepartition(const QVector<double> &repartition, const double tmin, const double step);
QPair<double, double> credibilityForTrace(const QVector<double>& trace, double thresh, double& exactThresholdResult, const QString description = "Credibility computation");
QPair<double, double> credibilityForTrace(const QVector<int>& trace, double thresh, double& exactThresholdResult, const QString description = "Credibility computation");
QPair<double, double> timeRangeFromTraces(const QVector<double>& trace1, const QVector<double>& trace2, const double thresh, const QString description ="Time Range Computation");


QPair<double, double> gapRangeFromTraces(const QVector<double>& trace1, const QVector<double>& trace2, const double thresh, const QString description ="Gap Range Computation");

QPair<double, double> transitionRangeFromTraces(const QVector<double> &trace1, const QVector<double> &trace2, const double thresh, const QString description ="Gap Range Computation");

QString intervalText(const QPair<double, QPair<double, double> >& interval, FormatFunc formatFunc = nullptr, const bool forCSV = false);
QString getHPDText(const QMap<double, double>& hpd, double thresh, const QString& unit = QString(), FormatFunc formatFunc = nullptr, const bool forCSV =false);

QList<QPair<double, QPair<double, double> > > intervalsForHpd(const QMap<double, double> &hpd, double thresh);

inline double rounddouble(const double f,const int prec)
{
    double result;
    if (prec > 0){
        const double factor = pow(10., (double)prec);
        result = round(f * factor) / factor;
    } else {
        result = round(f);
    }
    return result;
}

template <typename T>
bool isOdd( T value )
{
    return (value % 2!= 0 ? true : false);
}

template <typename T>
bool isEven( T value )
{
    return (value % 2 == 0 ? true : false);
}


/**
 * @brief gammaQuartile used with quantile, find the gamma coef corresponding to the
 * type of R Calcul, to use with the general formula
 * @param trace
 * @param quartileType
 * @param p
 * @return
 */
template <typename T>
QPair<int, double> gammaQuartile(const QVector<T> &trace, const int quartileType, const double p)
{
    const int n (trace.size());
    int j (0);
    // We use jFloor which is the floor value of j but in the original double type
    // because when we cacul g in the 3 first cases we need the double format
    double jFloor(0.);

    double m (0.);
    double g (0.);
    double gamma (0.);
    double k (0.);


    switch (quartileType) {
    // Case 1 to 3 are discontinuous case
    case 1: // It is different to R but it is identique to QuantileType1 in the article "Quantile calculations in R"
        // http://tolstoy.newcastle.edu.au/R/e17/help/att-1067/Quartiles_in_R.pdf
        m = 0.;
        jFloor = floor((n * p) + m);
        j = (int)jFloor;
        g = n*p + m - jFloor;

        gamma = (g<1e-10 ? 0 : 1.) ;
 //qDebug()<<n<<p<<m<<jFloor<<j<<g<<gamma;
        break;

    case 2: // same probleme as type 1
        m = 0.;
        jFloor = floor((n * p) + m);
        j = (int)jFloor;
        g = n*p + m - jFloor;
        gamma = (g==0. ? 0.5 : 1.) ;
        break;

    case 3: // OK with R
        m = -0.5;
        jFloor = floor((n * p) + m);
        j = (int)jFloor;
        g = n*p + m - jFloor;
        gamma = (g==0. && isEven(j) ? 0. : 1.);
        break;

    // Case 4 to 9 are continuous case
    case 4: // OK with R
        m = 0.;
        k = p * n;
        g = k - floor(k);
        j = (int) floor(k);
        gamma = g ;
        break;
    case 5: // OK with R
        m = 0.;
        k = (p * n) + 0.5;
        g = k - floor(k);
        j = (int) floor(k);
        gamma = g ;
        break;
    case 6:
        k = p * (n+1);
        g = k - floor(k);
        j = (int) floor(k);
        gamma = g;
        break;
    case 7: // OK with R, this is the default type in R software
        k = (p*(n-1) + 1);
        g = k - floor(k);
        j = (int) floor(k);
        gamma = g ;
        break;
    case 8: // OK with R, it is the formula with Bos-Levenbach (1953) parameter
        // http://www.barringer1.com/wa_files/The-plotting-of-observations-on-probability-paper.pdf
        k = p * (n + 0.4) + 0.3;
        g = k - floor(k);
        j = (int) floor(k);
        gamma = g ;
        break;
    case 9:
        k = p * (n + 2./8.) + 3./8.;
        g = k - floor(k);
        j = (int) floor(k);
        gamma = g;
        break;

    default:
        gamma = 0.;
        break;
    }

    return qMakePair(j, gamma);
}

/**
 * @brief Compute quartile according to a type defined in R software
 * @param quartileType the type defined in R
 * @param p is the confidence must be between [0 , 1]
 * @return Q1(confidance) Q2(50%) Q3(1-confidance)
 */
template <typename T>
Quartiles quartilesType(const QVector<T>& trace, const int quartileType, const double p)
{
    Q_ASSERT(&trace);
    Quartiles Q;
    QVector<T> traceSorted (trace);

    QPair<int, double> parQ1 = gammaQuartile(trace, quartileType, p); // first is j and secand is gamma
    QPair<int, double> parQ2 = gammaQuartile(trace, quartileType, 0.5);
    QPair<int, double> parQ3 = gammaQuartile(trace, quartileType, 1-p);

    std::sort(traceSorted.begin(), traceSorted.end());

    // Q1 determination
    if (parQ1.first<=0)
       Q.Q1 = (double)traceSorted.first();

    else if (parQ1.first < traceSorted.size())
            Q.Q1 = (1.- parQ1.second)*(double)traceSorted.at(parQ1.first-1) + parQ1.second*(double)traceSorted.at(parQ1.first);
    else
        Q.Q1 = (double)traceSorted.last();

    // Q2 determination
    if (parQ2.first<=0)
       Q.Q2 = (double)traceSorted.first();

    else if (parQ2.first < traceSorted.size())
            Q.Q2 = (1.- parQ2.second)*(double)traceSorted.at(parQ2.first-1) + parQ2.second*(double)traceSorted.at(parQ2.first);
    else
        Q.Q2 = (double)traceSorted.last();

    // Q3 determination
    if (parQ3.first<=0)
       Q.Q3 = (double)traceSorted.first();

    else if (parQ3.first < traceSorted.size())
            Q.Q3 = (1.- parQ3.second)*(double)traceSorted.at(parQ3.first-1) + parQ3.second*(double)traceSorted.at(parQ3.first);
    else
        Q.Q3 = (double)traceSorted.last();

    return Q;
}


#endif
