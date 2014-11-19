#ifndef QTUTILITIES_H
#define QTUTILITIES_H

#include <QStringList>
#include <QColor>

bool colorIsDark(const QColor& color);
void sortIntList(QList<int>& list);

QList<QStringList> readCSV(const QString& filePath, const QString& separator = ",");
int defaultDpiX();
qreal dpiScaled(qreal value);
QColor getContrastedColor(const QColor& color);
QList<int> stringListToIntList(const QString& listStr, const QString& separator = ",");

bool isComment(const QString& str);

#endif