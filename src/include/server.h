#ifndef SERVER_H
#define SERVER_H

#include <cstdlib>
#include <iostream>

/*
 * You may need to define some global variables for the information of the game map here.
 * Although we don't encourage to use global variables in real cpp projects, you may have to use them because the use of
 * class is not taught yet. However, if you are member of A-class or have learnt the use of cpp class, member functions,
 * etc., you're free to modify this structure.
 */
int rows;         // The count of rows of the game map. You MUST NOT modify its name.
int columns;      // The count of columns of the game map. You MUST NOT modify its name.
int total_mines;  // The count of mines of the game map. You MUST NOT modify its name. You should initialize this
                  // variable in function InitMap. It will be used in the advanced task.
int game_state;  // The state of the game, 0 for continuing, 1 for winning, -1 for losing. You MUST NOT modify its name.

// Additional global variables
bool is_mine[35][35];      // Whether a grid contains a mine
bool is_visited[35][35];   // Whether a grid has been visited
bool is_marked[35][35];    // Whether a grid has been marked as mine
int mine_count[35][35];    // Number of adjacent mines for each grid

/**
 * @brief The definition of function InitMap()
 *
 * @details This function is designed to read the initial map from stdin. For example, if there is a 3 * 3 map in which
 * mines are located at (0, 1) and (1, 2) (0-based), the stdin would be
 *     3 3
 *     .X.
 *     ...
 *     ..X
 * where X stands for a mine block and . stands for a normal block. After executing this function, your game map
 * would be initialized, with all the blocks unvisited.
 */
void InitMap() {
  std::cin >> rows >> columns;

  // Initialize all arrays
  total_mines = 0;
  game_state = 0;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      is_visited[i][j] = false;
      is_marked[i][j] = false;
      mine_count[i][j] = 0;
    }
  }

  // Read the map
  for (int i = 0; i < rows; i++) {
    std::string line;
    std::cin >> line;
    for (int j = 0; j < columns; j++) {
      is_mine[i][j] = (line[j] == 'X');
      if (is_mine[i][j]) {
        total_mines++;
      }
    }
  }

  // Calculate mine counts for each grid
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (!is_mine[i][j]) {
        int count = 0;
        // Check all 8 adjacent cells
        for (int di = -1; di <= 1; di++) {
          for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;
            int ni = i + di;
            int nj = j + dj;
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && is_mine[ni][nj]) {
              count++;
            }
          }
        }
        mine_count[i][j] = count;
      }
    }
  }
}

/**
 * @brief The definition of function VisitBlock(int, int)
 *
 * @details This function is designed to visit a block in the game map. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call VisitBlock(0, 0), the return value would be 0 (game continues), and the game map would
 * be
 *     1??
 *     ???
 *     ???
 * If you call VisitBlock(0, 1) after that, the return value would be -1 (game ends and the players loses) , and the
 * game map would be
 *     1X?
 *     ???
 *     ???
 * If you call VisitBlock(0, 2), VisitBlock(2, 0), VisitBlock(1, 2) instead, the return value of the last operation
 * would be 1 (game ends and the player wins), and the game map would be
 *     1@1
 *     122
 *     01@
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
void VisitBlock(int r, int c) {
  // Check if coordinates are valid
  if (r < 0 || r >= rows || c < 0 || c >= columns) {
    return;
  }

  // If already visited or marked, do nothing
  if (is_visited[r][c] || is_marked[r][c]) {
    return;
  }

  // Mark as visited
  is_visited[r][c] = true;

  // If it's a mine, game over
  if (is_mine[r][c]) {
    game_state = -1;
    return;
  }

  // If mine count is 0, recursively visit all adjacent cells
  if (mine_count[r][c] == 0) {
    for (int di = -1; di <= 1; di++) {
      for (int dj = -1; dj <= 1; dj++) {
        if (di == 0 && dj == 0) continue;
        int ni = r + di;
        int nj = c + dj;
        if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
          VisitBlock(ni, nj);
        }
      }
    }
  }

  // Check if all non-mine grids have been visited (win condition)
  int visited_count = 0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (!is_mine[i][j] && is_visited[i][j]) {
        visited_count++;
      }
    }
  }

  if (visited_count == rows * columns - total_mines) {
    game_state = 1;
  }
}

/**
 * @brief The definition of function MarkMine(int, int)
 *
 * @details This function is designed to mark a mine in the game map.
 * If the block being marked is a mine, show it as "@".
 * If the block being marked isn't a mine, END THE GAME immediately. (NOTE: This is not the same rule as the real
 * game) And you don't need to
 *
 * For example, if we use the same map as before, and the current state is:
 *     1?1
 *     ???
 *     ???
 * If you call MarkMine(0, 1), you marked the right mine. Then the resulting game map is:
 *     1@1
 *     ???
 *     ???
 * If you call MarkMine(1, 0), you marked the wrong mine(There's no mine in grid (1, 0)).
 * The game_state would be -1 and game ends immediately. The game map would be:
 *     1?1
 *     X??
 *     ???
 * This is different from the Minesweeper you've played. You should beware of that.
 *
 * @param r The row coordinate (0-based) of the block to be marked.
 * @param c The column coordinate (0-based) of the block to be marked.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
void MarkMine(int r, int c) {
  // Check if coordinates are valid
  if (r < 0 || r >= rows || c < 0 || c >= columns) {
    return;
  }

  // If already visited or marked, do nothing
  if (is_visited[r][c] || is_marked[r][c]) {
    return;
  }

  // Mark the grid
  is_marked[r][c] = true;

  // If it's not a mine, game over
  if (!is_mine[r][c]) {
    game_state = -1;
    return;
  }

  // Check if all non-mine grids have been visited (win condition)
  int visited_count = 0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (!is_mine[i][j] && is_visited[i][j]) {
        visited_count++;
      }
    }
  }

  if (visited_count == rows * columns - total_mines) {
    game_state = 1;
  }
}

/**
 * @brief The definition of function AutoExplore(int, int)
 *
 * @details This function is designed to auto-visit adjacent blocks of a certain block.
 * See README.md for more information
 *
 * For example, if we use the same map as before, and the current map is:
 *     ?@?
 *     ?2?
 *     ??@
 * Then auto explore is available only for block (1, 1). If you call AutoExplore(1, 1), the resulting map will be:
 *     1@1
 *     122
 *     01@
 * And the game ends (and player wins).
 */
void AutoExplore(int r, int c) {
  // Check if coordinates are valid
  if (r < 0 || r >= rows || c < 0 || c >= columns) {
    return;
  }

  // Can only auto-explore visited non-mine grids
  if (!is_visited[r][c] || is_mine[r][c]) {
    return;
  }

  // Count marked mines around this grid
  int marked_count = 0;
  for (int di = -1; di <= 1; di++) {
    for (int dj = -1; dj <= 1; dj++) {
      if (di == 0 && dj == 0) continue;
      int ni = r + di;
      int nj = c + dj;
      if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && is_marked[ni][nj]) {
        marked_count++;
      }
    }
  }

  // If marked count equals mine count, visit all non-marked adjacent grids
  if (marked_count == mine_count[r][c]) {
    for (int di = -1; di <= 1; di++) {
      for (int dj = -1; dj <= 1; dj++) {
        if (di == 0 && dj == 0) continue;
        int ni = r + di;
        int nj = c + dj;
        if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && !is_marked[ni][nj]) {
          VisitBlock(ni, nj);
        }
      }
    }
  }
}

/**
 * @brief The definition of function ExitGame()
 *
 * @details This function is designed to exit the game.
 * It outputs a line according to the result, and a line of two integers, visit_count and marked_mine_count,
 * representing the number of blocks visited and the number of marked mines taken respectively.
 *
 * @note If the player wins, we consider that ALL mines are correctly marked.
 */
void ExitGame() {
  // Output result
  if (game_state == 1) {
    std::cout << "YOU WIN!" << std::endl;
  } else {
    std::cout << "GAME OVER!" << std::endl;
  }

  // Count visited non-mine grids
  int visit_count = 0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (!is_mine[i][j] && is_visited[i][j]) {
        visit_count++;
      }
    }
  }

  // Count marked mines
  int marked_mine_count = 0;
  if (game_state == 1) {
    // If win, all mines are considered correctly marked
    marked_mine_count = total_mines;
  } else {
    // If lose, count only correctly marked mines
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < columns; j++) {
        if (is_mine[i][j] && is_marked[i][j]) {
          marked_mine_count++;
        }
      }
    }
  }

  std::cout << visit_count << " " << marked_mine_count << std::endl;
  exit(0);  // Exit the game immediately
}

/**
 * @brief The definition of function PrintMap()
 *
 * @details This function is designed to print the game map to stdout. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call PrintMap(), the stdout would be
 *    ???
 *    ???
 *    ???
 * If you call VisitBlock(2, 0) and PrintMap() after that, the stdout would be
 *    ???
 *    12?
 *    01?
 * If you call VisitBlock(0, 1) and PrintMap() after that, the stdout would be
 *    ?X?
 *    12?
 *    01?
 * If the player visits all blocks without mine and call PrintMap() after that, the stdout would be
 *    1@1
 *    122
 *    01@
 * (You may find the global variable game_state useful when implementing this function.)
 *
 * @note Use std::cout to print the game map, especially when you want to try the advanced task!!!
 */
void PrintMap() {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (is_visited[i][j]) {
        // Visited grid
        if (is_mine[i][j]) {
          std::cout << 'X';
        } else {
          std::cout << mine_count[i][j];
        }
      } else if (is_marked[i][j]) {
        // Marked grid
        if (game_state == 1) {
          // If won, show all mines as @
          std::cout << '@';
        } else if (is_mine[i][j]) {
          std::cout << '@';
        } else {
          std::cout << 'X';  // Marked non-mine (causes failure)
        }
      } else {
        // Unvisited and unmarked grid
        if (game_state == 1 && is_mine[i][j]) {
          // If won, show all mines as @
          std::cout << '@';
        } else {
          std::cout << '?';
        }
      }
    }
    std::cout << std::endl;
  }
}

#endif
