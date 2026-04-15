#ifndef WEB_TYPES_HPP
#define WEB_TYPES_HPP

#include <cstddef>
#include <string>
#include <vector>

struct FormInput {
  std::string maze_text;
  std::string maze_filename;
  std::string extra_file_text;
  std::string extra_filename;
  std::string team_name;
  std::string path_guess_str;
  long long path_guess = 0;
  bool path_guess_set = false;
  std::string verbosity;
  std::string coord_style;
  bool include_grid = false;
  bool mark_exit_row = false;
  std::string report_date;
  int max_paths_display = 50;
  std::string notes;
  std::vector<std::string> validation_errors;
};

struct MazeGrid {
  int rows = 0;
  int cols = 0;
  std::vector<std::vector<char>> cells;
};

#endif
