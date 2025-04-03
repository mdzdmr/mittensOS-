#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QWidget>
#include <QMap>
#include <QPixmap>
#include <QPoint>
#include <QMouseEvent>
#include <QTimer>
#include <QPainter>
#include <QThread>
#include <QMutex>
#include "gamestate.h"
#include "chessai.h"

/**
 * @class ChessBoard
 * @brief Graphical chess board with game logic and AI integration
 *
 * The ChessBoard class provides a complete chess game interface with:
 * - Interactive graphical board
 * - Move validation and animation
 * - Game state tracking
 * - AI opponent using minimax algorithm
 * - Move history display
 * - Keyboard shortcuts
 *
 * @author Group 69 (mittensOS)
 */
class ChessBoard : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructor for the ChessBoard widget
     * @param parent The parent widget (default: nullptr)
     */
    explicit ChessBoard(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor for the ChessBoard widget
     */
    ~ChessBoard();

    /** @brief Total size of the chess board in pixels */
    static constexpr int BOARD_SIZE = 512;
    
    /** @brief Number of squares per side (standard chess is 8×8) */
    static constexpr int DIMENSION = 8;
    
    /** @brief Size of each square in pixels */
    static constexpr int SQ_SIZE = BOARD_SIZE / DIMENSION;
    
    /** @brief Width of the move log panel in pixels */
    static constexpr int MOVE_LOG_PANEL_WIDTH = 512;
    
    /** @brief Height of the move log panel in pixels */
    static constexpr int MOVE_LOG_PANEL_HEIGHT = 250;

    /**
     * @brief Current game state containing board position and move history
     */
    GameState* gs;
    
    /**
     * @brief List of valid moves for the current position
     */
    QVector<Move> validMoves;
    
    /**
     * @brief Flag indicating if a move was just made
     */
    bool moveMade;
    
    /**
     * @brief Flag indicating if move animation is active
     */
    bool animate;
    
    /**
     * @brief Currently selected square on the board
     */
    QPoint selectedSquare;
    
    /**
     * @brief Clicked squares (for move input)
     */
    QVector<QPoint> playerClicks;
    
    /**
     * @brief Flag indicating if the game is over (checkmate or stalemate)
     */
    bool gameOver;
    
    /**
     * @brief Flag indicating if the AI is calculating a move
     */
    bool aiThinking;
    
    /**
     * @brief Flag indicating if a move was just undone
     */
    bool moveUndone;
    
    /**
     * @brief Flag indicating if the game is human vs AI
     */
    bool humanVsAi;
    
    /**
     * @brief Flag indicating if human player is playing as white
     */
    bool humanPlaysWhite;

    /**
     * @brief AI engine for computer player
     */
    ChessAI* ai;
    
    /**
     * @brief Thread for running AI calculations without blocking UI
     */
    QThread aiThread;

    /**
     * @brief Map of piece identifiers to their images
     *
     * Keys are two-character strings:
     * - First character is color ('w' for white, 'b' for black)
     * - Second character is piece type ('K', 'Q', 'R', 'B', 'N', 'p')
     */
    QMap<QString, QPixmap> images;

    /**
     * @brief Color for light squares on the chess board
     */
    QColor lightSquareColor;
    
    /**
     * @brief Color for dark squares on the chess board
     */
    QColor darkSquareColor;
    
    /**
     * @brief Color for highlighting selected squares
     */
    QColor highlightColor;
    
    /**
     * @brief Color for highlighting the last move made
     */
    QColor lastMoveColor;

    /**
     * @brief Timer for controlling move animations
     */
    QTimer* animationTimer;
    
    /**
     * @brief Move currently being animated
     */
    Move animatedMove;
    
    /**
     * @brief Current frame of the animation
     */
    int animationFrame;
    
    /**
     * @brief Total number of frames in the animation
     */
    int totalFrames;

    /**
     * @brief Loads chess piece images from resources
     *
     * Initializes the images map with scaled piece images for all
     * chess pieces (white and black).
     */
    void loadImages();

    /**
     * @brief Resets the game to its initial state
     *
     * Resets the board, game state, and all flags to start a new game.
     * If in human vs AI mode and AI goes first, triggers an AI move.
     */
    void resetGame();

    /**
     * @brief Sets the game mode to Human vs AI or Human vs Human
     *
     * @param enabled True to enable Human vs AI mode, false for Human vs Human
     * @param humanWhite True if human plays white, false if human plays black
     */
    void setHumanVsAI(bool enabled, bool humanWhite);

protected:
    /**
     * @brief Handles painting of the chess board and its elements
     *
     * Renders the chess board, pieces, highlights, animations, move log,
     * and endgame text if applicable.
     *
     * @param event The paint event
     */
    void paintEvent(QPaintEvent* event) override;

    /**
     * @brief Handles mouse press events for piece selection and movement
     *
     * Manages the logic for selecting and moving chess pieces.
     * Validates moves, updates the game state, and triggers
     * AI moves when appropriate.
     *
     * @param event The mouse event containing click position
     */
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * @brief Handles keyboard events for game controls
     *
     * Processes key presses:
     * - Z key: Undo the last move (or last two moves in AI mode)
     * - R key: Reset the game
     *
     * @param event The key event
     */
    void keyPressEvent(QKeyEvent* event) override;

private:
    /**
     * @brief Draws the chess board grid
     *
     * Renders the alternating light and dark squares of the chess board.
     *
     * @param painter The QPainter to use for drawing
     */
    void drawBoard(QPainter& painter);

    /**
     * @brief Draws the chess pieces on the board
     *
     * Renders each chess piece at its current position on the board.
     *
     * @param painter The QPainter to use for drawing
     */
    void drawPieces(QPainter& painter);

    /**
     * @brief Highlights relevant squares on the board
     *
     * Highlights the following:
     * - AI thinking state (text indicator)
     * - Last move made
     * - Selected piece
     * - Valid moves for the selected piece
     *
     * @param painter The QPainter to use for drawing
     */
    void highlightSquares(QPainter& painter);

    /**
     * @brief Draws the move log panel
     *
     * Displays a history of moves made in the game in standard chess notation.
     *
     * @param painter The QPainter to use for drawing
     */
    void drawMoveLog(QPainter& painter);

    /**
     * @brief Draws the endgame message when the game is over
     *
     * Displays a message indicating checkmate or stalemate.
     *
     * @param painter The QPainter to use for drawing
     */
    void drawEndgameText(QPainter& painter);

    /**
     * @brief Starts an animation for a chess piece move
     *
     * Initializes the animation parameters and starts the timer
     * to animate a piece moving from its starting position to
     * its destination.
     *
     * @param move The move to animate
     */
    void animateMove(const Move& move);

    /**
     * @brief Draws the current frame of a move animation
     *
     * Renders a chess piece at an interpolated position between
     * its starting and ending points based on animation progress.
     *
     * @param painter The QPainter to use for drawing
     */
    void drawAnimatedMove(QPainter& painter);

private slots:
    /**
     * @brief Updates the animation state for each frame
     *
     * Called by the animation timer to advance the animation.
     * When animation completes, updates the game state.
     */
    void updateAnimation();

    /**
     * @brief Handles moves returned by the AI
     *
     * Processes the move found by the AI thread, validates it,
     * applies it to the game state, and updates the UI.
     *
     * @param move The move returned by the AI
     */
    void handleAIMove(Move move);

signals:
    /**
     * @brief Signal to request the AI to find a move
     *
     * Emitted when it's the AI's turn to move. The AI engine
     * will receive the current game state and valid moves.
     *
     * @param gs The current game state
     * @param validMoves List of valid moves for the current position
     */
    void findAIMove(GameState* gs, QVector<Move> validMoves);
};

#endif // CHESSBOARD_H