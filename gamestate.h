#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <QString>
#include <QVector>
#include <QPair>
#include <QMap>
#include <functional>

// Forward declaration
class Move;

/**
 * @struct PinInfo
 * @brief Structure that stores information about a pinned piece or checking piece
 *
 * Contains the position of the piece (row, col) and the direction
 * vector to the king (dirRow, dirCol).
 */
struct PinInfo {
    /** @brief Row position of the piece */
    int row;
    
    /** @brief Column position of the piece */
    int col;
    
    /** @brief Row component of direction vector */
    int dirRow;
    
    /** @brief Column component of direction vector */
    int dirCol;

    /** @brief Default constructor */
    PinInfo() : row(0), col(0), dirRow(0), dirCol(0) {}
    
    /**
     * @brief Constructor with all fields
     * @param r Row position
     * @param c Column position
     * @param dr Row direction
     * @param dc Column direction
     */
    PinInfo(int r, int c, int dr, int dc) : row(r), col(c), dirRow(dr), dirCol(dc) {}
};

/**
 * @struct PinsAndChecksInfo
 * @brief Combined structure for check status and pinned/checking pieces
 *
 * Used to return comprehensive information about the current
 * check state and all pins and checks in one object.
 */
struct PinsAndChecksInfo {
    /** @brief Whether the king is in check */
    bool inCheck;
    
    /** @brief List of pinned pieces (pieces that can't move freely because they protect the king) */
    QVector<PinInfo> pins;
    
    /** @brief List of checking pieces (enemy pieces giving check to the king) */
    QVector<PinInfo> checks;

    /** @brief Default constructor */
    PinsAndChecksInfo() : inCheck(false) {}
    
    /**
     * @brief Constructor with all fields
     * @param check Whether king is in check
     * @param p Vector of pin information
     * @param c Vector of check information
     */
    PinsAndChecksInfo(bool check, const QVector<PinInfo>& p, const QVector<PinInfo>& c)
        : inCheck(check), pins(p), checks(c) {}
};

/**
 * @struct CastleRights
 * @brief Structure that tracks castling availability for both sides
 *
 * Stores four boolean flags for kingside and queenside castling
 * rights for both white and black.
 */
struct CastleRights {
    /** @brief White kingside castling right */
    bool wks;
    
    /** @brief Black kingside castling right */
    bool bks;
    
    /** @brief White queenside castling right */
    bool wqs;
    
    /** @brief Black queenside castling right */
    bool bqs;

    /**
     * @brief Constructor with optional castling rights
     * @param _wks White kingside castling (default: true)
     * @param _bks Black kingside castling (default: true)
     * @param _wqs White queenside castling (default: true)
     * @param _bqs Black queenside castling (default: true)
     */
    CastleRights(bool _wks = true, bool _bks = true, bool _wqs = true, bool _bqs = true)
        : wks(_wks), bks(_bks), wqs(_wqs), bqs(_bqs) {}
};

/**
 * @class GameState
 * @brief Main chess game logic class
 *
 * Implements the core chess engine including:
 * - Board representation
 * - Move generation and validation
 * - Check/checkmate/stalemate detection
 * - Special move handling (castling, en passant, etc.)
 * - Game state tracking
 *
 * @author Group 69 (mittensOS)
 */
class GameState {
public:
    /**
     * @brief Constructor that initializes a new chess game
     *
     * Sets up the board in the standard chess starting position,
     * initializes game state variables, and prepares move functions.
     */
    GameState();

    /**
     * @brief 8×8 board representation
     *
     * Each square contains a two-character string:
     * - First character is color ('w' for white, 'b' for black, '-' for empty)
     * - Second character is piece type ('K', 'Q', 'R', 'B', 'N', 'p', or '-' for empty)
     */
    QVector<QVector<QString>> board;

    /** @brief Flag indicating whose turn it is (true for white, false for black) */
    bool whiteToMove;
    
    /** @brief List of all moves made in the game */
    QVector<Move> moveLog;
    
    /** @brief Current position of the white king (row, col) */
    QPair<int, int> whiteKingLocation;
    
    /** @brief Current position of the black king (row, col) */
    QPair<int, int> blackKingLocation;
    
    /** @brief Flag indicating if the current position is checkmate */
    bool checkmate;
    
    /** @brief Flag indicating if the current position is stalemate */
    bool stalemate;
    
    /** @brief Flag indicating if the current player is in check */
    bool inCheck;
    
    /** @brief List of pinned pieces in the current position */
    QVector<PinInfo> pins;
    
    /** @brief List of checking pieces in the current position */
    QVector<PinInfo> checks;
    
    /** @brief Square where en passant capture is possible (row, col), or (-1, -1) if none */
    QPair<int, int> enPassantPossible;
    
    /** @brief History of en passant possibilities for all game positions */
    QVector<QPair<int, int>> enPassantPossibleLog;
    
    /** @brief Current castling rights for both players */
    CastleRights castlingRights;
    
    /** @brief History of castling rights for all game positions */
    QVector<CastleRights> castlingRightsLog;

    /**
     * @brief Makes a move on the board
     *
     * Updates the board position, handles special moves (castling, en passant, promotion),
     * updates game state variables, and logs the move.
     *
     * @param move The move to make
     */
    void makeMove(const Move& move);
    
    /**
     * @brief Undoes the last move
     *
     * Restores the board position, turns, castling rights, and other game state variables
     * to their state before the last move was made.
     */
    void undoMove();
    
    /**
     * @brief Updates castling rights after a move
     *
     * Checks if a king or rook has moved, or if a rook has been captured,
     * and updates the castling rights accordingly.
     *
     * @param move The move that was just made
     */
    void updateCastleRights(const Move& move);
    
    /**
     * @brief Gets all valid moves for the current position
     *
     * Determines all legal moves by checking pins, checks, and move legality.
     * Sets checkmate and stalemate flags if there are no valid moves.
     *
     * @return A vector of all valid moves
     */
    QVector<Move> getValidMoves();
    
    /**
     * @brief Gets all possible moves without considering check
     *
     * Generates all moves for each piece without checking if they would
     * leave the king in check.
     *
     * @return A vector of all possible moves
     */
    QVector<Move> getAllPossibleMoves();

    /**
     * @brief Checks if the current player is in check
     *
     * @return True if the current player's king is in check, false otherwise
     */
    bool isInCheck();
    
    /**
     * @brief Checks if a square is under attack by opponent pieces
     *
     * @param row Row of the square to check
     * @param col Column of the square to check
     * @return True if the square is under attack, false otherwise
     */
    bool squareUnderAttack(int row, int col);
    
    /**
     * @brief Checks for pins and checks in the current position
     *
     * Identifies pins (pieces that can't move because they're protecting the king)
     * and checks (pieces attacking the king).
     *
     * @return A struct containing information about pins and checks
     */
    PinsAndChecksInfo checkForPinsAndChecks();

private:
    /**
     * @brief Generates all valid pawn moves from a position
     *
     * Handles pawn forward moves, captures, en passant, and promotions.
     * Respects pin constraints.
     *
     * @param row The pawn's current row
     * @param col The pawn's current column
     * @param moves The vector to add valid moves to
     */
    void getPawnMoves(int row, int col, QVector<Move>& moves);
    
    /**
     * @brief Generates all valid rook moves from a position
     *
     * Handles rook moves in all four orthogonal directions.
     * Respects pin constraints.
     *
     * @param row The rook's current row
     * @param col The rook's current column
     * @param moves The vector to add valid moves to
     */
    void getRookMoves(int row, int col, QVector<Move>& moves);
    
    /**
     * @brief Generates all valid knight moves from a position
     *
     * Handles knight moves in all eight L-shaped directions.
     * Respects pin constraints.
     *
     * @param row The knight's current row
     * @param col The knight's current column
     * @param moves The vector to add valid moves to
     */
    void getKnightMoves(int row, int col, QVector<Move>& moves);
    
    /**
     * @brief Generates all valid bishop moves from a position
     *
     * Handles bishop moves in all four diagonal directions.
     * Respects pin constraints.
     *
     * @param row The bishop's current row
     * @param col The bishop's current column
     * @param moves The vector to add valid moves to
     */
    void getBishopMoves(int row, int col, QVector<Move>& moves);
    
    /**
     * @brief Generates all valid queen moves from a position
     *
     * Combines rook and bishop moves to get queen moves.
     *
     * @param row The queen's current row
     * @param col The queen's current column
     * @param moves The vector to add valid moves to
     */
    void getQueenMoves(int row, int col, QVector<Move>& moves);
    
    /**
     * @brief Generates all valid king moves from a position
     *
     * Handles king moves to all eight adjacent squares.
     * Ensures moves don't put the king in check.
     *
     * @param row The king's current row
     * @param col The king's current column
     * @param moves The vector to add valid moves to
     */
    void getKingMoves(int row, int col, QVector<Move>& moves);
    
    /**
     * @brief Generates all valid castling moves for a king
     *
     * Handles both kingside and queenside castling.
     * Verifies castling rights and path safety.
     *
     * @param row The king's current row
     * @param col The king's current column
     * @param moves The vector to add valid moves to
     */
    void getCastleMoves(int row, int col, QVector<Move>& moves);
    
    /**
     * @brief Generates kingside castling moves
     *
     * Checks if the path is clear and safe for kingside castling.
     *
     * @param row The king's current row
     * @param col The king's current column
     * @param moves The vector to add valid moves to
     */
    void getKingsideCastleMoves(int row, int col, QVector<Move>& moves);
    
    /**
     * @brief Generates queenside castling moves
     *
     * Checks if the path is clear and safe for queenside castling.
     *
     * @param row The king's current row
     * @param col The king's current column
     * @param moves The vector to add valid moves to
     */
    void getQueensideCastleMoves(int row, int col, QVector<Move>& moves);

    /**
     * @brief Map of move functions for each piece type
     *
     * Uses the piece character as key (e.g., 'p', 'R', 'N')
     * and maps to the corresponding move generation function.
     */
    QMap<QChar, std::function<void(int, int, QVector<Move>&)>> moveFunctions;
};

/**
 * @class Move
 * @brief Class representing a chess move
 *
 * Stores all information about a move including:
 * - Start and end positions
 * - Pieces involved
 * - Special move flags (promotion, en passant, castling)
 * - Move notation generation
 */
class Move {
public:
    /**
     * @brief Default constructor
     *
     * Creates an empty move with default values.
     */
    Move() : startRow(0), startCol(0), endRow(0), endCol(0),
             pieceMoved("--"), pieceCaptured("--"),
             isPawnPromotion(false), isEnpassantMove(false),
             isCastleMove(false), isCapture(false), moveID(0) {}

    /**
     * @brief Main constructor
     *
     * Creates a move with specified start/end positions and board state.
     *
     * @param startSq Starting square coordinates (row, col)
     * @param endSq Ending square coordinates (row, col)
     * @param board Current board state
     * @param isEnpassantMove Flag for en passant captures
     * @param isCastleMove Flag for castling moves
     */
    Move(QPair<int, int> startSq, QPair<int, int> endSq, const QVector<QVector<QString>>& board,
         bool isEnpassantMove = false, bool isCastleMove = false);

    /** @brief Row index of the starting square */
    int startRow;
    
    /** @brief Column index of the starting square */
    int startCol;
    
    /** @brief Row index of the destination square */
    int endRow;
    
    /** @brief Column index of the destination square */
    int endCol;
    
    /** @brief Two-character string representing the moved piece */
    QString pieceMoved;
    
    /** @brief Two-character string representing the captured piece, or "--" if none */
    QString pieceCaptured;
    
    /** @brief Flag for pawn promotion moves */
    bool isPawnPromotion;
    
    /** @brief Flag for en passant capture moves */
    bool isEnpassantMove;
    
    /** @brief Flag for castling moves */
    bool isCastleMove;
    
    /** @brief Flag indicating if the move is a capture */
    bool isCapture;
    
    /** @brief Unique ID for move comparison, computed from start/end positions */
    int moveID;

    /**
     * @brief Gets the chess notation for a move
     *
     * Returns the move in standard algebraic notation (SAN).
     *
     * @return The move in chess notation
     */
    QString getChessNotation() const;
    
    /**
     * @brief Converts board coordinates to algebraic notation (e.g., "e4")
     *
     * @param row The row index
     * @param col The column index
     * @return The square in algebraic notation
     */
    QString getRankFile(int row, int col) const;

    /**
     * @brief Equality operator for move comparison
     *
     * Compares moves based on their unique moveID.
     *
     * @param other The move to compare with
     * @return True if the moves are the same, false otherwise
     */
    bool operator==(const Move& other) const {
        return moveID == other.moveID;
    }

    /**
     * @brief Gets a string representation of the move
     *
     * Returns the move in a simplified form of algebraic notation.
     *
     * @return A string representation of the move
     */
    QString toString() const;
 
    /** @brief Maps rank notation ("1"-"8") to internal row indices (7-0) */
    static QMap<QString, int> ranksToRows;
    
    /** @brief Maps internal row indices (0-7) to rank notation ("8"-"1") */
    static QMap<int, QString> rowsToRanks;
    
    /** @brief Maps file notation ("a"-"h") to internal column indices (0-7) */
    static QMap<QString, int> filesToCols;
    
    /** @brief Maps internal column indices (0-7) to file notation ("a"-"h") */
    static QMap<int, QString> colsToFiles;
};

#endif // GAMESTATE_H