﻿#ifndef MHVARIABLE_H
#define MHVARIABLE_H

#include "MetropolisVariable.h"


class MHVariable: public MetropolisVariable
{
public:
    MHVariable();
    virtual ~MHVariable();
    
    virtual void reset();
    
    double getCurrentAcceptRate();
    void saveCurrentAcceptRate();
    
    bool tryUpdate(const double x, const double rapportToTry);
    
    QVector<double> acceptationForChain(const QList<Chain>& chains, int index);
    void generateGlobalRunAcceptation(const QList<Chain>& chains);
    
    void generateNumericalResults(const QList<Chain>& chains);
    QString resultsText(const QString& noResultMessage = QObject::tr("No result to display")) const;

    void saveToStream(QDataStream *out); // ajout PhD
    void loadFromStream(QDataStream *in); // ajout PhD
    
public:
    double mSigmaMH;
    
    // Buffer glissant de la taille d'un batch pour calculer la courbe d'évolution
    // du taux d'acceptation chaine par chaine
    QVector<bool> mLastAccepts;
    int mLastAcceptsLength;
    
    // Buffer contenant toutes les acceptations cumulées pour toutes les chaines
    // sur les parties acquisition uniquement.
    // A stocker dans le fichier résultats .dat !
    QVector<bool> mAllAccepts;
    
    // Computed at the end as numerical result :
    double mGlobalAcceptation;
    
    // Buffer contenant tous les taux d'acceptation calculés (1 par batch)
    // On en affiche des sous-parties (correspondant aux chaines) dans la vue des résultats
    // A stocker dans les résultats!
    QVector<double> mHistoryAcceptRateMH;
   // QVector<float> mHistoryAcceptRateMH;
    QString mProposal;
};

#endif
