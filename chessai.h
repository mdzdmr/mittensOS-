#ifndef CHESSAI_H
#define CHESSAI_H

#include <QObject>
#include <QMap>
#include <QVector>
#include "gamestate.h"

/**
 * @class ChessAI
 * @brief Chess artificial intelligence engine
 *
 * The ChessAI class implements a chess engine using the negamax algorithm
 * with alpha-beta pruning. It evaluates board positions based on material
 * and piece positioning to determine the best move.
 *
 * @author Group 69 (mittensOS)
 */
class ChessAI : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor for the ChessAI class
     * @param parent The parent QObject (default: nullptr)
     */
    explicit ChessAI(QObject *parent = nullptr);

    /** @brief Value assigned to a checkmate position */
    static const int CHECKMATE = 1000;
    
    /** @brief Value assigned to a stalemate position */
    static const int STALEMATE = 0;
    
    /** @brief Maximum search depth for the AI */
    static const int DEPTH = 3;
    
    /**
     * @brief Position evaluation table for knights
     *
     * Higher values represent better squares for knights.
     * Knights are most effective in the center of the board.
     */
    QVector<QVector<double>> knightScores;
    
    /**
     * @brief Position evaluation table for bishops
     *
     * Higher values represent better squares for bishops.
     * Bishops are most effective on long diagonals.
     */
    QVector<QVector<double>> bishopScores;
    
    /**
     * @brief Position evaluation table for rooks
     *
     * Higher values represent better squares for rooks.
     * Rooks are most effective on open files and the 7th rank.
     */
    QVector<QVector<double>> rookScores;
    
    /**
     * @brief Position evaluation table for queens
     *
     * Higher values represent better squares for queens.
     * Queens combine the power of rooks and bishops.
     */
    QVector<QVector<double>> queenScores;
    
    /**
     * @brief Position evaluation table for pawns
     *
     * Higher values represent better squares for pawns.
     * Pawns gain value as they advance toward promotion.
     */
    QVector<QVector<double>> pawnScores;
    
    /**
     * @brief Mapping of piece types to their material values
     *
     * Standard chess piece values:
     * - Pawn: 1
     * - Knight: 3
     * - Bishop: 3
     * - Rook: 5
     * - Queen: 9
     * - King: 0 (not used for evaluation, just prevent capture)
     */
    QMap<QChar, int> pieceScore;
    
    /**
     * @brief Mapping of piece identifiers to their position evaluation tables
     *
     * Maps piece identifiers (e.g., "wN" for white knight) to their
     * corresponding position evaluation tables.
     */
    QMap<QString, QVector<QVector<double>>> piecePositionScores;
    
    /**
     * @brief Initializes the position evaluation tables for all piece types
     *
     * Creates and populates the tables that assign positional values to each
     * piece type on different squares of the board. Creates separate tables
     * for white and black pieces (black tables are mirror images of white).
     */
    void initScoreTables();

public slots:
    /**
     * @brief Finds the best move for the current game state
     *
     * Uses the negamax algorithm with alpha-beta pruning to search
     * for the best move. If no strong move is found, falls back to
     * selecting a random move.
     *
     * @param gs The current game state
     * @param validMoves List of valid moves for the current position
     * @return The best move found by the AI
     */
    Move findBestMove(GameState* gs, const QVector<Move>& validMoves);
    
    /**
     * @brief Selects a random move from the list of valid moves
     *
     * Used as a fallback when the AI cannot find a preferred move
     * or to add variety to the AI's play.
     *
     * @param validMoves List of valid moves to choose from
     * @return A randomly selected move
     */
    Move findRandomMove(const QVector<Move>& validMoves);

private:
    /**
     * @brief The best move found by the search algorithm
     */
    Move nextMove;
    
    /**
     * @brief Implements the negamax algorithm with alpha-beta pruning
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
     * @return Score of the best move found
     */
    int findMoveNegaMaxAlphaBeta(GameState* gs, const QVector<Move>& validMoves,
                                int depth, int alpha, int beta, int turnMultiplier);
    
    /**
     * @brief Evaluates the current board position
     *
     * Assigns a score to the current board state by considering:
     * 1. Material value of all pieces
     * 2. Positional value of pieces based on their location
     * 3. Game-ending conditions (checkmate, stalemate)
     *
     * @param gs Current game state to evaluate
     * @return Score from white's perspective (positive is good for white)
     */
    int scoreBoard(GameState* gs);
    
    /**
     * @brief Validates if a move is in the list of valid moves
     *
     * Checks if the candidate move exists in the provided list of valid moves.
     *
     * @param move The move to validate
     * @param validMoves List of valid moves to check against
     * @return True if the move is valid, false otherwise
     */
    bool isValidMove(const Move& move, const QVector<Move>& validMoves);

signals:
    /**
     * @brief Signal emitted when the best move has been found
     *
     * This signal is emitted by findBestMove() when the search is complete.
     * It carries the best move found by the AI.
     *
     * @param move The best move found by the AI
     */
    void findBestMoveFinished(Move move);
};

#endif // CHESSAI_H