﻿#ifndef PROJECTSETTINGS_H
#define PROJECTSETTINGS_H

#include <QString>
#include <QJsonObject>
#include "StateKeys.h"

#define STATE_SETTINGS_TMIN_DEF 0
#define STATE_SETTINGS_TMAX_DEF 0
#define STATE_SETTINGS_STEP_DEF 1
#define STATE_SETTINGS_STEP_FORCED_DEF false


class ProjectSettings
{
public:
    ProjectSettings();
    ProjectSettings(const ProjectSettings& s);
    ProjectSettings& operator=(const ProjectSettings& s);
    bool operator!=(const ProjectSettings& s);
    bool operator==(const ProjectSettings& s);
    bool isEqual(const ProjectSettings& s);
    void copyFrom(const ProjectSettings& s);
    virtual ~ProjectSettings();
    
    static ProjectSettings fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
    
    static float getStep(const float tmin, const float tmax);

    float getTminFormated() const;
    float getTmaxFormated() const ;

public:
    float mTmin;
    float mTmax;
    float mStep;
    bool mStepForced;   
};

#endif
