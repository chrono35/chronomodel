#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QString>
//class QString;

#define APP_SETTINGS_DEFAULT_AUTO_SAVE false
#define APP_SETTINGS_DEFAULT_AUTO_SAVE_DELAY_SEC 300
#define APP_SETTINGS_DEFAULT_SHOW_HELP true
#define APP_SETTINGS_DEFAULT_CELL_SEP ","
#define APP_SETTINGS_DEFAULT_DEC_SEP "."
#define APP_SETTINGS_DEFAULT_OPEN_PROJ true
#define APP_SETTINGS_DEFAULT_PIXELRATIO 1

#define APP_SETTINGS_DEFAULT_FORMATDATE "BC/AD"

#define APP_SETTINGS_STR_AUTO_SAVE "auto_save_enabled"
#define APP_SETTINGS_STR_AUTO_SAVE_DELAY_SEC "auto_save_delay"
#define APP_SETTINGS_STR_SHOW_HELP "show_help"
#define APP_SETTINGS_STR_CELL_SEP "csv_cell_sep"
#define APP_SETTINGS_STR_DEC_SEP "csv_dec_sep"
#define APP_SETTINGS_STR_OPEN_PROJ "auto_open_project"

extern QString g_FormatDate; ;

class AppSettings
{
public:
    AppSettings();
    AppSettings(const AppSettings& s);
    AppSettings& operator=(const AppSettings& s);
    void copyFrom(const AppSettings& s);
    virtual ~AppSettings();

public:
    bool mAutoSave;
    int mAutoSaveDelay;
    bool mShowHelp;
    QString mCSVCellSeparator;
    QString mCSVDecSeparator;
    bool mOpenLastProjectAtLaunch;
    short mPixelRatio;
    QString mFormatDate;
    
    
};

#endif
