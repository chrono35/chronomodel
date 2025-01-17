#include "AppSettings.h"
#include <QString>
#include <QLocale>

AppSettings::AppSettings():
mAutoSave(APP_SETTINGS_DEFAULT_AUTO_SAVE),
mAutoSaveDelay(APP_SETTINGS_DEFAULT_AUTO_SAVE_DELAY_SEC),
mShowHelp(APP_SETTINGS_DEFAULT_SHOW_HELP),

mOpenLastProjectAtLaunch(APP_SETTINGS_DEFAULT_OPEN_PROJ),
mPixelRatio(APP_SETTINGS_DEFAULT_PIXELRATIO),
mDpm(APP_SETTINGS_DEFAULT_DPM),
mImageQuality(APP_SETTINGS_DEFAULT_IMAGE_QUALITY),
mFormatDate(APP_SETTINGS_DEFAULT_FORMATDATE),
mPrecision(APP_SETTINGS_DEFAULT_PRECISION)
{
    QLocale newLoc(QLocale::system());
    mLanguage = newLoc.language();
    mCountry = newLoc.country();
    newLoc.setNumberOptions(QLocale::OmitGroupSeparator);
    QLocale::setDefault(newLoc);

    if(newLoc.decimalPoint()==',') {
        mCSVCellSeparator=";";
        mCSVDecSeparator=",";
    }
    else {
        mCSVCellSeparator=",";
        mCSVDecSeparator=".";
    }
}
AppSettings::AppSettings(const AppSettings& s)
{
    copyFrom(s);
   
}

AppSettings& AppSettings::operator=(const AppSettings& s)
{
    copyFrom(s);
    return *this;
}

void AppSettings::copyFrom(const AppSettings& s)
{
    mLanguage = s.mLanguage;
    mCountry = s.mCountry;
    mAutoSave = s.mAutoSave;
    mAutoSaveDelay = s.mAutoSaveDelay;
    mShowHelp = s.mShowHelp;
    mCSVCellSeparator = s.mCSVCellSeparator;
    mCSVDecSeparator = s.mCSVDecSeparator;
    mOpenLastProjectAtLaunch = s.mOpenLastProjectAtLaunch;
    mPixelRatio = s.mPixelRatio;
    mDpm = s.mDpm;
    mImageQuality = s.mImageQuality;
    mFormatDate = s.mFormatDate;
    mPrecision = s.mPrecision;
}
AppSettings::~AppSettings()
{
    
}
