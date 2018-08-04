QT *= network websockets
QT -= gui

# define protocol versions
API_VERSION_MAJOR=0
API_VERSION_MINOR=1
DEFINES += API_VERSION_STRING=\\\"$${API_VERSION_MAJOR}.$${API_VERSION_MINOR}\\\"

CONFIG += c++11 console

QMAKE_CXXFLAGS *= -Werror -std=c++11 -g
QMAKE_LFLAGS *= -std=c++11

top_srcdir=$$PWD
top_builddir=$$shadowed($$PWD)
