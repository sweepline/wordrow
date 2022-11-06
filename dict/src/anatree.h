#ifndef ANATREE_H
#define ANATREE_H

#include<algorithm>
#include<assert.h>
#include<memory>
#include<sstream>
#include<string>
#include<unordered_set>

class anatree {
public:
  typedef std::string string;

private:
  class node {
  public:
    typedef std::shared_ptr<node> ptr; // <-- TODO: 'std::unique_ptr'

    static constexpr char NIL = '~';

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Character in this node
    ////////////////////////////////////////////////////////////////////////////
    char _char = '~';

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Binary choice on children.
    ///
    /// Follow the 'false' pointer, if '_char' does not occur in the word.
    /// Otherwise follow the 'true' pointer, if it does.
    ////////////////////////////////////////////////////////////////////////////
    ptr _children[2] = { nullptr, nullptr }; // <-- TODO: variable out-degree

    ////////////////////////////////////////////////////////////////////////////
    /// \brief
    ////////////////////////////////////////////////////////////////////////////
    std::unordered_set<string> _words;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Initialize a NIL node
    ////////////////////////////////////////////////////////////////////////////
    void init(char c, ptr f_ptr = nullptr, ptr t_ptr = nullptr)
    {
      assert(_char == NIL && c != NIL);
      _char = c;
      _children[false] = f_ptr ? f_ptr : std::make_shared<node>();
      _children[true]  = t_ptr ? t_ptr : std::make_shared<node>();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Empty (NIL) constructor
    ////////////////////////////////////////////////////////////////////////////
    node() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Empty (NIL) ptr constructor
    ////////////////////////////////////////////////////////////////////////////
    static ptr make_node()
    { return std::make_shared<node>(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ////////////////////////////////////////////////////////////////////////////
    node(const node&) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Non-nil ptr constructor
    ////////////////////////////////////////////////////////////////////////////
    node(const char c, ptr f_ptr = make_node(), ptr t_ptr = make_node())
    { init(c, f_ptr, t_ptr); };

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Non-nil constructor
    ////////////////////////////////////////////////////////////////////////////
    static ptr make_node(char c, ptr f_ptr = make_node(), ptr t_ptr = make_node())
    { return std::make_shared<node>(c, f_ptr, t_ptr); }

  public:
    string to_string()
    {
      std::stringstream ss;
      ss << "{ char: " << _char << ", children: { " << _children[false] << ", " << _children[true] << " }, words [ ";
      for (const string &w : _words) {
        ss << w << " ";
      }
      ss << "] }";
      return ss.str();
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Recursively inserts a word into the anatree.
  //////////////////////////////////////////////////////////////////////////////
  node::ptr insert_word(const node::ptr& p,
                        const string& w,
                        string::iterator curr_char,
                        const string::iterator end)
  {
    assert(p != nullptr);

    // Case: Iterator done
    // -> Insert word
    if (curr_char == end) {
      p->_words.insert(w);
      return p;
    }

    // Case: NIL
    // -> Turn into non-NIL node
    if (p->_char == node::NIL) {
      assert(p->_children[false] == nullptr && p->_children[true] == nullptr);
      p->init(*curr_char);
      _size += 2;
      p->_children[true] = insert_word(p->_children[true], w, ++curr_char, end);
      return p;
    }

    // Case: Iterator behind
    // -> Insert new node in-between
    if (*curr_char < p->_char) {
      const node::ptr np = node::make_node(*curr_char, p, node::make_node());
      np->_words = p->_words;
      p->_words = std::unordered_set<string>();
      _size += 1;
      np->_children[true]  = insert_word(np->_children[true], w, ++curr_char, end);
      return np;
    }

    // Case: Iterator ahead
    // -> Follow 'false' child
    if (p->_char < *curr_char) {
      p->_children[false] = insert_word(p->_children[false], w, curr_char, end);
      return p;
    }

    // Case: Iterator and node matches
    // -> Follow 'true' child
    p->_children[true] = insert_word(p->_children[true], w, ++curr_char, end);
    return p;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Recursively gets all word on the path that matches the iterator.
  //////////////////////////////////////////////////////////////////////////////
  std::unordered_set<string> get_words(const node::ptr p, string::iterator curr, const string::iterator end) const
  {
    // Case: Iterator or Anatree is done
    if (curr == end || p->_char == node::NIL) {
      return p->_words;
    }

    // Case: Iterator behind
    // -> Insert new node in-between
    if (*curr < p->_char) {
      // Skip missing characters.
      while (*curr < p->_char && curr != end) { ++curr; }
      return get_words(p, curr, end);
    }

    // Case: Iterator ahead
    // -> Follow 'false' child
    if (p->_char < *curr) {
      std::unordered_set<string> ret(p->_words);
      std::unordered_set<string> rec = get_words(p->_children[false], curr, end);
      ret.insert(rec.begin(), rec.end());
      return ret;
    }

    // Case: Iterator and node matches
    // -> Follow both children, merge results and add words on current node
    ++curr;

    std::unordered_set<string> ret(p->_words);
    std::unordered_set<string> rec_false = get_words(p->_children[false], curr, end);
    std::unordered_set<string> rec_true = get_words(p->_children[true], curr, end);
    ret.insert(rec_false.begin(), rec_false.end());
    ret.insert(rec_true.begin(), rec_true.end());
    return ret;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Recursively obtain all words at the leaves.
  //////////////////////////////////////////////////////////////////////////////
  std::unordered_set<string> get_leaves(const node::ptr p) const
  {
    std::unordered_set<string> ret;
    if (p->_char == node::NIL) {
      if (p->_words.size() > 0) {
        ret.insert(*p->_words.begin());
      }
      return ret;
    }

    std::unordered_set<string> rec_false = get_leaves(p->_children[false]);
    std::unordered_set<string> rec_true = get_leaves(p->_children[true]);
    ret.insert(rec_false.begin(), rec_false.end());
    ret.insert(rec_true.begin(), rec_true.end());
    return ret;
  }

private:
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Root of the anatree (initially a NIL pointer).
  //////////////////////////////////////////////////////////////////////////////
  node::ptr _root = node::make_node();
  size_t _size = 1u;

public:
  anatree() = default;
  anatree(const anatree&) = default;

private:
  //////////////////////////////////////////////////////////////////////////////
  /// \brief
  //////////////////////////////////////////////////////////////////////////////
  string sorted_string(const string& w) const
  {
    string ret(w);
    std::sort(ret.begin(), ret.end()); // <-- TODO: frequency-based ordering?
    return ret;
  }

public:
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Adds the word 'w' to the anatree.
  //////////////////////////////////////////////////////////////////////////////
  void insert(const string& w)
  {
    string key = sorted_string(w);
    _root = insert_word(_root, w, key.begin(), key.end());
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain all words that are anagrams of 'w'.
  //////////////////////////////////////////////////////////////////////////////
  std::unordered_set<string> keys() const
  {
    return get_leaves(_root);
  }


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain all words that are anagrams of 'w'.
  //////////////////////////////////////////////////////////////////////////////
  std::unordered_set<string> anagrams_of(const string& w) const
  {
    string key = sorted_string(w);
    return get_words(_root, key.begin(), key.end());
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Remove all nodes/anagrams.
  //////////////////////////////////////////////////////////////////////////////
  void erase()
  {
    _root = node::make_node();
    _size = 1u;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Number of nodes.
  //////////////////////////////////////////////////////////////////////////////
  size_t size()
  {
    return _size;
  }
};

#endif // ANATREE_H
