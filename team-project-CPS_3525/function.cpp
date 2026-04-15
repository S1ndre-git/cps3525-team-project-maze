#include "function.hpp"

#include <algorithm>
#include <cstddef>
#include <sstream>
#include <string>

namespace {

constexpr long long kMaxPathCount = 500000LL;

void append_escaped(std::string *out, const std::string &raw) {
  for (unsigned char ch : raw) {
    if (ch == '&') {
      out->append("&amp;");
    } else if (ch == '<') {
      out->append("&lt;");
    } else if (ch == '>') {
      out->append("&gt;");
    } else if (ch == '"') {
      out->append("&quot;");
    } else {
      out->push_back(static_cast<char>(ch));
    }
  }
}

void dfs_count_only(int r, int c, const MazeGrid &grid,
                    std::vector<std::vector<bool>> *visited,
                    long long *counter, bool *capped) {
  if (*capped) {
    return;
  }
  const int rows = grid.rows;
  const int cols = grid.cols;
  auto &vis = *visited;
  if (grid.cells[static_cast<size_t>(r)][static_cast<size_t>(c)] != 'H') {
    return;
  }
  vis[static_cast<size_t>(r)][static_cast<size_t>(c)] = true;
  if (r == rows - 1) {
    if (*counter >= kMaxPathCount) {
      *capped = true;
      vis[static_cast<size_t>(r)][static_cast<size_t>(c)] = false;
      return;
    }
    ++(*counter);
  }
  static const int dr[4] = {-1, 1, 0, 0};
  static const int dc[4] = {0, 0, -1, 1};
  for (int k = 0; k < 4; ++k) {
    const int nr = r + dr[k];
    const int nc = c + dc[k];
    if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) {
      continue;
    }
    if (vis[static_cast<size_t>(nr)][static_cast<size_t>(nc)]) {
      continue;
    }
    if (grid.cells[static_cast<size_t>(nr)][static_cast<size_t>(nc)] != 'H') {
      continue;
    }
    dfs_count_only(nr, nc, grid, visited, counter, capped);
    if (*capped) {
      break;
    }
  }
  vis[static_cast<size_t>(r)][static_cast<size_t>(c)] = false;
}

void dfs_collect_paths(
    int r, int c, const MazeGrid &grid, std::vector<std::vector<bool>> *visited,
    std::vector<MazeStep> *current, std::vector<std::vector<MazeStep>> *out,
    int max_display, long long *paths_emitted, bool *truncated) {
  const int rows = grid.rows;
  const int cols = grid.cols;
  auto &vis = *visited;
  if (grid.cells[static_cast<size_t>(r)][static_cast<size_t>(c)] != 'H') {
    return;
  }
  MazeStep step;
  step.row = r;
  step.col = c;
  current->push_back(step);
  vis[static_cast<size_t>(r)][static_cast<size_t>(c)] = true;
  if (r == rows - 1) {
    if (static_cast<int>(out->size()) < max_display) {
      out->push_back(*current);
    } else {
      *truncated = true;
    }
    ++(*paths_emitted);
  }
  static const int dr[4] = {-1, 1, 0, 0};
  static const int dc[4] = {0, 0, -1, 1};
  for (int k = 0; k < 4; ++k) {
    const int nr = r + dr[k];
    const int nc = c + dc[k];
    if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) {
      continue;
    }
    if (vis[static_cast<size_t>(nr)][static_cast<size_t>(nc)]) {
      continue;
    }
    if (grid.cells[static_cast<size_t>(nr)][static_cast<size_t>(nc)] != 'H') {
      continue;
    }
    dfs_collect_paths(nr, nc, grid, visited, current, out, max_display,
                      paths_emitted, truncated);
  }
  vis[static_cast<size_t>(r)][static_cast<size_t>(c)] = false;
  current->pop_back();
}

void count_paths_from_entrances(const MazeGrid &grid, long long *total,
                                bool *capped) {
  *total = 0;
  *capped = false;
  const int cols = grid.cols;
  std::vector<std::vector<bool>> visited(
      static_cast<size_t>(grid.rows),
      std::vector<bool>(static_cast<size_t>(grid.cols), false));
  for (int c = 0; c < cols; ++c) {
    if (grid.cells[0][static_cast<size_t>(c)] != 'H') {
      continue;
    }
    dfs_count_only(0, c, grid, &visited, total, capped);
    if (*capped) {
      break;
    }
  }
}

void collect_paths_from_entrances(const MazeGrid &grid, int max_display,
                                  std::vector<std::vector<MazeStep>> *paths,
                                  long long *emitted, bool *truncated) {
  paths->clear();
  *emitted = 0;
  *truncated = false;
  const int cols = grid.cols;
  std::vector<std::vector<bool>> visited(
      static_cast<size_t>(grid.rows),
      std::vector<bool>(static_cast<size_t>(grid.cols), false));
  std::vector<MazeStep> current;
  current.reserve(static_cast<size_t>(grid.rows * grid.cols));
  for (int c = 0; c < cols; ++c) {
    if (grid.cells[0][static_cast<size_t>(c)] != 'H') {
      continue;
    }
    dfs_collect_paths(0, c, grid, &visited, &current, paths, max_display,
                      emitted, truncated);
  }
}

void evaluate_guess(const FormInput &form, long long counted, bool count_capped,
                    bool *evaluated, bool *correct) {
  *evaluated = false;
  *correct = false;
  if (!form.path_guess_set) {
    return;
  }
  *evaluated = true;
  if (count_capped) {
    *correct = false;
    return;
  }
  *correct = (form.path_guess == counted);
}

std::string step_label(const MazeStep &s, int cols, const std::string &style) {
  std::ostringstream oss;
  if (style == "linear") {
    const int idx = s.row * cols + s.col;
    oss << idx;
  } else {
    oss << "(" << s.row << "," << s.col << ")";
  }
  return oss.str();
}

}

void solve_maze_paths(const MazeGrid &grid, int max_display, MazeSolveResult *out,
                      const FormInput &form) {
  long long total = 0;
  bool count_capped = false;
  count_paths_from_entrances(grid, &total, &count_capped);
  out->paths_counted = total;
  out->count_capped = count_capped;
  out->truncated_display = false;
  out->stored_paths.clear();
  long long emitted = 0;
  bool trunc = false;
  const int cap = std::max(1, max_display);
  if (form.verbosity != "summary") {
    collect_paths_from_entrances(grid, cap, &out->stored_paths, &emitted,
                                 &trunc);
    out->truncated_display = trunc;
  }
  evaluate_guess(form, total, count_capped, &out->guess_evaluated,
                 &out->guess_correct);
}

std::string format_paths_report_html(const MazeSolveResult &res,
                                     const FormInput &form,
                                     const MazeGrid &grid) {
  std::ostringstream html;
  html << "<section class=\"report\">\n";
  html << "<h2>Maze path report</h2>\n";
  html << "<p>Grid size: " << grid.rows << " rows &times; " << grid.cols
       << " columns.</p>\n";
  if (res.count_capped) {
    html << "<p class=\"warn\">Path count reached the internal limit (" << kMaxPathCount
         << "). Exact total is unknown; your path-count guess cannot be judged.</p>\n";
  } else {
    html << "<p>Total valid paths from top entrances to bottom exits: <strong>"
         << res.paths_counted << "</strong>.</p>\n";
  }
  if (res.guess_evaluated) {
    if (res.guess_correct) {
      html << "<p class=\"ok\">Your path-count guess is <strong>correct</strong>.</p>\n";
    } else {
      html << "<p class=\"bad\">Your path-count guess is <strong>incorrect</strong>.</p>\n";
    }
  } else {
    html << "<p>No path-count guess was provided; skipping comparison.</p>\n";
  }
  if (form.include_grid) {
    html << "<h3>Maze grid (H = path, B = background)</h3>\n<table class=\"maze\">\n";
    for (int r = 0; r < grid.rows; ++r) {
      html << "<tr>";
      for (int c = 0; c < grid.cols; ++c) {
        char ch = grid.cells[static_cast<size_t>(r)][static_cast<size_t>(c)];
        std::string cell(1, ch);
        if (form.mark_exit_row && r == grid.rows - 1 && ch == 'H') {
          cell = "<span class=\"exit\">H</span>";
        }
        html << "<td>" << cell << "</td>";
      }
      html << "</tr>\n";
    }
    html << "</table>\n";
  }
  if (form.verbosity == "summary") {
    html << "<p>Summary mode: step coordinates are not listed.</p>\n";
    html << "</section>\n";
    return html.str();
  }
  if (form.verbosity == "paths") {
    html << "<h3>Step-by-step H cell coordinates per path</h3>\n";
  } else {
    html << "<h3>Step-by-step H cell coordinates per path (full)</h3>\n";
  }
  if (res.stored_paths.empty()) {
    html << "<p>No paths stored (maze may be unreachable or display limit is 0).</p>\n";
  } else {
    int idx = 0;
    for (const auto &path : res.stored_paths) {
      ++idx;
      html << "<div class=\"pathblock\"><h4>Path #" << idx << " (steps "
           << path.size() << ")</h4><ol>";
      int step_no = 0;
      for (const auto &st : path) {
        ++step_no;
        html << "<li>Step " << step_no << ": H @ "
             << step_label(st, grid.cols, form.coord_style) << "</li>\n";
      }
      html << "</ol></div>\n";
    }
    if (res.truncated_display) {
      html << "<p class=\"warn\">Showing only the first " << res.stored_paths.size()
           << " paths; additional paths were omitted.</p>\n";
    }
  }
  if (!form.notes.empty()) {
    html << "<h3>Notes (your submitted text)</h3><blockquote>";
    std::string esc;
    append_escaped(&esc, form.notes);
    html << esc;
    html << "</blockquote>\n";
  }
  html << "</section>\n";
  return html.str();
}
