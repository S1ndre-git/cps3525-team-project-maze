#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include "web_types.hpp"

#include <string>
#include <vector>

struct MazeStep {
  int row;
  int col;
};

struct MazeSolveResult {
  long long paths_counted = 0;
  bool count_capped = false;
  bool truncated_display = false;
  std::vector<std::vector<MazeStep>> stored_paths;
  bool guess_evaluated = false;
  bool guess_correct = false;
};

void solve_maze_paths(const MazeGrid &grid, int max_display,
                      MazeSolveResult *out, const FormInput &form);

std::string format_paths_report_html(const MazeSolveResult &res,
                                     const FormInput &form,
                                     const MazeGrid &grid);

#endif
