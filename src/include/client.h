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
char client_map[35][35];           // Current state of the map
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
      client_map[i][j] = '?';
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
      client_map[i][j] = line[j];
    }
  }

  // Debug: print the map to stderr
  // std::cerr << "Current map:" << std::endl;
  // for (int i = 0; i < rows; i++) {
  //   for (int j = 0; j < columns; j++) {
  //     std::cerr << client_map[i][j];
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
// Helper function to get neighbors
std::vector<std::pair<int, int>> get_neighbors(int r, int c) {
  std::vector<std::pair<int, int>> neighbors;
  for (int di = -1; di <= 1; di++) {
    for (int dj = -1; dj <= 1; dj++) {
      if (di == 0 && dj == 0) continue;
      int ni = r + di;
      int nj = c + dj;
      if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
        neighbors.push_back({ni, nj});
      }
    }
  }
  return neighbors;
}

void Decide() {
  // Strategy:
  // 1. Try to find cells that are definitely safe or definitely mines using basic rules
  // 2. Try advanced constraint satisfaction
  // 3. Mark certain mines
  // 4. Visit certain safe cells
  // 5. Use auto-explore when possible
  // 6. If no certain moves, make a guess

  // First pass: basic rules
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] >= '0' && client_map[i][j] <= '8') {
        int num = client_map[i][j] - '0';

        // Count adjacent unknown and marked cells
        int unknown_count = 0;
        int marked_count = 0;
        std::vector<std::pair<int, int>> unknown_cells;

        auto neighbors = get_neighbors(i, j);
        for (auto& n : neighbors) {
          if (client_map[n.first][n.second] == '?') {
            unknown_count++;
            unknown_cells.push_back(n);
          } else if (client_map[n.first][n.second] == '@') {
            marked_count++;
          }
        }

        // If all remaining unknown cells must be mines
        if (unknown_count > 0 && marked_count + unknown_count == num) {
          for (auto& cell : unknown_cells) {
            if (!is_mine_certain[cell.first][cell.second]) {
              is_mine_certain[cell.first][cell.second] = true;
              Execute(cell.first, cell.second, 1);  // Mark mine
              return;
            }
          }
        }

        // If all mines are already marked, remaining cells are safe
        if (unknown_count > 0 && marked_count == num) {
          for (auto& cell : unknown_cells) {
            if (!is_safe_certain[cell.first][cell.second]) {
              is_safe_certain[cell.first][cell.second] = true;
              Execute(cell.first, cell.second, 0);  // Visit safe cell
              return;
            }
          }
        }

        // Try auto-explore if all adjacent mines are marked
        if (marked_count == num) {
          bool has_unvisited_unmarked = false;
          for (auto& n : neighbors) {
            if (client_map[n.first][n.second] == '?') {
              has_unvisited_unmarked = true;
              break;
            }
          }

          if (has_unvisited_unmarked) {
            Execute(i, j, 2);  // Auto-explore
            return;
          }
        }
      }
    }
  }

  // Second pass: advanced constraint satisfaction
  // Try to find patterns where two cells share constraints
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] >= '0' && client_map[i][j] <= '8') {
        int num1 = client_map[i][j] - '0';
        auto neighbors1 = get_neighbors(i, j);

        std::set<std::pair<int, int>> unknown1;
        int marked1 = 0;
        for (auto& n : neighbors1) {
          if (client_map[n.first][n.second] == '?') {
            unknown1.insert(n);
          } else if (client_map[n.first][n.second] == '@') {
            marked1++;
          }
        }

        int remaining1 = num1 - marked1;
        if (unknown1.empty()) continue;

        // Check other revealed cells nearby
        for (int i2 = 0; i2 < rows; i2++) {
          for (int j2 = 0; j2 < columns; j2++) {
            if (i == i2 && j == j2) continue;
            if (client_map[i2][j2] >= '0' && client_map[i2][j2] <= '8') {
              int num2 = client_map[i2][j2] - '0';
              auto neighbors2 = get_neighbors(i2, j2);

              std::set<std::pair<int, int>> unknown2;
              int marked2 = 0;
              for (auto& n : neighbors2) {
                if (client_map[n.first][n.second] == '?') {
                  unknown2.insert(n);
                } else if (client_map[n.first][n.second] == '@') {
                  marked2++;
                }
              }

              int remaining2 = num2 - marked2;
              if (unknown2.empty()) continue;

              // Pattern 1: If unknown1 is a subset of unknown2
              bool is_subset = true;
              for (auto& u : unknown1) {
                if (unknown2.find(u) == unknown2.end()) {
                  is_subset = false;
                  break;
                }
              }

              if (is_subset && unknown1.size() < unknown2.size()) {
                // unknown2 - unknown1 has (remaining2 - remaining1) mines
                std::set<std::pair<int, int>> diff;
                for (auto& u : unknown2) {
                  if (unknown1.find(u) == unknown1.end()) {
                    diff.insert(u);
                  }
                }

                int diff_mines = remaining2 - remaining1;

                // If all cells in diff must be mines
                if (diff_mines == (int)diff.size() && diff_mines > 0) {
                  for (auto& cell : diff) {
                    if (!is_mine_certain[cell.first][cell.second]) {
                      is_mine_certain[cell.first][cell.second] = true;
                      Execute(cell.first, cell.second, 1);
                      return;
                    }
                  }
                }

                // If all cells in diff must be safe
                if (diff_mines == 0 && diff.size() > 0) {
                  for (auto& cell : diff) {
                    if (!is_safe_certain[cell.first][cell.second]) {
                      is_safe_certain[cell.first][cell.second] = true;
                      Execute(cell.first, cell.second, 0);
                      return;
                    }
                  }
                }
              }

              // Pattern 2: Check for overlapping constraints
              std::set<std::pair<int, int>> intersection;
              std::set<std::pair<int, int>> only_in_1;
              std::set<std::pair<int, int>> only_in_2;

              for (auto& u : unknown1) {
                if (unknown2.find(u) != unknown2.end()) {
                  intersection.insert(u);
                } else {
                  only_in_1.insert(u);
                }
              }

              for (auto& u : unknown2) {
                if (unknown1.find(u) == unknown1.end()) {
                  only_in_2.insert(u);
                }
              }

              // If we have overlapping cells, try to deduce
              if (!intersection.empty() && !only_in_1.empty() && !only_in_2.empty()) {
                // If remaining1 - remaining2 == only_in_1.size(), all in only_in_1 are mines
                if (remaining1 - remaining2 == (int)only_in_1.size() && remaining1 > remaining2) {
                  for (auto& cell : only_in_1) {
                    if (!is_mine_certain[cell.first][cell.second]) {
                      is_mine_certain[cell.first][cell.second] = true;
                      Execute(cell.first, cell.second, 1);
                      return;
                    }
                  }
                }

                // If remaining2 - remaining1 == only_in_2.size(), all in only_in_2 are mines
                if (remaining2 - remaining1 == (int)only_in_2.size() && remaining2 > remaining1) {
                  for (auto& cell : only_in_2) {
                    if (!is_mine_certain[cell.first][cell.second]) {
                      is_mine_certain[cell.first][cell.second] = true;
                      Execute(cell.first, cell.second, 1);
                      return;
                    }
                  }
                }

                // If remaining1 == remaining2, then only_in_1 and only_in_2 have same number of mines
                if (remaining1 == remaining2) {
                  // If only_in_1 has 0 mines, all are safe
                  if (only_in_1.size() == 0 && only_in_2.size() > 0) {
                    // This means all mines are in intersection, so only_in_2 are safe
                    for (auto& cell : only_in_2) {
                      if (!is_safe_certain[cell.first][cell.second]) {
                        is_safe_certain[cell.first][cell.second] = true;
                        Execute(cell.first, cell.second, 0);
                        return;
                      }
                    }
                  }
                  if (only_in_2.size() == 0 && only_in_1.size() > 0) {
                    for (auto& cell : only_in_1) {
                      if (!is_safe_certain[cell.first][cell.second]) {
                        is_safe_certain[cell.first][cell.second] = true;
                        Execute(cell.first, cell.second, 0);
                        return;
                      }
                    }
                  }
                }
              }
            }
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
      if (client_map[i][j] == '?') {
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
              if (client_map[ni][nj] >= '0' && client_map[ni][nj] <= '8') {
                revealed_neighbors++;
                // Prefer cells adjacent to 0s (safest)
                score += (8 - (client_map[ni][nj] - '0')) * 10;
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