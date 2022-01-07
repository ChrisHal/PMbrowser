QT       += core gui
QT += widgets

CONFIG += c++17

TEMPLATE = app
TARGET = QtPMbrowser
CONFIG += release
#CONFIG += debug
CONFIG(release, debug|release) {
DEFINES += NDEBUG
OBJECTS_DIR += release
DESTDIR = ./release
}
#CONFIG(debug) {
#OBJECTS_DIR += debug
#DESTDIR = ./debug
#}
#LIBS += -L"../../../../../openssl/lib" \
#    -L"../../../../../Utils/my_sql/mysql-5.7.25-winx64/lib" \
#    -L"../../../../../Utils/postgresql/pgsql/lib"
DEPENDPATH += .
MOC_DIR += .
#OBJECTS_DIR += debug
UI_DIR += .
RCC_DIR += GeneratedFiles
include(QtPMbrowser.pri)

# the following are now managed by
# VS on Windows builds
RC_FILE = QtPMbrowser.rc
#RC_ICONS = myappico.ico
RESOURCES+=QtPMbrowser.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
