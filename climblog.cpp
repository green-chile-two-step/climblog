#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace cl {

enum climb_type : int {
  BOULDER,
  SPORT,
  TOP_ROPE,
  TRAD
};

enum climb_style : int {
  LEAD,
  TR,
  SOLO
};

enum climb_performance : int {
  FELL,
  FLASH,
  HUNG,
  ONSIGHT,
  REDPOINT
};

struct attempt {
  std::string comments;
  climb_style style;
  climb_performance perfomance;
  int num_takes;
};

struct climb {
  std::string name;
  std::string location;
  climb_type type;
  int grade_loc;
  std::vector<attempt> attempts;
};

static std::vector<climb> my_climbs;

static const std::vector<std::string> valid_inputs = {
  "q",
  "quit",
  "h",
  "help",
  "add climb",
  "remove climb",
  "print"
};

static const std::vector<std::string> valid_v_grades = {
  "VB",
  "V0-", "V0", "V0+",
  "V1-", "V1", "V1+",
  "V2-", "V2", "V2+",
  "V3-", "V3", "V3+",
  "V4-", "V4", "V4+",
  "V5-", "V5", "V5+",
  "V6-", "V6", "V6+",
  "V7-", "V7", "V7+",
  "V8-", "V8", "V8+",
  "V9-", "V9", "V9+",
  "V10-", "V10", "V10+",
  "V11-", "V11", "V11+",
  "V12-", "V12", "V12+",
  "V13-", "V13", "V13+",
  "V14-", "V14", "V14+",
  "V15-", "V15", "V15+",
  "V16-", "V16", "V16+",
  "V17-", "V17", "V17+"
};

static const std::vector<std::string> valid_yds_grades = {
  "5.4",
  "5.5",
  "5.6",
  "5.7-", "5.7", "5.7+",
  "5.8-", "5.8", "5.8+",
  "5.9-", "5.9", "5.9+",
  "5.10-", "5.10a", "5.10b", "5.10c", "5.10d", "5.10+",
  "5.11-", "5.11a", "5.11b", "5.11c", "5.11d", "5.11+",
  "5.12-", "5.12a", "5.12b", "5.12c", "5.12d", "5.12+",
  "5.13-", "5.13a", "5.13b", "5.13c", "5.13d", "5.13+",
  "5.14-", "5.14a", "5.14b", "5.14c", "5.14d", "5.14+",
  "5.15-", "5.15a", "5.15b", "5.15c", "5.15d", "5.15+"
};

static void print_help() {
  std::cout << "valid commands\n";
  std::cout << "  > q             [quit the application]\n";
  std::cout << "  > quit          [quit the application]\n";
  std::cout << "  > h             [prompt this help message]\n";
  std::cout << "  > help          [prompt this help message]\n";
  std::cout << "  > add climb     [add a climb]\n";
  std::cout << "  > remove climb  [remove a climb]\n";
  std::cout << "  > print         [print all climbs]\n";
}

static void print_invalid_input(std::string const& input) {
  std::cout << "invalid input: `" << input << "`\n";
}

static void print_climb_exists(climb const& c) {
  std::cout << "climb: `" << c.name << "` at `"
    << c.location << "` already exists\n";
}

static void print_climb_not_found(climb const& c) {
  std::cout << "climb: `" << c.name << "` at`"
    << c.location << "` not found\n";
}

static bool is_input_valid(std::string const& input) {
  auto start = valid_inputs.begin();
  auto end = valid_inputs.end();
  if (std::find(start, end, input) != end) return true;
  else return false;
}

bool is_equal(std::string const& a, std::string const& b) {
  return std::equal(a.begin(), a.end(), b.begin(), b.end(),
      [] (char a, char b) { return std::tolower(a) == std::tolower(b); });
}

static void trim_left(std::string& s) {
  auto f = std::find_if(s.begin(), s.end(),
      [] (char c) { return !std::isspace(c); });
  s.erase(s.begin(), f);
}

static void trim_right(std::string& s) {
  auto f = std::find_if(s.rbegin(), s.rend(),
       [] (char c) { return !std::isspace(c); });
  s.erase(f.base(), s.end());
}

static void trim(std::string& s) {
  trim_left(s);
  trim_right(s);
}

static std::string to_string(climb_type const& type) {
  std::string s;
  if (type == BOULDER) s = "BOULDER";
  if (type == SPORT) s = "SPORT";
  if (type == TOP_ROPE) s = "TOP ROPE";
  if (type == TRAD) s = "TRAD";
  return s;
}

static int find_loc(std::string const& grade, std::vector<std::string> const& grades) {
  int loc = -1;
  auto it = std::find(grades.begin(), grades.end(), grade);
  if (it != grades.end()) loc = std::distance(grades.begin(), it);
  return loc;
}

static int find_loc(climb const& c, std::vector<climb> const& climbs) {
  int ctr = 0;
  for (auto& existing_climb : climbs) {
    if (is_equal(existing_climb.name, c.name)) {
      if (is_equal(existing_climb.location, c.location)) {
        return ctr;
      }
    }
    ctr++;
  }
  return -1;
}

static bool is_already_climb(climb const& c) {
  int const loc = find_loc(c, my_climbs);
  if (loc == -1) return false;
  else return true;
}

static void get_climb_name(climb& c) {
  std::cout << "  > name: ";
  std::getline(std::cin, c.name);
  trim(c.name);
}

static void get_climb_location(climb& c) {
  std::cout << "  > location: ";
  std::getline(std::cin, c.location);
  trim(c.location);
}

template <class HEADER, class F>
void parse_arg(HEADER const& header, F const& f, climb& c) {
  std::string arg;
  header();
  while (std::getline(std::cin, arg)) {
    trim(arg);
    if (f(c, arg)) break;
    print_invalid_input(arg);
    header();
  }
}

static void print_climb_type_header() {
  std::cout << "  > type: [b]oulder, [s]port, [tr] top rope, [t]rad: ";
}

static bool set_climb_type(climb& c, std::string const& type) {
  bool was_set = true;
  if      (type == "b") c.type = BOULDER;
  else if (type == "boulder") c.type = BOULDER;
  else if (type == "s") c.type = SPORT;
  else if (type == "sport") c.type = SPORT;
  else if (type == "tr") c.type = TOP_ROPE;
  else if (type == "top rope") c.type = TOP_ROPE;
  else if (type == "trad") c.type = TRAD;
  else if (type == "t") c.type = TRAD;
  else was_set = false;
  return was_set;
}

static void print_v_grade_header() {
  std::cout << "  > grade: V[B-17][-/+]: ";
}

static bool set_v_grade(climb& c, std::string const& grade) {
  bool was_set = true;
  int const loc = find_loc(grade, valid_v_grades);
  if (loc != -1) c.grade_loc = loc;
  else was_set = false;
  return was_set;
}

static void print_yds_grade_header() {
  std::cout << "  > grade: 5.[4-15][a-d][-/+]: ";
}

static bool set_yds_grade(climb& c, std::string const& grade) {
  bool was_set = true;
  int const loc = find_loc(grade, valid_yds_grades);
  if (loc != -1) c.grade_loc = loc;
  else was_set = false;
  return was_set;
}

static void add_climb() {
  climb c;
  get_climb_name(c);
  get_climb_location(c);
  if (is_already_climb(c)) {
    print_climb_exists(c);
    return;
  }
  parse_arg(print_climb_type_header, set_climb_type, c);
  if (c.type == BOULDER) parse_arg(print_v_grade_header, set_v_grade, c);
  else parse_arg(print_yds_grade_header, set_yds_grade, c);
  my_climbs.push_back(c);
}

static bool should_remove(climb const& c) {
  std::string arg;
  std::cout << "this will permanantently erase `"
    << c.name << "` at `" << c.location << "`\n";
  std::cout << "proceed [y/n]: ";
  while (std::getline(std::cin, arg)) {
    trim(arg);
    if (arg == "y" || arg == "yes") return true;
    if (arg == "n" || arg == "no") return false;
    print_invalid_input(arg);
    std::cout << "proceed [y/n]: ";
  }
  return false;
}

static void remove_climb() {
  climb tmp;
  get_climb_name(tmp);
  get_climb_location(tmp);
  int const loc = find_loc(tmp, my_climbs);
  if (loc == -1) { print_climb_not_found(tmp); return; }
  if (should_remove(tmp)) {
    my_climbs.erase(my_climbs.begin() + loc);
  }
}

static void print_climbs(std::vector<climb> const& climbs) {
  std::cout << " -----------------------------------------------------------------------------------------------------------------------------\n";
  std::cout << "|                     name                      |                   location                    |   type   | grade | attempts |\n";
  std::cout << " -----------------------------------------------------------------------------------------------------------------------------\n";
  for (auto& c : climbs) {
    std::string grade;
    if (c.type == BOULDER) grade = valid_v_grades[c.grade_loc];
    else grade = valid_yds_grades[c.grade_loc];
    std::cout << "| ";
    std::cout << std::setw(45) << std::left << c.name;
    std::cout << " | ";
    std::cout << std::setw(45) << std::left << c.location;
    std::cout << " | ";
    std::cout << std::setw(8) << std::left << to_string(c.type);
    std::cout << " | ";
    std::cout << std::setw(5) << std::left << grade;
    std::cout << " | ";
    std::cout << std::setw(8) << std::left << c.attempts.size();
    std::cout << " | ";
    std::cout << "\n";
  }
}

static bool act_on_input(std::string const& input) {
  if (!is_input_valid(input)) print_invalid_input(input);;
  if (input == "q" || input == "quit") return true;
  if (input == "h" || input == "help") print_help();
  if (input == "add climb") add_climb();
  if (input == "remove climb") remove_climb();
  if (input == "print") print_climbs(my_climbs);
  return false;
}

}

int main() {
  std::cout << "----------------------\n";
  std::cout << " welcome to climb log \n";
  std::cout << "----------------------\n";
  cl::print_help();
  std::cout << "> ";
  std::string input;
  while (std::getline(std::cin, input)) {
    bool const quit = cl::act_on_input(input);
    if (quit) break;
    std::cout << "> ";
  }
  return 0;
}
