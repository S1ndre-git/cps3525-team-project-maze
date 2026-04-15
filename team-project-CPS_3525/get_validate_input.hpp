#ifndef GET_VALIDATE_INPUT_HPP
#define GET_VALIDATE_INPUT_HPP

#include "web_types.hpp"

#include <string>

bool parse_multipart_body(const std::string &content_type,
                          const std::string &raw_body, FormInput &form);

bool validate_all_web_inputs(FormInput *form, MazeGrid *maze);

#endif
