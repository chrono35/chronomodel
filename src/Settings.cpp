#include "Settings.h"


Settings::Settings():
mAutoSave(true),
mAutoSaveDelay(300)
{
    
}
Settings::Settings(const Settings& s)
{
    copyFrom(s);
}
Settings& Settings::operator=(const Settings& s)
{
    copyFrom(s);
    return *this;
}
void Settings::copyFrom(const Settings& s)
{
    mAutoSave = s.mAutoSave;
    mAutoSaveDelay = s.mAutoSaveDelay;
}
Settings::~Settings()
{
    
}
