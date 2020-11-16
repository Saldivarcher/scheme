#include <array>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <variant>

enum class object_t { FIXNUM, BOOLEAN, CHARACTER, STRING, EMPTY_LIST, PAIR };

struct object {
  // TODO: Figure out a way to use `unique_ptr` rather than `shared_ptr`.
  //       That might be impossible due to the state holding those objects.
  using objectptr_t = std::shared_ptr<object>;
  using object_pair = std::pair<objectptr_t, objectptr_t>;

  static objectptr_t make_object() { return std::make_shared<object>(); }

  static objectptr_t make_object(object_t type, auto &&data) {
    auto obj = make_object();
    obj->type = type;
    obj->data = std::move(data);
    return obj;
  }

  object_t type;
  std::variant<char, long, std::string, object_pair> data;
};

using objectptr_t = object::objectptr_t;

void error(const char *msg, int8_t status) {
  std::fprintf(stderr, "%s", msg);
  exit(status);
}

// Rather than using global variables for the true and false objects,
// lets keep them in a state.
class State {
public:
  State() {
    true_object = object::make_object(object_t::BOOLEAN, 't');
    false_object = object::make_object(object_t::BOOLEAN, 'f');

    empty_list_object = object::make_object();
    empty_list_object->type = object_t::EMPTY_LIST;
  }

  State(const State &) = delete;
  State &operator=(const State &) = delete;
  ~State() = default;

  objectptr_t get_true_object() { return true_object; }
  objectptr_t get_false_object() { return false_object; }
  objectptr_t get_empty_list_object() { return empty_list_object; }

private:
  objectptr_t true_object;
  objectptr_t false_object;
  objectptr_t empty_list_object;
};

class Reader {
public:
  Reader(State &s) : s(s) {}

  objectptr_t read(std::istringstream &&input) { return inner_read(input); }

private:
  inline void eat_whitespace(std::istringstream &input) {
    char ch;
    while ((ch = input.get()) != EOF) {
      if (std::isspace(ch))
        continue;
      else if (ch == ';') {
        while (((ch = input.get()) != EOF) && (ch != '\n'))
          ;
        continue;
      }
      input.unget();
      break;
    }
  }

  inline bool is_delimiter(char ch) {
    return std::isspace(ch) || ch == EOF || ch == '(' || ch == ')' ||
           ch == '"' || ch == ';';
  }

  objectptr_t read_character(std::istringstream &input) {
    auto eat_input_and_compare = [&](const char *expected, int size) -> bool {
      char buffer[size];
      input.get(buffer, size);
      return std::string(buffer) == expected;
    };

    char ch = input.get();
    switch (ch) {
    case 'n':
      // Lets look for newline
      if (input.peek() == 'e') {
        if (eat_input_and_compare("ewline", 7)) {
          if (!is_delimiter(input.peek()))
            error("Invalid character!\n", 4);
          return object::make_object(object_t::CHARACTER, '\n');
        }
      }
      break;
    case 's':
      // Lets look for space
      if (input.peek() == 'p') {
        if (eat_input_and_compare("pace", 5)) {
          if (!is_delimiter(input.peek()))
            error("Invalid character!\n", 4);
          return object::make_object(object_t::CHARACTER, ' ');
        }
      }
      break;
    default:
      break;
    }
    if (!is_delimiter(input.peek()))
      error("Invalid character!\n", 4);
    return object::make_object(object_t::CHARACTER, ch);
  }

  objectptr_t read_pair(std::istringstream &input) {
    eat_whitespace(input);

    char ch = input.get();

    // Handle 'empty' list object.
    if (ch == ')')
      return s.get_empty_list_object();

    input.unget();

    auto car_obj = inner_read(input);

    eat_whitespace(input);
    if (ch = input.get(); ch == '.') {
      if (!is_delimiter(input.peek()))
        error("dot not followed by delimiter!!\n", 1);

      auto cdr_obj = inner_read(input);
      eat_whitespace(input);

      if (ch = input.get(); ch != ')')
        error("No matching paren?!\n", 10);
      return object::make_object(object_t::PAIR,
                                 object::object_pair(car_obj, cdr_obj));
    } else {
      input.unget();
      auto cdr_obj = read_pair(input);
      return object::make_object(object_t::PAIR,
                                 object::object_pair(car_obj, cdr_obj));
    }
  }

  objectptr_t inner_read(std::istringstream &input) {
    auto get_or_getchar = [&](const char &ch) -> char {
      // So when a user enters a string and starts a newline before
      // entering a '"'. We need to get the next incoming line and insert
      // it into the istringstream object.
      if (ch == '\n') {
        std::string future_input;
        std::getline(std::cin, future_input);
        input.clear();
        input.str(future_input + "\n");
        return input.get();
      }
      return input.get();
    };

    eat_whitespace(input);

    char ch = input.get();

    if (std::isdigit(ch) || (ch == '-' && std::isdigit(input.peek()))) {
      // Read a fixnum
      short sign = 1;
      long num = 0;
      if (ch == '-')
        sign = -1;
      else
        input.unget();

      while (std::isdigit(ch = input.get()))
        num = (num * 10) + (ch - '0');

      num *= sign;

      if (is_delimiter(ch)) {
        input.unget();
        return object::make_object(object_t::FIXNUM, num);
      } else
        error("Number not followed by delimiter\n", 2);
    } else if (ch == '#') {
      // Read a bool or char
      ch = input.get();
      switch (ch) {
      case 't':
        return s.get_true_object();
      case 'f':
        return s.get_false_object();
      case '\\':
        return read_character(input);
      default:
        error("Unexpected character after '#'!", 4);
      }
    } else if (ch == '"') {
      // Read a string
      std::string temp_str;
      while ((ch = get_or_getchar(ch)) != '"') {
        if (ch == '\\') {
          if (input.peek() == '"') {
            // Eat '\' char
            temp_str += ch;

            ch = input.get();

            // Eat '"' char
            temp_str += ch;
            continue;
          }
        }
        if (ch == EOF)
          error("There shouldn't be a null-terminated character here!", 5);
        temp_str += ch;
      }

      return object::make_object(object_t::STRING, temp_str);
    } else if (ch == '(') {
      // Read a pair
      return read_pair(input);
    } else
      error("Bad input!\n", 3);

    error("Illegal state\n", 4);
    return nullptr;
  }

  State &s;
};

class Writer {
public:
  Writer() = default;

  void write(objectptr_t obj) {
    switch (obj->type) {
    case object_t::BOOLEAN:
      std::printf("#%c", std::get<char>(obj->data));
      break;
    case object_t::CHARACTER: {
      std::printf("#\\");
      char ch = std::get<char>(obj->data);
      // These are the cases where the user just inputs '#\ ' or '#\'
      switch (ch) {
      case '\n':
        std::printf("newline");
        break;
      case ' ':
        std::printf("space");
        break;
      default:
        std::printf("%c", ch);
        break;
      }
    } break;
    case object_t::FIXNUM:
      std::printf("%ld", std::get<long>(obj->data));
      break;
    case object_t::STRING: {
      std::string data = std::get<std::string>(obj->data);
      std::printf("\"");
      for (const auto &ch : data) {
        switch (ch) {
        case '\n':
          std::printf("\\n");
          break;
        default:
          std::printf("%c", ch);
          break;
        }
      }
      std::printf("\"");
      break;
    }
    case object_t::EMPTY_LIST:
      std::printf("()");
      break;
    case object_t::PAIR:
      std::printf("(");
      write_pair(obj);
      std::printf(")");
      break;
    default:
      error("Unknown type!\n", 1);
    }
  }

private:
  void write_pair(objectptr_t obj) {
    auto cons = std::get<object::object_pair>(obj->data);
    auto car = cons.first;
    auto cdr = cons.second;
    write(car);

    if (cdr->type == object_t::PAIR) {
      std::printf(" ");
      write_pair(cdr);
    } else if (cdr->type == object_t::EMPTY_LIST) {
      return;
    } else {
      std::printf(" . ");
      write(cdr);
    }
  }
};

class Driver {
public:
  Driver() : s(), w(), r(s) {}

  void drive() {
    std::string input;
    std::printf("> ");
    while (std::getline(std::cin, input)) {
      w.write(eval(r.read(std::istringstream(input + '\n'))));
      std::printf("\n");
      std::printf("> ");
    }
  }

private:
  State s;
  Writer w;
  Reader r;

  objectptr_t eval(objectptr_t obj) { return obj; }
};

int main() {
  Driver d;
  d.drive();

  return 0;
}
