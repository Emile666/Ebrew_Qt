QT += widgets

CONFIG += static

HEADERS = hmi_objects.h \
    MainEbrew.h \
    dialogbrewdaysettings.h \
    dialogeditfixparameters.h \
    dialogeditmashscheme.h \
    dialogoptionsmeasurements.h \
    dialogoptionspidsettings.h \
    draw_hmi_screen.h \
    scheduler.h

SOURCES = main.cpp hmi_objects.cpp \
    MainEbrew.cpp \
    dialogbrewdaysettings.cpp \
    dialogeditfixparameters.cpp \
    dialogeditmashscheme.cpp \
    dialogoptionsmeasurements.cpp \
    dialogoptionspidsettings.cpp \
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
    dialogoptionsmeasurements.ui \
    dialogoptionspidsettings.ui
