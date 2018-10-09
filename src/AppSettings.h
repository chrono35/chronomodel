#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "Singleton.h"
#include "DateUtils.h"

#include <qsystemdetection.h>
#include <QString>
#include <QLocale>
#include <QFont>


//#define APP_SETTINGS_DEFAULT_LANGUAGE QLocale::English
//#define APP_SETTINGS_DEFAULT_COUNTRY QLocale::UnitedKingdom
/*
#ifdef Q_OS_MAC
   #define APP_SETTINGS_DEFAULT_FONT_FAMILY "Calibri" // "Helvetica" //"Zapfino"
   #define APP_SETTINGS_DEFAULT_FONT_SIZE 12.
#endif

#ifdef Q_OS_WIN
    #define APP_SETTINGS_DEFAULT_FONT_FAMILY "Calibri"// "Caladea" //"Calibri""Helvetica"
    #define APP_SETTINGS_DEFAULT_FONT_SIZE  12
#endif

#ifdef Q_OS_LINUX
    #define APP_SETTINGS_DEFAULT_FONT_FAMILY "Sans Serif"
    #define APP_SETTINGS_DEFAULT_FONT_SIZE  10
#endif
*/

#define APP_SETTINGS_DEFAULT_ICON_SIZE 3

#define APP_SETTINGS_DEFAULT_AUTO_SAVE false
#define APP_SETTINGS_DEFAULT_AUTO_SAVE_DELAY_SEC 300
#define APP_SETTINGS_DEFAULT_SHOW_HELP false
#define APP_SETTINGS_DEFAULT_CELL_SEP ","
#define APP_SETTINGS_DEFAULT_DEC_SEP "."
#define APP_SETTINGS_DEFAULT_OPEN_PROJ false
#define APP_SETTINGS_DEFAULT_PIXELRATIO 1
#define APP_SETTINGS_DEFAULT_DPM 96
#define APP_SETTINGS_DEFAULT_IMAGE_QUALITY 100
#define APP_SETTINGS_DEFAULT_FORMATDATE DateUtils::eBCAD
#define APP_SETTINGS_DEFAULT_PRECISION 0
#define APP_SETTINGS_DEFAULT_SHEET 10

#define APP_SETTINGS_STR_LANGUAGE "language"
#define APP_SETTINGS_STR_COUNTRY "country"
// #define APP_SETTINGS_STR_FONT_FAMILY "font_family"
//#define APP_SETTINGS_STR_FONT_SIZE "font_size"
#define APP_SETTINGS_STR_ICON_SIZE "icon_size"

#define APP_SETTINGS_STR_AUTO_SAVE "auto_save_enabled"
#define APP_SETTINGS_STR_AUTO_SAVE_DELAY_SEC "auto_save_delay"
#define APP_SETTINGS_STR_SHOW_HELP "show_help"
#define APP_SETTINGS_STR_CELL_SEP "csv_cell_sep"
#define APP_SETTINGS_STR_DEC_SEP "csv_dec_sep"
#define APP_SETTINGS_STR_OPEN_PROJ "auto_open_project"
#define APP_SETTINGS_STR_PIXELRATIO "pixel_ratio"
#define APP_SETTINGS_STR_DPM "dpm"
#define APP_SETTINGS_STR_IMAGE_QUALITY "image_quality"
#define APP_SETTINGS_STR_FORMATDATE "format_date"
#define APP_SETTINGS_STR_PRECISION "precision"
#define APP_SETTINGS_STR_SHEET "sheet"


class AppSettings
{
public:
    AppSettings();
   // AppSettings(const AppSettings& s);
   // AppSettings& operator=(const AppSettings& s);
   // void copyFrom(const AppSettings& s);
    virtual ~AppSettings();

    static void readSettings();
    static void writeSettings();

 //   static QFont font();
 //   static void setFont(const QFont &font);
    static int widthUnit();
    static int heigthUnit();
 //   static int fontDescent();

    static void setWidthUnit(int &width) {mWidthUnit = width;}
    static void setHeigthUnit(int &heigth) {mHeigthUnit = heigth;}

public:
   // static int mButtonWidth; // must be define with mWidthUnit and mHeigthUnit, according to the def screen

    static QLocale::Language mLanguage;
    static QLocale::Country mCountry;

    static bool mAutoSave;
    static int mAutoSaveDelay;
    static bool mShowHelp;
    static QString mCSVCellSeparator;
    static QString mCSVDecSeparator;
    static bool mOpenLastProjectAtLaunch;
    static int mPixelRatio;
    static int mIconSize;
    static int mDpm;
    static int mImageQuality;
    static  DateUtils::FormatDate mFormatDate;
    static int mPrecision;
    static int mNbSheet;
 //   static QString mFontFamily;
//    static  int  mFontPointSize;

    static QString mLastDir;
    static QString mLastFile;

    static QSize mLastSize;
    static QPoint mLastPosition;

private:
//  static  QFont mFont;
  static int mWidthUnit;
  static int mHeigthUnit;
//  static int mFontDescent;


};

#endif
