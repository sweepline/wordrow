#include <chrono>

typedef std::chrono::steady_clock::time_point time_point;

inline time_point get_timestamp() {
  return std::chrono::steady_clock::now();
}

typedef unsigned long int time_duration;

inline unsigned long int duration_of(const time_point &before, const time_point &after) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(after - before).count();
}

#include <getopt.h>
#include <iostream>
#include <sstream>
#include <string>
#include <regex>

#include "anatree.h" // <-- TODO: Use <anatree.h> instead
#include "dict.h"

size_t word_size(const std::string& w) {
  // Regex of all characters that actually are two letters.
  std::regex double_char("æ|ø|å");

  // https://stackoverflow.com/a/8283994
  const size_t double_chars = std::distance(std::sregex_iterator(w.begin(), w.end(), double_char),
                                            std::sregex_iterator());

  return w.size() - double_chars;
}

struct lexicographical_lt
{
  bool operator()(const std::string &a, const std::string &b)
  {
    const size_t a_length = word_size(a);
    const size_t b_length = word_size(b);
    return a_length != b_length ? a_length < b_length : a < b;
  }
};

std::string gen_json(const std::unordered_set<std::string> &words)
{
  std::vector words_sorted(words.begin(), words.end());
  std::sort(words_sorted.begin(), words_sorted.end(), lexicographical_lt());

  std::stringstream ss;
  ss << "{" << std::endl
     << "  \"anagrams\": [" << std::endl;

  for (const std::string &w : words_sorted) {
    ss << "    \"" << w << "\"," << std::endl;
  }
  ss.seekp(-2,ss.cur);
  ss << std::endl
     << "  ]" << std::endl
     << "}" << std::endl;

  return ss.str();
}

int main(int argc, char* argv[]) {
  // -----------------------------------------------------------------
  // Parse arguments from user
  if (argc < 4) {
    std::cerr << "Need 4 arguments" << std::endl
              << " min-length : integer" << std::endl
              << " max-length : integer" << std::endl
              << " .dic path  : string"  << std::endl
              << " .aff path  : string"  << std::endl
      ;
    return -1;
  }

  const size_t MIN_LENGTH = std::stoul(argv[1]);
  const size_t MAX_LENGTH = std::stoul(argv[2]);

  const std::string dic_file_path = argv[3];
  const std::string aff_file_path = argv[4];

  // -----------------------------------------------------------------
  // Open dictionary and populate Anatree
  dict d(dic_file_path, aff_file_path);
  anatree a;

  size_t total_words = 0u;
  size_t total_chars = 0u;

  size_t used_words = 0u;
  size_t used_chars = 0u;

  const std::regex is_lower_char("[a-zæøå]*");

  size_t dict_parse__time = 0;
  size_t anatree_insert__time = 0;

  while(d.can_pull()) {
    const time_point pull__start_time = get_timestamp();
    std::string w = d.pull();
    const time_point pull__end_time = get_timestamp();
    dict_parse__time += duration_of(pull__start_time, pull__end_time);

    total_words += 1;
    total_chars += word_size(w);

    if (MIN_LENGTH <= word_size(w) && word_size(w) <= MAX_LENGTH && regex_match(w, is_lower_char)) {
      used_words += 1;
      used_chars += word_size(w);

      //      std::cout << w << " " << word_size(w) << std::endl;

      const time_point insert__start_time = get_timestamp();
      a.insert(w);
      const time_point insert__end_time = get_timestamp();
      anatree_insert__time += duration_of(insert__start_time, insert__end_time);
    }
  }

  const time_point keys__start_time = get_timestamp();
  std::unordered_set<std::string> keys = a.keys();
  const time_point keys__end_time = get_timestamp();

  std::cout << "Dictionary:" << std::endl
            << "| # Words:           " << total_words << " words" << std::endl
            << "| Time:              " << std::endl
            << "| | Parsing:         " << dict_parse__time << " ns" << std::endl
    ;

  std::cout << std::endl;

  std::cout << "Anatree:" << std::endl
            << "| # Words:           " << used_words << " words (" << used_chars << " characters)." << std::endl
            << "| # Nodes:           " << a.size() << std::endl
            << "| # Keys:            " << keys.size() << std::endl
            << "| Time:              " << std::endl
            << "| | Insertion:       " << anatree_insert__time << " ns" << std::endl
            << "| | Keys:            " << duration_of(keys__start_time, keys__end_time) << " ns" << std::endl
    ;

  // -----------------------------------------------------------------
  // Create game instances
  size_t idx = 0;
  size_t skipped__short_keys = 0;
  size_t skipped__short_games = 0;
  size_t anagrams__time = 0;

  for (std::string k : keys) {
    // Ignore keys that would lead to a game with the longest word not being of
    // MAX length
    if (word_size(k) < MAX_LENGTH) {
      skipped__short_keys += 1;
      continue;
    }
    assert(word_size(k) == MAX_LENGTH);

    std::stringstream ss;
    ss << "./out/" << idx << ".json";

    std::ofstream out_file(ss.str());

    const time_point anagrams__start_time = get_timestamp();
    std::unordered_set<std::string> game = a.anagrams_of(k);
    const time_point anagrams__end_time = get_timestamp();
    anagrams__time += duration_of(anagrams__start_time, anagrams__end_time);

    // Ignore small games (def: 'small' less than half the answer)
    if (game.size() < 21) {
      skipped__short_games += 1;
      continue;
    }

    out_file << gen_json(a.anagrams_of(k));

    idx++;
  }
  {
    std::stringstream ss;
    ss << "./out/index.json";

    std::ofstream out_file(ss.str());
    out_file << "{" << std::endl
             << "  \"instances\": " << idx << std::endl
             << "}" << std::endl;
      ;
  }

  std::cout << "| | Anagrams:        " << anagrams__time << " ns" << std::endl;

  std::cout << std::endl;
  std::cout << "Created " << idx << " games in '.out/'" << std::endl;
  std::cout << "| Skipped " << skipped__short_keys << " keys that were too short." << std::endl;
  std::cout << "| Skipped " << skipped__short_games << " games that were too short." << std::endl;
}

