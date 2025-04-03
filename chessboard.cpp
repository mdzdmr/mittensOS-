#include "chessboard.h"
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <QFontMetrics>
#include <QMetaType>
#include <QMutexLocker>

/**
 * @brief Register custom types with Qt's meta-object system for thread communication
 */
Q_DECLARE_METATYPE(Move)
Q_DECLARE_METATYPE(QVector<Move>)

/**
 * @brief Constructor for the ChessBoard class
 * 
 * Initializes the chess board UI, game state, and sets up the AI thread.
 * Configures visual appearance, event handling, and threading mechanisms.
 * 
 * @param parent The parent widget (default: nullptr)
 * @author Group 69 (mittensOS)
 */
ChessBoard::ChessBoard(QWidget *parent) : QWidget(parent) {
    // Register Move type for thread communication
    qRegisterMetaType<Move>("Move");
    qRegisterMetaType<QVector<Move>>("QVector<Move>");
    
    // Set fixed size for the widget
    setFixedSize(BOARD_SIZE + MOVE_LOG_PANEL_WIDTH, BOARD_SIZE);

    // Set focus policy to receive keyboard events
    setFocusPolicy(Qt::StrongFocus);

    // Initialize game state
    gs = new GameState();
    validMoves = gs->getValidMoves();

    // Initialize state variables
    moveMade = false;
    animate = false;
    selectedSquare = QPoint(-1, -1);
    gameOver = false;
    aiThinking = false;
    moveUndone = false;
    humanVsAi = false;
    humanPlaysWhite = true;
    
    // Initialize mutex for thread safety
    stateMutex = new QMutex();

    // Initialize colors
    lightSquareColor = QColor(255, 255, 255);  // White
    darkSquareColor = QColor(102, 205, 170);   // Aquamarine3
    highlightColor = QColor(0, 0, 255, 100);   // Blue with transparency
    lastMoveColor = QColor(0, 255, 0, 100);    // Green with transparency

    // Load images
    loadImages();

    // Setup animation timer
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &ChessBoard::updateAnimation);
    animationFrame = 0;
    totalFrames = 10;

    // Initialize with a valid but empty move
    animatedMove = Move();

    // Setup AI
    ai = new ChessAI();
    ai->moveToThread(&aiThread);
    
    // Use explicit queued connections for cross-thread signals
    connect(this, &ChessBoard::findAIMove, ai, &ChessAI::findBestMove, Qt::QueuedConnection);
    connect(ai, &ChessAI::findBestMoveFinished, this, &ChessBoard::handleAIMove, Qt::QueuedConnection);
    
    aiThread.start(QThread::HighPriority);
}

/**
 * @brief Destructor for the ChessBoard class
 * 
 * Cleans up allocated resources and ensures the AI thread
 * is properly terminated before destruction.
 */
ChessBoard::~ChessBoard() {
    // Clean up
    delete gs;
    delete ai;
    delete stateMutex;

    // Stop the AI thread
    aiThread.quit();
    aiThread.wait();
}

/**
 * @brief Loads the chess piece images from resources
 * 
 * Loads images for all chess pieces (white and black) and
 * scales them to fit the square size of the chess board.
 */
void ChessBoard::loadImages() {
    // List of piece names
    QStringList pieces = {"wp", "wR", "wN", "wB", "wQ", "wK", "bp", "bR", "bN", "bB", "bQ", "bK"};

    // Load each piece image
    for (const QString& piece : pieces) {
        QPixmap pixmap(QString(":/images/%1.png").arg(piece));
        images[piece] = pixmap.scaled(SQ_SIZE, SQ_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
}

/**
 * @brief Resets the game to its initial state
 * 
 * Creates a new game state, resets all state variables,
 * and triggers an AI move if playing against AI and AI goes first.
 * This method is thread-safe.
 */
void ChessBoard::resetGame() {
    QMutexLocker locker(stateMutex);
    
    // Cancel any AI move in progress
    aiThinking = false;
    
    // Create a new game state
    delete gs;
    gs = new GameState();
    validMoves = gs->getValidMoves();

    // Reset state variables
    moveMade = false;
    animate = false;
    selectedSquare = QPoint(-1, -1);
    playerClicks.clear();
    gameOver = false;
    moveUndone = false;
    
    // Initialize with a valid but empty move
    animatedMove = Move();
    
    // Stop any ongoing animation
    if (animationTimer->isActive()) {
        animationTimer->stop();
    }

    // Update the display
    update();
    
    // If in human vs AI mode and AI goes first, trigger AI move with a delay
    if (humanVsAi && !humanPlaysWhite && gs->whiteToMove) {
        QTimer::singleShot(500, this, [this]() {
            QMutexLocker locker(stateMutex);
            if (!gameOver && !aiThinking) {
                aiThinking = true;
                // Create a copy of the current state for the AI to use
                GameState* stateCopy = new GameState(*gs);
                QVector<Move> movesCopy = validMoves;
                
                // Queue up the AI move calculation
                emit findAIMove(stateCopy, movesCopy);
                update(); // Force UI update to show AI is thinking
            }
        });
    }
}

/**
 * @brief Sets the game mode to Human vs AI or Human vs Human
 * 
 * Configures the game mode and triggers an AI move if needed.
 * This method is thread-safe.
 * 
 * @param enabled True to enable Human vs AI mode, false for Human vs Human
 * @param humanWhite True if human plays white, false if human plays black
 */
void ChessBoard::setHumanVsAI(bool enabled, bool humanWhite) {
    QMutexLocker locker(stateMutex);
    
    humanVsAi = enabled;
    humanPlaysWhite = humanWhite;

    // If AI's turn, trigger AI move with a delay
    if (humanVsAi && ((gs->whiteToMove && !humanPlaysWhite) || (!gs->whiteToMove && humanPlaysWhite))) {
        if (!aiThinking && !gameOver) {
            QTimer::singleShot(500, this, [this]() {
                QMutexLocker locker(stateMutex);
                if (!gameOver && !aiThinking) {
                    aiThinking = true;
                    // Create a copy of the current state for the AI to use
                    GameState* stateCopy = new GameState(*gs);
                    QVector<Move> movesCopy = validMoves;
                    
                    emit findAIMove(stateCopy, movesCopy);
                    update(); // Force UI update
                }
            });
        }
    }
}

/**
 * @brief Handles painting of the chess board and its elements
 * 
 * Renders the chess board, pieces, highlights, animations, move log,
 * and endgame text if applicable.
 * 
 * @param event The paint event (unused)
 */
void ChessBoard::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw the board
    drawBoard(painter);

    // Highlight squares
    highlightSquares(painter);

    // Draw the pieces
    drawPieces(painter);

    // Draw animated move if active
    if (animationTimer->isActive()) {
        drawAnimatedMove(painter);
    }

    // Draw move log
    drawMoveLog(painter);

    // Draw endgame text if game is over
    if (gameOver) {
        drawEndgameText(painter);
    }
}

/**
 * @brief Handles mouse press events for piece selection and movement
 * 
 * Manages the logic for selecting and moving chess pieces. Validates moves,
 * updates the game state, and triggers AI moves when appropriate.
 * This method is thread-safe.
 * 
 * @param event The mouse event containing click position
 */
void ChessBoard::mousePressEvent(QMouseEvent* event) {
    QMutexLocker locker(stateMutex);
    
    if (event->button() != Qt::LeftButton || gameOver || aiThinking) return;

    int col = event->pos().x() / SQ_SIZE;
    int row = event->pos().y() / SQ_SIZE;

    if (col >= 0 && col < DIMENSION && row >= 0 && row < DIMENSION) {
        bool humanTurn = !humanVsAi ||
                         (gs->whiteToMove && humanPlaysWhite) ||
                         (!gs->whiteToMove && !humanPlaysWhite);

        if (humanTurn) {
            if (selectedSquare == QPoint(row, col)) {
                selectedSquare = QPoint(-1, -1);
                playerClicks.clear();
            } else {
                selectedSquare = QPoint(row, col);
                playerClicks.append(selectedSquare);

                if (playerClicks.size() == 2) {
                    Move move(
                        qMakePair(playerClicks[0].x(), playerClicks[0].y()),
                        qMakePair(playerClicks[1].x(), playerClicks[1].y()),
                        gs->board
                    );

                    bool isMoveValid = false;
                    Move validMove;
                    
                    for (const Move& vm : validMoves) {
                        if (move.startRow == vm.startRow && 
                            move.startCol == vm.startCol &&
                            move.endRow == vm.endRow &&
                            move.endCol == vm.endCol) {
                            
                            isMoveValid = true;
                            validMove = vm;
                            break;
                        }
                    }

                    if (isMoveValid) {
                        gs->makeMove(validMove);
                        selectedSquare = QPoint(-1, -1);
                        playerClicks.clear();

                        validMoves = gs->getValidMoves();
                        gameOver = gs->checkmate || gs->stalemate;

                        animateMove(validMove); 

                        // If it's now AI's turn
                        if (humanVsAi && !gameOver && 
                           ((gs->whiteToMove && !humanPlaysWhite) ||
                            (!gs->whiteToMove && humanPlaysWhite))) 
                        {
                            QTimer::singleShot(500, this, [this]() {
                                QMutexLocker locker(stateMutex);
                                if (!gameOver && !aiThinking) {
                                    aiThinking = true;
                                    // Create a copy of the current state for the AI
                                    GameState* stateCopy = new GameState(*gs);
                                    QVector<Move> movesCopy = validMoves;
                                    
                                    emit findAIMove(stateCopy, movesCopy);
                                    update();
                                }
                            });
                        }
                    } else {
                        // Keep the second click as the new first click
                        playerClicks = {selectedSquare};
                    }
                }
            }
        }
    }
    
    update();
}

/**
 * @brief Handles keyboard events for game controls
 * 
 * Processes key presses:
 * - Z key: Undo the last move (or last two moves in AI mode)
 * - R key: Reset the game
 * 
 * @param event The key event
 */
void ChessBoard::keyPressEvent(QKeyEvent* event) {
    // Undo move (Z key)
    if (event->key() == Qt::Key_Z) {
        gs->undoMove();
        if (humanVsAi) {
            // Undo both human and AI moves
            gs->undoMove();
        }
        moveMade = true;
        animate = false;
        gameOver = false;
        if (aiThinking) {
            // Cancel AI move by setting flag
            aiThinking = false;
            // Note: A more robust solution would use a request ID system to ignore pending AI moves
            // but that would require changes to the signal/slot mechanism
        }
        moveUndone = true;
        // Recalculate valid moves after undoing
        validMoves = gs->getValidMoves();
        update();
    }

    // Reset game (R key)
    else if (event->key() == Qt::Key_R) {
        resetGame();
    }
}

/**
 * @brief Draws the chess board grid
 * 
 * Renders the alternating light and dark squares of the chess board.
 * 
 * @param painter The QPainter to use for drawing
 */
void ChessBoard::drawBoard(QPainter& painter) {
    // Draw alternating colored squares
    for (int row = 0; row < DIMENSION; row++) {
        for (int col = 0; col < DIMENSION; col++) {
            QRect rect(col * SQ_SIZE, row * SQ_SIZE, SQ_SIZE, SQ_SIZE);

            // Alternate colors
            QColor color = (row + col) % 2 == 0 ? lightSquareColor : darkSquareColor;
            painter.fillRect(rect, color);
        }
    }
}

/**
 * @brief Draws the chess pieces on the board
 * 
 * Renders each chess piece at its current position on the board.
 * 
 * @param painter The QPainter to use for drawing
 */
void ChessBoard::drawPieces(QPainter& painter) {
    // Draw each piece on the board
    for (int row = 0; row < DIMENSION; row++) {
        for (int col = 0; col < DIMENSION; col++) {
            QString piece = gs->board[row][col];

            // Skip empty squares
            if (piece != "--") {
                QRect rect(col * SQ_SIZE, row * SQ_SIZE, SQ_SIZE, SQ_SIZE);
                painter.drawPixmap(rect, images[piece]);
            }
        }
    }
}

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
void ChessBoard::highlightSquares(QPainter& painter) {
    // Show AI thinking state visually if active
    if (aiThinking) {
        QFont font("Arial", 14, QFont::Bold);
        painter.setFont(font);
        painter.setPen(QColor(200, 0, 0));
        painter.drawText(10, 20, gs->whiteToMove ? "White (AI) thinking..." : "Black (AI) thinking...");
    }
    
    // Highlight last move
    if (!gs->moveLog.isEmpty()) {
        Move lastMove = gs->moveLog.back();
        if (lastMove.endRow >= 0 && lastMove.endRow < DIMENSION && 
            lastMove.endCol >= 0 && lastMove.endCol < DIMENSION) {
            QRect endRect(lastMove.endCol * SQ_SIZE, lastMove.endRow * SQ_SIZE, SQ_SIZE, SQ_SIZE);
            painter.fillRect(endRect, lastMoveColor);
        }
    }

    // Highlight selected square
    if (selectedSquare.x() >= 0 && selectedSquare.y() >= 0) {
        int row = selectedSquare.x();
        int col = selectedSquare.y();

        if (row >= 0 && row < DIMENSION && col >= 0 && col < DIMENSION) {
            QString piece = gs->board[row][col];
            if (piece != "--" && ((piece[0] == 'w' && gs->whiteToMove) || (piece[0] == 'b' && !gs->whiteToMove))) {
                QRect selectedRect(col * SQ_SIZE, row * SQ_SIZE, SQ_SIZE, SQ_SIZE);
                painter.fillRect(selectedRect, highlightColor);

                QColor validMoveColor(255, 255, 0, 100);
                for (const Move& move : validMoves) {
                    if (move.startRow == row && move.startCol == col) {
                        QRect moveRect(move.endCol * SQ_SIZE, move.endRow * SQ_SIZE, SQ_SIZE, SQ_SIZE);
                        painter.fillRect(moveRect, validMoveColor);
                    }
                }
            }
        }
    }
}

/**
 * @brief Draws the move log panel
 * 
 * Displays a history of moves made in the game in standard chess notation.
 * 
 * @param painter The QPainter to use for drawing
 */
void ChessBoard::drawMoveLog(QPainter& painter) {
    // Set up move log area
    QRect moveLogRect(BOARD_SIZE, 0, MOVE_LOG_PANEL_WIDTH, MOVE_LOG_PANEL_HEIGHT);
    painter.fillRect(moveLogRect, QColor(0, 0, 0));

    // Set up text style
    painter.setPen(QColor(255, 255, 255));
    QFont font("Arial", 12);
    painter.setFont(font);
    QFontMetrics metrics(font);

    // Generate move texts
    QStringList moveTexts;
    for (int i = 0; i < gs->moveLog.size(); i += 2) {
        QString moveString = QString("%1. %2").arg(i / 2 + 1).arg(gs->moveLog[i].toString());
        if (i + 1 < gs->moveLog.size()) {
            moveString += QString(" %1  ").arg(gs->moveLog[i + 1].toString());
        }
        moveTexts.append(moveString);
    }

    // Display moves
    int movesPerRow = 3;
    int padding = 5;
    int lineSpacing = 2;
    int textY = padding;

    for (int i = 0; i < moveTexts.size(); i += movesPerRow) {
        QString text;
        for (int j = 0; j < movesPerRow; j++) {
            if (i + j < moveTexts.size()) {
                text += moveTexts[i + j];
            }
        }

        QRect textRect(moveLogRect.x() + padding, moveLogRect.y() + textY,
                      moveLogRect.width() - 2 * padding, metrics.height());
        painter.drawText(textRect, Qt::AlignLeft, text);
        textY += metrics.height() + lineSpacing;
    }
}

/**
 * @brief Draws the endgame message when the game is over
 * 
 * Displays a message indicating checkmate or stalemate
 * with a shadow effect for better visibility.
 * 
 * @param painter The QPainter to use for drawing
 */
void ChessBoard::drawEndgameText(QPainter& painter) {
    QString text;
    if (gs->checkmate) {
        text = gs->whiteToMove ? "Black wins by checkmate!" : "White wins by checkmate!";
    } else if (gs->stalemate) {
        text = "Stalemate";
    }

    if (!text.isEmpty()) {
        // Set up text style
        QFont font("Helvetica", 24, QFont::Bold);
        painter.setFont(font);
        QFontMetrics metrics(font);

        // Calculate text position
        QRect textRect = metrics.boundingRect(text);
        int x = (BOARD_SIZE - textRect.width()) / 2;
        int y = (BOARD_SIZE - textRect.height()) / 2;

        // Draw shadow effect
        painter.setPen(QColor(150, 150, 150));
        painter.drawText(x + 2, y + 2 + metrics.ascent(), text);

        // Draw main text
        painter.setPen(QColor(0, 0, 0));
        painter.drawText(x, y + metrics.ascent(), text);
    }
}

/**
 * @brief Starts an animation for a chess piece move
 * 
 * Initializes the animation parameters and starts the timer
 * to animate a piece moving from its starting position to
 * its destination.
 * 
 * @param move The move to animate
 */
void ChessBoard::animateMove(const Move& move) {
    animatedMove = move;
    animationFrame = 0;
    totalFrames = 10;
    animationTimer->start(20);
}

/**
 * @brief Draws the current frame of a move animation
 * 
 * Renders a chess piece at an interpolated position between
 * its starting and ending points based on animation progress.
 * 
 * @param painter The QPainter to use for drawing
 */
void ChessBoard::drawAnimatedMove(QPainter& painter) {
    if (animatedMove.pieceMoved.isEmpty()) return;
    
    int startX = animatedMove.startCol * SQ_SIZE;
    int startY = animatedMove.startRow * SQ_SIZE;
    int endX = animatedMove.endCol * SQ_SIZE;
    int endY = animatedMove.endRow * SQ_SIZE;
    
    float progress = static_cast<float>(animationFrame) / totalFrames;
    int currentX = startX + (endX - startX) * progress;
    int currentY = startY + (endY - startY) * progress;
    
    QRect pieceRect(currentX, currentY, SQ_SIZE, SQ_SIZE);
    
    // Make sure the piece image exists
    if (images.contains(animatedMove.pieceMoved)) {
        painter.drawPixmap(pieceRect, images[animatedMove.pieceMoved]);
    }
}

/**
 * @brief Updates the animation state for each frame
 * 
 * Called by the animation timer to advance the animation.
 * When animation completes, updates the game state and
 * triggers AI moves if needed.
 * This method is thread-safe.
 */
void ChessBoard::updateAnimation() {
    QMutexLocker locker(stateMutex);
    animationFrame++;

    if (animationFrame > totalFrames) {
        animationTimer->stop();
        animationFrame = 0;

        validMoves = gs->getValidMoves();
        moveMade = false;
        animate = false;
        moveUndone = false;
        gameOver = gs->checkmate || gs->stalemate;
    }

    update();
}

/**
 * @brief Handles moves returned by the AI
 * 
 * Processes the move found by the AI thread, validates it,
 * applies it to the game state, and updates the UI.
 * Handles error cases by selecting a fallback move if needed.
 * This method is thread-safe.
 * 
 * @param move The move returned by the AI
 */
void ChessBoard::handleAIMove(Move move) {
    QMutexLocker locker(stateMutex);
    
    // Check if we should still process this AI move
    if (aiThinking && !gameOver) {
        qDebug() << "AI returned move: " << move.toString();
        
        // Verify the move is valid
        bool moveIsValid = false;
        for (const Move& validMove : validMoves) {
            if (move.startRow == validMove.startRow && 
                move.startCol == validMove.startCol &&
                move.endRow == validMove.endRow &&
                move.endCol == validMove.endCol) {
                
                moveIsValid = true;
                // Use the valid move from our list rather than the one returned by AI
                move = validMove;
                break;
            }
        }
        
        if (moveIsValid) {
            // Make the AI move
            gs->makeMove(move);
            
            // Start animation
            animateMove(move);
            
            // Recalculate valid moves
            validMoves = gs->getValidMoves();
            gameOver = gs->checkmate || gs->stalemate;
        } else {
            qDebug() << "Warning: AI returned invalid move!";
            
            // If AI returned an invalid move, let's pick the first valid one
            if (!validMoves.isEmpty()) {
                Move safeMove = validMoves.first();
                gs->makeMove(safeMove);
                animateMove(safeMove);
                validMoves = gs->getValidMoves();
                gameOver = gs->checkmate || gs->stalemate;
            }
        }
        
        // AI thinking completed
        aiThinking = false;
        
        // Force update to show AI move immediately
        update();
    }
}