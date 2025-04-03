/**
 * @file mainwindow.h
 * @brief Header for the main application window
 *
 * Defines the MainWindow class which serves as the primary UI container
 * for the chess application, including the game board, controls, and menus.
 *
 * @author Chess Game Team
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QRadioButton>
#include "chessboard.h"

/**
 * @class MainWindow
 * @brief Main application window for the Chess Game
 *
 * Provides the main user interface for the chess application including:
 * - Chess board display
 * - Menu system with game controls
 * - Game mode selection (Human vs Human, Human vs AI)
 * - Color selection for AI games
 *
 * @author Group 69 (mittensOS)
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    /**
     * @brief Constructor for the MainWindow
     *
     * Initializes the main window, sets up the user interface,
     * creates menus and actions, and sets the initial game mode.
     *
     * @param parent Parent widget (default: nullptr)
     */
    explicit MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor for the MainWindow
     */
    ~MainWindow();
    
private:
    // UI Components
    /** @brief Chess board widget */
    ChessBoard* chessBoard;
    
    /** @brief Main central widget */
    QWidget* centralWidget;
    
    /** @brief Main vertical layout */
    QVBoxLayout* mainLayout;
    
    /** @brief Horizontal layout for controls */
    QHBoxLayout* controlLayout;
    
    /** @brief Button group for game mode selection */
    QButtonGroup* gameMode;
    
    /** @brief Radio button for Human vs Human mode */
    QRadioButton* humanVsHumanRadio;
    
    /** @brief Radio button for Human vs AI mode */
    QRadioButton* humanVsAiRadio;
    
    /** @brief Button group for color choice */
    QButtonGroup* colorChoice;
    
    /** @brief Radio button for playing as white */
    QRadioButton* playWhiteRadio;
    
    /** @brief Radio button for playing as black */
    QRadioButton* playBlackRadio;
    
    // Menu Actions
    /** @brief Action for starting a new game */
    QAction* newGameAction;
    
    /** @brief Action for exiting the application */
    QAction* exitAction;
    
    /** @brief Action for showing the about dialog */
    QAction* aboutAction;
    
    // Setup functions
    /**
     * @brief Sets up the user interface
     *
     * Creates and arranges all UI elements including:
     * - The chess board
     * - Game mode selection (Human vs Human, Human vs AI)
     * - Color choice for AI games (Play as White/Black)
     * - Connects signals and slots for UI interaction
     */
    void setupUI();
    
    /**
     * @brief Creates the application's menu structure
     *
     * Sets up the File and Help menus and adds the appropriate
     * actions to each menu.
     */
    void createMenus();
    
    /**
     * @brief Creates actions for the menu system
     *
     * Sets up action objects with icons, shortcuts, and connects
     * them to their respective handler methods.
     */
    void createActions();
    
private slots:
    /**
     * @brief Handles changes to the game mode
     *
     * When the game mode changes between Human vs Human and Human vs AI,
     * this method updates the UI state and configures the chess board.
     *
     * @param id ID of the selected radio button (0: Human vs Human, 1: Human vs AI)
     */
    void onGameModeChanged(int id);
    
    /**
     * @brief Handles changes to the color choice
     *
     * When the player changes between playing as white or black in AI mode,
     * this method updates the chess board configuration.
     *
     * @param id ID of the selected radio button (0: Play as White, 1: Play as Black)
     */
    void onColorChoiceChanged(int id);
    
    /**
     * @brief Starts a new chess game
     *
     * Resets the chess board to its initial state and configures
     * the game mode based on the current UI settings.
     */
    void newGame();
    
    /**
     * @brief Shows the About dialog
     *
     * Displays a message box with information about the application,
     * including version, controls, and available game modes.
     */
    void showAbout();
};

#endif // MAINWINDOW_H