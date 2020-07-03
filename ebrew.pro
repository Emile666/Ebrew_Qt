QT += widgets

HEADERS = hmi_objects.h
SOURCES = main.cpp hmi_objects.cpp

# install
target.path = $$PWD/./
INSTALLS += target
