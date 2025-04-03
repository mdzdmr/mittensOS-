#include "chessai.h"
#include <QRandomGenerator>
#include <QDebug>
#include <algorithm>

/**
 * @brief Constructor for the ChessAI class
 * 
 * Initializes the AI with piece scores and position tables
 * for various chess pieces.
 * 
 * @param parent The parent QObject (default: nullptr)
 * @author Group 69 (mittensOS)
 */
ChessAI::ChessAI(QObject *parent) : QObject(parent) {
    // Initialize piece scores
    pieceScore = {
        {'K', 0},  ///< King value (not used for evaluation, just prevent capture)
        {'Q', 9},  ///< Queen value
        {'R', 5},  ///< Rook value
        {'B', 3},  ///< Bishop value
        {'N', 3},  ///< Knight value
        {'p', 1}   ///< Pawn value
    };

    // Initialize score tables
    initScoreTables();
}

/**
 * @brief Initializes position evaluation tables for all chess pieces
 * 
 * Creates tables that assign positional values to each piece type
 * on different squares of the board. Creates separate tables for
 * white and black pieces (black tables are mirror images of white).
 */
void ChessAI::initScoreTables() {
    // Knight position scores - knights are more valuable in the center
    knightScores = {
        {0.0, 0.1, 0.2, 0.2, 0.2, 0.2, 0.1, 0.0},
        {0.1, 0.3, 0.5, 0.5, 0.5, 0.5, 0.3, 0.1},
        {0.2, 0.5, 0.6, 0.65, 0.65, 0.6, 0.5, 0.2},
        {0.2, 0.55, 0.65, 0.7, 0.7, 0.65, 0.55, 0.2},
        {0.2, 0.5, 0.65, 0.7, 0.7, 0.65, 0.5, 0.2},
        {0.2, 0.55, 0.6, 0.65, 0.65, 0.6, 0.55, 0.2},
        {0.1, 0.3, 0.5, 0.55, 0.55, 0.5, 0.3, 0.1},
        {0.0, 0.1, 0.2, 0.2, 0.2, 0.2, 0.1, 0.0}
    };

    // Bishop position scores - bishops prefer diagonals and open positions
    bishopScores = {
        {0.0, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.0},
        {0.2, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.2},
        {0.2, 0.4, 0.5, 0.6, 0.6, 0.5, 0.4, 0.2},
        {0.2, 0.5, 0.5, 0.6, 0.6, 0.5, 0.5, 0.2},
        {0.2, 0.4, 0.6, 0.6, 0.6, 0.6, 0.4, 0.2},
        {0.2, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.2},
        {0.2, 0.5, 0.4, 0.4, 0.4, 0.4, 0.5, 0.2},
        {0.0, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.0}
    };

    // Rook position scores - rooks prefer open files and 7th rank
    rookScores = {
        {0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25},
        {0.5, 0.75, 0.75, 0.75, 0.75, 0.75, 0.75, 0.5},
        {0.0, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.0},
        {0.0, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.0},
        {0.0, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.0},
        {0.0, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.0},
        {0.0, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.0},
        {0.25, 0.25, 0.25, 0.5, 0.5, 0.25, 0.25, 0.25}
    };

    // Queen position scores - queens are valuable but should not be developed too early
    queenScores = {
        {0.0, 0.2, 0.2, 0.3, 0.3, 0.2, 0.2, 0.0},
        {0.2, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.2},
        {0.2, 0.4, 0.5, 0.5, 0.5, 0.5, 0.4, 0.2},
        {0.3, 0.4, 0.5, 0.5, 0.5, 0.5, 0.4, 0.3},
        {0.4, 0.4, 0.5, 0.5, 0.5, 0.5, 0.4, 0.3},
        {0.2, 0.5, 0.5, 0.5, 0.5, 0.5, 0.4, 0.2},
        {0.2, 0.4, 0.5, 0.4, 0.4, 0.4, 0.4, 0.2},
        {0.0, 0.2, 0.2, 0.3, 0.3, 0.2, 0.2, 0.0}
    };

    // Pawn position scores - pawns are valuable as they advance
    pawnScores = {
        {0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8},  ///< About to promote
        {0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7},  ///< Advanced pawns
        {0.3, 0.3, 0.4, 0.5, 0.5, 0.4, 0.3, 0.3},  ///< Central pawns
        {0.25, 0.25, 0.3, 0.45, 0.45, 0.3, 0.25, 0.25},
        {0.2, 0.2, 0.2, 0.4, 0.4, 0.2, 0.2, 0.2},  ///< Center control
        {0.25, 0.15, 0.1, 0.2, 0.2, 0.1, 0.15, 0.25},
        {0.25, 0.3, 0.3, 0.0, 0.0, 0.3, 0.3, 0.25},
        {0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2}   ///< Starting position
    };

    // Create reversed copies for black pieces (mirror images)
    QVector<QVector<double>> knightScoresReversed = knightScores;
    std::reverse(knightScoresReversed.begin(), knightScoresReversed.end());

    QVector<QVector<double>> bishopScoresReversed = bishopScores;
    std::reverse(bishopScoresReversed.begin(), bishopScoresReversed.end());

    QVector<QVector<double>> rookScoresReversed = rookScores;
    std::reverse(rookScoresReversed.begin(), rookScoresReversed.end());

    QVector<QVector<double>> queenScoresReversed = queenScores;
    std::reverse(queenScoresReversed.begin(), queenScoresReversed.end());

    QVector<QVector<double>> pawnScoresReversed = pawnScores;
    std::reverse(pawnScoresReversed.begin(), pawnScoresReversed.end());

    // Map piece identifiers to their position score tables
    piecePositionScores = {
        {"wN", knightScores},
        {"bN", knightScoresReversed},
        {"wB", bishopScores},
        {"bB", bishopScoresReversed},
        {"wQ", queenScores},
        {"bQ", queenScoresReversed},
        {"wR", rookScores},
        {"bR", rookScoresReversed},
        {"wp", pawnScores},
        {"bp", pawnScoresReversed}
    };
}

/**
 * @brief Finds the best move for the current game state
 * 
 * Uses the negamax algorithm with alpha-beta pruning to search
 * for the best move. If no strong move is found, falls back to
 * selecting a random move.
 * 
 * @param gs The current game state
 * @param validMoves List of valid moves for the current position
 * @return Move The best move found by the AI
 */
Move ChessAI::findBestMove(GameState* gs, const QVector<Move>& validMoves) {
    // Safety check: if no valid moves, return empty move
    if (validMoves.isEmpty()) {
        qDebug() << "Warning: No valid moves available for AI!";
        return Move();
    }
    
    nextMove = Move();  // Initialize with default constructor

    // Shuffle the valid moves for randomness when multiple moves have the same score
    QVector<Move> shuffledMoves = validMoves;
    for (int i = shuffledMoves.size() - 1; i > 0; i--) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        if (i != j) {
            std::swap(shuffledMoves[i], shuffledMoves[j]);
        }
    }

    // Find the best move using negamax with alpha-beta pruning
    findMoveNegaMaxAlphaBeta(gs, shuffledMoves, DEPTH, -CHECKMATE, CHECKMATE, gs->whiteToMove ? 1 : -1);

    // If no good move found, use a random move
    if (nextMove.moveID == 0 || !isValidMove(nextMove, validMoves)) {
        qDebug() << "Using random move as fallback";
        nextMove = findRandomMove(validMoves);
    }

    // Debug info
    qDebug() << "AI selected move: " << nextMove.toString();

    // Emit signal with the found move
    emit findBestMoveFinished(nextMove);

    return nextMove;
}

/**
 * @brief Validates if a move is in the list of valid moves
 * 
 * Checks if the candidate move exists in the provided list of valid moves.
 * 
 * @param move The move to validate
 * @param validMoves List of valid moves to check against
 * @return bool True if the move is valid, false otherwise
 */
bool ChessAI::isValidMove(const Move& move, const QVector<Move>& validMoves) {
    // Check if move is in the valid moves list
    for (const Move& validMove : validMoves) {
        if (move == validMove) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Implements the NegaMax algorithm with alpha-beta pruning
 * 
 * Recursively evaluates positions by simulating moves and calculating
 * the best possible outcome assuming optimal play by both sides.
 * Alpha-beta pruning optimizes the search by skipping branches that
 * won't affect the final decision.
 * 
 * @param gs Current game state
 * @param validMoves List of valid moves to consider
 * @param depth Current search depth
 * @param alpha Alpha value for pruning
 * @param beta Beta value for pruning
 * @param turnMultiplier 1 for white, -1 for black (for score negation)
 * @return int Score of the best move found
 */
int ChessAI::findMoveNegaMaxAlphaBeta(GameState* gs, const QVector<Move>& validMoves,
                                     int depth, int alpha, int beta, int turnMultiplier) {
    // Base case: reached maximum depth
    if (depth == 0) {
        return turnMultiplier * scoreBoard(gs);
    }

    int maxScore = -CHECKMATE;

    // Evaluate each possible move
    for (const Move& move : validMoves) {
        // Make the move
        gs->makeMove(move);

        // Get valid moves for the next position
        QVector<Move> nextMoves = gs->getValidMoves();

        // Recursive call with negated parameters (minimax with negation)
        int score = -findMoveNegaMaxAlphaBeta(gs, nextMoves, depth - 1, -beta, -alpha, -turnMultiplier);

        // Undo the move
        gs->undoMove();

        // Update max score
        if (score > maxScore) {
            maxScore = score;

            // If this is the root call, update the best move
            if (depth == DEPTH) {
                nextMove = move;
            }
        }

        // Alpha-beta pruning
        if (maxScore > alpha) {
            alpha = maxScore;
        }

        if (alpha >= beta) {
            break;  // Beta cutoff - opponent won't allow this position
        }
    }

    return maxScore;
}

/**
 * @brief Evaluates the current board position
 * 
 * Assigns a score to the current board state by considering:
 * 1. Material value of all pieces
 * 2. Positional value of pieces based on their location
 * 3. Game-ending conditions (checkmate, stalemate)
 * 
 * @param gs Current game state to evaluate
 * @return int Score from white's perspective (positive is good for white)
 */
int ChessAI::scoreBoard(GameState* gs) {
    // Check for game-ending conditions
    if (gs->checkmate) {
        return gs->whiteToMove ? -CHECKMATE : CHECKMATE;  // Negative if white is checkmated
    }

    if (gs->stalemate) {
        return STALEMATE;
    }

    int score = 0;

    // Evaluate each piece on the board
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            QString piece = gs->board[row][col];

            if (piece != "--") {
                // Position score
                double piecePositionScore = 0.0;

                // Don't evaluate king position since it's not in the position tables
                if (piece[1] != 'K') {
                    piecePositionScore = piecePositionScores[piece][row][col];
                }

                // Add or subtract score based on piece color
                if (piece[0] == 'w') {
                    score += pieceScore[piece[1]] + piecePositionScore;
                } else {
                    score -= pieceScore[piece[1]] + piecePositionScore;
                }
            }
        }
    }

    return score;
}

/**
 * @brief Selects a random move from the list of valid moves
 * 
 * Used as a fallback when the AI cannot find a preferred move
 * or to add variety to the AI's play.
 * 
 * @param validMoves List of valid moves to choose from
 * @return Move A randomly selected move
 */
Move ChessAI::findRandomMove(const QVector<Move>& validMoves) {
    if (validMoves.isEmpty()) {
        qDebug() << "Warning: No valid moves for random selection!";
        // Return a dummy move if there are no valid moves
        return Move();
    }

    int randomIndex = QRandomGenerator::global()->bounded(validMoves.size());
    return validMoves[randomIndex];
}