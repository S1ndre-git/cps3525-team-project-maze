#include "web_types.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

namespace {

std::string trim_copy(const std::string &s) {
  size_t i = 0;
  while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) {
    ++i;
  }
  size_t j = s.size();
  while (j > i && std::isspace(static_cast<unsigned char>(s[j - 1]))) {
    --j;
  }
  return s.substr(i, j - i);
}

bool extract_boundary(const std::string &content_type, std::string &out_boundary) {
  const std::string key = "boundary=";
  auto pos = content_type.find(key);
  if (pos == std::string::npos) {
    return false;
  }
  pos += key.size();
  while (pos < content_type.size() &&
         std::isspace(static_cast<unsigned char>(content_type[pos]))) {
    ++pos;
  }
  if (pos < content_type.size() && content_type[pos] == '"') {
    ++pos;
    auto end = content_type.find('"', pos);
    if (end == std::string::npos) {
      return false;
    }
    out_boundary = content_type.substr(pos, end - pos);
    return !out_boundary.empty();
  }
  auto semi = content_type.find(';', pos);
  if (semi == std::string::npos) {
    out_boundary = trim_copy(content_type.substr(pos));
  } else {
    out_boundary = trim_copy(content_type.substr(pos, semi - pos));
  }
  return !out_boundary.empty();
}

std::string header_value(const std::string &line, const char *prefix) {
  const size_t len = std::strlen(prefix);
  if (line.size() < len) {
    return {};
  }
  if (line.compare(0, len, prefix) != 0) {
    return {};
  }
  return trim_copy(line.substr(len));
}

void parse_disposition(const std::string &disp, std::string &name,
                       std::string &filename) {
  name.clear();
  filename.clear();
  std::istringstream ss(disp);
  std::string segment;
  while (std::getline(ss, segment, ';')) {
    segment = trim_copy(segment);
    if (segment.empty()) {
      continue;
    }
    const size_t eq = segment.find('=');
    if (eq == std::string::npos) {
      continue;
    }
    std::string key = trim_copy(segment.substr(0, eq));
    std::string val = trim_copy(segment.substr(eq + 1));
    if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
      val = val.substr(1, val.size() - 2);
    }
    if (key == "name") {
      name = val;
    } else if (key == "filename") {
      filename = val;
    }
  }
}

void assign_field(FormInput &form, const std::string &name,
                  const std::string &value, const std::string &filename,
                  const std::string &body) {
  if (name == "maze_file") {
    if (!filename.empty()) {
      form.maze_filename = filename;
    }
    form.maze_text = body;
    return;
  }
  if (name == "extra_file") {
    if (!filename.empty()) {
      form.extra_filename = filename;
    }
    form.extra_file_text = body;
    return;
  }
  if (name == "team_name") {
    form.team_name = value;
    return;
  }
  if (name == "path_guess") {
    form.path_guess_str = value;
    return;
  }
  if (name == "verbosity") {
    form.verbosity = value;
    return;
  }
  if (name == "coord_style") {
    form.coord_style = value;
    return;
  }
  if (name == "include_grid") {
    form.include_grid = (value == "on" || value == "1" || value == "yes");
    return;
  }
  if (name == "mark_exit") {
    form.mark_exit_row = (value == "on" || value == "1" || value == "yes");
    return;
  }
  if (name == "report_date") {
    form.report_date = value;
    return;
  }
  if (name == "max_paths") {
    form.max_paths_display = static_cast<int>(std::strtol(value.c_str(), nullptr, 10));
    return;
  }
  if (name == "notes") {
    form.notes = value;
    return;
  }
}

}

bool parse_multipart_body(const std::string &content_type,
                          const std::string &raw_body, FormInput &form) {
  std::string boundary;
  if (!extract_boundary(content_type, boundary)) {
    form.validation_errors.push_back("Invalid or missing multipart boundary.");
    return false;
  }
  const std::string marker = std::string("--") + boundary;
  size_t pos = 0;
  while (pos < raw_body.size()) {
    if (raw_body.compare(pos, marker.size(), marker) != 0) {
      ++pos;
      continue;
    }
    pos += marker.size();
    if (pos + 1 < raw_body.size() && raw_body[pos] == '-' && raw_body[pos + 1] == '-') {
      break;
    }
    if (pos < raw_body.size() && (raw_body[pos] == '\r' || raw_body[pos] == '\n')) {
      if (raw_body[pos] == '\r') {
        ++pos;
      }
      if (pos < raw_body.size() && raw_body[pos] == '\n') {
        ++pos;
      }
    }
    const size_t header_end = raw_body.find("\r\n\r\n", pos);
    if (header_end == std::string::npos) {
      break;
    }
    std::string headers = raw_body.substr(pos, header_end - pos);
    size_t body_start = header_end + 4;
    size_t next = raw_body.find(marker, body_start);
    if (next == std::string::npos) {
      next = raw_body.size();
    }
    std::string part_body = raw_body.substr(body_start, next - body_start);
    if (part_body.size() >= 2 && part_body.compare(part_body.size() - 2, 2, "\r\n") == 0) {
      part_body.resize(part_body.size() - 2);
    } else if (!part_body.empty() && part_body.back() == '\n') {
      part_body.pop_back();
      if (!part_body.empty() && part_body.back() == '\r') {
        part_body.pop_back();
      }
    }
    std::string disp_line;
    {
      std::istringstream hs(headers);
      std::string line;
      while (std::getline(hs, line)) {
        if (!line.empty() && line.back() == '\r') {
          line.pop_back();
        }
        const std::string cl = header_value(line, "Content-Disposition:");
        if (!cl.empty()) {
          disp_line = cl;
          break;
        }
      }
    }
    std::string fname;
    std::string pname;
    if (!disp_line.empty()) {
      parse_disposition(disp_line, pname, fname);
    }
    if (!pname.empty()) {
      assign_field(form, pname, part_body, fname, part_body);
    }
    pos = next;
  }
  return true;
}

static bool parse_dimensions_line(const std::string &line, int *rows, int *cols) {
  std::istringstream iss(line);
  if (!(iss >> *rows >> *cols)) {
    return false;
  }
  return *rows > 0 && *cols > 0 && *rows <= 5000 && *cols <= 5000;
}

static bool fill_grid_from_text(const std::string &text, int rows, int cols,
                                std::vector<std::vector<char>> *out_cells) {
  out_cells->assign(static_cast<size_t>(rows),
                    std::vector<char>(static_cast<size_t>(cols), 'B'));
  std::istringstream iss(text);
  std::string token;
  int r = 0;
  int c = 0;
  while (iss >> token) {
    if (token.size() == 1) {
      char ch = static_cast<char>(std::toupper(static_cast<unsigned char>(token[0])));
      if (ch != 'B' && ch != 'H') {
        return false;
      }
      if (r >= rows) {
        return false;
      }
      (*out_cells)[static_cast<size_t>(r)][static_cast<size_t>(c)] = ch;
      ++c;
      if (c == cols) {
        c = 0;
        ++r;
      }
      continue;
    }
    for (char ch0 : token) {
      char ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch0)));
      if (ch != 'B' && ch != 'H') {
        return false;
      }
      if (r >= rows) {
        return false;
      }
      (*out_cells)[static_cast<size_t>(r)][static_cast<size_t>(c)] = ch;
      ++c;
      if (c == cols) {
        c = 0;
        ++r;
      }
    }
  }
  return r == rows && c == 0;
}

bool parse_and_validate_maze_text(const std::string &raw, MazeGrid *out,
                                  std::vector<std::string> *errors) {
  errors->clear();
  std::istringstream stream(raw);
  std::string first;
  if (!std::getline(stream, first)) {
    errors->push_back("Maze file is empty.");
    return false;
  }
  if (!first.empty() && first.back() == '\r') {
    first.pop_back();
  }
  int rows = 0;
  int cols = 0;
  if (!parse_dimensions_line(first, &rows, &cols)) {
    errors->push_back("First line must be two positive integers: rows cols.");
    return false;
  }
  std::string rest;
  {
    std::ostringstream oss;
    oss << stream.rdbuf();
    rest = oss.str();
  }
  if (!fill_grid_from_text(rest, rows, cols, &out->cells)) {
    errors->push_back(
        "Grid data must contain exactly rows*cols cells using only B or H.");
    return false;
  }
  bool top_h = false;
  bool bot_h = false;
  for (int c = 0; c < cols; ++c) {
    if (out->cells[0][static_cast<size_t>(c)] == 'H') {
      top_h = true;
    }
    if (out->cells[static_cast<size_t>(rows - 1)][static_cast<size_t>(c)] == 'H') {
      bot_h = true;
    }
  }
  if (!top_h) {
    errors->push_back("Maze must have at least one H on the top row (entrance).");
  }
  if (!bot_h) {
    errors->push_back("Maze must have at least one H on the bottom row (exit).");
  }
  if (!errors->empty()) {
    return false;
  }
  out->rows = rows;
  out->cols = cols;
  return true;
}

void normalize_form_defaults(FormInput *form) {
  if (form->verbosity != "full" && form->verbosity != "summary" &&
      form->verbosity != "paths") {
    form->verbosity = "full";
  }
  if (form->coord_style != "pair" && form->coord_style != "linear") {
    form->coord_style = "pair";
  }
  if (form->max_paths_display < 1) {
    form->max_paths_display = 50;
  }
  if (form->max_paths_display > 20000) {
    form->max_paths_display = 20000;
  }
  if (!form->path_guess_str.empty()) {
    char *end = nullptr;
    long long v = std::strtoll(form->path_guess_str.c_str(), &end, 10);
    if (end != form->path_guess_str.c_str() && *end == '\0') {
      form->path_guess = v;
      form->path_guess_set = true;
    }
  }
}

bool validate_all_web_inputs(FormInput *form, MazeGrid *maze) {
  form->validation_errors.clear();
  if (form->maze_text.empty()) {
    form->validation_errors.push_back("Please upload a maze text file.");
    return false;
  }
  if (!parse_and_validate_maze_text(form->maze_text, maze, &form->validation_errors)) {
    return false;
  }
  normalize_form_defaults(form);
  return true;
}
