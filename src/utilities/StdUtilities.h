﻿#ifndef STDUTILITIES_H
#define STDUTILITIES_H

#include <vector>
#include <map>
#include <random>
#include <cmath>
#include <algorithm>
#include <math.h>

#include <QMap>
#include <QVector>
#include <QList>
#include <QDebug>

#define DEBUG
#ifdef DEBUG
#include <iostream>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846 //usefull to Windows
#endif


typedef QString (*FormatFunc)(const float);


int compareStrings(const std::string &s1, const std::string &s2);


double safeExp(const double& x, int n = 10);
double safeLog(const double& x, int n = 5);

void checkFloatingPointException(const QString& infos = QString());

template <typename T, typename V>
V interpolate(const T& x, const T& x1, const T& x2, const V& y1, const V& y2)
{
    return (y1 + (y2 - y1) * (V)((x - x1) / (x2 - x1)) );
}

template <typename T, typename U>
T interpolateValueInQMap(const U& key, const QMap<U, T>& map)
{
    auto lIter = map.lowerBound(key);
    T valueUpper = lIter.value();
    T keyUpper = lIter.key();
    
    if (key<=keyUpper)
        return valueUpper;
    
    else if (lIter!=map.end()) {
        T valueUnder = (lIter - 1).value();
        T keyUnder =(lIter - 1).key();
        
        return interpolate(key, keyUnder, keyUpper, valueUnder, valueUpper);
    }
    else {
        return map.last();
    }
}


template <class T>
T vector_max_value(const QList<T>& aVector)
{
    typename QList<T>::const_iterator it = std::max_element(aVector.cbegin(), aVector.cend());
    if(it != aVector.cend())
        return *it;
    return 0;
}

template <class T>
T vector_min_value(const QList<T>& aVector)
{
    typename QList<T>::const_iterator it = std::min_element(aVector.cbegin(), aVector.cend());
    if(it != aVector.cend())
        return *it;
    return 0;
}

template <class T>
T vector_max_value(const QVector<T>& aVector)
{
    typename QVector<T>::const_iterator it = std::max_element(aVector.cbegin(), aVector.cend());
    if(it != aVector.cend())
        return *it;
    return 0;
}

template <class T>
T vector_min_value(const QVector<T>& aVector)
{
    typename QVector<T>::const_iterator it = std::min_element(aVector.cbegin(), aVector.cend());
    if(it != aVector.cend())
        return *it;
    return 0;
}

 template<typename T>
 const T inRange(const T minimum, const T value ,const T maximum)
{
    // see qBound in qglobal
    return std::max(minimum, std::min(maximum, value));
}

template <class U, class T>
T map_max_value(const QMap<U, T>& aMap)
{
    if(aMap.isEmpty()) return (T)0;
    QMapIterator<U, T> iter(aMap);
    T max = iter.hasNext() ?  iter.next().value()  :  (T) 0 ;
    
    while(iter.hasNext())
    {
        iter.next();
        max = qMax(max, iter.value());
    }
    return max;
}

template <class U, class T>
T map_min_value(const QMap<U, T>& aMap)
{
    QMapIterator<U, T> iter(aMap);
    T min = iter.hasNext() ?  iter.next().value()  :   0 ;
   
    while(iter.hasNext())
    {
        iter.next();
        min = qMin(min, iter.value());
    }
    return min;
}

// --------------------------------
template<typename T>
T sum(const QVector<T>& vector){
    T s = 0;
   /* std::for_each(vector.begin(), vector.end(), [&s](T& v){
        s += v;
    });*/
    foreach (const T v, vector) {
        s += v;
    }
    return s;
}

template<typename T>
T sum2(const QVector<T>& vector){
    T sum = 0;
    /*std::for_each(vector.cbegin(), vector.cend(), [&sum](T& v){
        sum += v * v;
    });*/
    foreach (const T v, vector) {
        sum += v * v;
    }
    return sum;
}

template<typename T>
T sumShifted(const QVector<T>& vector, const T& shift){
    T sum = 0;
    /*std::for_each(vector.cbegin(), vector.cend(), [&sum, &shift](T& v){
        sum += v + shift;
    });*/
    foreach (const T v, vector) {
        sum += v + shift;
    }
    return sum;
}

template<typename T>
T sum2Shifted(const QVector<T>& vector, const T& shift){
    T sum = 0;
    /*std::for_each(vector.cbegin(), vector.cend(), [&sum, &shift](T& v){
        sum += (v + shift) * (v + shift);
    });*/
    foreach (const T v, vector)
        sum += (v + shift) * (v + shift);


    return sum;
}
// --------------------------------
template<typename T>
QMap<T, T> normalize_map(const QMap<T, T>& aMap, const T max = 1)
{
    T max_value = map_max_value(aMap);

    QMap<T, T> result;
    // can be done with std::generate !!
    for( typename QMap<T, T>::const_iterator it = aMap.begin(); it != aMap.end(); ++it)
        result[it.key()] = (it.value() / max_value)*max;

    return result;
}

QVector<double> normalize_vector(const QVector<double>& aVector);
QVector<float> normalize_vector(const QVector<float>& aVector);

QVector<double> stretch_vector(const QVector<double>& aVector, const double from, const double to);
QVector<float> stretch_vector(const QVector<float>& aVector, const float from, const float to);

//QMap<double, double> normalize_map(const QMap<double, double>& aMap);
//QMap<float, float> normalize_map(const QMap<float, float>& aMap);

QMap<double, double> equal_areas(const QMap<double, double>& mapToModify, const QMap<double, double>& mapWithTargetArea);
QMap<float, float> equal_areas(const QMap<float, float>& mapToModify, const QMap<float, float>& mapWithTargetArea);
QMap<double, double> equal_areas(const QMap<double, double>& mapToModify, const double targetArea);
QMap<float, float> equal_areas(const QMap<float, float>& mapToModify, const float targetArea);

QVector<double> equal_areas(const QVector<double>& data, const double step, const double area);
QVector<float> equal_areas(const QVector<float>& data, const float step, const float area);

QMap<float, float> vector_to_map(const QVector<float>& data, const float min, const float max, const float step);
double vector_interpolate_idx_for_value(const double value, const QVector<double> &vector);
float vector_interpolate_idx_for_value(const float value, const QVector<float> &vector);

double map_area(const QMap<double, double>& map);
float map_area(const QMap<float, float>& map);
const QMap<float, float> create_HPD(const QMap<float, float>& aMap, const float threshold);
QVector<float> vector_to_histo(const QVector<float>& dataScr, const float tmin, const float tmax, const int nbPts);
#endif
