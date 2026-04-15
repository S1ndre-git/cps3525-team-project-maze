#include "function.hpp"
#include "get_validate_input.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace {

const char *env_cstr(const char *name) {
  const char *v = std::getenv(name);
  return v ? v : "";
}

std::string read_stdin_bytes(long long len) {
  if (len <= 0) {
    return {};
  }
  std::string buf;
  try {
    buf.resize(static_cast<size_t>(len));
  } catch (...) {
    return {};
  }
  std::cin.read(&buf[0], static_cast<std::streamsize>(len));
  const std::streamsize got = std::cin.gcount();
  if (got >= 0) {
    buf.resize(static_cast<size_t>(got));
  }
  return buf;
}

void print_header() {
  std::cout << "Content-Type: text/html; charset=utf-8\r\n\r\n";
}

void print_html_shell_start() {
  std::cout << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
  std::cout << "<meta charset=\"utf-8\"/>\n";
  std::cout << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/>\n";
  std::cout << "<title>CPS3525 Maze Path CGI</title>\n";
  std::cout << "<style>\n";
  std::cout << "body{font-family:system-ui,Segoe UI,Helvetica,Arial,sans-serif;"
             "margin:24px;background:#0f172a;color:#e2e8f0;}\n";
  std::cout << "main{max-width:960px;margin:0 auto;background:#111827;"
             "padding:28px;border-radius:12px;border:1px solid #334155;}\n";
  std::cout << "h1{font-size:1.35rem;margin-top:0;color:#f8fafc;}\n";
  std::cout << ".warn{color:#fbbf24;} .ok{color:#34d399;} .bad{color:#f87171;}\n";
  std::cout << "table.maze{border-collapse:collapse;margin:12px 0;font-family:ui-monospace,monospace;}\n";
  std::cout << "table.maze td{border:1px solid #475569;padding:4px 6px;text-align:center;"
             "min-width:1.2em;background:#1e293b;}\n";
  std::cout << "span.exit{background:#14532d;padding:0 4px;border-radius:4px;}\n";
  std::cout << ".pathblock{border:1px solid #334155;border-radius:8px;padding:12px;margin:12px 0;"
             "background:#0b1220;}\n";
  std::cout << "ol{margin:6px 0 0 18px;}\n";
  std::cout << "blockquote{margin:0;padding:10px 14px;border-left:4px solid #64748b;"
             "background:#0b1220;}\n";
  std::cout << "</style>\n</head>\n<body>\n<main>\n";
}

void print_html_shell_end() {
  std::cout << "</main>\n</body>\n</html>\n";
}

void print_error_page(const std::vector<std::string> &errors) {
  print_header();
  print_html_shell_start();
  std::cout << "<h1>Validation failed</h1>\n<ul>\n";
  for (const auto &e : errors) {
    std::cout << "<li>";
    std::string esc;
    for (unsigned char ch : e) {
      if (ch == '&') {
        esc += "&amp;";
      } else if (ch == '<') {
        esc += "&lt;";
      } else if (ch == '>') {
        esc += "&gt;";
      } else {
        esc.push_back(static_cast<char>(ch));
      }
    }
    std::cout << esc << "</li>\n";
  }
  std::cout << "</ul>\n";
  std::cout << "<p>Go back, fix the input, and submit again.</p>\n";
  print_html_shell_end();
}

void print_get_stub() {
  print_header();
  print_html_shell_start();
  std::cout << "<h1>Use POST</h1>\n";
  std::cout << "<p>Submit this CGI from <code>project3.html</code> using "
             "<code>multipart/form-data</code>.</p>\n";
  print_html_shell_end();
}

}

int main() {
  std::ios::sync_with_stdio(false);
  const char *method = env_cstr("REQUEST_METHOD");
  if (std::strcmp(method, "POST") != 0) {
    print_get_stub();
    return 0;
  }
  const char *clen_str = env_cstr("CONTENT_LENGTH");
  long long content_length = std::strtoll(clen_str, nullptr, 10);
  if (content_length <= 0) {
    print_header();
    print_html_shell_start();
    std::cout << "<h1>Empty request body</h1>\n";
    print_html_shell_end();
    return 0;
  }
  const std::string raw_body = read_stdin_bytes(content_length);
  if (static_cast<long long>(raw_body.size()) != content_length) {
    print_header();
    print_html_shell_start();
    std::cout << "<h1>Incomplete request body read</h1>\n";
    print_html_shell_end();
    return 0;
  }
  const std::string content_type = env_cstr("CONTENT_TYPE");
  FormInput form;
  if (!parse_multipart_body(content_type, raw_body, form)) {
    print_error_page(form.validation_errors);
    return 0;
  }
  MazeGrid maze;
  if (!validate_all_web_inputs(&form, &maze)) {
    print_error_page(form.validation_errors);
    return 0;
  }
  MazeSolveResult result;
  solve_maze_paths(maze, form.max_paths_display, &result, form);
  print_header();
  print_html_shell_start();
  std::cout << "<h1>Maze analysis result</h1>\n";
  if (!form.team_name.empty()) {
    std::cout << "<p>Team / name: <strong>";
    std::string esc;
    for (unsigned char ch : form.team_name) {
      if (ch == '&') {
        esc += "&amp;";
      } else if (ch == '<') {
        esc += "&lt;";
      } else if (ch == '>') {
        esc += "&gt;";
      } else {
        esc.push_back(static_cast<char>(ch));
      }
    }
    std::cout << esc << "</strong></p>\n";
  }
  if (!form.report_date.empty()) {
    std::cout << "<p>Report date: " << form.report_date << "</p>\n";
  }
  if (!form.maze_filename.empty()) {
    std::cout << "<p>Maze file: <code>" << form.maze_filename << "</code></p>\n";
  }
  if (!form.extra_filename.empty()) {
    std::cout << "<p>Extra file: <code>" << form.extra_filename << "</code> ("
              << form.extra_file_text.size() << " bytes)</p>\n";
  }
  std::cout << format_paths_report_html(result, form, maze);
  print_html_shell_end();
  return 0;
}
