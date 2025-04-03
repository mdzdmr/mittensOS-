/**
 * @file main.cpp
 * @brief Entry point for the Chess Game application
 * 
 * Initializes the Qt application, registers required meta types for
 * cross-thread communication, sets application information, and
 * creates and shows the main window.
 * 
 * @author Group 69 (mittensOS)
 */

#include "mainwindow.h"
#include <QApplication>
#include "gamestate.h"

/**
 * @brief Main application entry point
 * 
 * Initializes the Qt application framework, registers the Move vector
 * type for cross-thread communication, sets up application metadata,
 * and launches the main window.
 * 
 * @param argc Command line argument count
 * @param argv Command line argument values
 * @return Application exit code
 */
int main(int argc, char *argv[]) {
    // Create Qt application
    QApplication app(argc, argv);
    
    // Register custom types for cross-thread signal/slot communication
    qRegisterMetaType<QVector<Move>>("QVector<Move>");
    
    // Set application information
    app.setApplicationName("Chess Game");
    app.setOrganizationName("Chess Game Project");
    
    // Create and show the main window
    MainWindow mainWindow;
    mainWindow.show();
    
    // Run the application event loop
    return app.exec();
}