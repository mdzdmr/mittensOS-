/**
 * @file mainwindow.cpp
 * @brief Implementation of the main application window
 *
 * Contains implementation of the MainWindow class which provides
 * the main user interface for the chess application, including
 * the game board, menu system, and control options for game modes.
 *
 * @author Group 69 (mittensOS)
 */

#include "mainwindow.h"
#include <QMessageBox>

/**
 * @brief Constructor for the MainWindow
 *
 * Initializes the main window, sets up the user interface,
 * creates menus and actions, and sets the initial game mode.
 *
 * @param parent Parent widget (default: nullptr)
 */
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Set window title
    setWindowTitle("Chess Game");

    // Create actions, menus, and UI
    createActions();
    createMenus();
    setupUI();

    // Set initial game mode (Human vs Human)
    onGameModeChanged(0);
}

/**
 * @brief Destructor for the MainWindow
 *
 * Cleanup is handled by Qt's parent-child relationship,
 * so no explicit deletion of child widgets is needed.
 */
MainWindow::~MainWindow() {
    // Clean up is handled by Qt's parent-child relationship
}

/**
 * @brief Sets up the user interface
 *
 * Creates and arranges all UI elements including:
 * - The chess board
 * - Game mode selection (Human vs Human, Human vs AI)
 * - Color choice for AI games (Play as White/Black)
 * - Connects signals and slots for UI interaction
 */
void MainWindow::setupUI() {
    // Create central widget and layout
    centralWidget = new QWidget(this);
    mainLayout = new QVBoxLayout(centralWidget);

    // Create chess board
    chessBoard = new ChessBoard(this);
    mainLayout->addWidget(chessBoard);

    // Create control panel layout
    controlLayout = new QHBoxLayout();

    // Create game mode radio buttons
    gameMode = new QButtonGroup(this);
    humanVsHumanRadio = new QRadioButton("Human vs Human", this);
    humanVsAiRadio = new QRadioButton("Human vs AI", this);
    gameMode->addButton(humanVsHumanRadio, 0);
    gameMode->addButton(humanVsAiRadio, 1);
    humanVsHumanRadio->setChecked(true);

    // Create color choice radio buttons
    colorChoice = new QButtonGroup(this);
    playWhiteRadio = new QRadioButton("Play as White", this);
    playBlackRadio = new QRadioButton("Play as Black", this);
    colorChoice->addButton(playWhiteRadio, 0);
    colorChoice->addButton(playBlackRadio, 1);
    playWhiteRadio->setChecked(true);

    // Add radio buttons to control layout
    controlLayout->addWidget(humanVsHumanRadio);
    controlLayout->addWidget(humanVsAiRadio);
    controlLayout->addSpacing(20);
    controlLayout->addWidget(playWhiteRadio);
    controlLayout->addWidget(playBlackRadio);
    controlLayout->addStretch();

    // Add control layout to main layout
    mainLayout->addLayout(controlLayout);

    // Set central widget
    setCentralWidget(centralWidget);

    // Connect signals and slots using idClicked (not buttonClicked)
    connect(gameMode, &QButtonGroup::idClicked,
        this, &MainWindow::onGameModeChanged);
    connect(colorChoice, &QButtonGroup::idClicked,
        this, &MainWindow::onColorChoiceChanged);
    
    // Disable color choice initially (for Human vs Human mode)
    playWhiteRadio->setEnabled(false);
    playBlackRadio->setEnabled(false);
}

/**
 * @brief Creates actions for the menu system
 *
 * Sets up action objects with icons, shortcuts, and connects
 * them to their respective handler methods.
 */
void MainWindow::createActions() {
    // New game action
    newGameAction = new QAction("New Game", this);
    newGameAction->setShortcut(QKeySequence::New);
    connect(newGameAction, &QAction::triggered, this, &MainWindow::newGame);

    // Exit action
    exitAction = new QAction("Exit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    // About action
    aboutAction = new QAction("About", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

/**
 * @brief Creates the application's menu structure
 *
 * Sets up the File and Help menus and adds the appropriate
 * actions to each menu.
 */
void MainWindow::createMenus() {
    // Create file menu
    QMenu* fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(newGameAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    // Create help menu
    QMenu* helpMenu = menuBar()->addMenu("Help");
    helpMenu->addAction(aboutAction);
}

/**
 * @brief Handles changes to the game mode
 *
 * When the game mode changes between Human vs Human and Human vs AI,
 * this method updates the UI state and configures the chess board.
 *
 * @param id ID of the selected radio button (0: Human vs Human, 1: Human vs AI)
 */
void MainWindow::onGameModeChanged(int id) {
    if (id == 0) {  // Human vs Human
        playWhiteRadio->setEnabled(false);
        playBlackRadio->setEnabled(false);
        chessBoard->setHumanVsAI(false, true);
    } else {  // Human vs AI
        playWhiteRadio->setEnabled(true);
        playBlackRadio->setEnabled(true);
        chessBoard->setHumanVsAI(true, playWhiteRadio->isChecked());
    }
}

/**
 * @brief Handles changes to the color choice
 *
 * When the player changes between playing as white or black in AI mode,
 * this method updates the chess board configuration.
 *
 * @param id ID of the selected radio button (0: Play as White, 1: Play as Black)
 */
void MainWindow::onColorChoiceChanged(int id) {
    if (humanVsAiRadio->isChecked()) {
        chessBoard->setHumanVsAI(true, id == 0);  // id == 0 -> Play as White
    }
}

/**
 * @brief Starts a new chess game
 *
 * Resets the chess board to its initial state and configures
 * the game mode based on the current UI settings.
 */
void MainWindow::newGame() {
    chessBoard->resetGame();

    // Update game mode
    if (humanVsAiRadio->isChecked()) {
        chessBoard->setHumanVsAI(true, playWhiteRadio->isChecked());
    } else {
        chessBoard->setHumanVsAI(false, true);
    }
}

/**
 * @brief Shows the About dialog
 *
 * Displays a message box with information about the application,
 * including version, controls, and available game modes.
 */
void MainWindow::showAbout() {
    QMessageBox::about(this, "About Chess Game",
                      "Chess Game v1.0\n\n"
                      "A simple chess game implemented in C++ using Qt.\n\n"
                      "Controls:\n"
                      "- Z: Undo move\n"
                      "- R: Reset game\n\n"
                      "Game modes:\n"
                      "- Human vs Human\n"
                      "- Human vs AI");
}