/* ---------------------------------------------------------------------

Copyright or © or Copr. CNRS	2014 - 2018

Authors :
	Philippe LANOS
	Helori LANOS
 	Philippe DUFRESNE

This software is a computer program whose purpose is to
create chronological models of archeological data using Bayesian statistics.

This software is governed by the CeCILL V2.1 license under French law and
abiding by the rules of distribution of free software.  You can  use,
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability.

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or
data to be ensured and,  more generally, to use and operate it in the
same conditions as regards security.

The fact that you are presently reading this means that you have had
knowledge of the CeCILL V2.1 license and that you accept its terms.
--------------------------------------------------------------------- */

#include "ChronocurveUtilities.h"
#include <algorithm>
#include <iostream>

std::vector<double> calculVecH(const std::vector<double>& vec)
{
    std::vector<double> result;
    for (unsigned long i = 0; i < (vec.size() - 1); ++i) {
        result.push_back(vec[i + 1] - vec[i]);
    }
    return result;
}

// --------------- Function with list of double value

std::vector<std::vector<double>> calculMatR(const std::vector<double>& vec)
{
    // Calcul de la matrice R, de dimension (n-2) x (n-2) contenue dans une matrice n x n
    // Par exemple pour n = 5 :
    // 0 0 0 0 0
    // 0 X X X 0
    // 0 X X X 0
    // 0 X X X 0
    // 0 0 0 0 0

    // vecH est de dimension n-1
    std::vector<double> vecH = calculVecH(vec);
    const int n = vec.size();

    // matR est de dimension n-2 x n-2, mais contenue dans une matrice nxn
    std::vector<std::vector<double>> matR = initMatrice(n, n);
    // On parcourt n-2 valeurs :
    for (int i = 1; i < n-1; ++i) {
        matR[i][i] = (vecH[i-1] + vecH[i]) / 3.;
        // Si on est en n-2 (dernière itération), on ne calcule pas les valeurs de part et d'autre de la diagonale (termes symétriques)
        if (i < n-2) {
            matR[i][i+1] = vecH[i] / 6.;
            matR[i+1][i] = vecH[i] / 6.;
        }
    }
    return matR;
}

std::vector<std::vector<double>> calculMatQ(const std::vector<double>& vec)
{
    // Calcul de la matrice Q, de dimension n x (n-2) contenue dans une matrice n x n
    // Les 1ère et dernière colonnes sont nulles
    // Par exemple pour n = 5 :
    // 0 X 0 0 0
    // 0 X X X 0
    // 0 X X X 0
    // 0 X X X 0
    // 0 0 0 X 0

    // vecH est de dimension n-1
    std::vector<double> vecH = calculVecH(vec);
    const int n = vec.size();

    // matQ est de dimension n x n-2, mais contenue dans une matrice nxn
    std::vector<std::vector<double>> matQ = initMatrice(n, n);
    // On parcourt n-2 valeurs :
    for (unsigned int i = 1; i < vecH.size(); ++i) {
        matQ[i-1][i] = 1. / vecH[i-1];
        matQ[i][i] = -((1./vecH[i-1]) + (1./vecH[i]));
        matQ[i+1][i] = 1. / vecH[i];
    }
    return matQ;
}

#pragma mark Init vecteurs et matrices

std::vector<double> initVecteur(int dim)
{
    std::vector<double> vec;
    for (int i = 0; i < dim; ++i) {
        vec.push_back(0.);
    }
    return vec;
    // normaly: return std::vector<double>(dim)
}

std::vector<std::vector<double>> initMatrice(const int rows, const int cols)
{
    std::vector<std::vector<double>> matrix;
    for (int r = 0; r < rows; ++r) {
        std::vector<double> row;
        for (int c = 0; c < cols; ++c) {
            row.push_back(0.);
        }
        matrix.push_back(row);
    }
    return matrix;
}


bool sortItems(const double& a, const double& b)
{
    return (a < b);
}

void display(const std::vector<double>& v)
{
    for (std::vector<double>::const_iterator it=v.begin(); it!=v.end(); ++it) {
        std::cout << *it << ' ' ;
    }
    std::cout << std::endl;
}

std::vector<double> ChronocurveUtilities::definitionNoeuds(const std::vector<double>& tabPts, const double minStep)
{
    display(tabPts);
    
    std::vector<double> result = tabPts;
    sort(result.begin(), result.end(), sortItems);
    
    // Espacement possible ?
    if ((result[result.size() - 1] - result[0]) < (result.size() - 1) * minStep) {
        std::cerr << "Pas assez de place pour écarter les points" << std::endl;
        exit(0);
    }
    
    display(result);
    
    // Il faut au moins 3 points
    if (result.size() >= 3) {
        // 0 veut dire qu'on n'a pas détecté d'égalité :
        unsigned long startIndex = 0;
        unsigned long endIndex = 0;
        
        for (unsigned long i=1; i<result.size(); ++i) {
            double value = result[i];
            double lastValue = result[i - 1];
            
            // Si l'écart n'est pas suffisant entre la valeur courante et la précedente,
            // alors on mémorise l'index précédent comme le début d'une égalité
            // (à condition de ne pas être déjà dans une égalité)
            if ((value - lastValue < minStep) && (startIndex == 0)) {
                // La valeur à l'index 0 ne pourra pas être déplacée vers la gauche !
                // S'il y a égalité dès le départ, on considère qu'elle commence à l'index 1.
                startIndex = (i == 1) ? 1 : (i-1);
            }
            
            std::cout << "i = " << i << " | value = " << value << " | lastValue = " << lastValue << " | startIndex = " << startIndex << std::endl;
            
            // Si on est à la fin du tableau et dans un cas d'égalité,
            // alors on s'assure d'avoir suffisamment d'espace disponible
            // en incluant autant de points précédents que nécessaire dans l'égalité.
            if ((i == result.size() - 1) && (startIndex != 0)) {
                endIndex = i-1;
                for (unsigned long j=startIndex; j>=1; j--) {
                    double delta = value - result[j-1];
                    double deltaMin = minStep * (i - j + 1);

                    if (delta >= deltaMin) {
                        startIndex = j;
                        std::cout << "=> Egalité finale | startIndex = " << startIndex << " | endIndex = " << endIndex << std::endl;
                        break;
                    }
                }
            }
            
            // Si l'écart entre la valeur courante et la précédente est suffisant
            // ET que l'on était dans un cas d'égalité (pour les valeurs précédentes),
            // alors il se peut qu'on ait la place de les espacer.
            if ((value - lastValue >= minStep) && (startIndex != 0)) {
                double startValue = result[startIndex-1];
                double delta = (value - startValue);
                double deltaMin = minStep * (i - startIndex + 1);
                
                std::cout << "=> Vérification de l'espace disponible | delta = " << delta << " | deltaMin = " << deltaMin << std::endl;
                
                if (delta >= deltaMin) {
                    endIndex = i-1;
                }
            }
            
            if (endIndex != 0) {
                std::cout << "=> On espace les valeurs entre les bornes " << result[startIndex - 1] << " et " << result[i] << std::endl;
                
                // On a la place d'espacer les valeurs !
                // - La borne inférieure ne peut pas bouger (en startIndex-1)
                // - La borne supérieure ne peut pas bouger (en endIndex)
                // => On espace les valeurs intermédiaires (de startIndex à endIndex-1) du minimum nécessaire
                double startSpread = result[endIndex] - result[startIndex];
                for (unsigned long j=startIndex; j<=endIndex; j++) {
                    if(result[j] - result[j-1] < minStep){
                        result[j] = result[j-1] + minStep;
                    }
                }
                // En espaçant les valeurs vers la droite, on a "décentré" l'égalité.
                // => On redécale tous les points de l'égalité vers la gauche pour les recentrer :
                double endSpread = result[endIndex] - result[startIndex];
                double shiftBack = (endSpread - startSpread) / 2;
                
                // => On doit prendre garde à ne pas trop se rappocher le la borne de gauche :
                if ((result[startIndex] - shiftBack) - result[startIndex-1] < minStep){
                    shiftBack = result[startIndex] - (result[startIndex-1] + minStep);
                }
                
                // On doit décaler suffisamment vers la gauche pour ne pas être trop près de la borne de droite :
                if (result[endIndex + 1] - (result[endIndex] - shiftBack) < minStep){
                    shiftBack = result[endIndex] - (result[endIndex + 1] - minStep);
                }
                for (unsigned long j=startIndex; j<=endIndex; j++) {
                    result[j] -= shiftBack;
                }
                
                // On marque la fin de l'égalité
                startIndex = 0;
                endIndex = 0;
            }
        }
    }
    
    return result;
    
    /*
    bool hasEquality = false;
    bool equalityEnded = false;
    vector<double>::iterator equalityStartIterator;
    vector<double>::iterator equalityEndIterator;
    double equalityStartValue;
    double equalityEndValue;
    
    for (vector<double>::iterator it=result.begin(); it!=result.end(); ++it)
    {
        double val = *it;
        double nextVal = NULL;
        
        ++it;
        if(it != result.end()){
            nextVal = *it;
            if(nextVal - val < minStep){
                // On a une égalité
                if(!hasEquality == true){
                    // C'est le début d'une nouvelle égalité
                    hasEquality = true;
                    // On se souvient de la borne inférieure
                    --it;
                    equalityStartIterator = it;
                    if(it != result.begin()){
                        --it;
                        equalityStartValue = *it;
                        ++it;
                    }else{
                        equalityStartValue = *it;
                    }
                    ++it;
                }else{
                    // On avait déjà une égalité. On attend juste d'en trouver la fin
                }
            }else{
                // On a pas égalité
                if(hasEquality == true){
                    // On a atteint la fin de l'égalite
                    // TODO : On vérifie si on a la place d'écarter les valeurs
                    // Si oui, alors on écarte les pts lors de cette itération
                    // Si non, on fait rentrer la valeur courante dans l'égalité et on continue à parcourir les pts jusqu'à avoir l'espace suffisant pour écarter les points en égalité
                    equalityEnded = true;
                    equalityEndIterator = it;
                    equalityEndValue = *it;
                }
            }
        }else{
            if(hasEquality == true){
                // On a atteint la fin sur une égalite : on garde la dernière borne, puis on espace les éléments
                equalityEnded = true;
                equalityEndIterator = it-1;
                equalityEndValue = *(it-1);
            }
        }
        --it;
        
        // Do the shift if necessary
        if(equalityEnded){
            cout << ' ' << *equalityStartIterator << ' ' << *equalityEndIterator;
            
            ResolT = ---
            
            t0 ---------- t1 --- t2 - t3 - t4 -- t5
            
            t0 ----- t1 --- t2 --- t3 --- t4 --- t5
            
            double start; // valeur de t1
            double end; // valeur de t6
            
            
            deltaCravate = t5 - t2;
            deltaDispo = t6 - t1;
            deltaTotal = 3 * minStep;
            deltaCumul = 0;
            
            for(t = t2 à t5)
            {
                
                
                t'2 = t2 - 3 * minStep / 2;
                t'3 = t2 - 1 * minStep / 2;
                t'4 = t2 + 1 * minStep / 2;
                t'5 = t2 + 3 * minStep / 2;
                
                deltaT = (1 + random()) * minStep;
                
                t = deltaCumul + deltaT;
                deltaCumul += deltaT;
                
                
                
                
                delta2 = minStep * (1 + random());
                t2 += delta2;
                deltaTotal += delta2;
            }
            
            
            hasEquality = false;
            equalityEnded = false;
        }
    }
    
    for (vector<double>::iterator it=result.begin(); it!=result.end(); ++it)
        cout << ' ' << *it;
    */
    return result;
}
