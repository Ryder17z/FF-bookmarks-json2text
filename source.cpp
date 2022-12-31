#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

std::string urldecode3(const std::string& src)
{
  std::string dst;
  char a, b;
  for (std::size_t i = 0; i < src.size(); ++i) {
    if (src[i] == '%' && i + 2 < src.size() &&
        (a = src[i + 1]) && (b = src[i + 2]) &&
        std::isxdigit(a) && std::isxdigit(b)) {
      if (a >= 'a') {
        a -= 'a' - 'A';
      }
      if (a >= 'A') {
        a -= 'A' - 10;
      } else {
        a -= '0';
      }
      if (b >= 'a') {
        b -= 'a' - 'A';
      }
      if (b >= 'A') {
        b -= 'A' - 10;
      } else {
        b -= '0';
      }
      dst += 16 * a + b;
      i += 2;
    } else if (src[i] == '+') {
      dst += ' ';
    } else {
      dst += src[i];
    }
  }
  return dst;
}

std::string extracturi(const std::string& src)
{
    auto result = std::search(src.begin(), src.end(), "https%3A%2F%2F", "https%3A%2F%2F" + 14);
    if (result != src.end()) {
        return std::string(result, src.end());
    } else {
        result = std::search(src.begin(), src.end(), "http%3A%2F%2F", "http%3A%2F%2F" + 13);
        if (result != src.end()) {
            return std::string(result, src.end());
        }
        // Neither http or https was found
    }
    return src;
}

std::string::const_iterator lazysearch(const std::string* src, const std::string& what) {
    size_t pos = 0;
    while (pos < src->size()) {
        size_t foundPos = src->find(what, pos);
        if (foundPos == std::string::npos) {
            break;  // what was not found
        }

        // what was found at foundPos
        return src->begin() + foundPos;  // Return an iterator to the found position
        pos = foundPos + 1;  // Start the next search at the next character
    }

    return src->end();  // what was not found, return an iterator to the end of the string
}

std::string parseline(const std::string& src)
{
    std::string uri_output;
    auto found_moz_uri = lazysearch(&src, "&u=http"); // moz-extension://page_title http(s)://url.com
    if (found_moz_uri != src.end()) {
        auto res = std::string(found_moz_uri+3, src.end());
        uri_output = extracturi(res);
    }else{
        auto found_moz_local = lazysearch(&src, "&u=file"); // moz-extension://page_title "local.file"
        if (found_moz_local != src.end()) {
            auto res = std::string(found_moz_local+3, src.end());
            uri_output = extracturi(res);
        }else{
            auto found_moz_other = lazysearch(&src, "moz-extension://"); // moz-extension:// other formats
            if (found_moz_other != src.end()) {
                auto res = std::string(found_moz_other, src.end());
                uri_output = extracturi(res);
            }
            else uri_output = src; // otherwise, copy source
        }
    }    
    
    std::string output_url = urldecode3(uri_output);
    return output_url;
}


class my_exception : public std::runtime_error {
    std::string msg;
public:
    my_exception(const std::string &arg, const char *file, int line) :
    std::runtime_error(arg) {
        std::ostringstream o;
        o << file << ":" << line << ": " << arg;
        msg = o.str();
    }
    ~my_exception() throw() {}
    const char *what() const throw() {
        return msg.c_str();
    }
};
#define throw_line(arg) throw my_exception(arg, __FILE__, __LINE__);

/*void f() {
    throw_line("Oh no!");
}*/

void remove_doublequotes(std::string& src)
{
    // Check if the string begins with '\"'
    if (src.front() == '\"') {
        // Remove the first character from the string
        src.erase(src.begin());
    }

    // Check if the string ends with '\"'
    if (src.back() == '\"') {
        // Remove the last character from the string
        src.pop_back();
    }
}


int main(int argc, char **argv) {
    
    try {

        // check for input
        if (argc != 2)
        {
            printf("Input file missing!\nDrag-n-drop a file onto this executable!\n");
            return EXIT_FAILURE;
       
        }

        printf("Attempting to parse %s\n", argv[1]);

        std::ifstream json_file(argv[1]);
        json data;
        json_file >> data;

        std::ofstream txt_file("uris.txt");

        for (const auto& child : data["children"]) {
            if (child.count("children")) {
                for (const auto& grandchild : child["children"]) {
                    txt_file << grandchild["uri"] << std::endl;
                }
            }
        }
        txt_file.close();

        int linecounter=0;
        std::ifstream in_file("uris.txt");
        std::ofstream out_file("url_list_decoded.txt");
        
        if (in_file.is_open()) {
            std::string line;
            while (std::getline(in_file, line)) {
                linecounter++;

                std::cout << linecounter << "   ";
                
                if (out_file.is_open()) {
                    // Call the function to remove the '\"' characters from the string
                    remove_doublequotes(line);

                    // continue processing
                    out_file << parseline(line);
    		out_file << std::endl;
                }
            }
            out_file.close();
            in_file.close();
        }
        std::cout << std::endl << std::endl;

        std::cout << "Processed " << linecounter << " amount of lines" << std::endl;

	std::remove("uris.txt"); // delete temporary file
    }
    catch (const std::runtime_error &ex) {
        std::cout << ex.what() << std::endl;
    }
    return 0;
}

