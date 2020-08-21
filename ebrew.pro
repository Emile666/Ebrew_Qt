QT += widgets serialport network

CONFIG += static

HEADERS = hmi_objects.h \
    MainEbrew.h \
    controlobjects.h \
    dialogbrewdaysettings.h \
    dialogeditfixparameters.h \
    dialogeditmashscheme.h \
    dialogeditterminal.h \
    dialogoptionsmeasurements.h \
    dialogoptionspidsettings.h \
    dialogoptionssystemsettings.h \
    dialogviewprogress.h \
    dialogviewstatusalarms.h \
    dialogviewtasklist.h \
    draw_hmi_screen.h \
    scheduler.h

SOURCES = main.cpp hmi_objects.cpp \
    MainEbrew.cpp \
    controlobjects.cpp \
    dialogbrewdaysettings.cpp \
    dialogeditfixparameters.cpp \
    dialogeditmashscheme.cpp \
    dialogeditterminal.cpp \
    dialogoptionsmeasurements.cpp \
    dialogoptionspidsettings.cpp \
    dialogoptionssystemsettings.cpp \
    dialogviewprogress.cpp \
    dialogviewstatusalarms.cpp \
    dialogviewtasklist.cpp \
    draw_hmi_screen.cpp \
    scheduler.cpp

RC_ICONS = img/beer.ico

RESOURCES += icons.qrc

# install
target.path = $$PWD/./
INSTALLS += target

FORMS += \
    dialogbrewdaysettings.ui \
    dialogeditfixparameters.ui \
    dialogeditmashscheme.ui \
    dialogeditterminal.ui \
    dialogoptionsmeasurements.ui \
    dialogoptionspidsettings.ui \
    dialogoptionssystemsettings.ui \
    dialogviewprogress.ui \
    dialogviewstatusalarms.ui \
    dialogviewtasklist.ui
