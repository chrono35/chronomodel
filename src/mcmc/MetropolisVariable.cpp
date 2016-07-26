#include "MetropolisVariable.h"
#include "StdUtilities.h"
#include "QtUtilities.h"
#include "Functions.h"
#include "DateUtils.h"

#if USE_FFT
#include "fftw3.h"
#endif

#include <QDebug>
#include <algorithm>
#include <assert.h>


MetropolisVariable::MetropolisVariable(QObject *parent):QObject(parent),
mX(0),
mSupport(eR),
mFormat(DateUtils::eNumeric),
mExactCredibilityThreshold(0),
mfftLenUsed(-1),
mBandwidthUsed(-1),
mThresholdUsed(-1),
mtminUsed(0),
mtmaxUsed(0)

{
    mCredibility = QPair<double,double>();
    mHPD = QMap<double,double>();
    QObject::connect(this, &MetropolisVariable::formatChanged, this, &MetropolisVariable::updateFormatedTrace);
}

MetropolisVariable::~MetropolisVariable()
{
    QObject::disconnect(this, &MetropolisVariable::formatChanged, this, &MetropolisVariable::updateFormatedTrace);
}

void MetropolisVariable::memo()
{
    mRawTrace.push_back(mX);
}

void MetropolisVariable::reset()
{
    mRawTrace.clear();
    mFormatedTrace.clear();
    
    mHisto.clear();
    mChainsHistos.clear();
    
    mCorrelations.clear();
    
    mHPD.clear();
    
    mChainsResults.clear();
}

MetropolisVariable& MetropolisVariable::copy(MetropolisVariable const& origin)
{
    mX = origin.mX;
    mRawTrace = origin.mRawTrace;
    mFormatedTrace = origin.mFormatedTrace;
    mSupport = origin.mSupport;
    mFormat = origin.mFormat;

    mHisto = origin.mHisto;
    mChainsHistos = origin.mChainsHistos;

    mCorrelations = origin.mCorrelations;

    mHPD = origin.mHPD;
    mCredibility = origin.mCredibility;


    mExactCredibilityThreshold = origin.mExactCredibilityThreshold;

    mResults = origin.mResults;
    mChainsResults = origin.mChainsResults;

    mfftLenUsed = origin.mBandwidthUsed;
    mBandwidthUsed = origin.mBandwidthUsed;
    mThresholdUsed = origin.mThresholdUsed;

    mtminUsed = origin.mtminUsed;
    mtmaxUsed = origin.mtmaxUsed;

    return *this;
}

MetropolisVariable& MetropolisVariable::operator=( MetropolisVariable const& origin)
{
    copy(origin);
    return *this;
}

void MetropolisVariable::setFormat(const DateUtils::FormatDate fm)
{
    if(fm != mFormat) {
        mFormat = fm;
        emit formatChanged();
    }
}

/**
 * @brief MetropolisVariable::updateFormatedTrace, it's a slot that transform or create mFormatedTrace
 * according to mFormat.
 */
void MetropolisVariable::updateFormatedTrace()
{
    mFormatedTrace.clear();
    if (mRawTrace.isEmpty())
       return;

    mRawTrace.squeeze(); // just cleaning, must be somewhere else to optimize


    if(mFormat == DateUtils::eNumeric)
        mFormatedTrace = mRawTrace;
    else {
        mFormatedTrace.resize(mRawTrace.size());
        std::transform(mRawTrace.cbegin(),mRawTrace.cend(),mFormatedTrace.begin(),[this](const double i){return DateUtils::convertToFormat(i,this->mFormat);});
       /* mFormatedTrace.reserve(mRawTrace.size());
        QVector<double>::const_iterator iter = mRawTrace.constBegin();
        while(iter!= mRawTrace.constEnd()){
            mFormatedTrace.append(DateUtils::convertToFormat(*iter, mFormat));
            ++iter;
        }*/
    }

}

/**
 @param[in] dataSrc is the trace, with for example one million data
 @remarks Produce a density with the area equal to 1. The smoothing is done with Hsilvermann method.
 **/
void MetropolisVariable::generateBufferForHisto(float* input, const QVector<double> &dataSrc, const int numPts, const double a, const double b)
{
    // Work with "double" precision here !
    // Otherwise, "denum" can be very large and lead to infinity contribs!
    
    const double delta = (b - a) / (numPts - 1);

    const double denum = dataSrc.size();
    
    //float* input = (float*) fftwf_malloc(numPts * sizeof(float));
    
    //memset(input, 0.f, numPts);
    for(int i=0; i<numPts; ++i)
        input[i]= 0.f;
    
    QVector<double>::const_iterator iter = dataSrc.cbegin();
    for(; iter != dataSrc.cend(); ++iter)
    {
        const double t = *iter;
        
        const double idx = (t - a) / delta;
        const double idx_under = floor(idx);
        const double idx_upper = idx_under + 1.;
        
        const float contrib_under = (idx_upper - idx) / denum;
        const float contrib_upper = (idx - idx_under) / denum;
        
        if(std::isinf(contrib_under) || std::isinf(contrib_upper))
        {
            qDebug() << "FFT input : infinity contrib!";
        }
        if(idx_under < 0 || idx_under >= numPts || idx_upper < 0 || idx_upper > numPts)
        {
            qDebug() << "FFT input : Wrong index";
        }
        if(idx_under < numPts)
            input[(int)idx_under] += contrib_under;
        if(idx_upper < numPts) // This is to handle the case when matching the last point index !
            input[(int)idx_upper] += contrib_upper;
    }
    //return input;
}

/**
  @param[in] bandwidth corresponds to the bandwidth factor
  @param[in] dataSrc is the trace of the raw data
  @brief the FFTW function transform the area such that the area output is the area input multiplied by fftLen. So we have to corret it.
  The result is migth be not with regular step between value.
 **/
QMap<double, double> MetropolisVariable::generateHisto(const QVector<double>& dataSrc, const int fftLen, const double bandwidth, const double tmin, const double tmax)
{
    mfftLenUsed = fftLen;
    mBandwidthUsed = bandwidth;
    mtmaxUsed = tmax;
    mtminUsed = tmin;

    const int inputSize = fftLen;
    const int outputSize = 2 * (inputSize / 2 + 1);

    const double sigma = dataStd(dataSrc);
    QMap<double, double> result;



    if (sigma == 0) {
        qDebug()<<"MetropolisVariable::generateHisto sigma == 0";
        if(dataSrc.size()>0) {
            // if sigma is null and there are several values, it means: this is the same
            // value. It can appear with a bound fixed
            result.insert(dataSrc.at(0)+tmin, 1) ;
            qDebug()<<"MetropolisVariable::generateHisto result = "<< (dataSrc.at(0)+tmin);
        }
        return result;
    }
    // DEBUG
   /*QVector<double> histo = vector_to_histo(dataSrc,tmin,tmax,fftLen);
    const double step = (tmax-tmin)/fftLen;
    result = vector_to_map(histo, tmin, tmax, step);
    return result;
    */// /// DEBUG

     const double h = bandwidth * sigma * pow(dataSrc.size(), -1./5.);
     const double a = vector_min_value(dataSrc) - 4. * h;
     const double b = vector_max_value(dataSrc) + 4. * h;

     float* input = (float*) fftwf_malloc(fftLen * sizeof(float));
     generateBufferForHisto(input, dataSrc, fftLen, a, b);

     float* output = (float*) fftwf_malloc(outputSize * sizeof(float));
    
    if(input != 0) {
        // ----- FFT -----
        // http://www.fftw.org/fftw3_doc/One_002dDimensional-DFTs-of-Real-Data.html#One_002dDimensional-DFTs-of-Real-Data
        //https://jperalta.wordpress.com/2006/12/12/using-fftw3/
        fftwf_plan plan_forward = fftwf_plan_dft_r2c_1d(inputSize, input, (fftwf_complex*)output, FFTW_ESTIMATE);
        fftwf_execute(plan_forward);

        for(int i=0; i<outputSize/2; ++i) {
            const double s = 2.f * M_PI * i / (b-a);
            const double factor = expf(-0.5f * s * s * h * h);

            output[2*i] *= factor;
            output[2*i + 1] *= factor;
        }

        fftwf_plan plan_backward = fftwf_plan_dft_c2r_1d(inputSize, (fftwf_complex*)output, input, FFTW_ESTIMATE);
        fftwf_execute(plan_backward);

        // ----- FFT Buffer to result map -----

        double tBegin = a;
        double tEnd = b;
        switch(mSupport)
        {
              case eR :// on R
                  // nothing to do already done by default
              break;
              case eRp : // on R+
                  tBegin = 0;
              break;
              case eRm :// on R-
                  tEnd = 0;
              break;
              case eRpStar : // on R+*
                  tBegin = 0;
              break;
              case eRmStar :// on R-*
                  tEnd = 0;
              break;
              case eBounded : // on [tmin;tmax]
                  tBegin = tmin;
                  tEnd = tmax;
              break;
        }
        const double delta = (b - a) / fftLen;
        for(int i=0; i<inputSize; ++i) {
             const double t = a + (double)i * delta;
             result[t] = input[i];
        }

        result = getMapDataInRange(result,tBegin,tEnd);


        fftwf_free(input);
        fftwf_free(output);
        input = 0;
        output = 0;
        fftwf_destroy_plan(plan_forward);
        fftwf_destroy_plan(plan_backward);

        result = equal_areas(result, 1.); // normalize the output area du to the fftw and the case (t >= tmin && t<= tmax)
    }
    return result; // return a map between a and b with a step delta = (b - a) / fftLen;
}


void MetropolisVariable::generateHistos(const QList<ChainSpecs>& chains, const int fftLen, const double bandwidth, const double tmin, const double tmax)
{
    const QVector<double> subFullTrace (fullRunTrace(chains));
    mHisto = generateHisto(subFullTrace, fftLen, bandwidth, tmin, tmax);

    mChainsHistos.clear();
    for (int i=0; i<chains.size(); ++i) {
        const QVector<double> subTrace ( runFormatedTraceForChain(chains, i));
        if (!subTrace.isEmpty()) {
            const QMap<double,double> histo (generateHisto(subTrace, fftLen, bandwidth, tmin, tmax) );
            mChainsHistos.append(histo);
        }
    }
}
void MetropolisVariable::memoHistoParameter(const int fftLen, const double bandwidth, const double tmin, const double tmax)
{
    mfftLenUsed = fftLen;
    mBandwidthUsed = bandwidth;
    mtminUsed = tmin;
    mtmaxUsed = tmax;
}

bool MetropolisVariable::HistoWithParameter(const int fftLen, const double bandwidth, const double tmin, const double tmax)
{
   return ((mfftLenUsed == fftLen) &&  (mBandwidthUsed == bandwidth) &&   (mtminUsed == tmin) &&  (mtmaxUsed == tmax) ? true: false);
}

void MetropolisVariable::generateHPD(const double threshold)
{
    if(!mHisto.isEmpty())
    {
        const double thresh = qBound(0.0, threshold, 100.0);
        if (thresh == 100.) {
            mHPD = mHisto;
            return;
        }
        //threshold = (threshold < 0 ? threshold = 0.0 : threshold);
        if (thresh == 0.) {
            mHPD.clear();
            return;
        }
        mHPD = create_HPD(mHisto, thresh);
        
        // No need to have HPD for all chains !
        //mChainsHPD.clear();
        //for(int i=0; i<mChainsHistos.size(); ++i)
        //  mChainsHPD.append(create_HPD(mChainsHistos[i], 1, threshold));
    }
    else
    {
        // This can happen on phase duration, if only one event inside.
        // alpha = beta => duration is always null !
        // We don't display the phase duration but we print the numerical HPD result.
        mHPD = QMap<double,double>();

        qDebug() << "WARNING : Cannot generate HPD on empty histo in MetropolisVariable::generateHPD";
    }
}

void MetropolisVariable::generateCredibility(const QList<ChainSpecs> &chains, double threshold)
{
    if (!mHisto.isEmpty())
        mCredibility = credibilityForTrace(fullRunTrace(chains), threshold, mExactCredibilityThreshold,"Compute credibility for "+getName());
    else
        mCredibility = QPair<double,double>();

}

void MetropolisVariable::generateCorrelations(const QList<ChainSpecs>& chains)
{
    const int hmax = 40;
    mCorrelations.clear();
    mCorrelations.reserve(chains.size());

    for (int c=0; c<chains.size(); ++c) {
        // Return the acquisition part of the trace
        const QVector<double> trace = runRawTraceForChain(chains, c);
        if(trace.size()<hmax) continue;
        QVector<double> results;
        results.reserve(hmax);

        const int n = trace.size();
        
        const double s = sum(trace);
        const double m = s / (double)n;
        const double s2 = sum2Shifted(trace, -m);
        
        // Correlation pour cette chaine

        for (int h=0; h<hmax; ++h) {
            double sH = 0;
            for (QVector<double>::const_iterator iter = trace.cbegin(); iter != trace.cbegin() + (n-h); ++iter) {
                sH += (*iter - m) * (*(iter + h) - m);
            }
            
            const double result = sH / s2;
            results.append(result);
        }
        // Correlation ajoutée à la liste (une courbe de corrélation par chaine)
        mCorrelations.append(results);

    }
}

void MetropolisVariable::generateNumericalResults(const QList<ChainSpecs> &chains)
{
    // Results for chain concatenation
    mResults.analysis = analyseFunction(mHisto);
    mResults.quartiles = quartilesForTrace(fullRunTrace(chains));
    
    // Results for individual chains
    mChainsResults.clear();
    for (int i = 0; i<mChainsHistos.size(); ++i) {
        DensityAnalysis result;
        result.analysis = analyseFunction(mChainsHistos.at(i));
        result.quartiles = quartilesForTrace(runFormatedTraceForChain(chains, i));
        mChainsResults.append(result);
    }
}

#pragma mark getters (no calculs)
QMap<double, double>& MetropolisVariable::fullHisto()
{
    return mHisto;
}

QMap<double, double>& MetropolisVariable::histoForChain(const int index)
{

    Q_ASSERT(index < mChainsHistos.size());    
    return mChainsHistos[index];
}

/**
 * @brief MetropolisVariable::fullTraceForChain
 * @param chains QList of the ChainSpecs in the Model
 * @param index
 * @return The complet trace (Burning, adaptation, acquire) corresponding to chain n°index
 */
QVector<double> MetropolisVariable::fullTraceForChain(const QList<ChainSpecs>& chains, const int index)
{
    QVector<double> trace(0);

    const int reserveSize = (int) ceil(chains.at(index).mNumRunIter /chains.at(index).mThinningInterval );
    trace.reserve(reserveSize);

    int shift = 0;
    
    for (int i=0; i<chains.size(); ++i) {
        const int traceSize = chains.at(i).mNumBurnIter + (chains.at(i).mBatchIndex * chains.at(i).mNumBatchIter) + chains.at(i).mNumRunIter / chains.at(i).mThinningInterval;

        if (i == index) {
            trace = mFormatedTrace.mid(shift , traceSize);
            break;
        }
        shift += traceSize;
    }
    return trace;
}

QVector<double> MetropolisVariable::fullRunTrace(const QList<ChainSpecs>& chains)
{
    QVector<double> trace(0);
    // calcul reserve space
    int reserveSize = 0;

    for (const ChainSpecs& chain:chains)
        reserveSize += chain.mNumRunIter / chain.mThinningInterval;

    trace.reserve(reserveSize);

    int shift = 0;
    for (int i = 0; i<chains.size(); ++i) {
        const ChainSpecs& chain = chains.at(i);
        
        const int burnAdaptSize = chain.mNumBurnIter + (chain.mBatchIndex * chain.mNumBatchIter);
        const int traceSize = chain.mNumRunIter / chain.mThinningInterval;

        trace += mFormatedTrace.mid(shift + burnAdaptSize, traceSize);
        
        shift += traceSize + burnAdaptSize;
    }
    return trace;
}
/**
 * @brief MetropolisVariable::runTraceForChain
 * @param chains
 * @param index the number of the Trace to extract
 * @return a QVector containing juste the acquisition Trace for one chaine n° index
 */
QVector<double> MetropolisVariable::runRawTraceForChain(const QList<ChainSpecs> &chains, const int index)
{
    QVector<double> trace(0);
    if (mRawTrace.empty()) {
        qDebug() << "in MetropolisVariable::runRawTraceForChain -> mRawTrace empty";
        return trace ;
        
    } else {
        int shift = 0;
        const int reserveSize = (int) ceil(chains.at(index).mNumRunIter /chains.at(index).mThinningInterval );
        trace.reserve(reserveSize);
        for (int i=0; i<chains.size(); ++i) {
            const ChainSpecs& chain = chains.at(i);
            
            const int burnAdaptSize = int (chain.mNumBurnIter + chain.mBatchIndex * chain.mNumBatchIter);
            const int traceSize = int (burnAdaptSize + chain.mNumRunIter / chain.mThinningInterval);
            
            if (i == index) {
                trace = mRawTrace.mid(shift + burnAdaptSize, traceSize - burnAdaptSize);
                break;
            }
            shift += traceSize;
        }
        return trace;
    }
}

QVector<double> MetropolisVariable::runFormatedTraceForChain(const QList<ChainSpecs> &chains, const int index)
{
    QVector<double> trace(0);
    if (mFormatedTrace.empty()) {
        qDebug() << "in MetropolisVariable::runFormatedTraceForChain -> mFormatedTrace empty";
        return trace ;
    }  else  {
        const int reserveSize = (int) ceil(chains.at(index).mNumRunIter /chains.at(index).mThinningInterval );
        trace.reserve(reserveSize);

        int shift = 0;
        for (int i=0; i<chains.size(); ++i)  {
            const ChainSpecs& chain = chains.at(i);

            const int burnAdaptSize = (int) (chain.mNumBurnIter + chain.mBatchIndex * chain.mNumBatchIter);
            const int traceSize = (int) (chain.mNumRunIter / chain.mThinningInterval);

            if (i == index) {
                trace = mFormatedTrace.mid(shift + burnAdaptSize-1, traceSize);
                break;
            }
            shift += traceSize + burnAdaptSize ;
        }
        return trace;
    }
}


QVector<double> MetropolisVariable::correlationForChain(const int index)
{
    if (index < mCorrelations.size())
        return mCorrelations.at(index);
    return QVector<double>();
}


QString MetropolisVariable::resultsString(const QString& nl, const QString& noResultMessage, const QString& unit, FormatFunc formatFunc) const
{
    if (mHisto.isEmpty())
        return noResultMessage;
    
    const QLocale locale;
    QString result = densityAnalysisToString(mResults, nl) + nl;
    
    if (!mHPD.isEmpty())
        result += "HPD Region (" + locale.toString(mThresholdUsed, 'f', 1) + "%) : " + getHPDText(mHPD, mThresholdUsed, unit, formatFunc) + nl;

    
    if (mCredibility != QPair<double,double>()) {
        if (formatFunc)
            result += "Credibility Interval (" + locale.toString(mExactCredibilityThreshold * 100.f, 'f', 1) + "%) : [" + formatFunc(mCredibility.first) + ", " + formatFunc(mCredibility.second) + "] " + unit;
        else
            result += "Credibility Interval (" + locale.toString(mExactCredibilityThreshold * 100.f, 'f', 1) + "%) : [" + DateUtils::dateToString(mCredibility.first) + ", " + DateUtils::dateToString(mCredibility.second) + "]";

   }
   return result;
}

QStringList MetropolisVariable::getResultsList(const QLocale locale, const bool withDateFormat)
{
    QStringList list;
    if (withDateFormat) {

        list << locale.toString(mResults.analysis.mode);
        list << locale.toString(mResults.analysis.mean);
        list << DateUtils::dateToString(mResults.analysis.stddev);
        list << locale.toString(mResults.quartiles.Q1);
        list << locale.toString(mResults.quartiles.Q2);
        list << locale.toString(mResults.quartiles.Q3);
        list << locale.toString(mExactCredibilityThreshold * 100.f, 'f', 1);
        list << locale.toString(mCredibility.first);
        list << locale.toString(mCredibility.second);

        const QList<QPair<double, QPair<double, double> > > intervals = intervalsForHpd(mHPD, mThresholdUsed);
        QStringList results;
        for (int i=0; i<intervals.size(); ++i) {
            list << locale.toString(intervals.at(i).first, 'f', 1);
            list << locale.toString(intervals.at(i).second.first);
            list << locale.toString(intervals.at(i).second.second);
        }

    } else {
        list << locale.toString(DateUtils::convertFromAppSettingsFormat(mResults.analysis.mode));
        list << locale.toString(DateUtils::convertFromAppSettingsFormat(mResults.analysis.mean));
        list << DateUtils::dateToString(mResults.analysis.stddev);
        list << locale.toString(DateUtils::convertFromAppSettingsFormat(mResults.quartiles.Q1));
        list << locale.toString(DateUtils::convertFromAppSettingsFormat(mResults.quartiles.Q2));
        list << locale.toString(DateUtils::convertFromAppSettingsFormat(mResults.quartiles.Q3));
        list << locale.toString(mExactCredibilityThreshold * 100.f, 'f', 1);
        list << locale.toString(DateUtils::convertFromAppSettingsFormat(mCredibility.first));
        list << locale.toString(DateUtils::convertFromAppSettingsFormat(mCredibility.second));

        const QList<QPair<double, QPair<double, double> > > intervals = intervalsForHpd(mHPD, mThresholdUsed);
        QStringList results;
        for (int i=0; i<intervals.size(); ++i)  {
            list << locale.toString(intervals.at(i).first, 'f', 1);
            list << locale.toString(DateUtils::convertFromAppSettingsFormat(intervals.at(i).second.first));
            list << locale.toString(DateUtils::convertFromAppSettingsFormat(intervals.at(i).second.second));
        }
    }



    return list;
}
QDataStream &operator<<( QDataStream &stream, const MetropolisVariable &data )
{
    switch (data.mSupport) {
       case MetropolisVariable::eR : stream << (quint8)(0); // on R
        break;
       case MetropolisVariable::eRp: stream << (quint8)(1); // on R+
          break;
       case MetropolisVariable::eRm : stream << (quint8)(2); // on R-
          break;
       case MetropolisVariable::eRpStar : stream << (quint8)(3); // on R+*
          break;
       case MetropolisVariable::eRmStar : stream << (quint8)(4); // on R-*
          break;
       case  MetropolisVariable::eBounded : stream << (quint8)(5); // on bounded support
          break;
    };

    switch (data.mFormat) {
       case DateUtils::eUnknown : stream << (qint16)(-2);
        break;
       case DateUtils::eNumeric : stream << (qint16)(-1);
          break;
       case DateUtils::eBCAD : stream << (qint16)(0);
          break;
       case DateUtils::eCalBP : stream << (qint16)(1);
          break;
       case DateUtils::eCalB2K : stream << (qint16)(2);
          break;
       case  DateUtils::eDatBP : stream << (qint16)(3);
          break;
       case DateUtils::eDatB2K : stream << (qint16)(4);
          break;
    };

    stream << data.mRawTrace;

    qDebug()<<"&operator<<( QDataStream &stream, const MetropolisVariable &data )"<<data.mRawTrace.size();

    // *out << this->mFormatedTrace; // useless

    return stream;
}

QDataStream &operator>>( QDataStream &stream, MetropolisVariable &data )
{
    quint8 support;
    stream >> support;
    switch ((int) support) {
      case 0 : data.mSupport = MetropolisVariable::eR; // on R
       break;
      case 1 : data.mSupport = MetropolisVariable::eRp; // on R+
         break;
      case 2 : data.mSupport = MetropolisVariable::eRm; // on R-
         break;
      case 3 : data.mSupport = MetropolisVariable::eRpStar; // on R+*
         break;
      case 4 : data.mSupport = MetropolisVariable::eRmStar; // on R-*
         break;
      case 5 : data.mSupport = MetropolisVariable::eBounded; // on bounded support
         break;
   };

    qint16 formatDate;
    stream >> formatDate;
    switch (formatDate) {
      case -2 : data.mFormat = DateUtils::eUnknown;
       break;
      case -1 : data.mFormat = DateUtils::eNumeric;
         break;
      case 0 :  data.mFormat = DateUtils::eBCAD;
         break;
      case 1 : data.mFormat = DateUtils::eCalBP;
        break;
      case 2 : data.mFormat = DateUtils::eCalB2K;
        break;
      case 3 : data.mFormat = DateUtils::eDatBP;
         break;
      case 4 : data.mFormat = DateUtils::eDatB2K;
         break;
   };
    stream >> data.mRawTrace;

    // regeneration of this->mFormatedTrace
    data.updateFormatedTrace();

    return stream;

}


/* Obsolete function */
void MetropolisVariable::saveToStream(QDataStream &out)
{
    switch (mSupport) {
       case eR : out << quint32(0); // on R
        break;
       case eRp: out << quint32(1); // on R+
          break;
       case eRm : out << quint32(2); // on R-
          break;
       case eRpStar : out << quint32(3); // on R+*
          break;
       case eRmStar : out << quint32(4); // on R-*
          break;
       case  eBounded : out << quint32(5); // on bounded support
          break;
    };

    switch (mFormat) {// error mFormat can not be quint32
       case DateUtils::eUnknown : out << quint32(-2);
        break;
       case DateUtils::eNumeric : out << quint32(-1);
          break;
       case DateUtils::eBCAD : out << quint32(0);
          break;
       case DateUtils::eCalBP : out << quint32(1);
          break;
       case DateUtils::eCalB2K : out << quint32(2);
          break;
       case  DateUtils::eDatBP : out << quint32(3);
          break;
       case DateUtils::eDatB2K : out << quint32(4);
          break;
    };

    out << mRawTrace;
   // *out << this->mFormatedTrace; // useless
}

void MetropolisVariable::saveToStreamOfQByteArray(QDataStream* out)
{
    switch (mSupport) {
       case eR : *out << QByteArray::number(0); // on R
        break;
       case eRp: *out << QByteArray::number(1); // on R+
          break;
       case eRm : *out << QByteArray::number(2); // on R-
          break;
       case eRpStar : *out << QByteArray::number(3); // on R+*
          break;
       case eRmStar : *out << QByteArray::number(4); // on R-*
          break;
       case  eBounded : *out << QByteArray::number(5); // on bounded support
          break;
    };

    switch (mFormat) {
       case DateUtils::eUnknown : *out << QByteArray::number(-2);
        break;
       case DateUtils::eNumeric : *out << QByteArray::number(-1);
          break;
       case DateUtils::eBCAD : *out << QByteArray::number(0);
          break;
       case DateUtils::eCalBP : *out << QByteArray::number(1);
          break;
       case DateUtils::eCalB2K : *out << QByteArray::number(2);
          break;
       case  DateUtils::eDatBP : *out << QByteArray::number(3);
          break;
       case DateUtils::eDatB2K : *out << QByteArray::number(4);
          break;
    };

    // *out << this->mRawTrace;
    //assert(mRawTrace.size()>0);
    *out << QByteArray::number(mRawTrace.size());
    qDebug()<<"MetropolisVariable::saveToStreamOfQByteArray"<<QByteArray::number(mRawTrace.size());
    /*for (const double& d : mRawTrace)
        *out << QByteArray::number(d);*/
    for (int i=0; i<mRawTrace.size(); ++i)
        *out << QByteArray::number(mRawTrace.at(i));
    // *out << this->mFormatedTrace; // useless
}

void MetropolisVariable::loadFromStream(QDataStream &in)
{
    quint32 support;
    in >> support;
    switch (support) {
      case 0 : mSupport = eR; // on R
       break;
      case 1 : mSupport = eRp; // on R+
         break;
      case 2 : mSupport = eRm; // on R-
         break;
      case 3 : mSupport = eRpStar; // on R+*
         break;
      case 4 : mSupport = eRmStar; // on R-*
         break;
      case 5 : mSupport =  eBounded; // on bounded support
         break;
   };

    quint32 formatDate;
    in >> formatDate;
    switch (formatDate) {
      case -2 : mFormat = DateUtils::eUnknown;
       break;
      case -1 : mFormat = DateUtils::eNumeric;
         break;
      case 0 :  mFormat = DateUtils::eBCAD;
         break;
      case 1 : mFormat = DateUtils::eCalBP;
        break;
      case 2 : mFormat = DateUtils::eCalB2K;
        break;
      case 3 : mFormat = DateUtils::eDatBP;
         break;
      case 4 :  mFormat = DateUtils::eDatB2K;
         break;
   };
    quint32 reserveTrace;
    in >> reserveTrace;

    mRawTrace.resize((int)reserveTrace);
    qDebug()<<"MetropolisVariable::loadFromStream"<<quint32(mRawTrace.size());
    int it = 0;
    for (; it <mRawTrace.size(); ++it) {
        qreal tmp;
        in >> tmp;
        mRawTrace[it]= (double) tmp;
    }
    qDebug()<<"MetropolisVariable::loadFromStream it="<<it;
   // std::generate(mRawTrace.begin(),mRawTrace.end(),[in]{qreal tmp; *in >> tmp; return (double) tmp;});

   // *in >>  mRawTrace;

    // regeneration of this->mFormatedTrace
    updateFormatedTrace();
    /*
    *in >>  this->mFormatedTrace;*/

}

void MetropolisVariable::loadFromStreamOfQByteArray(QDataStream *in)
{
    QByteArray tmpArray;
    *in >> tmpArray;
    switch (tmpArray.toInt()) {
      case 0 : mSupport = eR; // on R
       break;
      case 1 : mSupport = eRp; // on R+
         break;
      case 2 : mSupport = eRm; // on R-
         break;
      case 3 : mSupport = eRpStar; // on R+*
         break;
      case 4 : mSupport = eRmStar; // on R-*
         break;
      case 5 : mSupport =  eBounded; // on bounded support
         break;
   };

    *in >> tmpArray;
    switch (tmpArray.toInt()) {
      case -2 : mFormat = DateUtils::eUnknown;
       break;
      case -1 : mFormat = DateUtils::eNumeric;
         break;
      case 0 :  mFormat = DateUtils::eBCAD;
         break;
      case 1 : mFormat = DateUtils::eCalBP;
        break;
      case 2 : mFormat = DateUtils::eCalB2K;
        break;
      case 3 : mFormat = DateUtils::eDatBP;
         break;
      case 4 :  mFormat = DateUtils::eDatB2K;
         break;
   };
    // reserveTrace;
    *in >> tmpArray;

    mRawTrace.resize(tmpArray.toInt());
    qDebug()<<"MetropolisVariable::loadFromStreamOfQByteArray"<<mRawTrace.size();
   /* int it = 0;
    for (; it <mRawTrace.size(); ++it) {
        qreal tmp;
        *in >> tmp;
        mRawTrace[it]= (double) tmp;
    }
    qDebug()<<"MetropolisVariable::loadFromStream it="<<it; */

    std::generate(mRawTrace.begin(),mRawTrace.end(),[in]{QByteArray tmp; *in >> tmp; return tmp.toDouble();});

   // *in >>  mRawTrace;

    // regeneration of this->mFormatedTrace
    updateFormatedTrace();
    /*
    *in >>  this->mFormatedTrace;*/

}
