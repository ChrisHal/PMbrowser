# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Tools.
# ------------------------------------------------------

QT       += core gui
QT += widgets

CONFIG += c++11

TEMPLATE = app
TARGET = QtPMbrowser
DESTDIR = ./release
CONFIG += release
LIBS += -L"../../../../../openssl/lib" \
    -L"../../../../../Utils/my_sql/mysql-5.7.25-winx64/lib" \
    -L"../../../../../Utils/postgresql/pgsql/lib"
DEPENDPATH += .
MOC_DIR += .
OBJECTS_DIR += release
UI_DIR += .
RCC_DIR += GeneratedFiles
include(QtPMbrowser.pri)

# the following are now managed by
# VS on Windows builds
#RC_FILE = QtPMbrowser.rc
#RC_ICONS = myappico.ico
RESOURCES+=QtPMbrowser.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
