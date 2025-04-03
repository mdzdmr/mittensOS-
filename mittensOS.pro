##
# @file ChessGame.pro
# @brief QMake project file for the Chess Game application
#
# This file configures the build system for the Chess Game application,
# including dependencies, source files, and deployment settings.
#
# @author Group 69 (mittensOS)
##

# Qt modules required for the application
QT       += core gui

# Add widgets module for Qt 5+ compatibility
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Application name
TARGET = ChessGame

# Project type
TEMPLATE = app

# Enable warnings for deprecated Qt features
# This helps identify code that might need updating in future Qt versions
DEFINES += QT_DEPRECATED_WARNINGS

# Uncomment to make the code fail to compile if using deprecated APIs
# Use this to ensure forward compatibility with future Qt versions
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# C++11 standard is required
CONFIG += c++11

# Source files included in the project
SOURCES += \
        main.cpp \
        mainwindow.cpp \
        chessboard.cpp \
        gamestate.cpp \
        chessai.cpp

# Header files included in the project
HEADERS += \
        mainwindow.h \
        chessboard.h \
        gamestate.h \
        chessai.h

# Resource files (images, etc.)
RESOURCES += \
        resources.qrc

# Deployment settings for different platforms
# For QNX
qnx: target.path = /tmp/$${TARGET}/bin
# For Unix/Linux (non-Android)
else: unix:!android: target.path = /opt/$${TARGET}/bin
# Install target if path is specified
!isEmpty(target.path): INSTALLS += target