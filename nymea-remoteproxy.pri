QT *= network
QT -= gui

CONFIG += c++11 console

QMAKE_CXXFLAGS *= -Werror -std=c++11 -g
QMAKE_LFLAGS *= -std=c++11

top_srcdir=$$PWD
top_builddir=$$shadowed($$PWD)
