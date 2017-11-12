#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <set>
#include <algorithm>
#include <map>
#include <queue>
#include <math.h>
#include <assert.h>
#include <string.h>
using namespace std; // Sorry for this!

//#define PROFILE

//========================
//   Utilities
//========================

// Compare the first n elements of two vectors
template <class E>
bool vec_n_eq(const vector<E> &v1, const vector<E> &v2, size_t n) {
  size_t l1 = v1.size(), l2 = v1.size();
  if (l1 != l2 && (l1 < n || l2 < n))
	return false;
  if (l1 < n)
	n = l1;
  for (size_t i = 0; i < n; ++i)
	if (v1[i] != v2[i])
	  return false;
  return true;
}

template <class T> class ConstSmartPtr {
  T *p;

public:
  ConstSmartPtr(T *_p) : p(_p) { ++_p->nuser; }
  ConstSmartPtr(const ConstSmartPtr<T> &_p) : p(_p.p) { ++p->nuser; }
  ~ConstSmartPtr() {
	if (!--p->nuser)
	  delete p;
  }
  T *operator->() { return p; }
};

void reportDataNz(int val, const char *message) {
  if (val)
	cerr << "Info: " << message << ": " << val << endl;
}

int NumberOfSetBits(int i) {
  i = i - ((i >> 1) & 0x55555555);
  i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
  return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

// returns nxt to x value not containing any bit that's set in msk
inline int masked_inc(unsigned int x, unsigned int msk) {
  return ((x | msk) + 1) & ~msk;
}

// count trailing zeros
int ctz(unsigned int x) {
  if (x == 0)
	return 32;
  int n = 0;
  if ((x & 0x0000FFFF) == 0)
	n = n + 16, x = x >> 16;
  if ((x & 0x000000FF) == 0)
	n = n + 8, x = x >> 8;
  if ((x & 0x0000000F) == 0)
	n = n + 4, x = x >> 4;
  if ((x & 0x00000003) == 0)
	n = n + 2, x = x >> 2;
  if ((x & 0x00000001) == 0)
	n = n + 1;
  return n;
}

int clz(unsigned int x) {
  if (x == 0)
	return 32;
  int n = 0;
  if ((x & 0xFFFF0000) == 0)
	n = n + 16, x = x << 16;
  if ((x & 0xFF000000) == 0)
	n = n + 8, x = x << 8;
  if ((x & 0xF0000000) == 0)
	n = n + 4, x = x << 4;
  if ((x & 0xC0000000) == 0)
	n = n + 2, x = x << 2;
  if ((x & 0x80000000) == 0)
	n = n + 1;
  return n;
}

inline double log_approx(double x) {
  if (x < 1.)
	return 0;
  int xn = x; // truncated
  int pow2 = 31 - clz(x);
  return pow2 + (x - (1 << pow2)) / (1 << pow2);
}

// Splits a string into an array
std::vector<std::wstring> split(const std::wstring &s, wchar_t delim,
								bool empty_ok = false) {
  std::vector<std::wstring> elems;
  std::wstringstream ss(s);
  std::wstring item;
  while (std::getline(ss, item, delim)) {
	if (empty_ok || !item.empty())
	  elems.push_back(item);
  }
  return elems;
}

// removes leading and trailing spaces
template <class S>
static void trim(S &s, const typename S::value_type *spaces) {
  int lp = s.find_last_not_of(spaces);
  int fp = s.find_first_not_of(spaces);
  if (lp != std::string::npos)
	s.erase(lp + 1, std::string::npos);
  if (fp != std::string::npos)
	s.erase(0, fp);
}
inline void trim(std::string &s) { trim(s, " \r\t\n"); }
inline void trim(std::wstring &s) { trim(s, L" \r\t\n"); }

template <class E> struct more {
  bool operator()(const E &x, const E &y) { return x > y; }
};

inline wchar_t wproc(wchar_t c) { return towlower(c); }

// Unicode string to wide character one
std::wstring u2w(const char *s) {
  std::wstringbuf sbuf;
  int more = -1;
  wchar_t sumb = 0;
  char b;
  while ((b = *s) != 0) {
	/* Decode byte b as UTF-8, sumb collects incomplete chars */
	if ((b & 0xc0) == 0x80) {		  // 10xxxxxx (continuation byte)
	  sumb = (sumb << 6) | (b & 0x3f); // Add 6 bits to sumb
	  if (--more == 0)
		sbuf.sputc(wproc(sumb));	 // Add char to sbuf
	} else if ((b & 0x80) == 0x00) { // 0xxxxxxx (yields 7 bits)
	  sbuf.sputc(wproc(b));		  // Store in sbuf
	} else if ((b & 0xe0) == 0xc0) { // 110xxxxx (yields 5 bits)
	  sumb = b & 0x1f;
	  more = 1;					  // Expect 1 more byte
	} else if ((b & 0xf0) == 0xe0) { // 1110xxxx (yields 4 bits)
	  sumb = b & 0x0f;
	  more = 2;					  // Expect 2 more bytes
	} else if ((b & 0xf8) == 0xf0) { // 11110xxx (yields 3 bits)
	  sumb = b & 0x07;
	  more = 3;					  // Expect 3 more bytes
	} else if ((b & 0xfc) == 0xf8) { // 111110xx (yields 2 bits)
	  sumb = b & 0x03;
	  more = 4;						  // Expect 4 more bytes
	} else /*if ((b & 0xfe) == 0xfc)*/ { // 1111110x (yields 1 bit)
	  sumb = b & 0x01;
	  more = 5; // Expect 5 more bytes
	}
	/* We don't test if the UTF-8 encoding is well-formed */
	++s;
  }
  return sbuf.str();
}

// Wide character string to unicode one
std::string w2u(const wchar_t *s) {
  std::stringbuf sbuf;
  int more = -1;
  wchar_t b;
  while ((b = *s++) != 0) {
	if ((b & 0xff80) == 0)
	  if (b < 32)
		sbuf.sputc(b);
	  else
		sbuf.sputc(b);
	else {
	  // sbuf.sputc('?');
	  sbuf.sputc(0xc0 | (b & 0x07c0) >> 6);
	  sbuf.sputc(0x80 | (b & 0x3f));
	}
  }
  return sbuf.str();
}

std::string w2u(const wstring &s) { return w2u(s.c_str()); }

template <class F>
class FieldMap : public map<string, bool (*)(const wchar_t *, F *)> {};

// Reads a line from a file of any end-of-line convention
// Copied from
// http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf/6089413#6089413
std::istream &safeGetline(std::istream &is, std::string &t) {
  t.clear();

  // The characters in the stream are read one-by-one using a std::streambuf.
  // That is faster than reading them one-by-one using the std::istream.
  // Code that uses streambuf this way must be guarded by a sentry object.
  // The sentry object performs various tasks,
  // such as thread synchronization and updating the stream state.

  std::istream::sentry se(is, true);
  std::streambuf *sb = is.rdbuf();

  for (;;) {
	int c = sb->sbumpc();
	switch (c) {
	case '\n':
	  return is;
	case '\r':
	  if (sb->sgetc() == '\n')
		sb->sbumpc();
	  return is;
	case EOF:
	  // Also handle the case when the last line has no line ending
	  if (t.empty())
		is.setstate(std::ios::eofbit);
	  return is;
	default:
	  t += (char)c;
	}
  }
}

// Reads a file containing both plain text line and records of "key:value" type
template <class F>
void read_all_lines(const char *filename,
					map<wstring, bool (*)(const wchar_t *, F *)> &fieldMap,
					F *out_rec, vector<wstring> &out_plain) {
  ifstream ifs;
  string stxtline;
  wstring txtline;
  F *curr = 0;

  ifs.open(filename);
  if (!ifs.is_open()) {
	wcerr << L"Unable to open file" << endl;
	return;
  }
  // We are going to read an UTF-8 file
  while (safeGetline(ifs, stxtline)) {
	txtline = u2w(stxtline.c_str());
	trim(txtline);
	size_t j = txtline.find(L':');
	if (j == wstring::npos)
	  out_plain.push_back(txtline);
	else {
	  wstring key(txtline.substr(0, j));
	  j++;
	  while (txtline.length() > j && iswspace(txtline[j]))
		j++;
	  typename map<wstring, bool (*)(const wchar_t *, F *)>::iterator fieldI =
		  fieldMap.find(key);
	  if (fieldI != fieldMap.end() && out_rec)
		fieldI->second(txtline.substr(j, wstring::npos).c_str(), out_rec);
	  // else cerr << "Warning: \"" << w2u(key) << "\" record type is
	  // unknown\n";
	}
  }
}

template <typename T> struct ptr {
  //	const T* operator()(const  T& t) const {return &t;}
  T *operator()(T &t) const { return &t; }
};

template <class T, std::wstring T::*P> bool setField(const wchar_t *s, T *t) {
  t->*P = s;
  return true;
}

typedef enum {
  EMPTY_T,
  AUTO_BREAK_T,
  AUTO_UNIQ_T,
  BREAK_T,
  UNIQ_T,
  VOWEL_T = 32,
  CONS_T = 64,
  IGNORE_T = 128,
  REJECT_T,
  STOP_T
} TokTypeE;

//========================
//   Phoneme stuff
//========================

#define HARD_MAX_CONTRASTIVE_LEVEL 4

struct phon {
  wstring rep;
  int cardinality; // Number of different refinements for an abstracted phoneme
  int markup; // is this a markup quasi-phoneme
  bool uniq,	   // phoneme isn't generalizable
	  VCset,	   // vowel/consonant class is defined for this phoneme
	  VCsetAuto;   // was vowel/consonant class deried in an utomated way
  unsigned VCbits; // 0 - consonant, 1 - vowel.
  // Senior bits are intended for possible future use for bearing further
  // phoneme classification (say, 2 - rare consonant)
  int freq; // phoneme frequency in the vocabulary
  static phon **
	  VCabstractions; // Array of abstracted phonemes for various VCbit values
  static phon *PP_BEGIN;
  static phon *PP_END;
  static phon *
	  PP_DICT_MARKER; // Marker for a word that occurs to hit a known lexeme
  phon(const wstring &r, int mu = 0)
	  : rep(r), cardinality(0), uniq(false), VCset(false), VCsetAuto(false),
		VCbits(0u), freq(0), markup(mu) {}
  bool isVowel() { return VCset && (VCbits & 1); }
  bool isConsonant() { return VCset && !(VCbits & 1); }
  bool isVC() { return VCset; }
  void setVowel() {
	if (!isVowel()) {
	  uniq = false;
	  VCset = true;
	  VCbits = 1;
	}
  }
  void setConsonant() {
	if (!isConsonant()) {
	  uniq = false;
	  VCset = true;
	  VCbits = 0;
	}
  }
  bool isUniq() { return uniq; }
  bool setUniq() {
	uniq = true;
	VCset = false;
  }
  phon *abstracted() { return isVC() ? VCabstractions[VCbits] : 0; }
  bool abstractable() { return abstracted() != 0; }
};

typedef vector<phon *> Gram;

// Gram to wstring converter
wstring g2w(const Gram &x) {
  wstringbuf sbuf;
  wostream w(&sbuf);
  for (int i = 0; i < x.size(); ++i) {
	const wstring &s(x[i]->rep);
	sbuf.sputn(s.c_str(), s.length());
  }
  return sbuf.str();
}

struct Lexeme {
  wstring syl; // Syllables (currently unused)
  wstring lx; // Lexeme headword
  wstring CV; // Consonant/Vovel pattern (currently unused)
  typedef std::map<std::wstring, bool (*)(const wchar_t *, Lexeme *)> M;
  static M m;

  static M &fillMap() {
	if (m.empty()) {
	  m.insert(M::value_type(L"lx", &setField<Lexeme, &Lexeme::lx>));
	  m.insert(M::value_type(L"CV", &setField<Lexeme, &Lexeme::CV>));
	  m.insert(M::value_type(L"syl", &setField<Lexeme, &Lexeme::syl>));
	}
	return m;
  }
};

// change?
struct LexCompare {
  bool operator()(const Gram &g1, const Gram &g2) const {
	int l1 = g1.size(), l2 = g2.size(), l = min(l2, l1);
	int i = 0;
	for (; i < l; ++i) {
	  if (g1[i]->rep < g2[i]->rep)
		return true;
	  if (g2[i]->rep < g1[i]->rep)
		return false;
	}
	if (i < l2)
	  return true;
	return false;
  }
};

// Piecewise & piesewise linear function implementation
struct Scale {
  enum InterpolationType { I_CONST, I_LINEAR };
  map<double, pair<double, InterpolationType> > m;
  double zero_y;
  double get(double x) const {
	map<double, pair<double, InterpolationType> >::const_iterator i =
		m.upper_bound(x);
	if (i == m.begin())
	  return zero_y;
	if (i == m.end())
	  return (--i)->second.first;
	double upper = i->second.first, upper_x = i->first;
	--i;
	double lower = i->second.first, lower_x = i->first;
	switch (i->second.second) {
	case I_LINEAR:
	  if (upper_x == lower_x)
		return lower;
	  // cerr << "Using Linear!"<<upper_x<<" "<<upper<<" "<<lower_x<<" " <<
	  // lower << " " << x<<
	  // (x-lower_x)/(upper_x-lower_x)*(upper-lower)+lower<<endl;
	  return (x - lower_x) / (upper_x - lower_x) * (upper - lower) + lower;
	case I_CONST:
	  return lower;
	}
  }
  void skip_sep(const char *&c, InterpolationType &interpolation) {
	// all non-alphanumerical chars are treated like separator
	while (*c && !isalnum(*c) && *c != '.' && *c != '-' && *c != ':' &&
		   !((*c == '/' || *c == '\\') && (interpolation = I_LINEAR, false)) &&
		   !((*c == '_') && (interpolation = I_CONST, false)))
	  ++c;
  }
  double read_num(const char *&c, const char *&s) {
	// skip_sep(c);
	char *e;
	double n = strtod(c, &e);
	if (c == e)
	  throw(string("Scale format error in \"") + s + "\": number expected, \"" +
			c + "\" found");
	c = e;
	return n;
  }

  void parse_insert(const char *s) {
	const char *c = s;
	InterpolationType itype = I_LINEAR;
	do {
	  skip_sep(c, itype);
	  double n = read_num(c, s);
	  skip_sep(c, itype);
	  if (*c == ':') {
		++c;
		m[n] = pair<double, InterpolationType>(read_num(c, s), itype);
	  } else
		zero_y = n;
	} while (*c);
  }
  Scale(double d) : zero_y(d) {}
};

// Configuration

// Abstracted NGram probability (P) smoothing mode
enum SmoothingMode {
  NO_SMOOTHING,	   // P ~ f * c
  LOG64_SMOOTHING,	  // P ~ log2(64 + c)
  BALANCED_SMOOTHING, // P ~ f
  KNESSER_NEY
};
// where f is abstraction factor for the NGram, c is NGram's frequency

enum InputMode { RECORD_PER_LINE, WHOLE_FILE_AS_ONE_LEXEME };

// Output word sort order
enum SortOrder { HISTORICAL_ORD, ALPHA_ORD, SCORE_ORD };

struct WGConfig {
  string lang;
  int max_ng; // Maximum NGram considered
  int syl_max; // Maximum num of quasi-syllables in a produced word
  double alpha_factor; // Probability multiplier applied per each abstracted
					   // phoneme except one
  string gen_log_name; // debug log file name
  long random_seed;
  Scale factor_by_confidence;
  int ngen; // Desired number of valid generated words
  int verbose;
  bool top_list, display_score;
  bool allow_hits, // Allow generated words hitting the training vocabulary
	  checker; // Mark such words with phon::PP_DICT_MARKER suffix
  double min_score; // Minimum acceptable probability score
  bool linearOnlyAbstraction; // Only allows contiguou prefix abstractions in
							  // NGrams
  // For example, abcde -> Vbcde, VCcde,VCCde, NOT aCcde
  SortOrder sort_order;
  double prune; // Minimum acceptable NGram confidence (prune otherwise)
  SmoothingMode smoothing_mode;
  double smoothing_delta; // Knesser Ney smoothing factor
  int preserve_ng_tail; //# of suffix phonemes exempt of abstraction when
						//producing next phoneme
  bool evaluate_precision;
  WGConfig()
	  : lang(), max_ng(5), syl_max(1024), alpha_factor(0.8), random_seed(-1L),
		ngen(100), verbose(0), top_list(false), allow_hits(false),
		min_score(0.), sort_order(ALPHA_ORD), checker(false),
		display_score(false), factor_by_confidence(.3),
		linearOnlyAbstraction(false), prune(0.0),
		smoothing_mode(BALANCED_SMOOTHING), smoothing_delta(0.9),
		preserve_ng_tail(2), evaluate_precision(false) {}
};

// We only use single variant now (the second one may be used for debugging)
const int VARIANTS = 2;

struct Selector;
template <class R, class I = int> struct Randomizer;
struct Proliferator;
// Selection struct stands for a possible transition from a prefix (N-1)Gram to
// a new (N-1)Gram
// For ex., 'abcd' (N-1)Gram -> producing 'e' -> 'bcde' (N-1)Gram, N=5.
struct Selection {
  int count[VARIANTS]; // Frequency of this (N-1)Gram->(N-1)Gram transition
					   // observed
  // in the training vocabulary
  double confidence, conf_factor;
  Selector *uffix; // Points to the respective Selector of 'new' (N-1)Gram
  Selection() : uffix(0), confidence(1.), conf_factor(1.) {
	fill(count, count + VARIANTS, 0);
  }
};

#define HARD_MAX_ABSTRACT 8
// Selector represents an (N-1) Gram and provides
// 1) A map of all observed Selections arising from its extension by a suffix
// phoneme
// 2) Pointers to available Selectors of the same (N-1)Gram but with one (more)
// abstracted phoneme
struct Selector {
  Selector *abstr[HARD_MAX_ABSTRACT]; // Pointers to more abstracted versions of
									  // (N-1)Gram
  // abstr[i] stands for a version with (N - preserve_ng_tail - i)th position
  // phoneme
  // replaced by its abstracted counterpart(say, V or C). (Here the beginning
  // position has number one).
  double confidence, conf_factor;
  int count; // Observed frequency of this (N-1)Gram in the training vocabulary
  Gram ngram; // for debug/reporting only - we don't use it while travesing a
			  // chain of selection
  vector<Selector *>
	  contrib; // Matching selectors with one more concrete phoneme
  map<phon *, Selection>
	  nxt; // Selection for observed single suffix phoneme extensions
  Proliferator *p;
  Randomizer<phon *> *r;
  template <class Tgt> void copy_insert(Tgt &r) {
	for (map<phon *, Selection>::iterator it = nxt.begin(); it != nxt.end();
		 ++it) {
	  int score = it->second.count[0];
	  if (score > 0)
		r.insert(std::pair<int, phon *>(score, it->first));
	}
  }
  Selector(Gram g)
	  : ngram(g), confidence(1.), conf_factor(1.), count(0), p(0), r(0) {
	fill(abstr, abstr + HARD_MAX_ABSTRACT, (Selector *)0);
  }
  double calcConfidence(const WGConfig &conf);
};

// Selector prefix tree node
struct StatTreeNode {
  map<phon *, StatTreeNode> nxt;
  Selector *select;
  StatTreeNode *treeNode(const Gram &x, int start, int end) {
	StatTreeNode *rc = this;
	for (int i = start; i < end; ++i)
	  rc = &rc->nxt[x[i]];
	return rc;
  }
  StatTreeNode() : select(0) {}
};
static set<Gram> dict_words, gen_words;
// A forest of concrete and abstracted (N-1)Grams prefix trees
// NGATRoot[bitmask=or(1<<j)] stores (N-1)Grams with abstracted phonemes at j
// positions
//(An NGATRoot[a] tree resident selector's abstr[j], if non-zero, points to an
// NGATRoot[w = a|(1<<j)] tree resident selector, w!=a. )
static StatTreeNode NGATRoot[1 << HARD_MAX_ABSTRACT];
static Selector *NGASelRoot = 0;

map<wstring, phon *> phonemes;

inline phon *update_phoneme(wstring s) {
  for (int i = 0; i < s.length(); ++i)
	if (s[i] == '~') {
	  s[i] = 0x0303;
	} // pseudodiacritic - hack;
  map<wstring, phon *>::iterator i = phonemes.find(s);
  if (i == phonemes.end()) {
	phon *r = new phon(s);
	phonemes[r->rep] = r;
	return r;
  }
  return i->second;
}

typedef map<phon *, StatTreeNode>::iterator StatTreeIter;

// Implements DFS search over Selector prefix tree
template <class Op> void statDFS(StatTreeNode &root, Op &op) {
  bool first_pass = true;
  vector<StatTreeIter> srcit, srcend;
  srcend.push_back(root.nxt.end());
  srcit.push_back(root.nxt.begin());
  do {
	if (srcend.back() != srcit.back()) {
	  if (!op.descend(srcit)) {
		++srcit.back();
		continue;
	  }
	  // descent
	  StatTreeNode &child = srcit.back()->second;
	  srcend.push_back(child.nxt.end());
	  srcit.push_back(child.nxt.begin());
	} else {
	  srcit.pop_back();
	  srcend.pop_back();
	  if (srcit.empty())
		break;
	  // action
	  op.ascend(srcit);
	  ++srcit.back();
	}
  } while (true);
}

// An element of phoneme-to-phoneme pair count statistics
struct CrossPhon {
  int count;
  int discount;
  typedef map<phon *, CrossPhon>::value_type XT;
  map<CrossPhon *, int> xphon;
  CrossPhon() : count(0), discount(0) {}
  static int lessCountPairPtr(const XT *x1, const XT *x2) {
	return x1->second.count < x2->second.count;
  }
};

// Does a phoneme _might_ be a Vowel or Consonant?
inline bool VCeligible(const phon *p) { return !p->markup && !p->uniq; }

// Collect pairs of word's (beginning,ending) phonemes from vocabulary
// statistics
bool collectWordLoopingPairs(map<phon *, CrossPhon> &out) {
  for (set<Gram>::iterator w = dict_words.begin(), we = dict_words.end();
	   we != w; ++w) {
	const Gram &word = *w;
	int b = 0, e = word.size() - 1;
	while (e > b && !VCeligible(word[e]))
	  --e;
	while (e > b && !VCeligible(word[b]))
	  ++b;
	if (e > b && word[e] != word[b]) {
	  CrossPhon *left = &out[word[b]], *right = &out[word[e]];
	  left->xphon.insert(pair<CrossPhon *, int>(right, 0)).first->second++;
	  right->xphon.insert(pair<CrossPhon *, int>(left, 0)).first->second++;
	}
  }
  return true;
}

// Collects pairs of adjacent phonemes from vocabulary statistics
class ContrastivePairCollector {
  map<phon *, CrossPhon> &out;
  unsigned int minorBits;

public:
  bool descend(vector<StatTreeIter> &) { return true; }
  void ascend(vector<StatTreeIter> &v) {
	if (Selector *s = v.back()->second.select)
	  for (map<phon *, Selection>::iterator right_i = s->nxt.begin(),
											right_e = s->nxt.end();
		   right_i != right_e; ++right_i)
		if (VCeligible(right_i->first)) {
		  vector<StatTreeIter>::reverse_iterator left_i = v.rbegin(),
												 left_e = v.rend();
		  while (left_i != left_e && (minorBits & ((*left_i)->first->VCbits ^
												   right_i->first->VCbits)))
			left_i++;
		  if (left_i != left_e && (*left_i)->first != right_i->first &&
			  VCeligible((*left_i)->first)) {
			int count = right_i->second.count[0];
			CrossPhon *left = &out[(*left_i)->first],
					  *right = &out[right_i->first];
			int c1, c2;
			c1 = left->xphon.insert(pair<CrossPhon *, int>(right, 0))
					 .first->second += count;
			c2 = right->xphon.insert(pair<CrossPhon *, int>(left, 0))
					 .first->second += count;
		  }
		}
  }
  void collect() { statDFS(NGATRoot[0], *this); }
  ContrastivePairCollector(map<phon *, CrossPhon> &_out, int _level)
	  : minorBits((1 << _level) - 1), out(_out) {}
};

// Auto detects missing Vovels/Consonants using Sukhotin's algorithm
//  (https://en.wikipedia.org/wiki/Sukhotin%27s_algorithm)
void computeVCbits() {
  // We only consider the least significnt bit
  //  (VCbits = 0 - Consonant vs. VCbits = 1 - Vowel)
  const int level = 0;

  map<phon *, CrossPhon> weights;
  // for the adjacent phoneme pair collection, use our n-gram statistics
  // rather than a direct word travesrsing, in order to save the performance
  ContrastivePairCollector(weights, level).collect();
  // treat a word's ending and beginning phonemes as adjacent in
  // a vowel vs. consonant detection
  collectWordLoopingPairs(weights);

  for (map<phon *, CrossPhon>::iterator i = weights.begin(), e = weights.end();
	   i != e; ++i)
	for (map<CrossPhon *, int>::iterator j = i->second.xphon.begin(),
										 f = i->second.xphon.end();
		 j != f; ++j)
	  if (i->first->VCbits & (1 << level))
		i->second.count -= j->second;
	  else {
		i->second.count += j->second;
	  }
  int nphonemes = weights.size();
  CrossPhon::XT *p[nphonemes];
  std::transform(weights.begin(), weights.end(), p, ptr<CrossPhon::XT>());
  std::make_heap(p, p + nphonemes, CrossPhon::lessCountPairPtr);

  while (nphonemes > 0 &&
		 p[0]->second.count > 0 /* || p[0]->second.discount */) {
	// cerr << w2u(p[0]-> first->rep) << " : " << p[0]->second.count << "-" <<
	// p[0]->second.discount <<")"<<endl;
	if (p[0]->second.discount) {
	  std::pop_heap(p, p + nphonemes, CrossPhon::lessCountPairPtr);
	  // Discount is negative here (so the real count is decreased).
	  // Thus, simply update the count and put the candidate vowel
	  // back to the heap for the future consideration. This seems to be an
	  // overhead, but in reward we save a lot of computations by avoiding
	  // keeping
	  // the ordering of phoneme counts constantly.
	  p[nphonemes - 1]->second.count += p[nphonemes - 1]->second.discount;
	  p[nphonemes - 1]->second.discount = 0;
	  std::push_heap(p, p + nphonemes, CrossPhon::lessCountPairPtr);
	} else {
	  // This one goes to the "less frequent phonemes" bin, in other words,
	  //   it becomes a vowel.
	  // Update pairwise statistics:
	  CrossPhon &p0s = p[0]->second;
	  if (p[0]->first != phon::PP_BEGIN) {
		for (map<CrossPhon *, int>::iterator j = p[0]->second.xphon.begin(),
											 f = p[0]->second.xphon.end();
			 j != f; ++j) {
		  assert(j->first != &p0s);
		  j->first->discount -= 2 * j->second;
		}
		if (!p[0]->first->VCset) {
		  p[0]->first->VCbits |= 1 << level;
		  p[0]->first->VCset = p[0]->first->VCsetAuto = true;
		}
	  }
	  // Pop and forget it.
	  pop_heap(p, p + nphonemes, CrossPhon::lessCountPairPtr);
	  nphonemes--;
	}
  }
  // Other phonemes remaining in the heap have non-positive final counts.
  // They are adopted as consonants.
  for (int i = 0; i < nphonemes; ++i) {
	p[i]->first->VCset = true;
  }
}

struct TokType {
  int start;
  int stop;
  int length() { return stop - start; }
  TokTypeE t;
  TokType(TokTypeE _t, int _start, int _stop)
	  : t(_t), start(_start), stop(_stop) {}
};

typedef map<wstring, vector<TokType> > CharCombMapT;

// Processes a phoneme list line read from dictionary file and adds respective
// records to dst
template <TokTypeE tok> bool addStrToks(const wchar_t *s, CharCombMapT *dst) {
  vector<wstring> st = split(s, ',');
  for (int i = 0; i < st.size(); ++i) {
	int nslash = 0;
	trim(st[i]);
	wstring sk;
	TokType t(tok, 0, 0);
	for (const wchar_t *p = st[i].c_str(); *p; p++)
	  if (*p == '~') {
		sk.push_back(0x0303); // pseudodiacritic - hack;
	  } else if (*p == L'/') {
		t.start = t.stop;
		t.stop = sk.length();
	  } else {
		sk.push_back(*p);
	  }
	if (nslash < 2)
	  t.stop = sk.length();
	if (nslash > 3)
	  cout << "Error: Too many slashes in \"" << w2u(st[i])
		   << "\" << phoneme specification";
	(*dst)[sk].push_back(t);
  }
  return 1;
}

// A map for word-to-phonemes
typedef map<wstring, bool (*)(const wchar_t *, CharCombMapT *)>
	CharCombFillMapT;
static CharCombFillMapT &charCombFillMap() {
  static map<wstring, bool (*)(const wchar_t *, CharCombMapT *)> m;
  if (m.empty()) {
	m.insert(CharCombFillMapT::value_type(L"vowels", &addStrToks<VOWEL_T>));
	m.insert(CharCombFillMapT::value_type(L"consonants", &addStrToks<CONS_T>));
	m.insert(CharCombFillMapT::value_type(L"extra", &addStrToks<UNIQ_T>));
	m.insert(CharCombFillMapT::value_type(L"ignore", &addStrToks<IGNORE_T>));
	m.insert(CharCombFillMapT::value_type(L"break", &addStrToks<BREAK_T>));
	m.insert(CharCombFillMapT::value_type(L"reject", &addStrToks<REJECT_T>));
	m.insert(CharCombFillMapT::value_type(L"stop", &addStrToks<STOP_T>));
  }
  return m;
}

void noRecordWarning(const char *missing, const char *keyWord) {
  cerr << "Warning: No " << missing << " specified. Please add a \"" << keyWord
	   << ": comma_separated_list\" line to the dictionary file" << endl;
}

static CharCombMapT charCombination;
static vector<wstring> headWord;

inline bool is_diacritic(wchar_t c) {
  switch (c & 0xfff0) {
  case 0x0300:
  case 0x0310:
  case 0x0320:
  case 0x0330:
  case 0x0340:
  case 0x0350:
  case 0x0360:
  // Combining Diacritical Marks (0300–036F), since version 1.0, with
  // modifications in subsequent versions down to 4.1
  case 0x1AB0:
  case 0x1AC0:
  case 0x1AD0:
  case 0x1AE0:
  case 0x1AF0:
  // Combining Diacritical Marks Extended (1AB0–1AFF), version 7.0
  case 0x1DC0:
  case 0x1DD0:
  case 0x1DE0:
  case 0x1DF0:
  // Combining Diacritical Marks Supplement (1DC0–1DFF), versions 4.1 to 5.2
  case 0x20D0:
  case 0x20E0:
  case 0x20F0:
  // Combining Diacritical Marks for Symbols (20D0–20FF), since version 1.0,
  // with modifications in subsequent versions down to 5.1
  case 0xFE20:
	// Combining Half Marks (FE20–FE2F), versions 1.0, updates in 5.2
	return true;
  default:
	return false;
  }
}

inline int is_letter_continuation(wchar_t c) { return is_diacritic(c); }

inline void scanCharCombination(const wchar_t *s, TokType **best_types) {
  size_t confirmed_len = 0, best = 0, j = 0;
  wstring p;
  while (confirmed_len >= j) {
	p.push_back(s[j]);
	if (j <= confirmed_len) {
	  CharCombMapT::iterator it = charCombination.lower_bound(p);

	  if (it != charCombination.end()) {
		const wchar_t *cs = it->first.c_str();
		// extend the match length
		confirmed_len = 0;
		while (cs[confirmed_len] == s[confirmed_len] && cs[confirmed_len])
		  ++confirmed_len;
		if (!cs[confirmed_len] &&
			!is_letter_continuation(s[confirmed_len])) { // exact match
		  for (int i = 0; i < it->second.size(); ++i) {
			if (!best_types[it->second[i].start] ||
				it->second[i].length() >
					best_types[it->second[i].start]->length())
			  best_types[it->second[i].start] = &it->second[i];
		  }
		}
	  }
	}
	++j;
  }
  s += best;
  best_types += best;
  return;
}

void dictionaryWarnings() {
  int has_vowel = false, has_consonant = false;
  vector<string> au;
  for (CharCombMapT::iterator it = charCombination.begin();
	   it != charCombination.end(); ++it)
	for (int i = 0; i < it->second.size(); ++i) {
	  if (it->second[i].t == VOWEL_T)
		has_vowel++;
	  if (it->second[i].t == CONS_T)
		has_consonant++;
	  if (it->second[i].t == AUTO_UNIQ_T)
		au.push_back(w2u(it->first));
	}
  if (!has_vowel)
	noRecordWarning("vowel", "Vowels");
  if (!has_consonant)
	noRecordWarning("consonant", "Consonants");
  if (has_vowel && has_consonant && !au.empty()) {
	cerr << "Warning: Letters ";
	for (int i = 0; i < au.size(); ++i)
	  cerr << (i > 0 ? ", " : "") << au[i];
	cerr << " aren't classified in the dictionary file. Please add each of "
			"them to one of the following records: \"Vowels:\", "
			"\"Consonants:\" or \"Extra:\"" << endl;
  }
}

void reportAutoClassifiedPhonemes() {
  int j = 0;
  for (map<std::wstring, phon *>::iterator it = phonemes.begin();
	   it != phonemes.end(); ++it)
	if (it->second->VCsetAuto && it->second->VCbits) {
	  if (!j++)
		cerr << "Info: Phonemes classified as vowels with Sukhotin algorithm: ";
	  else
		cerr << ", ";
	  cerr << w2u(it->second->rep);
	}
  if (j)
	cerr << endl;
}

inline void skip_to(const wchar_t term, const wchar_t *&e) {
  const wchar_t *p = wcschr(e, term);
  if (p)
	e = p + 1;
}

inline void skip_enclosed(const wchar_t *&e) {
  switch (*e) {
  case L'(':
	skip_to(L')', e);
	break;
  case L'[':
	skip_to(L']', e);
	break;
  case L'{':
	skip_to(L'}', e);
	break;
  case L'<':
	skip_to(L'>', e);
	break;
  }
}

// A table of probability multipliers applied to an NGram with i abstracted
// phonemes:
// P(i) = factors[i]
static double *factors = 0;

int phoneme_decompose(vector<Gram> &outv /*out, cumulative*/, const wchar_t *s,
					  bool ins_end_marks) {
  size_t size0 = outv.size();
  Gram out;
  int sl = wcslen(s);
  TokType **best_types_a = new TokType *[sl], **best_types = best_types_a;
  fill(best_types, best_types + sl, (TokType *)0);

  if (ins_end_marks)
	out.push_back(phon::PP_BEGIN);
  while (*s) {
	TokTypeE kind = AUTO_UNIQ_T;
	const wchar_t *e = s;
	scanCharCombination(e, best_types);
	if (best_types[0]) {
	  int l = best_types[0]->length();
	  kind = best_types[0]->t;
	  assert(l > 0);
	  e += l;
	} else // prefix not found in the table
	{
	  skip_enclosed(e);
	  while (*e == L'\'' || *e == L'-')
		e++;
	  if (e == s) {
		int len = 1;
		if (true || iswalpha(*e)) {
		  while (is_letter_continuation(e[len]))
			len++; // diacritic;
		  charCombination[wstring(e, len)].push_back(
			  TokType(kind = AUTO_UNIQ_T, 0, len));
		} else
		  kind = AUTO_BREAK_T;
		e += len;

	  } else {
		kind = /*AUTO_BREAK_T; */ AUTO_UNIQ_T;
	  }
	}
	best_types += (e - s);
	switch (kind) {
	case IGNORE_T:
	  break;
	case REJECT_T:
	  return outv.size() - size0;
	  break;
	case STOP_T:
	  s = L""; // stop the reading
	  break;
	case VOWEL_T:
	  out.push_back(update_phoneme(wstring(s, e - s)));
	  out.back()->setVowel();
	  break;
	case CONS_T:
	  out.push_back(update_phoneme(wstring(s, e - s)));
	  out.back()->setConsonant();
	  break;
	case UNIQ_T:
	case AUTO_UNIQ_T:
	  out.push_back(update_phoneme(wstring(s, e - s)));
	  if (kind == UNIQ_T || !iswalpha(*s))
		out.back()->setUniq();
	  break;
	case BREAK_T:
	case AUTO_BREAK_T:
	  if (out.size() > !!ins_end_marks) {
		if (ins_end_marks)
		  out.push_back(phon::PP_END);
		outv.push_back(out); // opt?
		out.clear();
		if (ins_end_marks)
		  out.push_back(phon::PP_BEGIN);
	  }
	  break;
	}

	s = e;
  }

  if (out.size() > !!ins_end_marks) {
	if (ins_end_marks)
	  out.push_back(phon::PP_END);
	outv.push_back(out); // opt?;
  }
  return outv.size() - size0;
}

ostream &operator<<(ostream &o, const std::vector<phon *> &x) {
  for (int i = 0; i < x.size(); ++i)
	o << w2u(x[i]->rep);
  return o;
}
static int rejected_gen_hit = 0, rejected_dict_hit = 0, rejected_low_score = 0,
		   rejected_too_long = 0;
static int nPruned = 0, nGenPhon = 0;

// Count quasi-syllables (Vowels that follow a Consonant immediately)
static int quasiSylCount(const Gram &w) {
  bool prev_vowel = false;
  int rc = 0;
  for (int i = 0; i < w.size(); ++i)
	if (w[i]->isVowel()) {
	  if (!prev_vowel)
		++rc;
	  prev_vowel = true;
	} else
	  prev_vowel = false;
  return rc;
}

// Filter out repeated words and ones those don't comply to configuration
// requirements
inline bool filter(const map<Gram, double, LexCompare> &generated,
				   const WGConfig &conf, double score,
				   Gram &word /* may modify */) {
  if (conf.syl_max < 100 && conf.syl_max < quasiSylCount(word)) {
	++rejected_too_long;
	return false;
  }

  // Use it only in normal mode?
  if (generated.find(word) != generated.end()) {
	++rejected_gen_hit;
	return false;
  }

  if (conf.min_score > score) {
	++rejected_low_score;
	return false;
  }

  if (!conf.allow_hits || conf.checker) {
	bool found = (dict_words.find(word) != dict_words.end());
	if (found)
	  if (!conf.allow_hits) {
		++rejected_dict_hit;
		return false;
	  } else
		word.push_back(phon::PP_DICT_MARKER);
  }
  return true;
}

void report_rejected() {
  reportDataNz(rejected_too_long,
			   "# of words rejected due to the length exceed");
  reportDataNz(rejected_dict_hit,
			   "# of words rejected due to they are found in the dictionary");
  reportDataNz(rejected_gen_hit, "# of words rejected due to the repetition");
  reportDataNz(rejected_low_score,
			   "# of words rejected due to the low probability");
  reportDataNz(nPruned, "# of words pruned off in superconcrete mode");
}

// Count of rejected word generation attempts
int all_rejected() {
  return rejected_too_long + rejected_dict_hit + rejected_gen_hit +
		 rejected_low_score + nPruned;
}

static ofstream glog; // Correct!
typedef int AbstractionVector;

inline double range_factor(double x) { return x; }
inline int cardinality_sum(const Gram &g) {
  int rc = 0;
  for (int i = 0; i < g.size(); ++i)
	rc += g[i]->cardinality;
  return rc;
}

double Selector::calcConfidence(const WGConfig &conf) {
  double total = 0;
  int card = cardinality_sum(ngram); // optimize
  // int card = cardinality_sum(i->first);

  if (card > 0) {
	// take it out to be done once only
	for (int k = 0; k < this->contrib.size(); ++k) {
	  total += this->contrib[k]->confidence * this->contrib[k]->count;
	}
	float rcb = .0;
	if (total) {
	  double invMean = ((double)card) / total;
	  for (int k = 0; k < this->contrib.size(); ++k) {
		rcb += this->contrib[k]->confidence *
			   log_approx(1 +
						  this->contrib[k]->confidence *
							  this->contrib[k]->count * invMean);
	  }
	}

	double confidence = range_factor(rcb / /*log(2)/ */ card);
	this->confidence = confidence;
	// cerr << "rcb= " << rcb <<", card = "<<card<<" Confidence  = " <<
	// confidence << endl;
	this->confidence = confidence;
	return this->conf_factor = conf.factor_by_confidence.get(confidence);
  } else {
	this->conf_factor = this->confidence = 1.;
	return 1.;
  }
}

// Builds NGATRoot[a] NGram tree based on more concrete NGrams found in
// NGATRoot[a&~(1<<j)] trees (those are assumed to be already built)

void abstractOneStep(AbstractionVector a, const WGConfig &conf) {
  bool first_pass = true;
  StatTreeNode *dstelem[conf.max_ng + 1];
  for (int pos = 0; pos < conf.max_ng - 1; ++pos)
	if ((a >> pos) & 1) {
	  map<phon *, StatTreeNode>::iterator srcend[conf.max_ng + 1],
		  srcit[conf.max_ng + 1];
	  srcend[0] = NGATRoot[a & ~(1 << pos)].nxt.end();
	  srcit[0] = NGATRoot[a & ~(1 << pos)].nxt.begin();
	  phon *absgram[conf.max_ng + 1];
	  int l = 0, la = 0;
	  do {
		if (srcend[l] != srcit[l]) {
		  if (pos == l) {
			absgram[pos] = srcit[l]->first->abstracted();
			if (!absgram[pos]) {
			  ++srcit[l];
			  continue;
			} // not an abstractable phoneme - skip
		  } else
			absgram[l] = srcit[l]->first;
		  // descent
		  srcit[l + 1] = srcit[l]->second.nxt.begin();
		  srcend[l + 1] = srcit[l]->second.nxt.end();

		  ++l;
		} else {
		  --l;
		  if (l < 0)
			break;
		  if (l < la)
			la = l;
		  // action
		  if (srcit[l]->second.select && l >= pos) {
			int abspos = l + 1 - conf.preserve_ng_tail - pos,
				maxabspos = min(conf.max_ng, l + 2) - conf.preserve_ng_tail;
			la = 0;
			while (la <= l) {
			  dstelem[la] =
				  &(la > 0 ? dstelem[la - 1] : &NGATRoot[a])->nxt[absgram[la]];
			  ++la;
			}
			if (!dstelem[l]->select)
			  dstelem[l]->select = new Selector(Gram(absgram, absgram + l + 1));
			if (maxabspos > abspos && abspos >= 0)
			  srcit[l]->second.select->abstr[abspos] = dstelem[l]->select;
			dstelem[l]->select->count += srcit[l]->second.select->count;
			for (map<phon *, Selection>::iterator i =
					 srcit[l]->second.select->nxt.begin();
				 i != srcit[l]->second.select->nxt.end(); ++i) {
			  Selection &e = dstelem[l]->select->nxt[i->first];
			  if (first_pass)
				e.count[0] += i->second.count[0];
			  if (i->second.uffix)
				if (maxabspos > abspos + 1 && abspos + 1 >= 0)
				  e.uffix = i->second.uffix->abstr[abspos + 1];
				else
				  e.uffix = i->second.uffix;
			}
			dstelem[l]->select->contrib.push_back(srcit[l]->second.select);
		  }
		  ++srcit[l];
		}
	  } while (true);
	  first_pass = false;
	}

  int l = 0;
  int card[conf.max_ng + 1];
  map<phon *, StatTreeNode>::iterator dstit[conf.max_ng + 1],
	  dstend[conf.max_ng + 1];
  dstit[0] = NGATRoot[a].nxt.begin();
  dstend[0] = NGATRoot[a].nxt.end();
  card[0] = 0;

  do {
	if (dstit[l] != dstend[l]) {
	  card[l] = (l > 0 ? card[l - 1] : 0) + dstit[l]->first->cardinality;
	  // descend
	  dstit[l + 1] = dstit[l]->second.nxt.begin();
	  dstend[l + 1] = dstit[l]->second.nxt.end();
	  ++l;
	} else {
	  // ascend
	  --l;
	  if (l < 0)
		break;
	  // action
	  Selector *el = dstit[l]->second.select;
	  if (el)
		el->calcConfidence(conf);
	  ++dstit[l];
	}
  } while (true);
}

// Build NGATRoot[*] NGram trees iteratively based on concrete NGrams found in
// NGATRoot[0]
inline void cascadeAbstract(const WGConfig &conf) {
  int fixed_suffix = conf.preserve_ng_tail;
  int max_abstract = max(fixed_suffix, conf.max_ng) - fixed_suffix;

  for (AbstractionVector a = 0U; a < 1 << max_abstract; ++a)
	abstractOneStep(a, conf);
}

// Fill NGATRoot[0] concrete NGram tree using the training vocabulary
void statCount(const WGConfig &conf, vector<Lexeme> &lex, int v) {
  size_t word_length_sum = 0;
  vector<Gram> words;
  for (int n = 0; n < lex.size(); ++n)
	phoneme_decompose(words, lex[n].lx.c_str(), true);
  for (int n = 0; n < words.size(); ++n) {
	// NGram stat count
	Gram &word = words[n];
	vector<phon *> gram;
	//	cout << w2u(lex[n].lx) << " => " << word << endl;
	if (word.size() > 2) {
	  Selection *p = 0;
	  for (int e = 0, b = 0; e < word.size();
		   ++e, e - b >= conf.max_ng ? ++b : 0) {
		Selector *&pp = NGATRoot[0U].treeNode(word, b, e)->select;
		if (!pp)
		  pp = new Selector(Gram(word.begin() + b, word.begin() + e));
		if (p)
		  p->uffix = pp;
		p = &pp->nxt[word[e]];
		++p->count[0];
		++pp->count;
	  }

	  dict_words.insert(Gram(word.begin() + 1, word.end() - 1));
	  word_length_sum += word.size() - 2;
	  for (int i = 1; i < word.size() - 1; ++i) {
		word[i]->freq++;
	  }
	}
  }

  NGASelRoot = NGATRoot[0U].treeNode(Gram(1, phon::PP_BEGIN), 0, 1)->select;
}

// Gram (vector of phonemes) to unicode string converter
inline string g2u(const Gram &s) {
  stringbuf rcs;
  for (int i = 0; i < s.size(); ++i) {
	const string &cnv(w2u(s[i]->rep));
	rcs.sputn(cnv.c_str(), cnv.length());
  }
  return rcs.str();
}

// Abstraction position bits of an abstracted NGram
// (this is the NGATRoot[] tree index where a given NGram ought to be stored)
inline AbstractionVector abstrVec(const Gram &gra) {
  AbstractionVector a = 0U;
  for (int i = 0; i < gra.size(); ++i)
	if (gra[i]->cardinality)
	  a |= 1 << i;
  return a;
}

// Probability factor for a given number of abstracted phonemes in a NGram
// and that NGram confidence based probability factor
inline double factor(int nabs, double conf_factor) {
  return factors[nabs] * conf_factor;
}

class NoOptions {};
template <typename I> inline I rand2num(long randNum, I maxValue);
template <> inline double rand2num(long randNum, double maxValue) {
  return ((double)randNum) / (((double)RAND_MAX) + 1) * maxValue;
}
template <> inline long rand2num(long randNum, long maxValue) {
  return randNum % maxValue;
}
template <> inline int rand2num(long randNum, int maxValue) {
  return randNum % maxValue;
}

template <class E, class I> struct Randomizer {
  map<I, E> scale;
  bool initialized;
  bool n2;
  I total_score;
  void insert(std::pair<I, E> x) {
	if (x.first > 0)
	  scale[total_score += x.first] = x.second;
  }
  pair<E, double> random() {
	if (!scale.size())
	  throw NoOptions();
	I sel = rand2num(rand(), total_score);
	typename map<I, E>::iterator seli = scale.upper_bound(sel);
	// cerr<<"Sel: "<<sel<<" / " << total_score << "\n";
	if (seli == scale.end()) {
	  cerr << "Upper bound failure\n";
	  exit(121);
	}
	I sel_score;
	if (seli == scale.begin())
	  sel_score = seli->first;
	else {
	  typename map<I, E>::iterator prev = seli;
	  prev--;
	  sel_score = seli->first - prev->first;
	}
	// cerr << "Selected @ score = " << sel_score << " of " << total_score <<
	// endl;
	return pair<E, double>(seli->second, ((double)sel_score) / total_score);
  }
  Randomizer() : initialized(false), total_score(0), n2(false) {}
};

static void initFactors(const WGConfig &conf) {
  int n = conf.max_ng - conf.preserve_ng_tail + 1;
  if (n <= 0)
	return;
  factors = new double[n + 1];
  factors[0] = 1.;
  if (n > 0)
	factors[1] = 1;
  for (int i = 1 + 1; i <= n; ++i)
	factors[i] = factors[i - 1] * conf.alpha_factor;
#define DENOMINATE_FACTORS_BY_COMINATIONS
#ifdef DENOMINATE_FACTORS_BY_COMINATIONS
  if (n > 0) {
	int combinations = n;
	factors[1] /= combinations;
	// calculate C(i,n) combination count where n=n
	// and discount factors
	for (int i = 2; i <= n; ++i) {
	  combinations *= (n - i + 1);
	  combinations /= i;
	  factors[i] /= combinations;
	}
  }
#endif // DENOMINATE_FACTORS_BY_COMINATIONS
}

static void initIgnore() {
  const wchar_t *ignore[] = {L"'", L"|", 0};
  for (const wchar_t **p = ignore; *p; ++p)
	charCombination[*p] = vector<TokType>(1, TokType(IGNORE_T, 0, 1));
}

const string selng(Selector *s) {
  return s ? w2u(g2w(s->ngram)).c_str() : string(">0<");
}
const string selngxx(Selector *s) {
  if (!s)
	return string();
  stringbuf sbuf;
  for (map<phon *, Selection>::iterator it = s->nxt.begin(); it != s->nxt.end();
	   ++it) {
	string ps = w2u(it->first->rep);
	for (int i = 0; i < ps.length(); ++i)
	  sbuf.sputc(ps[i]);
	if (!it->second.uffix)
	  sbuf.sputc('!');
	sbuf.sputc(',');
  }
  return sbuf.str();
}

//#define DEBUG_ADVANCE_SELECTORS

inline bool selectorGood(int selector_idx, const Selector *selector,
						 const WGConfig &conf) {
  if (!selector)
	return false;
  switch (conf.smoothing_mode) {
  default:
	if (!conf.linearOnlyAbstraction)
	  break;
  // no break
  case KNESSER_NEY:
	// Should follow that a "regexp" pattern:
	// /<abstracted or unabstractable>* <concrete>*/
	{
	  int pos = conf.max_ng - conf.preserve_ng_tail - 1;
	  while (pos >= 0 && !selector->abstr[pos])
		--pos;
	  if (pos >= 0 && (selector_idx & ((1 << pos << 1) - 1)))
		// some of bits i[0..pos] != 0 :
		// this means some abstracted positions are in the range
		return false;
	}
	break;
  }
  return true;
}

// Given a bunch of concrete and abstracted NGram Selectors known
// for some concrete prefix (N-1)Gram
//(possibly never observed in a vocabulary, in which case selectors[0]==0),
// produces a bunch of selectors arising after appending ph suffix phoneme
// Indices in v should be (are guaranteed to be) abstracted phoneme position bit
// patterns
// (the same as ones in NGATRoot[] array, see the comment there) before (after,
// respectively) a call.
void advance_selectors(vector<Selector *> &v /*in,out*/, phon *ph) {
  vector<Selector *> tgt(v.size(), (Selector *)0);
#ifdef DEBUG_ADVANCE_SELECTORS
  cerr << "Trying   " << w2u(ph->rep) << endl;
#endif // DEBUG_ADVANCE_SELECTORS
  int tgti;
  int sz = v.size();
  assert(NumberOfSetBits(sz) == 1);
  for (int i = 0; i < v.size(); ++i) {
	if (!tgt[tgti = (i << 1) & ~sz] && v[i]) {
	  map<phon *, Selection>::iterator it = v[i]->nxt.find(ph);
	  if (it != v[i]->nxt.end()) {
		tgt[tgti] = it->second.uffix;
#ifdef DEBUG_ADVANCE_SELECTORS
		cerr << "Advancing [" << i << "] " << selng(v[i]) << ":"
			 << selngxx(v[i]);
		cerr << " with " << w2u(ph->rep) << " to [" << (tgti) << "]";
		cerr << selng(tgt[tgti]) << ":" << selngxx(tgt[tgti]) << endl;
#endif
		// propagate abstracted
		unsigned int msk = tgti;
		if (tgt[tgti]) // ths should be  true unless ph==PP_END
		  for (unsigned int j = masked_inc(tgti, msk); j < sz;
			   j = masked_inc(j, msk)) {
			int ap = ctz(j);
			Selector *abstr = tgt[tgti | (j & ~(1 << ap))]->abstr[ap];
			if (abstr) {
			  tgt[tgti | j] = abstr;
#ifdef DEBUG_ADVANCE_SELECTORS
			  cerr << "Abstrcted target [" << (tgti | (j & ~(1 << ap)))
				   << "] sent to [" << (tgti | j) << "] ";
			  cerr << selng(tgt[tgti | j]) << ":" << selngxx(tgt[tgti | j])
				   << endl;
#endif
			} else
			  msk |= (1 << ap);
		  }
	  }
	}
  }
  swap(v, tgt);
}

typedef Randomizer<Selector *, double> AbstrRandomizer;
typedef multimap<int, phon *, more<int> > ProliferatorMap;
// A sorted by probability phoneme producer
struct Proliferator {
  ProliferatorMap m; // relative score -> suffix phoneme
  int total_score;
  void insert(std::pair<int, phon *> x) {
	total_score += x.first;
	m.insert(x);
  }
  Proliferator() : total_score(0) {}
};

// A multiplier applied to a probability score just to make it being of integer
// format
#define ABS_SCORE_GRANULARITY (12800)

// Effective non-normalized probability of using this Selector's abstracted or
// concrete
// (N-1)Gram prefix ( w.r.t. all available matching (N-1)Grams ) for random
// phoneme production
double abstractedProbability(Selector *s, const WGConfig &conf, int nabs,
							 double &remainder) {
  double factor = s->conf_factor * factors[nabs]; // account for alpha factor
  switch (conf.smoothing_mode) {
  case NO_SMOOTHING:
	return s->count * factor;
  case BALANCED_SMOOTHING:
	return ABS_SCORE_GRANULARITY * factor;
  case LOG64_SMOOTHING:
	return log2(64+s->count) * ABS_SCORE_GRANULARITY * factor;
  case KNESSER_NEY: {
	double probability = remainder;
	remainder *= s->nxt.size() * conf.smoothing_delta / s->count;
	return (probability - remainder) * factor * ABS_SCORE_GRANULARITY;
  }
  }
}

class AbstrRandomizerBuilder {
  static map<Gram, Randomizer<phon *> > cache;

public:
  AbstrRandomizer &target;

public:
  void insert_from(Selector *s, double score) {
	if (!s->r) {
	  s->r = new Randomizer<phon *>();
	  s->copy_insert(*s->r);
	  // cerr << "---"<<nabs<<"-"<<s->r->total_score<<endl;
	}
	if (score > 0.)
	  target.insert(std::pair<double, Selector *>(/*ng_total_*/ score, s));
  }
  AbstrRandomizerBuilder(AbstrRandomizer &t) : target(t) {}
};

double pruningProbability(const WGConfig &conf, double confidence) {
  return (confidence < conf.prune) ? 1. : 0.;
}

// A summary confidence level for a bunch of abstracted and/or
// concrete NGram selectors matching some produced (N-1)Gram
double assumedConfidence(const vector<Selector *> &selectors) {
  // Currently it's merely the maximum member confidence
  double confidence(0.0);
  for (int i = 0; i < selectors.size(); ++i)
	if (selectors[i] && selectors[i]->count)
	  confidence = std::max(confidence, selectors[i]->confidence);
  return confidence;
}

//Add c (which may also be negative) occurences of given NGram
// to the NGram statistics counts.
//Only suitable for NGrams ever observed in the training vocabulary.
// applyConf: recalculate confidence levels
double discount(Gram w, const WGConfig &conf, int c, bool applyConf) {
  size_t i = 0, l = w.size();
  vector<Selector *> selectors;
  selectors.assign(1 << max(conf.max_ng - conf.preserve_ng_tail, 0),
				   (Selector *)0);
  selectors[0] = NGASelRoot;

  for (; i < l; ++i) {
	for (int j = 0; j < selectors.size(); ++j)
	  if (selectors[j]) {
		selectors[j]->count += c, selectors[j]->nxt[w[i]].count[0] += c;
		if (applyConf)
		  selectors[j]->calcConfidence(conf);
	  }
	advance_selectors(selectors, w[i]);
  }
}

//Estimate all-branch probability of a given word w generation
double honestScore(Gram w, const WGConfig &conf, bool applyDiscount,
				   bool applyConf = false) {
  double s = 1.0;
  size_t i = 0, l = w.size();
  if (l > 0 && w[i] == phon::PP_BEGIN)
	++i;
  if (l > 0 && w[l - 1] == phon::PP_DICT_MARKER) {
	w.pop_back();
	--l;
  }
  if (l > 0 && w[l - 1] != phon::PP_END) {
	w.push_back(phon::PP_END);
	++l;
  }

  vector<Selector *> selectors; // check bounds?
  selectors.assign(1 << max(conf.max_ng - conf.preserve_ng_tail, 0),
				   (Selector *)0);
  selectors[0] = NGASelRoot;
  if (applyDiscount) {
	discount(w, conf, -1, applyConf);
  }
  for (; i < l; ++i) {
	double num = 0., denom = 0., remainder = 1.;

	for (int j = 0; j < selectors.size(); ++j)
	  if (selectorGood(j, selectors[j], conf) && selectors[j]->count) {
		map<phon *, Selection>::iterator it = selectors[j]->nxt.find(w[i]);
		if (it != selectors[j]->nxt.end()) {
		  int nabs = NumberOfSetBits(j);
		  double share =
			  abstractedProbability(selectors[j], conf, nabs, remainder);
		  denom += share;
		  num += share * it->second.count[0] / selectors[j]->count;
		}
	  }
	if (denom)
	  (s *= num) /= denom;
	//  cout << "NWays: "<< nways<<" for " << w2u(w[i]->rep)<<"; "<<num<<" / "
	//  << denom << ": "<<s<< endl ;
	if (i == l - 1)
	  break;
	advance_selectors(selectors, w[i]);
	if (conf.prune)
	  s *= (1 - pruningProbability(conf, assumedConfidence(selectors)));
	// if (glog) glog << "[Pruned off]."<<endl;
  }
  if (applyDiscount) {
	discount(w, conf, +1, applyConf);
  }
  return s;
}
static int lastSelI = -1;

pair<phon *, double> select_random(vector<Selector *> &v,
								   const WGConfig &conf) {
  AbstrRandomizer abstr_r;
  AbstrRandomizerBuilder b(abstr_r);
  double remainder = 1.0;

  for (int i = 0; i < v.size(); ++i)
	if (selectorGood(i, v[i], conf)) {
	  int nabs = NumberOfSetBits(i);
	  b.insert_from(v[i], abstractedProbability(v[i], conf, nabs, remainder));
	  if (glog)
		glog << "Considered ngram (" << i << ") " << g2u(v[i]->ngram)
			 << " @ score = "
			 << abstractedProbability(v[i], conf, nabs, remainder) << endl;
	}

  pair<Selector *, double> r = abstr_r.random();
  lastSelI = -1;
  for (int i = 0; i < v.size(); ++i)
	if (v[i] == r.first)
	  lastSelI = i;
  pair<phon *, double> ans = r.first->r->random();
  // if (trueScore) {for all (abstr_r) s+=scoreof();}
  return pair<phon *, double>(ans.first, r.second * ans.second);
}

void printNGrams(WGConfig &conf) {
  map<phon *, StatTreeNode>::iterator dstit[conf.max_ng + 1],
	  dstend[conf.max_ng + 1];
  for (int a = 0; a < 1 << max(conf.max_ng - conf.preserve_ng_tail, 0); ++a) {
	dstit[0] = NGATRoot[a].nxt.begin();
	dstend[0] = NGATRoot[a].nxt.end();
	int l = 0;

	do {
	  if (dstit[l] != dstend[l]) {
		// descend
		dstit[l + 1] = dstit[l]->second.nxt.begin();
		dstend[l + 1] = dstit[l]->second.nxt.end();
		++l;
	  } else {
		// ascend
		--l;
		if (l < 0)
		  break;
		// action
		cout << selng(dstit[l]->second.select) << ". ";
		for (int j = 0; j <= l; ++j)
		  cout << w2u(dstit[j]->first->rep);
		//		cout<<endl;
		if (dstit[l]->second.select) {
		  for (int j = 0; j < conf.max_ng - conf.preserve_ng_tail; j++)
			cout << "\t" << selng(dstit[l]->second.select->abstr[j]);
		  cout << "\t" << dstit[l]->second.select->conf_factor << "\t"
			   << dstit[l]->second.select->confidence << "\t"
			   << dstit[l]->second.select->count;
#define OUT_CONFIDENCE_EXPLANATION
#ifdef OUT_CONFIDENCE_EXPLANATION
		  cout << "(conf = ";

		  for (int i = 0; i < dstit[l]->second.select->contrib.size(); ++i)
			cout << dstit[l]->second.select->contrib[i]->count << "!"
				 << dstit[l]->second.select->contrib[i]->confidence << ";";
		  cout << ")";
#endif
		  cout << endl;
		  for (map<phon *, Selection>::iterator it =
				   dstit[l]->second.select->nxt.begin();
			   it != dstit[l]->second.select->nxt.end(); ++it)
			cout << "\t|" << w2u(it->first->rep) << "\t"
				 << selng(it->second.uffix) << endl;

		} else
		  cout << endl;
		++dstit[l];
	  }
	} while (true);
  }
}

pair<Gram, double> generateWord(const WGConfig &conf) {
  //	phonemes[L"$"]=phon::PP_END;//no hack
  Gram gram;
  string err;
  wstringbuf sbuf;
  pair<Gram, double> ws;
  wostream word(&sbuf);
  double sel_score = 1.0;
  gram.push_back(phon::PP_BEGIN);
  bool odd = false;
  vector<Selector *> selectors; // check bounds?
  selectors.assign(1 << max(conf.max_ng - conf.preserve_ng_tail, 0),
				   (Selector *)0);
  selectors[0] = NGASelRoot;
  while (gram.back() != phon::PP_END) {
	// keeping it at most max_ng-1 long
	if (gram.size() >= conf.max_ng)
	  gram.erase(gram.begin());
	try {
	  pair<phon *, double> sel = select_random(selectors, conf);
	  sel_score *= sel.second;
	  if (lastSelI >= 0 && glog)
		glog << "Selected (" << lastSelI << ") "
			 << g2u(selectors[lastSelI]->ngram) << endl;
	  nGenPhon++;
	  if (sel.first == phon::PP_END)
		break;
	  gram.push_back(sel.first);
	  advance_selectors(selectors, sel.first);
	  // if (!selectors[0] && !selectors[1] && conf.prune > 0. &&
	  // rand2num(random(),1.0) < conf.prune)
	  if (conf.prune > 0. &&
		  rand2num(random(), 1.0) <
			  pruningProbability(conf, assumedConfidence(selectors))) {
		if (glog)
		  glog << "[Pruned off]." << endl;
		nPruned++;
		err = string("Pruned at ") + w2u(sbuf.str());
		return pair<Gram, double>(Gram(), 0.0);
	  }
	  ws.first.push_back(sel.first);
	  word << sel.first->rep;
	} catch (NoOptions noe) {
	  err = string("Unsuccessful at ") + w2u(sbuf.str());
	  cerr << err << endl;
	  return pair<Gram, double>(Gram(), 0.0);
	}
  }
  ws.second = sel_score;
  return ws;
}

void classifyPhonemes() {
  computeVCbits();
  for (map<wstring, phon *>::iterator ph_i = phonemes.begin();
	   ph_i != phonemes.end(); ph_i++) {
	if (ph_i->second->abstractable())
	  ++ph_i->second->abstracted()->cardinality;
  }
}

struct CmdArgs {
  std::vector<std::string> files, ifiles;
  bool http;
  const char *prog;
  WGConfig &config;
  struct Input {
	InputMode mode;
	string name;
	Input(InputMode m, const string &n) : mode(m), name(n) {}
  };
  vector<Input> inputs;

  void usage_exit() {
	std::cerr << "Usage: " << prog
			  << " [-h] {-i init_file} {-l language} {-n max_ngram} {-a "
				 "alpha_factor} {-s} {-f alpha_factor} {-c count} [-H] [-P]"
			  << std::endl;
	exit(2);
  }
  void next(int &i, int argc) {
	if (++i >= argc)
	  usage_exit();
  }
  CmdArgs(int argc, char **argv, WGConfig &conf)
	  : prog(argv[0]), config(conf), http(false) {
	InputMode inputmode = RECORD_PER_LINE;
	for (int i = 1; i < argc; ++i) {
	  if (*argv[i] == '-') {
		if (argv[i][1] == 0 || argv[i][2] != 0)
		  usage_exit();
		switch (argv[i][1]) {
		case 'h':
		  http = true;
		  break;
		case 'i':
		  next(i, argc);
		  ifiles.push_back(argv[i]);
		  break;
		case 'd':
		  next(i, argc);
		  config.gen_log_name = argv[i];
		  break;
		case '_':
		  next(i, argc);
		  config.smoothing_delta = strtod(argv[i], 0);
		  break;
		case 'l':
		  next(i, argc);
		  config.lang = argv[i];
		  break;
		case 'n':
		  next(i, argc);
		  config.max_ng = strtol(argv[i], 0, 0);
		  break;
		case 's':
		  next(i, argc);
		  config.syl_max = strtol(argv[i], 0, 0);
		  break;
		case 'p':
		  config.evaluate_precision = true;
		  break;
		case 't':
		  config.top_list = true;
		  break;
		case 'a':
		  next(i, argc);
		  config.alpha_factor = strtod(argv[i], 0);
		  break;
		case 'f':
		  next(i, argc);
		  try {
			config.factor_by_confidence.parse_insert(argv[i]);
		  } catch (string e) {
			cerr << e;
			exit(1);
		  }
		  break;
		case 'D':
		  next(i, argc);
		  config.preserve_ng_tail = strtod(argv[i], 0);
		  break;
		case 'r':
		  next(i, argc);
		  config.random_seed = strtol(argv[i], 0, 0);
		  break;
		case 'c':
		  next(i, argc);
		  config.ngen = strtol(argv[i], 0, 0);
		  break;
		case 'H':
		  config.allow_hits = true;
		  break;
		case 'm':
		  next(i, argc);
		  config.min_score = strtod(argv[i], 0);
		  break;
		case 'C':
		  config.checker = true;
		  break;
		case 'K':
		case 'N':
		  config.smoothing_mode = KNESSER_NEY;
		  ////	config.linearOnlyAbstraction=true;//impl. another way?
		  break;
		case 'L':
		  config.linearOnlyAbstraction = true;
		  break;
		case 'b':
		  next(i, argc);
		  config.prune = strtod(argv[i], 0);
		  break;
		case 'v':
		  next(i, argc);
		  config.verbose = strtol(argv[i], 0, 0);
		  break;
		case 'T':
		  config.sort_order = SCORE_ORD;
		  break;
		case 'G':
		  config.sort_order = HISTORICAL_ORD;
		  break;
		case '%':
		  config.display_score = true;
		  // config.showPrecision = true;
		  break;
		case 'w':
		  inputmode = WHOLE_FILE_AS_ONE_LEXEME;
		  break;
		default:
		  usage_exit();
		}
	  } else
		inputs.push_back(Input(inputmode, argv[i]));
	}
  }
};
void addHeadwordOnlyLexemes(vector<Lexeme> &lex) {
  for (int i = 0; i < headWord.size(); ++i) {
	lex.push_back(Lexeme());
	lex.back().lx = headWord[i];
  }
}

struct ProlIterBase;
struct ProlIter {
  ConstSmartPtr<ProlIterBase> b;
  ProliferatorMap::iterator i, e;
  double div_base_score; // divided by the total population of
						 // b.selector[my_inndex]
  double score() { return div_base_score * i->first; }
  ProlIter(ProlIterBase *base, double bs, ProliferatorMap &m)
	  : i(m.begin()), e(m.end()), div_base_score(bs), b(base) {}
};

struct ProlIterBase {
  static map<Gram, Proliferator> cache;
  multimap<double, ProlIter> &target_queue;
  vector<Selector *> selectors;
  double base_score;
  Gram word; // full concrete prefix from the word beginning
  int nuser;

public:
  void fire() {}

  ProlIterBase(const Gram &w, vector<Selector *> &v, double bs,
			   multimap<double, ProlIter> &t /*obsolete */
			   )
	  : nuser(0), word(w), base_score(bs), selectors(v), target_queue(t) {}
  void insert_from(Selector *s, vector<Selector *> &v,
				   multimap<double, ProlIter> &target_queue,
				   double share_score) {
	// if (share * ABS_SCORE_GRANULARITY< 1) return ;
	if (!s->p) {
	  s->p = new Proliferator();
	  s->copy_insert(*s->p);
	  // cerr << "---"<<nabs<<"-"<<s->r->total_score<<endl;
	}
	// burst to target queue
	if (!s->p->m.empty()) {
	  ProlIter entry(this, share_score / s->count, s->p->m);
	  target_queue.insert(pair<double, ProlIter>(entry.score(), entry));
	}
  }
};

inline void burst(Gram word /*copy!*/, vector<Selector *> v /*copy!*/,
				  phon *suff, double base_score,
				  multimap<double, ProlIter> &target_queue,
				  const WGConfig &conf) {
  word.push_back(suff);
  static set<Gram> bursten;
  if (bursten.insert(word).second) {
	int len = min(conf.max_ng - 1, (int)word.size());
	double denom = 0.;
	advance_selectors(v, suff);
	double remainder = 1.;
	vector<double> share;
	for (int i = 0; i < v.size(); ++i)
	  if (selectorGood(i, v[i], conf)) {
		int nabs = NumberOfSetBits(i);
		share.push_back(abstractedProbability(v[i], conf, nabs, remainder));
		denom += share.back();
	  }
	ProlIterBase *b = new ProlIterBase(word, v, base_score, target_queue);
	for (int i = 0, j = 0; i < v.size(); ++i, j++)
	  if (selectorGood(i, v[i], conf)) {
		int nabs = NumberOfSetBits(i);
		b->insert_from(v[i], v, target_queue, base_score * share[j] / denom);
	  }
  }
}
const int max_gen = 65535 * 32; // hard limit of generated random word count
const int max_burst = 4 * max_gen; // hard limit of phoneme advance attempts
								   // while generating top-probability words

// Pick top probability words
void pickTopScoreWords(map<Gram, double, LexCompare> &out,
					   const WGConfig &conf) {
  multimap<double, ProlIter> queue;

  Gram start;
  vector<Selector *> v(1 << (max(conf.max_ng - conf.preserve_ng_tail, 0)),
					   (Selector *)0);
  v[0] = NGATRoot[0].select;
  start.push_back(phon::PP_BEGIN);
  burst(Gram(), v, phon::PP_BEGIN, 1., queue, conf);
  int iter_count = 0;
  while (!queue.empty() && out.size() < conf.ngen && ++iter_count < max_burst) {
	pair<double, ProlIter> s = *(--queue.end());
	queue.erase(--queue.end());
	if (s.second.i->second == phon::PP_END) {
	  Gram word(s.second.b->word.begin() + 1,
				s.second.b->word.end()); // removing '^'
	  double score =
		  (conf.min_score > 0.) ? honestScore(word, conf, false) : s.first;
	  if (filter(out, conf, score, word))
		out.insert(pair<const Gram &, double>(word, score));
	} else
	  burst(s.second.b->word, s.second.b->selectors, s.second.i->second,
			s.first, queue, conf);
	if (++s.second.i != s.second.e)
	// re-insert
	{
	  s.first = s.second.score();
	  if (s.first >= conf.min_score)
		queue.insert(s);
	}
  }
}

bool cmpScore(const pair<const Gram, double> *s1,
			  const pair<const Gram, double> *s2) {
  // reverse compare for the ranking
  return s1->second > s2->second;
}

inline void printScore(ostream &out, const Gram &w, double score,
					   const WGConfig &conf) {
  if (conf.display_score) {
	out << "(";
	if (conf.min_score > 0.)
	  out << score; // honest score is already calculated
	else {
	  if (conf.verbose > 2)
		out << score << "/";
	  out << honestScore(w, conf, false);
	}
	out << ")";
  }
}

phon **newVCabstractions() {
  phon **a = new phon *[1 << HARD_MAX_CONTRASTIVE_LEVEL];
  for (unsigned int ii = 0; ii <= 1; ii++) {
	wchar_t name[2 + (1 << HARD_MAX_CONTRASTIVE_LEVEL)];
	int j = 0;
	name[j++] = ii ? 'V' : 'C';
	name[j] = 0;
	for (unsigned int i = ii; i < 1 << HARD_MAX_CONTRASTIVE_LEVEL; i += 2) {
	  a[i] = new phon(name, i + 100);
	  name[j++] = '+';
	  name[j] = 0;
	}
  }
  return a;
}

// Print precision figures evaluted over the training vocabulary
//(with temporary exclusion of each focus word from statistics)
// for a set of 'high hanging fruit' quantiles.
std::ostream &printQuantilePrecision(std::ostream &os, double overhead,
									 WGConfig &conf, bool applyDiscount = true,
									 bool applyConf = false) {
  int N = dict_words.size();
  if (N < 1)
	return os;
  std::multiset<double> probability_sorted;
  os << "{";
  double probability_sum = 0.;
  for (set<Gram>::iterator w = dict_words.begin(), e = dict_words.end(); e != w;
	   ++w)
	probability_sorted.insert(honestScore(*w, conf, applyDiscount, applyConf));
  double quantiles[5] = {0.05, 0.1, 0.25, 0.5, 1.0};
  int j = 0;
  multiset<double>::iterator i = probability_sorted.begin();
  for (int qi = 0; qi < sizeof(quantiles) / sizeof(double); ++qi) {
	if (qi)
	  os << ", ";
	while (j++ < probability_sorted.size() * (std::min(quantiles[qi], 1.)))
	  probability_sum += *i++;
	os << "\"" << quantiles[qi]
	   << "\": " << probability_sum / quantiles[qi] * overhead;
  }
  os << "}";
  return os;
}

int main(int argc, char **argv) {
  WGConfig conf;
  CmdArgs args(argc, argv, conf);
  if (conf.gen_log_name != "") {
	glog.open(conf.gen_log_name.c_str());
	if (!glog)
	  cerr << "Error: cannot open log file " << conf.gen_log_name << endl;
  }
  initFactors(conf);
  initIgnore();
  vector<Lexeme> lex;
  for (int i = 0; i < args.inputs.size(); ++i)
	switch (args.inputs[i].mode) {
	case WHOLE_FILE_AS_ONE_LEXEME:
	  try {
		std::ifstream f(args.inputs[i].name.c_str());
		headWord.push_back(
			u2w(std::string((std::istreambuf_iterator<char>(f)),
							std::istreambuf_iterator<char>()).c_str()));
	  } catch (exception e) {
		cerr << "Error: Cannot read " << args.inputs[i].name << std::endl;
	  }
	default:
	  read_all_lines(args.inputs[i].name.c_str(), charCombFillMap(),
					 &charCombination, headWord);
	}
  addHeadwordOnlyLexemes(lex);
  time_t t;
#ifdef PROFILE
  clock_t start_clock;
  start_clock = clock();
#endif
  statCount(conf, lex, 0);
#define AUTO_CLASSIFY_PHONEMES 1
#if AUTO_CLASSIFY_PHONEMES
  classifyPhonemes();
  if (conf.verbose)
	reportAutoClassifiedPhonemes();
#else
  dictionaryWarnings();
#endif
#ifdef PROFILE
  cerr << "Stat ready: " << clock() - start_clock << " ticks" << endl;
#endif
  cascadeAbstract(conf);
#ifdef PROFILE
  cerr << "Abstracted: " << clock() - start_clock << " ticks" << endl;
#endif
  if (conf.verbose > 11)
	printNGrams(conf);

  if (dict_words.empty()) {
	cerr << "Error: Empty dictionary." << endl;
	exit(1);
  }
  srand(conf.random_seed != -1L ? conf.random_seed : time(&t));
  map<Gram, double, LexCompare> rank;

  if (conf.top_list) {
	pickTopScoreWords(rank, conf);
  } else
	for (int i = 0, j = 0; i < max_gen && j < conf.ngen; ++i) {
	  pair<Gram, double> ws(generateWord(conf));
	  if (conf.min_score > 0.)
		ws.second = honestScore(ws.first, conf, false);
	  if (!ws.first.empty() && filter(rank, conf, ws.second, ws.first)) {
		if (rank.insert(ws).second) {
		  if (conf.sort_order == HISTORICAL_ORD) { // Output immediately
			cout << g2u(ws.first); //<< "#" << i;
			printScore(cout, ws.first, ws.second, conf);
			cout << endl;
		  }
		}
		++j;
	  }
	}
  vector<const pair<const Gram, double> *> v;
  if (conf.sort_order != HISTORICAL_ORD)
	for (map<Gram, double>::iterator it = rank.begin(); it != rank.end();
		 ++it) {
	  // it->second.erase(it->second.begin());
	  wstring w(g2w(it->first));
	  if (gen_words.find(it->first) == gen_words.end())
		if (conf.sort_order == SCORE_ORD)
		  v.push_back(&*it);
		else {
		  cout << w2u(w);
		  printScore(cout, it->first, it->second, conf);
		  cout << endl;
		}
	  gen_words.insert(it->first);
	}
  stable_sort(v.begin(), v.end(), cmpScore);
  for (int i = 0; i < v.size(); ++i) {
	wstring w(g2w(v[i]->first));
	cout << w2u(w);
	printScore(cout, v[i]->first, v[i]->second, conf);
	cout << endl;
  }

#ifdef PROFILE
  cerr << "Generated: " << clock() - start_clock << " ticks" << endl;
#endif
  if (conf.evaluate_precision) {
	int N = dict_words.size();
	double overhead = (1. + 1. * all_rejected() / N) * (N - 1) / N;
	cerr << "Info: Quantile precision: ";
	printQuantilePrecision(std::cerr, overhead, conf, true, true) << std::endl;
  }

  if (conf.verbose > 0)
	report_rejected();
  if (conf.verbose > 1)
	cerr << "Info: Total # of phonemes produced (including \"$\"): " << nGenPhon
		 << endl;
  exit(0);
  return 0;
}
map<Gram, Proliferator> ProlIterBase::cache; // Check smartpointer delete!!

map<Gram, Randomizer<phon *> > AbstrRandomizerBuilder::cache;
Lexeme::M Lexeme::m;

phon **phon::VCabstractions = newVCabstractions();
phon *phon::PP_BEGIN = new phon(L"^", 1);
phon *phon::PP_END = new phon(L"$", 2);
phon *phon::PP_DICT_MARKER = new phon(L"#", 17);
