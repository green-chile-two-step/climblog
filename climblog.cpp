#include <algorithm>
#include <cassert>
#include <fstream>
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
  REDPOINT,
  SEND
};

struct climb_date {
  int year;
  int month;
  int day;
};

struct attempt {
  climb_date date;
  climb_style style;
  climb_performance performance;
  std::string comments;
};

struct climb {
  std::string name;
  std::string location;
  climb_type type;
  int grade_loc;
  int stars;
  std::string comments;
  std::vector<attempt> attempts;
};

static std::vector<climb> my_climbs;
static std::vector<climb> filtered_climbs;
static const std::string db = "climblog.db";

static const std::vector<std::string> valid_inputs = {
  "q",
  "quit",
  "h",
  "help",
  "add climb",
  "add attempt",
  "remove climb",
  "flush",
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
  std::cout << "  > add attempt   [add a climb attempt]\n";
  std::cout << "  > remove climb  [remove a climb]\n";
  std::cout << "  > flush         [flush the database, remove all climbs]\n";
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

static bool is_equal(std::string const& a, std::string const& b) {
  return std::equal(a.begin(), a.end(), b.begin(), b.end(),
      [] (char a, char b) { return std::tolower(a) == std::tolower(b); });
}

static bool is_integer(const std::string& s) {
    return !s.empty() &&
      std::find_if(s.begin(), s.end(),
          [] (char c) { return !std::isdigit(c); }) == s.end();
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

static std::string to_string(climb_style const& style) {
  std::string s;
  if (style == LEAD) s = "LEAD";
  if (style == TR) s = "TR";
  if (style == SOLO) s = "SOLO";
  return s;
}

static std::string to_string(climb_performance const& perf) {
  std::string s;
  if (perf == FELL) s = "FELL";
  if (perf == FLASH) s = "FLASH";
  if (perf == HUNG) s = "HUNG";
  if (perf == ONSIGHT) s = "ONSIGHT";
  if (perf == REDPOINT) s = "REDPOINT";
  if (perf == SEND) s = "SEND";
  return s;
}

static std::string to_stars(int stars) {
  std::string s;
  if (stars == 0) s = "O";
  if (stars == 1) s = "*";
  if (stars == 2) s = "**";
  if (stars == 3) s = "***";
  if (stars == 4) s = "****";
  return s;
}

static climb_type to_type(std::string const& type) {
  climb_type t;
  if (type == "BOULDER") t = BOULDER;
  if (type == "SPORT") t = SPORT;
  if (type == "TOP ROPE") t = TOP_ROPE;
  if (type == "TRAD") t = TRAD;
  return t;
}

static climb_style to_style(std::string const& style) {
  climb_style s;
  if (style == "LEAD") s = LEAD;
  if (style == "TR") s = TR;
  if (style == "SOLO") s = SOLO;
  return s;
}

static climb_performance to_performance(std::string const& perf) {
  climb_performance p;
  if (perf == "FELL") p = FELL;
  if (perf == "FLASH") p = FLASH;
  if (perf == "HUNG") p = HUNG;
  if (perf == "ONSIGHT") p = ONSIGHT;
  if (perf == "REDPOINT") p = REDPOINT;
  if (perf == "SEND") p = SEND;
  return p;
}

static int find_loc(std::string const& grade, std::vector<std::string> const& grades) {
  int loc = -1;
  auto it = std::find(grades.begin(), grades.end(), grade);
  if (it != grades.end()) loc = std::distance(grades.begin(), it);
  return loc;
}

static int find_loc(climb const& c, std::vector<climb> const& climbs) {
  int ctr = 0;
  for (climb const& existing_climb : climbs) {
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

template <class HEADER, class F, class CLIMB_ATTEMPT>
void parse_arg(HEADER const& header, F const& f, CLIMB_ATTEMPT& ca) {
  std::string arg;
  header();
  while (std::getline(std::cin, arg)) {
    trim(arg);
    if (f(ca, arg)) break;
    print_invalid_input(arg);
    header();
  }
}

static void print_climb_type_header() {
  std::cout << "  > type: [b]oulder, [s]port, [tr] top rope, [t]rad: ";
}

static bool set_climb_type(climb& c, std::string const& type) {
  bool was_set = true;
  if      (is_equal(type, "b")) c.type = BOULDER;
  else if (is_equal(type, "boulder")) c.type = BOULDER;
  else if (is_equal(type, "s")) c.type = SPORT;
  else if (is_equal(type, "sport")) c.type = SPORT;
  else if (is_equal(type, "tr")) c.type = TOP_ROPE;
  else if (is_equal(type, "top rope")) c.type = TOP_ROPE;
  else if (is_equal(type, "trad")) c.type = TRAD;
  else if (is_equal(type, "t")) c.type = TRAD;
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

static void print_stars_header() {
  std::cout << "  > stars: [0-4]: ";
}

static bool set_stars(climb& c, std::string const& stars) {
  bool was_set = true;
  c.stars = 0;
  if (is_integer(stars)) c.stars = std::atoi(stars.c_str());
  else was_set = false;
  if (c.stars < 0) was_set = false;
  if (c.stars > 4) was_set = false;
  return was_set;
}

template <class T>
static void get_comments(T& ca) {
  std::cout << "  > comments: ";
  std::getline(std::cin, ca.comments);
  trim(ca.comments);
}

static void print_year_header() {
  std::cout << "  > year: ";
}

static bool set_year(attempt& a, std::string const& year) {
  bool was_set = true;
  if (is_integer(year)) a.date.year = std::atoi(year.c_str());
  else was_set = false;
  return was_set;
}

static void print_month_header() {
  std::cout << "  > month: ";
}

static bool set_month(attempt& a, std::string const& month) {
  bool was_set = true;
  if (is_integer(month)) a.date.month = std::atoi(month.c_str());
  else was_set = false;
  if (a.date.month < 1) was_set = false;
  if (a.date.month > 12) was_set = false;
  return was_set;
}

static void print_day_header() {
  std::cout << "  > day: ";
}

static bool set_day(attempt& a, std::string const& day) {
  bool was_set = true;
  if (is_integer(day)) a.date.day = std::atoi(day.c_str());
  else was_set = false;
  if (a.date.month < 1) was_set = false;
  if (a.date.month > 31) was_set = false;
  return was_set;
}


static void print_climb_style_header() {
  std::cout << "  > style: [l]ead, [t]op rope, [s]solo: ";
}

static bool set_climb_style(attempt& a, std::string const& style) {
  bool was_set = true;
  if      (is_equal(style, "l")) a.style = LEAD;
  else if (is_equal(style, "lead")) a.style = LEAD;
  else if (is_equal(style, "s")) a.style = SOLO;
  else if (is_equal(style, "solo")) a.style = SOLO;
  else if (is_equal(style, "t")) a.style = TR;
  else if (is_equal(style, "top rope")) a.style = TR;
  else was_set = false;
  return was_set;
}

static void print_climb_performance_header() {
  std::cout << "  > performance: [fe]ll, [fl]ash, [h]ung, [o]nsight, [r]edpoint, [s]end: ";
}

static bool set_climb_performance(attempt& a, std::string const& perf) {
  bool was_set = true;
  if      (is_equal(perf, "fe")) a.performance = FELL;
  else if (is_equal(perf, "fell")) a.performance = FELL;
  else if (is_equal(perf, "fl")) a.performance = FLASH;
  else if (is_equal(perf, "flash")) a.performance = FLASH;
  else if (is_equal(perf, "h")) a.performance = HUNG;
  else if (is_equal(perf, "hung")) a.performance = HUNG;
  else if (is_equal(perf, "r")) a.performance = REDPOINT;
  else if (is_equal(perf, "redpoint")) a.performance = REDPOINT;
  else if (is_equal(perf, "s")) a.performance = SEND;
  else if (is_equal(perf, "send")) a.performance = SEND;
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
  parse_arg(print_stars_header, set_stars, c);
  get_comments(c);
  my_climbs.push_back(c);
}

static void add_attempt() {
  climb tmp;
  get_climb_name(tmp);
  get_climb_location(tmp);
  int const loc = find_loc(tmp, my_climbs);
  if (loc == -1) { print_climb_not_found(tmp); return; }
  auto& c = my_climbs[loc];
  attempt a;
  parse_arg(print_year_header, set_year, a);
  parse_arg(print_month_header, set_month, a);
  parse_arg(print_day_header, set_day, a);
  parse_arg(print_climb_style_header, set_climb_style, a);
  parse_arg(print_climb_performance_header, set_climb_performance, a);
  get_comments(a);
  c.attempts.push_back(a);
}

static bool should_remove() {
  std::string arg;
  std::cout << "proceed [y/n]: ";
  while (std::getline(std::cin, arg)) {
    trim(arg);
    if (is_equal(arg, "y") || is_equal(arg, "yes")) return true;
    if (is_equal(arg, "n") || is_equal(arg, "no")) return false;
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
  std::cout << "this will permanantently erase `"
    << tmp.name << "` at `" << tmp.location << "`\n";
  if (should_remove()) {
    my_climbs.erase(my_climbs.begin() + loc);
    std::cout << "`" << tmp.name << "` at `" << "` erased\n";
  }
}

static void flush() {
  std::string arg;
  std::cout << "this will permananently erase all climbs\n";
  if (should_remove()) {
    my_climbs.resize(0);
    my_climbs.shrink_to_fit();
    std::cout << "all climbs erased\n";
  }
}

static void print_climbs(std::vector<climb> const& climbs) {
  std::cout << " ----------------------------------------------------------------------------------------------------------------------------------------------------------------------\n";
  std::cout << "|                  name                    |                 location                 |                  comments                |   type   | grade | stars | attempts |\n";
  std::cout << " ----------------------------------------------------------------------------------------------------------------------------------------------------------------------\n";
  for (climb const& c : climbs) {
    std::string grade;
    if (c.type == BOULDER) grade = valid_v_grades[c.grade_loc];
    else grade = valid_yds_grades[c.grade_loc];
    std::cout << "| ";
    std::cout << std::setw(40) << std::left << c.name;
    std::cout << " | ";
    std::cout << std::setw(40) << std::left << c.location;
    std::cout << " | ";
    std::cout << std::setw(40) << std::left << c.comments;
    std::cout << " | ";
    std::cout << std::setw(8) << std::left << to_string(c.type);
    std::cout << " | ";
    std::cout << std::setw(5) << std::left << grade;
    std::cout << " | ";
    std::cout << std::setw(5) << std::left << to_stars(c.stars);
    std::cout << " | ";
    std::cout << std::setw(8) << std::left << c.attempts.size();
    std::cout << " | ";
    std::cout << "\n";
    int ctr = 1;
    for (attempt const& a : c.attempts) {
      std::cout << "  [" << ctr++ << "]: ";
      std::cout << a.date.year << "-" << a.date.month << "-" << a.date.day << ", ";
      std::cout << to_string(a.style) << ", ";
      std::cout << to_string(a.performance) << ", ";
      std::cout << a.comments << "\n";
    }
  }
}

static bool act_on_input(std::string const& input) {
  if (!is_input_valid(input)) print_invalid_input(input);;
  if (is_equal(input, "q") || is_equal(input, "quit")) return true;
  if (is_equal(input, "h") || is_equal(input, "help")) print_help();
  if (is_equal(input, "add climb")) add_climb();
  if (is_equal(input, "add attempt")) add_attempt();
  if (is_equal(input, "remove climb")) remove_climb();
  if (is_equal(input, "flush")) flush();
  if (is_equal(input, "print")) print_climbs(my_climbs);
  return false;
}

static int read_int(std::ifstream& in) {
  char buf[4];
  in.read(buf, 4);
  return std::atoi(buf);
}

static void write_int(std::ofstream& out, int num) {
  std::string const s = std::to_string(num);
  out.write(s.c_str(), 4);
}

static std::string read_long_str(std::ifstream& in) {
  char buf[45];
  in.read(buf, 45);
  return std::string(buf);
}

static void write_long_str(std::ofstream& out, std::string const& str) {
  out.write(str.c_str(), 45);
}

static std::string read_short_str(std::ifstream& in) {
  char buf[8];
  in.read(buf, 8);
  return std::string(buf);
}

static void write_short_str(std::ofstream& out, std::string const& str) {
  out.write(str.c_str(), 8);
}

static void read_db() {
  std::ifstream in(cl::db, std::ios::in | std::ios::binary);
  assert(in.is_open());
  int const nclimbs = read_int(in);
  my_climbs.resize(nclimbs);
  for (int i = 0; i < nclimbs; ++i) {
    climb& c = my_climbs[i];
    c.name = read_long_str(in);
    c.location = read_long_str(in);
    c.comments = read_long_str(in);
    c.type = to_type(read_short_str(in));
    c.grade_loc = read_int(in);
    c.stars = read_int(in);
    c.attempts.resize(read_int(in));
    for (size_t j = 0; j < c.attempts.size(); ++j) {
      attempt& a = c.attempts[j];
      a.date.year = read_int(in);
      a.date.month = read_int(in);
      a.date.day = read_int(in);
      a.style = to_style(read_short_str(in));
      a.performance = to_performance(read_short_str(in));
      a.comments = read_long_str(in);
    }
  }
}

static void write_db() {
  std::ofstream out(cl::db, std::ios::out | std::ios::binary);
  assert(out.is_open());
  int const nclimbs = my_climbs.size();
  write_int(out, nclimbs);
  for (int i = 0; i < nclimbs; ++i) {
    climb const& c = my_climbs[i];
    write_long_str(out, c.name);
    write_long_str(out, c.location);
    write_long_str(out, c.comments);
    write_short_str(out, to_string(c.type));
    write_int(out, c.grade_loc);
    write_int(out, c.stars);
    write_int(out, c.attempts.size());
    for (size_t j = 0; j < c.attempts.size(); ++j) {
      attempt const& a = c.attempts[j];
      write_int(out, a.date.year);
      write_int(out, a.date.month);
      write_int(out, a.date.day);
      write_short_str(out, to_string(a.style));
      write_short_str(out, to_string(a.performance));
      write_long_str(out, a.comments);
    }
  }
}

}

int main() {
  std::cout << "----------------------\n";
  std::cout << " welcome to climb log \n";
  std::cout << "----------------------\n";
  cl::print_help();
  cl::read_db();
  std::cout << "> ";
  std::string input;
  while (std::getline(std::cin, input)) {
    bool const quit = cl::act_on_input(input);
    if (quit) break;
    std::cout << "> ";
  }
  cl::write_db();
  return 0;
}
