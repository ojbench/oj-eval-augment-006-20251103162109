#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>
#include <queue>
#include <set>
#include <algorithm>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

// Global variables for client
char map[35][35];           // Current state of the map
bool is_mine_certain[35][35];   // Cells we know are mines
bool is_safe_certain[35][35];   // Cells we know are safe

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  // Initialize all global variables
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      map[i][j] = '?';
      is_mine_certain[i][j] = false;
      is_safe_certain[i][j] = false;
    }
  }

  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  for (int i = 0; i < rows; i++) {
    std::string line;
    std::cin >> line;
    for (int j = 0; j < columns; j++) {
      map[i][j] = line[j];
    }
  }

  // Debug: print the map to stderr
  // std::cerr << "Current map:" << std::endl;
  // for (int i = 0; i < rows; i++) {
  //   for (int j = 0; j < columns; j++) {
  //     std::cerr << map[i][j];
  //   }
  //   std::cerr << std::endl;
  // }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  // std::cerr << "Decide() called" << std::endl;

  // Strategy:
  // 1. Try to find cells that are definitely safe or definitely mines
  // 2. Mark certain mines
  // 3. Visit certain safe cells
  // 4. Use auto-explore when possible
  // 5. If no certain moves, make a guess

  // First, analyze the map to find certain safe/mine cells
  bool found_certain = false;

  // Check each revealed number cell
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (map[i][j] >= '0' && map[i][j] <= '8') {
        int num = map[i][j] - '0';

        // Count adjacent unknown and marked cells
        int unknown_count = 0;
        int marked_count = 0;
        std::vector<std::pair<int, int>> unknown_cells;

        for (int di = -1; di <= 1; di++) {
          for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;
            int ni = i + di;
            int nj = j + dj;
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
              if (map[ni][nj] == '?') {
                unknown_count++;
                unknown_cells.push_back({ni, nj});
              } else if (map[ni][nj] == '@') {
                marked_count++;
              }
            }
          }
        }

        // If all remaining unknown cells must be mines
        if (unknown_count > 0 && marked_count + unknown_count == num) {
          // std::cerr << "Found certain mines at (" << i << ", " << j << ") with num=" << num << std::endl;
          for (auto& cell : unknown_cells) {
            if (!is_mine_certain[cell.first][cell.second]) {
              is_mine_certain[cell.first][cell.second] = true;
              // std::cerr << "Marking mine at (" << cell.first << ", " << cell.second << ")" << std::endl;
              Execute(cell.first, cell.second, 1);  // Mark mine
              return;
            }
          }
        }

        // If all mines are already marked, remaining cells are safe
        if (unknown_count > 0 && marked_count == num) {
          // std::cerr << "Found certain safe cells at (" << i << ", " << j << ") with num=" << num << std::endl;
          for (auto& cell : unknown_cells) {
            if (!is_safe_certain[cell.first][cell.second]) {
              is_safe_certain[cell.first][cell.second] = true;
              // std::cerr << "Visiting safe cell at (" << cell.first << ", " << cell.second << ")" << std::endl;
              Execute(cell.first, cell.second, 0);  // Visit safe cell
              return;
            }
          }
        }

        // Try auto-explore if all adjacent mines are marked
        // Only auto-explore if there are unvisited, unmarked cells around
        if (marked_count == num) {
          // Check if there are any unvisited, unmarked cells around
          bool has_unvisited_unmarked = false;
          for (int di = -1; di <= 1; di++) {
            for (int dj = -1; dj <= 1; dj++) {
              if (di == 0 && dj == 0) continue;
              int ni = i + di;
              int nj = j + dj;
              if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
                if (map[ni][nj] == '?') {
                  has_unvisited_unmarked = true;
                  break;
                }
              }
            }
            if (has_unvisited_unmarked) break;
          }

          if (has_unvisited_unmarked) {
            // std::cerr << "Auto-exploring at (" << i << ", " << j << ")" << std::endl;
            Execute(i, j, 2);  // Auto-explore
            return;
          }
        }
      }
    }
  }

  // If no certain moves found, make a guess
  // Prefer cells with lower risk (adjacent to lower numbers)
  int best_r = -1, best_c = -1;
  int best_score = -1000000;

  // std::cerr << "Looking for guesses..." << std::endl;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (map[i][j] == '?') {
        // std::cerr << "Found ? at (" << i << ", " << j << ")" << std::endl;
        // Calculate a heuristic score for this cell
        int score = 0;
        int revealed_neighbors = 0;

        for (int di = -1; di <= 1; di++) {
          for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;
            int ni = i + di;
            int nj = j + dj;
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
              if (map[ni][nj] >= '0' && map[ni][nj] <= '8') {
                revealed_neighbors++;
                // Prefer cells adjacent to 0s (safest)
                score += (8 - (map[ni][nj] - '0')) * 10;
              }
            }
          }
        }

        // Prefer cells with more revealed neighbors
        score += revealed_neighbors * 5;

        // If no revealed neighbors, just pick any cell
        if (revealed_neighbors == 0) {
          score = 1;
        }

        if (score > best_score) {
          best_score = score;
          best_r = i;
          best_c = j;
        }
      }
    }
  }

  if (best_r != -1) {
    // std::cerr << "Making guess at (" << best_r << ", " << best_c << ") with score " << best_score << std::endl;
    Execute(best_r, best_c, 0);  // Visit the best guess
    return;
  }

  // std::cerr << "No moves found! Game should be over." << std::endl;
  // If we reach here, the game should be over (no more moves)
}

#endif