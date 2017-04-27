#-------------------------------------------------
#
# Project created by QtCreator 2017-03-31T15:32:49
#
#-------------------------------------------------

QT       += testlib

#QT       -= gui

# Qt modules (must be deployed along with the application)
# in the Unit Tes, it must be the same as Chronomodel
QT += core gui widgets svg

TARGET = tst_chronomodeltest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

message("PWDnew : $$PWD")
message("PRO_PATH : $$_PRO_FILE_PWD_")
message("BUILD_DIR : $$BUILD_DIR")
message("DESTDIR : $$DESTDIR")
message("OBJECTS_DIR : $$OBJECTS_DIR")
message("MOC_DIR : $$MOC_DIR")
message("RCC_DIR : $$RCC_DIR")

SOURCES += tst_chronomodeltest.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

#########################################
# DEFINES
#########################################

DEFINES += _USE_MATH_DEFINES

# Activate this to use FFT kernel method on histograms

USE_FFT = 1
DEFINES += "USE_FFT=$${USE_FFT}"

# Choose the plugins to compile directly with the application

USE_PLUGIN_UNIFORM = 1
USE_PLUGIN_GAUSS = 1
USE_PLUGIN_14C = 1
USE_PLUGIN_TL = 1
USE_PLUGIN_AM = 1

DEFINES += "USE_PLUGIN_UNIFORM=$${USE_PLUGIN_UNIFORM}"
DEFINES += "USE_PLUGIN_GAUSS=$${USE_PLUGIN_GAUSS}"
DEFINES += "USE_PLUGIN_14C=$${USE_PLUGIN_14C}"
DEFINES += "USE_PLUGIN_TL=$${USE_PLUGIN_TL}"
DEFINES += "USE_PLUGIN_AM=$${USE_PLUGIN_AM}"

#########################################
# FFTW
#########################################

macx{
        # IMPORTANT NOTE :
        # We use FFTW 3.2.2 on Mac to support Mac OS X versions from 10.7.
        # (Using FFTW 3.3.4 is available for mac 10.9+)
        # We provide FFTW.3.2.2.dmg if you want to install it on your system, but this is not necessary!
        # The generated XCode project will locate FFTW files in the project directory and statically link against it.

        # this is to include fftw.h in the code :
        #INCLUDEPATH += $$_PRO_FILE_PWD_/lib/FFTW/mac    # this is the Chronomodel Code
        INCLUDEPATH += ../../lib/FFTW/mac


        # Link the application with FFTW library
        # If no dylib are present, static libs (.a) are used => that's why we moved .dylib files in a "dylib" folder.
        #LIBS += -L"$$_PRO_FILE_PWD_/lib/FFTW/mac" -lfftw3   # this is the Chronomodel Code
        LIBS += -L"../../lib/FFTW/mac" -lfftw3


        # If we were deploying FFTW as a dynamic library, we should :
        # - Move all files from "lib/FFTW/mac/dylib" to "lib/FFTW/mac"
        # - Uncomment the lines below to copy dylib files to the bundle
        # - We may also need to call install_name_tool on both dylib and chronomodel executable.
        #	This has not been tested, so use otool -L path/to/dylib/files to check dependencies

        #FFTW_FILES.path = Contents/Frameworks
        #FFTW_FILES.files += $$PRO_PATH/deploy/mac/FFTW/libfftw3f.dylib
        #QMAKE_BUNDLE_DATA += FFTW_FILES
        #QMAKE_POST_LINK += install_name_tool -id @executable_path/../Frameworks/libfftw3f.dylib $$PRO_PATH/deploy/mac/FFTW/libfftw3f.dylib
        #QMAKE_POST_LINK += install_name_tool -change old/path @executable_path/../Frameworks/libfftw3f.3.dylib $$PRO_PATH/Release/Chronomodel.app/Contents/MacOS/Chronomodel;
}
win32{
        INCLUDEPATH += lib/FFTW
        #LIBS += -L"$$_PRO_FILE_PWD_/lib/FFTW/win32" -lfftw3-3
        LIBS += -L"$$_PRO_FILE_PWD_/lib/FFTW/win64" -lfftw3-3 # to compile with a x64 machine
}
#linux :
unix:!macx{
        INCLUDEPATH += lib/FFTW
        LIBS += -lfftw3
}

#########################################
# TRANSLATIONS
#########################################
TRANSLATIONS = translations/Chronomodel_fr.ts \
                                translations/Chronomodel_en.ts

# For Microsoft Visual Studio only
CODECFORSRC = UTF-8

#########################################
# INCLUDES
#########################################

INCLUDEPATH += ../../src/
INCLUDEPATH += ../../src/mcmc/
INCLUDEPATH += ../../src/model/
INCLUDEPATH += ../../src/plugins/
INCLUDEPATH += ../../src/plugins/plugin_14C/
INCLUDEPATH += ../../src/plugins/plugin_am/
INCLUDEPATH += ../../src/plugins/plugin_gauss/
INCLUDEPATH += ../../src/plugins/plugin_tl/
INCLUDEPATH += ../../src/plugins/plugin_uniform/
INCLUDEPATH += ../../src/project/
INCLUDEPATH += ../../src/ui/
INCLUDEPATH += ../../src/ui/dialogs/
INCLUDEPATH += ../../src/ui/graphs/
INCLUDEPATH += ../../src/ui/lookandfeel/
INCLUDEPATH += ../../src/ui/panel_model/
INCLUDEPATH += ../../src/ui/panel_model/data/
INCLUDEPATH += ../../src/ui/panel_model/scenes/
INCLUDEPATH += ../../src/ui/panel_results/
INCLUDEPATH += ../../src/ui/panel_mcmc/
INCLUDEPATH += ../../src/ui/widgets/
INCLUDEPATH += ../../src/ui/window/
INCLUDEPATH += ../../src/utilities/

#########################################
# HEADERS
#########################################

HEADERS += ../../src/MainController.h
HEADERS += ../../src/AppSettings.h
HEADERS += ../../src/StateKeys.h
HEADERS += ../../src/ChronoApp.h

HEADERS += ../../src/mcmc/Functions.h
HEADERS += ../../src/mcmc/Generator.h
HEADERS += ../../src/mcmc/MCMCLoop.h
HEADERS += ../../src/mcmc/MCMCLoopMain.h
HEADERS += ../../src/mcmc/MCMCSettings.h
HEADERS += ../../src/mcmc/MetropolisVariable.h
HEADERS += ../../src/mcmc/MHVariable.h

HEADERS += ../../src/model/Constraint.h
HEADERS += ../../src/model/Date.h
HEADERS += ../../src/model/Event.h
HEADERS += ../../src/model/EventConstraint.h
HEADERS += ../../src/model/EventKnown.h
HEADERS += ../../src/model/Model.h
HEADERS += ../../src/model/ModelUtilities.h
HEADERS += ../../src/model/Phase.h
HEADERS += ../../src/model/PhaseConstraint.h

HEADERS += ../../src/plugins/CalibrationCurve.h
HEADERS += ../../src/plugins/GraphViewRefAbstract.h
HEADERS += ../../src/plugins/PluginAbstract.h
HEADERS += ../../src/plugins/PluginFormAbstract.h
HEADERS += ../../src/plugins/PluginRefCurveSettingsView.h
HEADERS += ../../src/plugins/PluginSettingsViewAbstract.h
HEADERS += ../../src/plugins/RefCurve.h

equals(USE_PLUGIN_TL, 1){
        HEADERS += ../../src/plugins/plugin_tl/PluginTL.h
        HEADERS += ../../src/plugins/plugin_tl/PluginTLForm.h
        HEADERS += ../../src/plugins/plugin_tl/PluginTLRefView.h
        HEADERS += ../../src/plugins/plugin_tl/PluginTLSettingsView.h
}
equals(USE_PLUGIN_14C, 1){
        HEADERS += ../../src/plugins/plugin_14C/Plugin14C.h
        HEADERS += ../../src/plugins/plugin_14C/Plugin14CForm.h
        HEADERS += ../../src/plugins/plugin_14C/Plugin14CRefView.h
        HEADERS += ../../src/plugins/plugin_14C/Plugin14CSettingsView.h
}
equals(USE_PLUGIN_GAUSS, 1){
        HEADERS += ../../src/plugins/plugin_gauss/PluginGauss.h
        HEADERS += ../../src/plugins/plugin_gauss/PluginGaussForm.h
        HEADERS += ../../src/plugins/plugin_gauss/PluginGaussRefView.h
        HEADERS += ../../src/plugins/plugin_gauss/PluginGaussSettingsView.h
}
equals(USE_PLUGIN_AM, 1){
        HEADERS += ../../src/plugins/plugin_am/PluginMag.h
        HEADERS += ../../src/plugins/plugin_am/PluginMagForm.h
        HEADERS += ../../src/plugins/plugin_am/PluginMagRefView.h
        HEADERS += ../../src/plugins/plugin_am/PluginMagSettingsView.h
}
equals(USE_PLUGIN_UNIFORM, 1){
        HEADERS += ../../src/plugins/plugin_uniform/PluginUniform.h
        HEADERS += ../../src/plugins/plugin_uniform/PluginUniformForm.h
}

HEADERS += ../../src/project/PluginManager.h
HEADERS += ../../src/project/Project.h
HEADERS += ../../src/project/ProjectSettings.h
HEADERS += ../../src/project/SetProjectState.h
HEADERS += ../../src/project/StateEvent.h

HEADERS += ../../src/ui/dialogs/AboutDialog.h
HEADERS += ../../src/ui/dialogs/AppSettingsDialog.h
HEADERS += ../../src/ui/dialogs/AppSettingsDialogItemDelegate.h
HEADERS += ../../src/ui/dialogs/ConstraintDialog.h
HEADERS += ../../src/ui/dialogs/DateDialog.h
HEADERS += ../../src/ui/dialogs/EventDialog.h
HEADERS += ../../src/ui/dialogs/MCMCProgressDialog.h
HEADERS += ../../src/ui/dialogs/MCMCSettingsDialog.h
HEADERS += ../../src/ui/dialogs/PhaseDialog.h
HEADERS += ../../src/ui/dialogs/PluginOptionsDialog.h
HEADERS += ../../src/ui/dialogs/StudyPeriodDialog.h
HEADERS += ../../src/ui/dialogs/TrashDialog.h

HEADERS += ../../src/ui/graphs/AxisTool.h
HEADERS += ../../src/ui/graphs/GraphCurve.h
HEADERS += ../../src/ui/graphs/GraphView.h
HEADERS += ../../src/ui/graphs/GraphViewAbstract.h
HEADERS += ../../src/ui/graphs/GraphZone.h
HEADERS += ../../src/ui/graphs/Ruler.h

HEADERS += ../../src/ui/lookandfeel/Painting.h

HEADERS += ../../src/ui/panel_model/EventPropertiesView.h
HEADERS += ../../src/ui/panel_model/EventsListItemDelegate.h
HEADERS += ../../src/ui/panel_model/ImportDataView.h
HEADERS += ../../src/ui/panel_model/ModelView.h
HEADERS += ../../src/ui/panel_model/SceneGlobalView.h

HEADERS += ../../src/ui/panel_model/data/CalibrationDrawing.h
HEADERS += ../../src/ui/panel_model/data/CalibrationView.h
HEADERS += ../../src/ui/panel_model/data/DatesList.h
HEADERS += ../../src/ui/panel_model/data/DatesListItemDelegate.h

HEADERS += ../../src/ui/panel_model/scenes/AbstractItem.h
HEADERS += ../../src/ui/panel_model/scenes/AbstractScene.h
HEADERS += ../../src/ui/panel_model/scenes/ArrowItem.h
HEADERS += ../../src/ui/panel_model/scenes/ArrowTmpItem.h
HEADERS += ../../src/ui/panel_model/scenes/DateItem.h
HEADERS += ../../src/ui/panel_model/scenes/EventItem.h
HEADERS += ../../src/ui/panel_model/scenes/EventKnownItem.h
HEADERS += ../../src/ui/panel_model/scenes/EventsScene.h
HEADERS += ../../src/ui/panel_model/scenes/PhaseItem.h
HEADERS += ../../src/ui/panel_model/scenes/PhasesScene.h

HEADERS += ../../src/ui/panel_results/GraphViewDate.h
HEADERS += ../../src/ui/panel_results/GraphViewEvent.h
HEADERS += ../../src/ui/panel_results/GraphViewPhase.h
HEADERS += ../../src/ui/panel_results/GraphViewResults.h
HEADERS += ../../src/ui/panel_results/ResultsView.h

HEADERS += ../../src/ui/widgets/Button.h
HEADERS += ../../src/ui/widgets/CheckBox.h
HEADERS += ../../src/ui/widgets/Collapsible.h
HEADERS += ../../src/ui/widgets/ColorPicker.h
HEADERS += ../../src/ui/widgets/GroupBox.h
HEADERS += ../../src/ui/widgets/HelpWidget.h
HEADERS += ../../src/ui/widgets/Label.h
HEADERS += ../../src/ui/widgets/LineEdit.h
HEADERS += ../../src/ui/widgets/Marker.h
HEADERS += ../../src/ui/widgets/RadioButton.h
HEADERS += ../../src/ui/widgets/ScrollCompressor.h
HEADERS += ../../src/ui/widgets/Tabs.h
HEADERS += ../../src/ui/widgets/ZoomControls.h

HEADERS += ../../src/ui/window/MainWindow.h
HEADERS += ../../src/ui/window/ProjectView.h

HEADERS += ../../src/utilities/DateUtils.h
HEADERS += ../../src/utilities/DoubleValidator.h
HEADERS += ../../src/utilities/QtUtilities.h
HEADERS += ../../src/utilities/Singleton.h
HEADERS += ../../src/utilities/StdUtilities.h


#########################################
# SOURCES
#########################################

SOURCES += ../../src/AppSettings.cpp
SOURCES += ../../src/ChronoApp.cpp
#SOURCES += ../../src/main.cpp # Don't use it while testing
SOURCES += ../../src/MainController.cpp

SOURCES += ../../src/mcmc/Functions.cpp
SOURCES += ../../src/mcmc/Generator.cpp
SOURCES += ../../src/mcmc/MCMCLoop.cpp
SOURCES += ../../src/mcmc/MCMCLoopMain.cpp
SOURCES += ../../src/mcmc/MCMCSettings.cpp
SOURCES += ../../src/mcmc/MetropolisVariable.cpp
SOURCES += ../../src/mcmc/MHVariable.cpp

SOURCES += ../../src/model/Constraint.cpp
SOURCES += ../../src/model/Date.cpp
SOURCES += ../../src/model/Event.cpp
SOURCES += ../../src/model/EventConstraint.cpp
SOURCES += ../../src/model/EventKnown.cpp
SOURCES += ../../src/model/Model.cpp
SOURCES += ../../src/model/ModelUtilities.cpp
SOURCES += ../../src/model/Phase.cpp
SOURCES += ../../src/model/PhaseConstraint.cpp

SOURCES += ../../src/plugins/CalibrationCurve.cpp
SOURCES += ../../src/plugins/PluginRefCurveSettingsView.cpp
SOURCES += ../../src/plugins/RefCurve.cpp

equals(USE_PLUGIN_TL, 1){
        SOURCES += ../../src/plugins/plugin_tl/PluginTL.cpp
        SOURCES += ../../src/plugins/plugin_tl/PluginTLForm.cpp
        SOURCES += ../../src/plugins/plugin_tl/PluginTLRefView.cpp
    SOURCES += ../../src/plugins/plugin_tl/PluginTLSettingsView.cpp
}
equals(USE_PLUGIN_14C, 1){
        SOURCES += ../../src/plugins/plugin_14C/Plugin14C.cpp
        SOURCES += ../../src/plugins/plugin_14C/Plugin14CForm.cpp
        SOURCES += ../../src/plugins/plugin_14C/Plugin14CRefView.cpp
        SOURCES += ../../src/plugins/plugin_14C/Plugin14CSettingsView.cpp
}
equals(USE_PLUGIN_GAUSS, 1){
        SOURCES += ../../src/plugins/plugin_gauss/PluginGauss.cpp
        SOURCES += ../../src/plugins/plugin_gauss/PluginGaussForm.cpp
        SOURCES += ../../src/plugins/plugin_gauss/PluginGaussRefView.cpp
        SOURCES += ../../src/plugins/plugin_gauss/PluginGaussSettingsView.cpp
}
equals(USE_PLUGIN_AM, 1){
        SOURCES += ../../src/plugins/plugin_am/PluginMag.cpp
        SOURCES += ../../src/plugins/plugin_am/PluginMagForm.cpp
        SOURCES += ../../src/plugins/plugin_am/PluginMagRefView.cpp
        SOURCES += ../../src/plugins/plugin_am/PluginMagSettingsView.cpp
}
equals(USE_PLUGIN_UNIFORM, 1){
        SOURCES += ../../src/plugins/plugin_uniform/PluginUniform.cpp
        SOURCES += ../../src/plugins/plugin_uniform/PluginUniformForm.cpp
}

SOURCES += ../../src/project/PluginManager.cpp
SOURCES += ../../src/project/Project.cpp
SOURCES += ../../src/project/ProjectSettings.cpp
SOURCES += ../../src/project/SetProjectState.cpp
SOURCES += ../../src/project/StateEvent.cpp

SOURCES += ../../src/ui/dialogs/AboutDialog.cpp
SOURCES += ../../src/ui/dialogs/AppSettingsDialog.cpp
SOURCES += ../../src/ui/dialogs/ConstraintDialog.cpp
SOURCES += ../../src/ui/dialogs/DateDialog.cpp
SOURCES += ../../src/ui/dialogs/EventDialog.cpp
SOURCES += ../../src/ui/dialogs/MCMCProgressDialog.cpp
SOURCES += ../../src/ui/dialogs/MCMCSettingsDialog.cpp
SOURCES += ../../src/ui/dialogs/PhaseDialog.cpp
SOURCES += ../../src/ui/dialogs/PluginOptionsDialog.cpp
SOURCES += ../../src/ui/dialogs/StudyPeriodDialog.cpp
SOURCES += ../../src/ui/dialogs/TrashDialog.cpp

SOURCES += ../../src/ui/graphs/AxisTool.cpp
SOURCES += ../../src/ui/graphs/GraphCurve.cpp
SOURCES += ../../src/ui/graphs/GraphView.cpp
SOURCES += ../../src/ui/graphs/GraphViewAbstract.cpp
SOURCES += ../../src/ui/graphs/GraphZone.cpp
SOURCES += ../../src/ui/graphs/Ruler.cpp

SOURCES += ../../src/ui/lookandfeel/Painting.cpp

SOURCES += ../../src/ui/panel_model/data/CalibrationDrawing.cpp
SOURCES += ../../src/ui/panel_model/data/CalibrationView.cpp
SOURCES += ../../src/ui/panel_model/data/DatesList.cpp

SOURCES += ../../src/ui/panel_model/EventPropertiesView.cpp
SOURCES += ../../src/ui/panel_model/ImportDataView.cpp
SOURCES += ../../src/ui/panel_model/ModelView.cpp
SOURCES += ../../src/ui/panel_model/SceneGlobalView.cpp

SOURCES += ../../src/ui/panel_model/scenes/AbstractItem.cpp
SOURCES += ../../src/ui/panel_model/scenes/AbstractScene.cpp
SOURCES += ../../src/ui/panel_model/scenes/ArrowItem.cpp
SOURCES += ../../src/ui/panel_model/scenes/ArrowTmpItem.cpp
SOURCES += ../../src/ui/panel_model/scenes/DateItem.cpp
SOURCES += ../../src/ui/panel_model/scenes/EventItem.cpp
SOURCES += ../../src/ui/panel_model/scenes/EventKnownItem.cpp
SOURCES += ../../src/ui/panel_model/scenes/EventsScene.cpp
SOURCES += ../../src/ui/panel_model/scenes/PhaseItem.cpp
SOURCES += ../../src/ui/panel_model/scenes/PhasesScene.cpp

SOURCES += ../../src/ui/panel_results/GraphViewDate.cpp
SOURCES += ../../src/ui/panel_results/GraphViewEvent.cpp
SOURCES += ../../src/ui/panel_results/GraphViewPhase.cpp
SOURCES += ../../src/ui/panel_results/GraphViewResults.cpp
SOURCES += ../../src/ui/panel_results/ResultsView.cpp

SOURCES += ../../src/ui/widgets/ColorPicker.cpp
SOURCES += ../../src/ui/widgets/ZoomControls.cpp
SOURCES += ../../src/ui/widgets/Collapsible.cpp
SOURCES += ../../src/ui/widgets/ScrollCompressor.cpp
SOURCES += ../../src/ui/widgets/Button.cpp
SOURCES += ../../src/ui/widgets/CheckBox.cpp
SOURCES += ../../src/ui/widgets/RadioButton.cpp
SOURCES += ../../src/ui/widgets/Label.cpp
SOURCES += ../../src/ui/widgets/LineEdit.cpp
SOURCES += ../../src/ui/widgets/GroupBox.cpp
SOURCES += ../../src/ui/widgets/HelpWidget.cpp
SOURCES += ../../src/ui/widgets/Tabs.cpp
SOURCES += ../../src/ui/widgets/Marker.cpp

SOURCES += ../../src/ui/window/MainWindow.cpp
SOURCES += ../../src/ui/window/ProjectView.cpp

SOURCES += ../../src/utilities/StdUtilities.cpp
SOURCES += ../../src/utilities/QtUtilities.cpp
SOURCES += ../../src/utilities/DoubleValidator.cpp
SOURCES += ../../src/utilities/DateUtils.cpp