#include "gamestate.h"

/**
 * @brief Static maps for converting between chess notation and board coordinates
 * 
 * These maps facilitate conversion between chess notation (e.g., "e4") and
 * the internal board representation using row/column indices.
 */
QMap<QString, int> Move::ranksToRows = {
    {"1", 7}, {"2", 6}, {"3", 5}, {"4", 4},
    {"5", 3}, {"6", 2}, {"7", 1}, {"8", 0}
};

QMap<int, QString> Move::rowsToRanks;
QMap<QString, int> Move::filesToCols = {
    {"a", 0}, {"b", 1}, {"c", 2}, {"d", 3},
    {"e", 4}, {"f", 5}, {"g", 6}, {"h", 7}
};
QMap<int, QString> Move::colsToFiles;

/**
 * @brief Constructor for GameState class
 * 
 * Initializes the chess board with the standard starting position,
 * sets up move functions for each piece type, initializes game state variables,
 * and creates the reverse mappings for chess notation conversion.
 */
GameState::GameState() {
    // Initialize board with starting position
    board = {
        {{"bR"}, {"bN"}, {"bB"}, {"bQ"}, {"bK"}, {"bB"}, {"bN"}, {"bR"}},
        {{"bp"}, {"bp"}, {"bp"}, {"bp"}, {"bp"}, {"bp"}, {"bp"}, {"bp"}},
        {{"--"}, {"--"}, {"--"}, {"--"}, {"--"}, {"--"}, {"--"}, {"--"}},
        {{"--"}, {"--"}, {"--"}, {"--"}, {"--"}, {"--"}, {"--"}, {"--"}},
        {{"--"}, {"--"}, {"--"}, {"--"}, {"--"}, {"--"}, {"--"}, {"--"}},
        {{"--"}, {"--"}, {"--"}, {"--"}, {"--"}, {"--"}, {"--"}, {"--"}},
        {{"wp"}, {"wp"}, {"wp"}, {"wp"}, {"wp"}, {"wp"}, {"wp"}, {"wp"}},
        {{"wR"}, {"wN"}, {"wB"}, {"wQ"}, {"wK"}, {"wB"}, {"wN"}, {"wR"}}
    };

    // Initialize move function map
    moveFunctions = {
        {'p', [this](int row, int col, QVector<Move>& moves) { this->getPawnMoves(row, col, moves); }},
        {'R', [this](int row, int col, QVector<Move>& moves) { this->getRookMoves(row, col, moves); }},
        {'N', [this](int row, int col, QVector<Move>& moves) { this->getKnightMoves(row, col, moves); }},
        {'B', [this](int row, int col, QVector<Move>& moves) { this->getBishopMoves(row, col, moves); }},
        {'Q', [this](int row, int col, QVector<Move>& moves) { this->getQueenMoves(row, col, moves); }},
        {'K', [this](int row, int col, QVector<Move>& moves) { this->getKingMoves(row, col, moves); }}
    };

    // Initialize game state variables
    whiteToMove = true;
    moveLog = QVector<Move>();
    whiteKingLocation = qMakePair(7, 4);
    blackKingLocation = qMakePair(0, 4);
    checkmate = false;
    stalemate = false;
    inCheck = false;
    pins = QVector<PinInfo>();
    checks = QVector<PinInfo>();
    enPassantPossible = qMakePair(-1, -1);
    enPassantPossibleLog.push_back(enPassantPossible);
    castlingRights = CastleRights(true, true, true, true);
    castlingRightsLog.push_back(castlingRights);

    // Initialize reverse mappings for Move class
    for (auto it = Move::ranksToRows.begin(); it != Move::ranksToRows.end(); ++it) {
        Move::rowsToRanks[it.value()] = it.key();
    }

    for (auto it = Move::filesToCols.begin(); it != Move::filesToCols.end(); ++it) {
        Move::colsToFiles[it.value()] = it.key();
    }
}

/**
 * @brief Makes a move on the chess board
 * 
 * Updates the board position, handles special moves (castling, en passant, promotion),
 * updates game state variables, and logs the move.
 * 
 * @param move The move to make
 */
void GameState::makeMove(const Move& move) {
    // Clear the starting square
    board[move.startRow][move.startCol] = "--";
    // Place the piece on the destination square
    board[move.endRow][move.endCol] = move.pieceMoved;
    // Add the move to the log
    moveLog.push_back(move);
    // Switch turns
    whiteToMove = !whiteToMove;

    // Update king location if the king moved
    if (move.pieceMoved == "wK") {
        whiteKingLocation = qMakePair(move.endRow, move.endCol);
    } else if (move.pieceMoved == "bK") {
        blackKingLocation = qMakePair(move.endRow, move.endCol);
    }

    // Handle pawn promotion
    if (move.isPawnPromotion) {
        board[move.endRow][move.endCol] = QString(move.pieceMoved[0]) + "Q";
    }

    // Handle en passant capture
    if (move.isEnpassantMove) {
        board[move.startRow][move.endCol] = "--";
    }

    // Update en passant possibility
    if (move.pieceMoved[1] == 'p' && qAbs(move.startRow - move.endRow) == 2) {
        enPassantPossible = qMakePair((move.startRow + move.endRow) / 2, move.startCol);
    } else {
        enPassantPossible = qMakePair(-1, -1);
    }

    // Handle castle move - move the rook
    if (move.isCastleMove) {
        if (move.endCol - move.startCol == 2) {  // King side castle
            board[move.endRow][move.endCol - 1] = board[move.endRow][move.endCol + 1];
            board[move.endRow][move.endCol + 1] = "--";
        } else {  // Queen side castle
            board[move.endRow][move.endCol + 1] = board[move.endRow][move.endCol - 2];
            board[move.endRow][move.endCol - 2] = "--";
        }
    }

    // Update en passant log
    enPassantPossibleLog.push_back(enPassantPossible);

    // Update castling rights
    updateCastleRights(move);
    castlingRightsLog.push_back(CastleRights(castlingRights.wks, castlingRights.bks,
                                         castlingRights.wqs, castlingRights.bqs));
}

/**
 * @brief Undoes the last move
 * 
 * Restores the board position, turns, castling rights, and other game state variables
 * to their state before the last move was made.
 */
void GameState::undoMove() {
    if (moveLog.isEmpty()) {
        return;
    }

    Move move = moveLog.back();
    moveLog.pop_back();

    // Restore the pieces
    board[move.startRow][move.startCol] = move.pieceMoved;
    board[move.endRow][move.endCol] = move.pieceCaptured;

    // Switch turns back
    whiteToMove = !whiteToMove;

    // Update king location if the king moved
    if (move.pieceMoved == "wK") {
        whiteKingLocation = qMakePair(move.startRow, move.startCol);
    } else if (move.pieceMoved == "bK") {
        blackKingLocation = qMakePair(move.startRow, move.startCol);
    }

    // Handle en passant move
    if (move.isEnpassantMove) {
        board[move.endRow][move.endCol] = "--";
        board[move.startRow][move.endCol] = move.pieceCaptured;
    }

    // Update en passant log
    enPassantPossibleLog.pop_back();
    enPassantPossible = enPassantPossibleLog.back();

    // Update castling rights log
    castlingRightsLog.pop_back();
    castlingRights = castlingRightsLog.back();

    // Handle castle move - move the rook back
    if (move.isCastleMove) {
        if (move.endCol - move.startCol == 2) {  // King side castle
            board[move.endRow][move.endCol + 1] = board[move.endRow][move.endCol - 1];
            board[move.endRow][move.endCol - 1] = "--";
        } else {  // Queen side castle
            board[move.endRow][move.endCol - 2] = board[move.endRow][move.endCol + 1];
            board[move.endRow][move.endCol + 1] = "--";
        }
    }

    // Reset checkmate and stalemate flags
    checkmate = false;
    stalemate = false;
}

/**
 * @brief Updates castling rights after a move
 * 
 * Checks if a king or rook has moved, or if a rook has been captured,
 * and updates the castling rights accordingly.
 * 
 * @param move The move that was just made
 */
void GameState::updateCastleRights(const Move& move) {
    // If a rook is captured
    if (move.pieceCaptured == "wR") {
        if (move.endCol == 0) {
            castlingRights.wqs = false;
        } else if (move.endCol == 7) {
            castlingRights.wks = false;
        }
    } else if (move.pieceCaptured == "bR") {
        if (move.endCol == 0) {
            castlingRights.bqs = false;
        } else if (move.endCol == 7) {
            castlingRights.bks = false;
        }
    }

    // If the king moves
    if (move.pieceMoved == "wK") {
        castlingRights.wqs = false;
        castlingRights.wks = false;
    } else if (move.pieceMoved == "bK") {
        castlingRights.bqs = false;
        castlingRights.bks = false;
    }

    // If a rook moves
    else if (move.pieceMoved == "wR") {
        if (move.startRow == 7) {
            if (move.startCol == 0) {
                castlingRights.wqs = false;
            } else if (move.startCol == 7) {
                castlingRights.wks = false;
            }
        }
    } else if (move.pieceMoved == "bR") {
        if (move.startRow == 0) {
            if (move.startCol == 0) {
                castlingRights.bqs = false;
            } else if (move.startCol == 7) {
                castlingRights.bks = false;
            }
        }
    }
}

/**
 * @brief Gets all valid moves for the current position
 * 
 * Determines all legal moves by checking pins, checks, and move legality.
 * Sets checkmate and stalemate flags if there are no valid moves.
 * 
 * @return A vector of all valid moves
 */
QVector<Move> GameState::getValidMoves()
{
    // Save current castling rights so we can restore them later
    CastleRights tempCastleRights = castlingRights;

    // First, determine pins/checks so we know if the king is in check
    PinsAndChecksInfo pinCheckInfo = checkForPinsAndChecks();
    inCheck = pinCheckInfo.inCheck;
    pins = pinCheckInfo.pins;
    checks = pinCheckInfo.checks;

    // We'll build our final list of moves here
    QVector<Move> moves;

    // Get the current king position
    int kingRow = whiteToMove ? whiteKingLocation.first : blackKingLocation.first;
    int kingCol = whiteToMove ? whiteKingLocation.second : blackKingLocation.second;

    // 1) If we are in check, handle special logic
    if (inCheck) {
        // Single check
        if (checks.size() == 1) {
            // Generate all possible moves
            moves = getAllPossibleMoves();

            // Now figure out which moves can block/capture the checking piece
            // to resolve the single check
            PinInfo check = checks[0];
            int checkRow = check.row;
            int checkCol = check.col;
            QString pieceChecking = board[checkRow][checkCol];

            // Collect squares that block or capture the checker
            QVector<QPair<int, int>> validSquares;

            // Knight cannot be blocked, so we must capture it directly
            if (pieceChecking[1] == 'N') {
                validSquares.push_back(qMakePair(checkRow, checkCol));
            } 
            else {
                // Add the ray from king to checker (including checker square)
                for (int i = 1; i < 8; i++) {
                    QPair<int, int> square(kingRow + check.dirRow * i,
                                           kingCol + check.dirCol * i);
                    validSquares.push_back(square);
                    if (square.first == checkRow && square.second == checkCol) {
                        break;  // stop once we reach the checking piece
                    }
                }
            }

            // Remove any moves that do not capture/block the checking piece
            for (int i = moves.size() - 1; i >= 0; i--) {
                // If this isn't a king move, it must block or capture
                if (moves[i].pieceMoved[1] != 'K') {
                    bool resolvesCheck = false;
                    for (const auto &sq : validSquares) {
                        if (moves[i].endRow == sq.first && 
                            moves[i].endCol == sq.second)
                        {
                            resolvesCheck = true;
                            break;
                        }
                    }
                    if (!resolvesCheck) {
                        moves.removeAt(i);
                    }
                }
            }
        }
        // Double check: only the king can move
        else {
            getKingMoves(kingRow, kingCol, moves);
        }
        
        // Even if we're in check, we can attempt castling if the logic 
        // inside getCastleMoves(...) disallows castling while in check 
        // (and/or checks if the king passes through check).
        // Typically, you can't castle out of check, so it might not add anything,
        // but if your getCastleMoves logic handles that, we can still call it.
        if (whiteToMove) {
            getCastleMoves(whiteKingLocation.first, whiteKingLocation.second, moves);
        } else {
            getCastleMoves(blackKingLocation.first, blackKingLocation.second, moves);
        }
    }
    // 2) If we are NOT in check, generate all possible moves normally
    else {
        // All moves
        moves = getAllPossibleMoves();

        // Also add potential castling moves
        if (whiteToMove) {
            getCastleMoves(whiteKingLocation.first, whiteKingLocation.second, moves);
        } else {
            getCastleMoves(blackKingLocation.first, blackKingLocation.second, moves);
        }
    }

    // 3) Determine if we now have any legal moves
    if (moves.isEmpty()) {
        if (inCheck) {
            checkmate = true;
            stalemate = false;
        } else {
            checkmate = false;
            stalemate = true;
        }
    } 
    else {
        checkmate = false;
        stalemate = false;
    }

    // 4) Restore castling rights to what they were before generating moves
    castlingRights = tempCastleRights;

    return moves;
}

/**
 * @brief Checks if the current player is in check
 * 
 * @return True if the current player's king is in check, false otherwise
 */
bool GameState::isInCheck() {
    if (whiteToMove) {
        return squareUnderAttack(whiteKingLocation.first, whiteKingLocation.second);
    } else {
        return squareUnderAttack(blackKingLocation.first, blackKingLocation.second);
    }
}

/**
 * @brief Checks if a square is under attack by opponent pieces
 * 
 * @param row Row of the square to check
 * @param col Column of the square to check
 * @return True if the square is under attack, false otherwise
 */
bool GameState::squareUnderAttack(int row, int col) {
    // Switch to opponent's turn temporarily
    whiteToMove = !whiteToMove;
    QVector<Move> opponentMoves = getAllPossibleMoves();
    whiteToMove = !whiteToMove;

    // Check if any opponent move attacks the square
    for (const Move& move : opponentMoves) {
        if (move.endRow == row && move.endCol == col) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Gets all possible moves without considering check
 * 
 * Generates all moves for each piece without checking if they would
 * leave the king in check.
 * 
 * @return A vector of all possible moves
 */
QVector<Move> GameState::getAllPossibleMoves() {
    QVector<Move> moves;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            QChar turn = board[row][col][0];
            if ((turn == 'w' && whiteToMove) || (turn == 'b' && !whiteToMove)) {
                QChar piece = board[row][col][1];
                if (moveFunctions.contains(piece)) {
                    moveFunctions[piece](row, col, moves);
                }
            }
        }
    }
    
    return moves;
}

/**
 * @brief Checks for pins and checks in the current position
 * 
 * Identifies pins (pieces that can't move because they're protecting the king)
 * and checks (pieces attacking the king).
 * 
 * @return A struct containing information about pins and checks
 */
PinsAndChecksInfo GameState::checkForPinsAndChecks() {
    QVector<PinInfo> pins;
    QVector<PinInfo> checks;
    bool inCheck = false;

    // Get king position and set team colors
    QString enemyColor = (whiteToMove) ? "b" : "w";
    QString teamColor = (whiteToMove) ? "w" : "b";
    int startRow = (whiteToMove) ? whiteKingLocation.first : blackKingLocation.first;
    int startCol = (whiteToMove) ? whiteKingLocation.second : blackKingLocation.second;

    // Direction vectors for checks and pins
    QVector<QPair<int, int>> directions = {
        {-1, 0}, {0, -1}, {1, 0}, {0, 1},  // Rook directions
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}  // Bishop directions
    };

    // Check each direction
    for (int j = 0; j < directions.size(); j++) {
        auto d = directions[j];
        PinInfo possiblePin;
        possiblePin.row = -1;
        possiblePin.col = -1;
        possiblePin.dirRow = d.first;
        possiblePin.dirCol = d.second;

        for (int i = 1; i < 8; i++) {
            int endRow = startRow + d.first * i;
            int endCol = startCol + d.second * i;

            // Check if square is on the board
            if (endRow >= 0 && endRow <= 7 && endCol >= 0 && endCol <= 7) {
                QString endPiece = board[endRow][endCol];

                // Check if piece is a potential pin
                if (endPiece[0] == teamColor[0] && endPiece[1] != 'K') {
                    if (possiblePin.row == -1) {
                        possiblePin.row = endRow;
                        possiblePin.col = endCol;
                    } else {
                        break;  // Second allied piece, no pin or check
                    }
                } else if (endPiece[0] == enemyColor[0]) {
                    QChar pieceType = endPiece[1];

                    // Check if piece type can attack the king
                    bool canCheck = false;

                    // Rook checks (horizontal/vertical)
                    if (j <= 3 && pieceType == 'R') {
                        canCheck = true;
                    }
                    // Bishop checks (diagonal)
                    else if (j >= 4 && pieceType == 'B') {
                        canCheck = true;
                    }
                    // Pawn checks (only directly adjacent diagonals)
                    else if (i == 1 && pieceType == 'p') {
                        if ((enemyColor == "w" && j >= 6 && j <= 7) ||
                            (enemyColor == "b" && j >= 4 && j <= 5)) {
                            canCheck = true;
                        }
                    }
                    // Queen checks (any direction)
                    else if (pieceType == 'Q') {
                        canCheck = true;
                    }
                    // King checks (only directly adjacent)
                    else if (i == 1 && pieceType == 'K') {
                        canCheck = true;
                    }

                    if (canCheck) {
                        if (possiblePin.row == -1) {  // No piece blocking, so check
                            inCheck = true;
                            checks.push_back(PinInfo(endRow, endCol, d.first, d.second));
                            break;
                        } else {  // Piece in the way, so pin
                            pins.push_back(possiblePin);
                            break;
                        }
                    } else {  // Enemy piece that doesn't give check
                        break;
                    }
                }
            } else {  // Off board
                break;
            }
        }
    }
    
    // Knight checks (cannot be pins)
    QVector<QPair<int, int>> knightMoves = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };

    for (const auto& move : knightMoves) {
        int endRow = startRow + move.first;
        int endCol = startCol + move.second;

        if (endRow >= 0 && endRow <= 7 && endCol >= 0 && endCol <= 7) {
            QString endPiece = board[endRow][endCol];
            if (endPiece[0] == enemyColor[0] && endPiece[1] == 'N') {
                inCheck = true;
                checks.push_back(PinInfo(endRow, endCol, move.first, move.second));
            }
        }
    }

    return PinsAndChecksInfo(inCheck, pins, checks);
}

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
void GameState::getPawnMoves(int row, int col, QVector<Move>& moves) {
    // Check if pawn is pinned
    bool piecePinned = false;
    QPair<int, int> pinDirection(0, 0);

    for (int i = pins.size() - 1; i >= 0; i--) {
        if (pins[i].row == row && pins[i].col == col) {
            piecePinned = true;
            pinDirection = qMakePair(pins[i].dirRow, pins[i].dirCol);
            pins.removeAt(i);
            break;
        }
    }

    // Determine move direction and start row based on color
    int moveAmount = (whiteToMove) ? -1 : 1;
    int startRow = (whiteToMove) ? 6 : 1;
    QString enemyColor = (whiteToMove) ? "b" : "w";
    QPair<int, int> kingPos = (whiteToMove) ? whiteKingLocation : blackKingLocation;

    // Forward move
    if (row + moveAmount >= 0 && row + moveAmount <= 7) {
        if (board[row + moveAmount][col] == "--") {
            if (!piecePinned || pinDirection == qMakePair(moveAmount, 0)) {
                moves.push_back(Move(qMakePair(row, col), qMakePair(row + moveAmount, col), board));

                // Two square pawn advance
                if (row == startRow && board[row + 2 * moveAmount][col] == "--") {
                    moves.push_back(Move(qMakePair(row, col), qMakePair(row + 2 * moveAmount, col), board));
                }
            }
        }

        // Captures to the left
        if (col - 1 >= 0) {
            if (!piecePinned || pinDirection == qMakePair(moveAmount, -1)) {
                if (board[row + moveAmount][col - 1][0] == enemyColor[0]) {
                    moves.push_back(Move(qMakePair(row, col), qMakePair(row + moveAmount, col - 1), board));
                }

                // En passant capture to the left
                if (enPassantPossible.first == row + moveAmount && enPassantPossible.second == col - 1) {
                    // Special case: check for horizontal pins
                    bool blockingPiece = false;
                    bool attackingPiece = false;

                    if (kingPos.first == row) {
                        if (kingPos.second < col) { // King is left of the pawn
                            // Check between king and pawn
                            for (int i = kingPos.second + 1; i < col - 1; i++) {
                                if (board[row][i] != "--") {
                                    blockingPiece = true;
                                    break;
                                }
                            }
                            // Check outside of pawn
                            for (int i = col + 1; i < 8; i++) {
                                QString square = board[row][i];
                                if (square[0] == enemyColor[0] && (square[1] == 'R' || square[1] == 'Q')) {
                                    attackingPiece = true;
                                    break;
                                } else if (square != "--") {
                                    blockingPiece = true;
                                    break;
                                }
                            }
                        } else { // King is right of the pawn
                            // Check between king and pawn
                            for (int i = kingPos.second - 1; i > col; i--) {
                                if (board[row][i] != "--") {
                                    blockingPiece = true;
                                    break;
                                }
                            }
                            // Check outside of pawn
                            for (int i = col - 2; i >= 0; i--) {
                                QString square = board[row][i];
                                if (square[0] == enemyColor[0] && (square[1] == 'R' || square[1] == 'Q')) {
                                    attackingPiece = true;
                                    break;
                                } else if (square != "--") {
                                    blockingPiece = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (!attackingPiece || blockingPiece) {
                        moves.push_back(Move(qMakePair(row, col), qMakePair(row + moveAmount, col - 1), board, true));
                    }
                }
            }
        }

        // Captures to the right
        if (col + 1 <= 7) {
            if (!piecePinned || pinDirection == qMakePair(moveAmount, 1)) {
                if (board[row + moveAmount][col + 1][0] == enemyColor[0]) {
                    moves.push_back(Move(qMakePair(row, col), qMakePair(row + moveAmount, col + 1), board));
                }

                // En passant capture to the right
                if (enPassantPossible.first == row + moveAmount && enPassantPossible.second == col + 1) {
                    // Special case: check for horizontal pins
                    bool blockingPiece = false;
                    bool attackingPiece = false;

                    if (kingPos.first == row) {
                        if (kingPos.second < col) { // King is left of the pawn
                            // Check between king and pawn
                            for (int i = kingPos.second + 1; i < col; i++) {
                                if (board[row][i] != "--") {
                                    blockingPiece = true;
                                    break;
                                }
                            }
                            // Check outside of pawn
                            for (int i = col + 2; i < 8; i++) {
                                QString square = board[row][i];
                                if (square[0] == enemyColor[0] && (square[1] == 'R' || square[1] == 'Q')) {
                                    attackingPiece = true;
                                    break;
                                } else if (square != "--") {
                                    blockingPiece = true;
                                    break;
                                }
                            }
                        } else { // King is right of the pawn
                            // Check between king and pawn
                            for (int i = kingPos.second - 1; i > col + 1; i--) {
                                if (board[row][i] != "--") {
                                    blockingPiece = true;
                                    break;
                                }
                            }
                            // Check outside of pawn
                            for (int i = col - 1; i >= 0; i--) {
                                QString square = board[row][i];
                                if (square[0] == enemyColor[0] && (square[1] == 'R' || square[1] == 'Q')) {
                                    attackingPiece = true;
                                    break;
                                } else if (square != "--") {
                                    blockingPiece = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (!attackingPiece || blockingPiece) {
                        moves.push_back(Move(qMakePair(row, col), qMakePair(row + moveAmount, col + 1), board, true));
                    }
                }
            }
        }
    }
}

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
void GameState::getRookMoves(int row, int col, QVector<Move>& moves) {
    // Check if rook is pinned
    bool piecePinned = false;
    QPair<int, int> pinDirection(0, 0);

    for (int i = pins.size() - 1; i >= 0; i--) {
        if (pins[i].row == row && pins[i].col == col) {
            piecePinned = true;
            pinDirection = qMakePair(pins[i].dirRow, pins[i].dirCol);
            // Remove pin only if not a queen (which uses both rook and bishop move logic)
            if (board[row][col][1] != 'Q') {
                pins.removeAt(i);
            }
            break;
        }
    }

    // Rook move directions (up, left, down, right)
    QVector<QPair<int, int>> directions = {{-1, 0}, {0, -1}, {1, 0}, {0, 1}};
    QString enemyColor = (whiteToMove) ? "b" : "w";

    for (const auto& d : directions) {
        for (int i = 1; i < 8; i++) {
            int endRow = row + d.first * i;
            int endCol = col + d.second * i;

            // Check if square is on the board
            if (endRow >= 0 && endRow <= 7 && endCol >= 0 && endCol <= 7) {
                // Check if move is valid with pin
                if (!piecePinned ||
                    pinDirection == d ||
                    pinDirection == qMakePair(-d.first, -d.second)) {

                    QString endPiece = board[endRow][endCol];
                    if (endPiece == "--") {  // Empty square
                        moves.push_back(Move(qMakePair(row, col), qMakePair(endRow, endCol), board));
                    } else if (endPiece[0] == enemyColor[0]) {  // Capture
                        moves.push_back(Move(qMakePair(row, col), qMakePair(endRow, endCol), board));
                        break;  // Can't move past a piece
                    } else {  // Friendly piece
                        break;  // Can't move past a piece
                    }
                }
            } else {  // Off board
                break;
            }
        }
    }
}

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
void GameState::getBishopMoves(int row, int col, QVector<Move>& moves) {
    // Check if bishop is pinned
    bool piecePinned = false;
    QPair<int, int> pinDirection(0, 0);

    for (int i = pins.size() - 1; i >= 0; i--) {
        if (pins[i].row == row && pins[i].col == col) {
            piecePinned = true;
            pinDirection = qMakePair(pins[i].dirRow, pins[i].dirCol);
            pins.removeAt(i);
            break;
        }
    }

    // Bishop move directions (diagonals)
    QVector<QPair<int, int>> directions = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    QString enemyColor = (whiteToMove) ? "b" : "w";

    for (const auto& d : directions) {
        for (int i = 1; i < 8; i++) {
            int endRow = row + d.first * i;
            int endCol = col + d.second * i;

            // Check if square is on the board
            if (endRow >= 0 && endRow <= 7 && endCol >= 0 && endCol <= 7) {
                // Check if move is valid with pin
                if (!piecePinned ||
                    pinDirection == d ||
                    pinDirection == qMakePair(-d.first, -d.second)) {

                    QString endPiece = board[endRow][endCol];
                    if (endPiece == "--") {  // Empty square
                        moves.push_back(Move(qMakePair(row, col), qMakePair(endRow, endCol), board));
                    } else if (endPiece[0] == enemyColor[0]) {  // Capture
                        moves.push_back(Move(qMakePair(row, col), qMakePair(endRow, endCol), board));
                        break;  // Can't move past a piece
                    } else {  // Friendly piece
                        break;  // Can't move past a piece
                    }
                }
            } else {  // Off board
                break;
            }
        }
    }
}

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
void GameState::getKnightMoves(int row, int col, QVector<Move>& moves) {
    // Check if knight is pinned
    bool piecePinned = false;
    
    for (int i = pins.size() - 1; i >= 0; i--) {
        if (pins[i].row == row && pins[i].col == col) {
            piecePinned = true;
            pins.removeAt(i);
            break;
        }
    }
    
    // Knight move patterns (L shapes)
    QVector<QPair<int, int>> knightMoves = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    
    QString teamColor = (whiteToMove) ? "w" : "b";
    
    for (const auto& m : knightMoves) {
        int endRow = row + m.first;
        int endCol = col + m.second;
        
        // Check if square is on the board
        if (endRow >= 0 && endRow <= 7 && endCol >= 0 && endCol <= 7) {
            if (!piecePinned) {  // Pinned knights can't move
                QString endPiece = board[endRow][endCol];
                if (endPiece[0] != teamColor[0]) {  // Not a friendly piece
                    moves.push_back(Move(qMakePair(row, col), qMakePair(endRow, endCol), board));
                }
            }
        }
    }
}

/**
 * @brief Generates all valid queen moves from a position
 * 
 * Combines rook and bishop moves to get queen moves.
 * 
 * @param row The queen's current row
 * @param col The queen's current column
 * @param moves The vector to add valid moves to
 */
void GameState::getQueenMoves(int row, int col, QVector<Move>& moves) {
    // Queen combines rook and bishop moves
    getBishopMoves(row, col, moves);
    getRookMoves(row, col, moves);
}

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
void GameState::getKingMoves(int row, int col, QVector<Move>& moves) {
    // King move patterns (all adjacent squares)
    QVector<QPair<int, int>> kingMoves = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1}, {0, 1},
        {1, -1}, {1, 0}, {1, 1}
    };

    QString teamColor = (whiteToMove) ? "w" : "b";

    for (const auto& m : kingMoves) {
        int endRow = row + m.first;
        int endCol = col + m.second;

        // Check if square is on the board
        if (endRow >= 0 && endRow <= 7 && endCol >= 0 && endCol <= 7) {
            QString endPiece = board[endRow][endCol];
            if (endPiece[0] != teamColor[0]) {  // Not a friendly piece
                // Temporarily move king to check if the move is safe
                if (whiteToMove) {
                    whiteKingLocation = qMakePair(endRow, endCol);
                } else {
                    blackKingLocation = qMakePair(endRow, endCol);
                }

                // Get in-check status after the move
                PinsAndChecksInfo pinCheckTuple = checkForPinsAndChecks();
                bool inCheck = pinCheckTuple.inCheck;

                // If the move doesn't put king in check, it's valid
                if (!inCheck) {
                    moves.push_back(Move(qMakePair(row, col), qMakePair(endRow, endCol), board));
                }

                // Restore king position
                if (whiteToMove) {
                    whiteKingLocation = qMakePair(row, col);
                } else {
                    blackKingLocation = qMakePair(row, col);
                }
            }
        }
    }
}

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
void GameState::getCastleMoves(int row, int col, QVector<Move>& moves) {
    // Check if king is in check (can't castle out of check)
    if (squareUnderAttack(row, col)) {
        return;
    }

    // Kingside castle
    if ((whiteToMove && castlingRights.wks) || (!whiteToMove && castlingRights.bks)) {
        getKingsideCastleMoves(row, col, moves);
    }

    // Queenside castle
    if ((whiteToMove && castlingRights.wqs) || (!whiteToMove && castlingRights.bqs)) {
        getQueensideCastleMoves(row, col, moves);
    }
}

/**
 * @brief Generates kingside castling moves
 * 
 * Checks if the path is clear and safe for kingside castling.
 * 
 * @param row The king's current row
 * @param col The king's current column
 * @param moves The vector to add valid moves to
 */
void GameState::getKingsideCastleMoves(int row, int col, QVector<Move>& moves) {
    if (col + 2 > 7) {  // Boundary check
        return;
    }

    // Check if squares between king and rook are empty
    if (board[row][col + 1] == "--" && board[row][col + 2] == "--") {
        // Check if squares king moves through are not under attack
        if (!squareUnderAttack(row, col + 1) && !squareUnderAttack(row, col + 2)) {
            moves.push_back(Move(qMakePair(row, col), qMakePair(row, col + 2), board, false, true));
        }
    }
}

/**
 * @brief Generates queenside castling moves
 * 
 * Checks if the path is clear and safe for queenside castling.
 * 
 * @param row The king's current row
 * @param col The king's current column
 * @param moves The vector to add valid moves to
 */
void GameState::getQueensideCastleMoves(int row, int col, QVector<Move>& moves) {
    if (col - 2 < 0 || col - 3 < 0) {  // Boundary check
        return;
    }

    // Check if squares between king and rook are empty
    if (board[row][col - 1] == "--" && board[row][col - 2] == "--" && board[row][col - 3] == "--") {
        // Check if squares king moves through are not under attack
        if (!squareUnderAttack(row, col - 1) && !squareUnderAttack(row, col - 2)) {
            moves.push_back(Move(qMakePair(row, col), qMakePair(row, col - 2), board, false, true));
        }
    }
}

/**
 * @brief Constructor for the Move class
 * 
 * Creates a move with start/end positions and additional properties.
 * 
 * @param startSq Starting square coordinates (row, col)
 * @param endSq Ending square coordinates (row, col)
 * @param board Current board state
 * @param isEnpassantMove Flag for en passant captures
 * @param isCastleMove Flag for castling moves
 */
Move::Move(QPair<int, int> startSq, QPair<int, int> endSq, const QVector<QVector<QString>>& board,
           bool isEnpassantMove, bool isCastleMove) {
    startRow = startSq.first;
    startCol = startSq.second;
    endRow = endSq.first;
    endCol = endSq.second;

    pieceMoved = board[startRow][startCol];
    pieceCaptured = board[endRow][endCol];

    // Pawn promotion
    isPawnPromotion = (pieceMoved == "wp" && endRow == 0) || (pieceMoved == "bp" && endRow == 7);

    // En passant
    this->isEnpassantMove = isEnpassantMove;
    if (isEnpassantMove) {
        pieceCaptured = (pieceMoved == "wp") ? "bp" : "wp";
    }

    // Castling
    this->isCastleMove = isCastleMove;

    // Capture flag
    isCapture = (pieceCaptured != "--");

    // Unique move ID for comparison
    moveID = startRow * 1000 + startCol * 100 + endRow * 10 + endCol;
}

/**
 * @brief Gets the chess notation for a move
 * 
 * Returns the move in standard algebraic notation (SAN).
 * 
 * @return The move in chess notation
 */
QString Move::getChessNotation() const {
    // Handle pawn promotion
    if (isPawnPromotion) {
        return getRankFile(endRow, endCol) + "Q";
    }

    // Handle castling
    if (isCastleMove) {
        if (endCol == 1) {
            return "O-O-O";  // Queenside castle
        } else {
            return "O-O";    // Kingside castle
        }
    }

    // Handle en passant
    if (isEnpassantMove) {
        return QString(pieceMoved[1]) + "x" + getRankFile(endRow, endCol) + " e.p.";
    }

    // Handle captures
    if (pieceCaptured != "--") {
        if (pieceMoved[1] == 'p') {  // Pawn capture
            return QString(getRankFile(startRow, startCol)[0]) + "x" + getRankFile(endRow, endCol);
        } else {  // Other piece capture
            return QString(pieceMoved[1]) + "x" + getRankFile(endRow, endCol);
        }
    } else {  // Regular move
        if (pieceMoved[1] == 'p') {  // Pawn move
            return getRankFile(endRow, endCol);
        } else {  // Other piece move
            return QString(pieceMoved[1]) + getRankFile(endRow, endCol);
        }
    }
}

/**
 * @brief Converts board coordinates to algebraic notation (e.g., "e4")
 * 
 * @param row The row index
 * @param col The column index
 * @return The square in algebraic notation
 */
QString Move::getRankFile(int row, int col) const {
    return colsToFiles[col] + rowsToRanks[row];
}

/**
 * @brief Gets a string representation of the move
 * 
 * Returns the move in a simplified form of algebraic notation.
 * 
 * @return A string representation of the move
 */
QString Move::toString() const {
    // Castling notation
    if (isCastleMove) {
        return (endCol == 6) ? "O-O" : "O-O-O";
    }

    QString endSquare = getRankFile(endRow, endCol);

    // Pawn moves
    if (pieceMoved[1] == 'p') {
        if (isCapture) {
            return colsToFiles[startCol] + "x" + endSquare;
        } else {
            return isPawnPromotion ? endSquare + "Q" : endSquare;
        }
    }

    // Piece moves
    QString moveString = QString(pieceMoved[1]);
    if (isCapture) {
        moveString += "x";
    }

    return moveString + endSquare;
}