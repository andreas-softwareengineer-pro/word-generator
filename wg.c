#include <iostream>
#include <fstream>
#include <string>
#include <vector>
//#include <string>
#include <sstream>
#include <set>
#include <algorithm>
#include <map>
#include <math.h>
#include <assert.h>
using namespace std;    // Sorry for this!

#define O1 0
//#define PROFILE

//========================
//   Utilities
//========================

//Compare the first n elements of two vectors
template <class E> bool vec_n_eq(const vector<E>& v1, const vector<E>& v2,
	size_t n)
{
	size_t l1 = v1.size(), l2 = v1.size();
	if (l1 != l2 && (l1 < n || l2 < n)) return false;
	if (l1 < n) n = l1;
	for (size_t i=0; i<n; ++i) if (v1[i] != v2[i]) return false;
	return true;
}


template <class T> class ConstSmartPtr {
	T* p;
public:
	ConstSmartPtr(T* _p) :p(_p) {++_p->nuser;}
	ConstSmartPtr(const ConstSmartPtr<T>& _p) :p(_p.p) {++p->nuser;}
	~ConstSmartPtr() {if (!--p->nuser) delete p;}
	T* operator ->		 () {return p;}
};

void reportDataNz(int val, const char* message)
{
	if (val)
		cerr << "Info: "<<message<<": "<<val<<endl;
}

int NumberOfSetBits(int i)
{
     i = i - ((i >> 1) & 0x55555555);
     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
     return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

//returns nxt to x value not containing any bit that's set in msk
inline int masked_inc(unsigned int x, unsigned int msk) {
	return ((x|msk)+1)&~msk ;
}

//count trailing zeros
int ctz(unsigned int x) {
    if (x == 0) return 32;
    int n = 0;
    if ((x & 0x0000FFFF) == 0) n = n + 16, x = x >> 16;
    if ((x & 0x000000FF) == 0) n = n +  8, x = x >>  8;
    if ((x & 0x0000000F) == 0) n = n +  4, x = x >>  4;
    if ((x & 0x00000003) == 0) n = n +  2, x = x >>  2;
    if ((x & 0x00000001) == 0) n = n +  1;
    return n;
}

int clz(unsigned int x) {
    if (x == 0) return 32;
    int n = 0;
    if ((x & 0xFFFF0000) == 0) n = n + 16, x = x << 16;
    if ((x & 0xFF000000) == 0) n = n +  8, x = x <<  8;
    if ((x & 0xF0000000) == 0) n = n +  4, x = x <<  4;
    if ((x & 0xC0000000) == 0) n = n +  2, x = x <<  2;
    if ((x & 0x80000000) == 0) n = n +  1;
    return n;
}

inline double log_approx(double x) {
	if (x<1.) return 0;
	int xn = x;//truncated
	int pow2 = 31 - clz(x);
	return pow2 + (x-(1<<pow2))/(1<<pow2);
}

//Splits a string into an array
std::vector<std::wstring> split(const std::wstring &s, wchar_t delim,
  bool empty_ok=false) {
    std::vector<std::wstring> elems;
    std::wstringstream ss(s);
    std::wstring item;
    while (std::getline(ss, item, delim)) {
        if (empty_ok || !item.empty()) elems.push_back(item);
    }
    return elems;
}

//removes leading and trailing spaces
template <class S> static void trim(S &s, const typename S::value_type *spaces)
{
   int lp = s.find_last_not_of(spaces);
   int fp = s.find_first_not_of(spaces);
   if (lp!=std::string::npos)
     s.erase(lp+1,std::string::npos);
   if (fp!=std::string::npos)
     s.erase(0,fp);
}
inline void trim(std::string  & s) {trim(s, " \r\t\n");}
inline void trim(std::wstring & s) {trim(s, L" \r\t\n");}

template <class E> struct more {
	bool operator()(const E& x, const E& y) {
		return x > y;
	}
};

inline wchar_t wproc(wchar_t c) {return towlower(c);}

//Unicode string to wide character one
std::wstring u2w(const char* s) {
  std::wstringbuf sbuf;
  int more = -1;
  wchar_t sumb = 0;
  char b;
  while ((b=*s)!=0) {
      /* Decode byte b as UTF-8, sumb collects incomplete chars */
      if ((b & 0xc0) == 0x80) {			// 10xxxxxx (continuation byte)
	sumb = (sumb << 6) | (b & 0x3f) ;	// Add 6 bits to sumb
	if (--more == 0) sbuf.sputc(wproc(sumb)) ; // Add char to sbuf
      } else if ((b & 0x80) == 0x00) {		// 0xxxxxxx (yields 7 bits)
	sbuf.sputc(wproc(b)) ;			// Store in sbuf
      } else if ((b & 0xe0) == 0xc0) {		// 110xxxxx (yields 5 bits)
	sumb = b & 0x1f;
	more = 1;				// Expect 1 more byte
      } else if ((b & 0xf0) == 0xe0) {		// 1110xxxx (yields 4 bits)
	sumb = b & 0x0f;
	more = 2;				// Expect 2 more bytes
      } else if ((b & 0xf8) == 0xf0) {		// 11110xxx (yields 3 bits)
	sumb = b & 0x07;
	more = 3;				// Expect 3 more bytes
      } else if ((b & 0xfc) == 0xf8) {		// 111110xx (yields 2 bits)
	sumb = b & 0x03;
	more = 4;				// Expect 4 more bytes
      } else /*if ((b & 0xfe) == 0xfc)*/ {	// 1111110x (yields 1 bit)
	sumb = b & 0x01;
	more = 5;				// Expect 5 more bytes
      }
      /* We don't test if the UTF-8 encoding is well-formed */
  ++s;
  }
  return sbuf.str();
}

//Wide character string to unicode one
std::string w2u(const wchar_t* s) {
  std::stringbuf sbuf;
  int more = -1;
  wchar_t b;
  while ((b=*s++)!=0) {
	  if ((b & 0xff80) == 0) 
	  if (b<32) sbuf.sputc('?'); else sbuf.sputc(b);
	  else {
		  //sbuf.sputc('?');
		  sbuf.sputc(0xc0|(b&0x07c0)>>6);
		  sbuf.sputc(0x80|(b&0x3f));
	  }
  }
    return sbuf.str();
}
	    
std::string w2u(const wstring & s) {
		return w2u(s.c_str());
}	  
	
template <class F>
class FieldMap:public map<string, bool(*)(const wchar_t*, F*) > {};

//Reads a line from a file of any end-of-line convention 
//Copied from http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf/6089413#6089413
std::istream& safeGetline(std::istream& is, std::string& t)
{
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();

    for(;;) {
        int c = sb->sbumpc();
        switch (c) {
        case '\n':
            return is;
        case '\r':
            if(sb->sgetc() == '\n')
                sb->sbumpc();
            return is;
        case EOF:
            // Also handle the case when the last line has no line ending
            if(t.empty())
                is.setstate(std::ios::eofbit);
            return is;
        default:
            t += (char)c;
        }
    }
}

//Reads a file containing both plain text line and records of "key:value" type
template <class F>
void read_all_lines(const char *filename,  
  map<wstring, bool(*)(const wchar_t*, F*) >& fieldMap,
  F* out_rec,
  vector<wstring>& out_plain)
{
    ifstream ifs;
    string stxtline;
    wstring txtline;
    F* curr = 0;

    ifs.open(filename);
    if(!ifs.is_open())
    {
        wcerr << L"Unable to open file" << endl;
        return;
    }
    // We are going to read an UTF-8 file
    //wifs.imbue(utf8_locale);
    //wifs.imbue(locale(wifs.getloc(), new codecvt_utf8<wchar_t, 0x10ffff, /*consume_header*/ 4>()));
    while(safeGetline(ifs, stxtline)) {
	    txtline = u2w(stxtline.c_str());
	    trim(txtline);
	    size_t j = txtline.find(L':');
	    if (j==wstring::npos) out_plain.push_back(txtline);
	    else {
		    wstring key(txtline.substr(0,j) );
		    j++;
		    while (txtline.length() > j && iswspace(txtline[j])) j++;
		    typename map<wstring, bool(*)(const wchar_t*, F*) >::iterator fieldI = fieldMap.find(key);
		    if (fieldI != fieldMap.end() && out_rec)
		      fieldI->second(txtline.substr(j,wstring::npos).c_str(),out_rec);
		    //else cerr << "Warning: \"" << w2u(key) << "\" record type is unknown\n";
	    }
    }

}

template <class T, std::wstring  T::* P> bool  setField(const wchar_t* s, T* t) {
  t->*P = s;
  return true;
  }

//========================
//   Phoneme stuff
//========================

static int threshold_vowel_freq = 0;

  struct phon {
	  wstring rep;
	  int cardinality;
	  int vowel;
	  int consonant;
	  int markup;
	  int freq;
	  static phon* PP_BEGIN;
	  static phon* PP_END;
	  static phon* PP_DICT_MARKER;
	  static phon* PP_VOWEL;
	  static phon* PP_FREQUENT_VOWEL;
	  static phon* PP_CONSONANT;
	  phon(const wstring& r, int mu = 0) :rep(r), cardinality(0), vowel(0), freq(0), consonant(0), markup(mu) {}
	  bool isVowel() {return vowel>consonant;}
	  bool isConsonant() {return consonant>vowel;}
	  bool isVC() {return vowel!=consonant;}
	  phon* abstracted() {return (vowel>consonant)? 
	    (freq > threshold_vowel_freq? PP_FREQUENT_VOWEL: PP_VOWEL): (consonant>vowel)? PP_CONSONANT : 0; }
	  void voteVowel() {if (!markup) vowel++;}
	  void voteConsonant() {if (!markup) consonant++;}
	  bool abstractable() {return abstracted()!=0;}
  };

typedef vector<phon*> Gram;

wstring g2w(const Gram & x) {
    wstringbuf sbuf;
    wostream w(&sbuf);
  for (int i = 0; i<x.size(); ++i) 
  {
		const wstring& s(x[i]->rep);
		sbuf.sputn(s.c_str(),s.length());
  }
  return sbuf.str();
}

struct Lexeme {
  wstring syl;
  wstring lx;
  wstring CV;
  typedef std::map<std::wstring ,bool(*)(const wchar_t*, Lexeme*)> M;
  static M m;

  static M& fillMap() {
	   if (m.empty()) {
		   m.insert(M::value_type(L"lx",&setField<Lexeme,&Lexeme::lx>));
		   m.insert(M::value_type(L"CV",&setField<Lexeme,&Lexeme::CV>));
		   m.insert(M::value_type(L"syl",&setField<Lexeme,&Lexeme::syl>));
		}
	return m;
  }
};

//change?
struct LexCompare{
	bool operator ()(const Gram & g1, const Gram & g2) const {
		int l1 = g1.size(), l2 = g2.size(),
		l = min(l2,l1);
		int i = 0;
		for (; i<l; ++i) {
			if (g1[i]->rep < g2[i]->rep)
			  return true;
			if (g2[i]->rep < g1[i]->rep)
			  return false;
		}
		if (i < l2) return true;
		return false;
	}
};

struct Scale {
	map<double,double> m;
	double zero_y;
	double get(double x) const {
		map<double,double>::const_iterator i = m.upper_bound(x);
		if (i == m.begin()) return zero_y;
		--i;
		return i->second;
	}
	void skip_sep(const char*& c) {
		//all non-alphanumerical chars are treated like separator
		while (*c && !isalnum(*c) && *c!='.' && *c!='-' && *c!=':') ++c;
	}
	double read_num(const char*& c, const char*& s) {
		skip_sep(c);
		char* e;
		double n = strtod(c,&e);
			if (c == e) throw(string("Scale format error in \"")+s+"\": number expected, \"" + c + "\" found");
			c = e;
			return n;
			}
	
	void parse_insert(const char* s) {
		const char* c = s;
		do {
			double n = read_num(c,s);
			skip_sep(c);
			if (*c == ':') {
				++c;
				m[n] = read_num(c,s);}
				
			else zero_y = n;
		}
			while (*c);
	}
	Scale (double d) : zero_y(d) {}
	};
		

//Config
struct WGConfig {
	string lang;
	int max_ng;
	int max_abstracted;
	int syl_max;
	double alpha_factor;
	double freq_vowel_confidence;
	bool scientific;
	long random_seed;
	Scale factor_by_confidence;
	int ngen;
	int verbose;
	bool pr_stat, top_list, sort_score, display_score;
	bool allow_hits, checker;
	double min_score;
	WGConfig() : lang(),max_ng(5),max_abstracted(3), syl_max(1024), alpha_factor(0.8),
	  freq_vowel_confidence(.5), scientific(false), random_seed(-1L), ngen(100), verbose(0),   pr_stat(false), top_list(false),
	  allow_hits(false), min_score(0.), sort_score(false), checker(false), display_score(false), factor_by_confidence(.3) {}
};
	inline double absconf(int count) {
		return log(1 + count);
	}

  const int VARIANTS = 2;
  
  struct StatAtom {
    wstring example;
    int count[VARIANTS];
    float confb, conf_factor;
    vector<pair<int, double> > vara;
    StatAtom() : confb(1.), conf_factor(1.) {
			fill(count,&count[VARIANTS],0);}
    StatAtom& operator += (const wstring& ex) {count[0]++;example=ex;return *this;}
  };
  
  typedef map<vector<phon*>,StatAtom> PhonVecStat;
  PhonVecStat NGramStat, NGramStat2, NGAStat[1<<10];
  
  struct Selector;
  template <class R> struct Randomizer;
  struct Proliferator;
  struct Selection {
	  int count[VARIANTS];
	  double confidence, conf_factor;
	  Selector* uffix;
	  Selection()
	  :uffix(0), confidence(1.), conf_factor(1.) {
		  fill(count,count+VARIANTS,0);
	}
  };
  
  #define HARD_MAX_ABSTRACT 8
  struct Selector {
	  Selector* abstr[HARD_MAX_ABSTRACT];
	  double confidence, conf_factor;
	  int count;
	  Gram ngram; //for debug
	  vector<pair<int, double> > vara;
	  map<phon*,Selection> nxt;
	  Proliferator* p;
	  Randomizer<phon*>* r;
	  Selector(Gram g) 
		:ngram(g), confidence(1.), conf_factor(1.), count(0),p(0),r(0) {
					  fill(abstr, abstr+HARD_MAX_ABSTRACT,(Selector*)0);
		}
	  double factor() {return absconf(count)*conf_factor;}
  };
  
  struct StatTreeNode {
	  map<phon*,StatTreeNode> nxt;
	  Selector* select;
	  StatTreeNode* treeNode(const Gram &x, int start, int end) {
		  StatTreeNode* rc = this;
		  for (int i=start; i<end; ++i) rc = &rc->nxt[x[i]];
		  return rc;
	  }
	  StatTreeNode ()	:select(0) {}
  };
  static StatTreeNode NGATRoot[8]; //init
  static Selector*	 NGASelRoot = 0;

  map<wstring,phon*> phonemes;
inline phon* update_phoneme(wstring s) {
    for (int i=0; i<s.length(); ++i)
      	  if(s[i] == '~') {
	  	  s[i]=0x0303;} //pseudodiacritic - hack;
    map<wstring,phon*>::iterator i = phonemes.find(s);
    if (i == phonemes.end()) {
	    phon* r = new phon(s);
	    phonemes[r->rep] = r;
	    return r;
    }
    return i->second;
  }

  
typedef enum {
	EMPTY_T,
	AUTO_BREAK_T,
	AUTO_UNIQ_T,
	BREAK_T,
	UNIQ_T,
	VOWEL_T,
	CONS_T,
	IGNORE_T,
	REJECT_T,
	STOP_T
} TokTypeE;

  struct TokType {
	  int start;
	  int stop;
	  int length() {return stop-start;}
	  TokTypeE t ;
	  TokType(TokTypeE _t, int _start, int _stop): t(_t), start(_start), stop(_stop) {} 
  };
  
  typedef map<wstring,vector<TokType> > CharCombMapT;

  template <TokTypeE tok> bool addStrToks(const wchar_t* s, CharCombMapT * dst) {
	vector <wstring> st = split(s,',');
	for (int i=0; i<st.size(); ++i) {
		int nslash = 0;
		trim(st[i]);
		wstring sk;
		TokType t(tok,0,0);
		for (const wchar_t* p=st[i].c_str();*p;p++)
      	  if(*p == '~') {
	  	  sk.push_back(0x0303); //pseudodiacritic - hack;
		} else if (*p== L'/') {
					t.start = t.stop;
					t.stop = sk.length();
				} else {
					sk.push_back(*p);
				}
		if (nslash < 2) t.stop = sk.length();
		if (nslash > 3) cout<<"Error: Too many slashes in \"" << w2u(st[i]) << "\" << phoneme specification";
		(*dst)[sk].push_back(t);
	}
	return 1;
}

 typedef map<wstring, bool(*)(const wchar_t*, CharCombMapT*) > CharCombFillMapT;
  static CharCombFillMapT& charCombFillMap() {
	static map<wstring, bool(*)(const wchar_t*, CharCombMapT*) > m;
	if (m.empty()) {
		   m.insert(CharCombFillMapT::value_type(L"vowels",&addStrToks<VOWEL_T>));
		   m.insert(CharCombFillMapT::value_type(L"consonants",&addStrToks<CONS_T>));
		   m.insert(CharCombFillMapT::value_type(L"extra",&addStrToks<UNIQ_T>));
		   m.insert(CharCombFillMapT::value_type(L"ignore",&addStrToks<IGNORE_T>));
		   m.insert(CharCombFillMapT::value_type(L"break",&addStrToks<BREAK_T>));
		   m.insert(CharCombFillMapT::value_type(L"reject",&addStrToks<REJECT_T>));
		   m.insert(CharCombFillMapT::value_type(L"stop",&addStrToks<STOP_T>));
		}
	return m;
  }

  void noRecordWarning(const char* missing, const char* keyWord) {
	cerr << "Warning: No " << missing << " specified. Please add a \""
		 << keyWord
		 << ": comma_separated_list\" line to the dictionary file" << endl;
  }

  CharCombMapT charCombination;
  vector<wstring> headWord;

inline bool is_diacritic(wchar_t c) {
	switch(c&0xfff0) {
		case 0x0300: case 0x0310: case 0x0320: case 0x0330: case 0x0340: case 0x0350: case 0x0360:
		//Combining Diacritical Marks (0300–036F), since version 1.0, with modifications in subsequent versions down to 4.1
		case 0x1AB0: case 0x1AC0: case 0x1AD0: case 0x1AE0: case 0x1AF0: 
		//Combining Diacritical Marks Extended (1AB0–1AFF), version 7.0
		case 0x1DC0: case 0x1DD0: case 0x1DE0: case 0x1DF0:
		//Combining Diacritical Marks Supplement (1DC0–1DFF), versions 4.1 to 5.2
		case 0x20D0: case 0x20E0: case 0x20F0: 
		//Combining Diacritical Marks for Symbols (20D0–20FF), since version 1.0, with modifications in subsequent versions down to 5.1
		case 0xFE20: 
		//Combining Half Marks (FE20–FE2F), versions 1.0, updates in 5.2
			return true;
		default:
			return false;
		}
  }
  
  inline int is_letter_continuation(wchar_t c)
  {
	return is_diacritic(c);
	} 

  inline void scanCharCombination(const wchar_t*s, TokType** best_types)
  {
  	 size_t confirmed_len = 0, best=0, j = 0;
  	wstring p;
  	while (confirmed_len >= j) {
		p.push_back(s[j]);
		if (j <= confirmed_len) {
		   CharCombMapT::iterator it = charCombination.lower_bound(p);
				
		   if (it != charCombination.end())
		   {
			   const wchar_t* cs = it->first.c_str();
		   //extend the match length
		   confirmed_len = 0;
		   while (cs[confirmed_len]==
		     s[confirmed_len] && cs[confirmed_len])
		      ++confirmed_len;
		   if (!cs[confirmed_len] && !is_letter_continuation(s[confirmed_len])) { //exact match
		   for (int i=0; i<it->second.size(); ++i) {
		   	 if (! best_types[it->second[i].start] || it->second[i].length() > best_types[it->second[i].start]->length())
		   	 best_types[it->second[i].start]=&it->second[i];
		   }}
		   }}
		   ++j;
		   }
		   s += best;
		   best_types += best;
		   return;
		}

  void dictionaryWarnings()
	{  
	  int has_vowel=false, has_consonant=false;
	  vector<string> au;
	  for (CharCombMapT::iterator it = charCombination.begin(); it != charCombination.end(); ++it)
	  for (int i=0; i<it->second.size(); ++i)
	  {
	  if (it->second[i].t==VOWEL_T) has_vowel++;
	  if (it->second[i].t==CONS_T) has_consonant++;
	  if (it->second[i].t==AUTO_UNIQ_T) au.push_back(w2u(it->first));
	  }
	  if (!has_vowel) noRecordWarning("vowel", "Vowels");
      if (!has_consonant) noRecordWarning("consonant", "Consonants");
      if (has_vowel && has_consonant && !au.empty()) {
	   cerr << "Warning: Letters ";
	   for (int i = 0; i<au.size(); ++i)
			cerr<<(i>0? ", " :"")<<au[i];
       cerr << " aren't classified in the dictionary file. Please add each of them to one of the following records: \"Vowels:\", \"Consonants:\" or \"Extra:\"" << endl;
		}
      }
  
  inline void skip_to(const   wchar_t term, const wchar_t* & e)
  {
	  const wchar_t* p = wcschr(e,term);
	  if (p) e = p+1;
  }
  
  inline void skip_enclosed(const  wchar_t* & e)
  {
	  switch(*e) {
		  case L'(': skip_to(L')',e);
		  break;
		  case L'[': skip_to(L']',e);
		  break;
		  case L'{': skip_to(L'}',e);
		  break;
		  case L'<': skip_to(L'>',e);
		  break;
	  }		  
  }
  
  static double* factors = 0;

  int phoneme_decompose(vector<Gram>& outv /*out, cumulative*/, const wchar_t*  s, bool ins_end_marks)
  {
	size_t size0 = outv.size();
	Gram out;
	int sl = wcslen(s);
	TokType * best_types_a[sl], **best_types = best_types_a ;
	fill(best_types,best_types+sl,(TokType *)0);
	
	if (ins_end_marks) out.push_back(phon::PP_BEGIN);
	while (*s)
	{
	TokTypeE kind = AUTO_UNIQ_T;
	  const wchar_t* e = s;
	  scanCharCombination(e,best_types);
	  	if (best_types[0]) {
		  	int l = best_types[0]->length();
		  	kind  = best_types[0]->t;
		  	assert(l>0);
		  	e+=l;
	  	} else //prefix not found in the table
	  {
	  skip_enclosed(e);
	  while (*e == L'\'' || *e == L'-') e++;
	  if (e==s) {
	  
	  //if (*e == L'n' && *(e + 1) == L'r')
	  //{
		//  e++; best_types = ...
	  //}
	int len = 1;
	  if (iswalpha(*e)) {
  	  	  while (is_letter_continuation(e[len])) len++; //diacritic;
	      charCombination[wstring(e,len)].push_back(TokType(kind = AUTO_UNIQ_T,0,len));
      } else kind = AUTO_BREAK_T;
      e += len;
	  
	  } else {kind = AUTO_BREAK_T;}
  }
  	  best_types+=(e-s);
	  switch (kind) {
		  case IGNORE_T:
		  break;
		  case REJECT_T:
		  return outv.size()-size0;
		  break;
		  case STOP_T:
		  s=L""; //stop the reading
		  break;
		  case VOWEL_T:
		  out.push_back(update_phoneme(wstring(s,e-s)));
		  out.back()->voteVowel();
		  break;
		  case CONS_T:
		  out.push_back(update_phoneme(wstring(s,e-s)));
		  out.back()->voteConsonant();
		  break;
		  case UNIQ_T: case AUTO_UNIQ_T:
		  out.push_back(update_phoneme(wstring(s,e-s)));
		  break;
		  case BREAK_T: case AUTO_BREAK_T:
		  if (out.size()>!!ins_end_marks) {
			  	if (ins_end_marks) out.push_back(phon::PP_END);
				outv.push_back(out);//opt?
				out.clear();
				if (ins_end_marks) out.push_back(phon::PP_BEGIN);
		}
		  break;
	  }

		  
  	  s = e;
	}

	if (out.size()>!!ins_end_marks)  {
		if (ins_end_marks) out.push_back(phon::PP_END);
		outv.push_back(out);//opt?;
	}
	return outv.size() - size0;
  }
    
  set<Gram> dict_words, gen_words;
  ostream& operator << (ostream& o, const std::vector<phon*>& x)
  {
	  for (int i=0; i<x.size(); ++i) o<<w2u(x[i]->rep);
      return o;
  }
static int rejected_gen_hit = 0, rejected_dict_hit = 0, rejected_low_score = 0, rejected_too_long = 0;
static int quasiSylCount(const Gram& w)
{
	bool prev_vowel = false;
	int rc = 0;
	for (int i=0; i<w.size(); ++i) 
		if (w[i] -> isVowel()) {
			if (!prev_vowel) ++rc;
			prev_vowel = true;
		} else prev_vowel = false;
	return rc;
}

  inline bool filter (const map<Gram,double,LexCompare> &generated, const WGConfig& conf, 
    double score, Gram& word /* may modify */) {
	  if (conf.syl_max < 100 && conf.syl_max < quasiSylCount(word)) {
		++rejected_too_long;
		return false;
	  }
	  
	  //Use it only in normal mode?
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
		  if (found) if (!conf.allow_hits)
		  {
		  ++rejected_dict_hit;
	 	  return false;
 	  } else word.push_back(phon::PP_DICT_MARKER);
	  }
	  return true;
  }

  	void report_rejected() {
		reportDataNz(rejected_too_long,"# of words rejected due to the length exceed");
		reportDataNz(rejected_dict_hit,"# of words rejected due to they are found in the dictionary");
		reportDataNz(rejected_gen_hit,"# of words rejected due to the repetition");
		reportDataNz(rejected_low_score,"# of words rejected due to the low probability");	  	
	}
	
    typedef int AbstractionVector;
    
    inline double range_factor(double x) {
	    return x;
    }
    inline int cardinality_sum(const Gram& g) {
	    int rc = 0;
	    for (int i=0; i<g.size(); ++i) rc += g[i] -> cardinality;
    	return rc;
	}
	void abstractOneStep(AbstractionVector a, const WGConfig& conf) {
	    map<Gram,StatAtom>& dst_m = NGAStat[a];
		    int nabs = NumberOfSetBits(a);
		    bool first_pass = true;
		    for (int pos = 0; pos<conf.max_ng; ++pos) if ((a>>pos)&1) {
	    map<Gram,StatAtom>& src_m = NGAStat[a&~(1<<pos)];
	    
	    for (map<Gram,StatAtom>::iterator j = src_m.begin(); j != src_m.end(); ++j) if (j->first.size() > pos + 2)
	    {
		    Gram g(j->first); //optimize reusing original?
		    phon* orph = g[pos];
		    if (g[pos]->abstractable()) {
			    g[pos] = g[pos]->abstracted();
		        if (g[pos]) {
			        //optimize dst[m]!
			        if (first_pass)
				        ++dst_m[g].count[0] += j-> second. count[0];
			        dst_m[g].vara.push_back(pair<int,float>(j-> second. count[0],j->second.confb));
		        }
	        }
	    }
	    first_pass = false;
    	}
    
	    for (map<Gram,StatAtom>::iterator i = dst_m.begin(); i != dst_m.end(); ++i)
	    {
			        int total = 0;
			        int card = cardinality_sum(i->first);

			//take it out to be done once only
			        for (int k = 0; k < i->second.vara. size(); ++k)
			{
				        total  +=  i->second.vara[k].  second*i->second. vara[k].  first;
	        }
			        float rcb=.0;
			        if (total) {
			        double invMean = ((double)card)/total;
			        for (int k = 0; k < i->second.vara.size(); ++k) 
			{
				        rcb  += i->second.vara[k].second * log_approx(1+ i->second.vara[k].  second* i->second.vara[k].  first*invMean);
	        }
        }
	        
	        double conf_factor;
	        if (card > 0) {
		        double confidence  =  range_factor(rcb/ /*log(2) - divide with log used, no need when log_approx is in place/*/card);
	        i->second.confb  = confidence;
	        //cerr << "rcb= " << rcb <<", card = "<<card<<" Confidence  = " << confidence << endl;
	        conf_factor = conf.factor_by_confidence.get(confidence);
        } else
	        i->second.confb  = conf_factor = 1.;
	    i->second.conf_factor = conf_factor*factors[nabs]; //accont for alpha factor
	    }
   }

void abstractOneStep1(AbstractionVector a, const WGConfig& conf) {
		    bool first_pass = true;
StatTreeNode * dstelem[conf.max_ng+1];
    for (int pos = 0; pos<conf.max_ng-2; ++pos) if ((a>>pos)&1) {
map<phon*,StatTreeNode>::iterator srcend[conf.max_ng+1], srcit[conf.max_ng+1];
srcend[0] = NGATRoot[a&~(1<<pos)].nxt.end();
srcit[0]  = NGATRoot[a&~(1<<pos)].nxt.begin();
phon* absgram[conf.max_ng+1];
int l = 0, la=0;
do {
	if (srcend[l] != srcit[l]) {
	   if (pos == l) {
	   	 absgram[pos] = srcit[l]->first->abstracted();
	   	 if (!absgram[pos]) {
		   	 ++srcit[l]; continue;
	   	 } //not an abstractable phoneme - skip
   	 } else
   	 	absgram[l] = srcit[l]->first;
 	//descent
	srcit  [l+1] = srcit[l]->second.nxt.begin();
	srcend  [l+1] = srcit[l]->second.nxt.end();
  	 	
	   ++l;
	} else {
		--l;
		if (l < 0) break;
		if (l < la) la=l;
  //action
		if (   srcit[l]->second.select) {
					la=0; while (la<=l) { dstelem[la] = & (la >0? dstelem[la-1]:&NGATRoot[a]) ->  nxt[absgram[la]]; ++la; }
					if (!dstelem[l] -> select) dstelem[l]->select = new Selector(Gram(absgram,absgram+l+1));
				        if (l>pos) srcit[l]->second.select->abstr[l-1-pos] =  dstelem[l]->select ;
					    dstelem[l]->select->count += srcit[l]->second.select->count;
				        for (map<phon*,Selection>::iterator i=srcit[l]->second.select->nxt.begin();
								i!=srcit[l]->second.select -> nxt.end(); ++i)
							{
										Selection& e = dstelem[l]->select->nxt[i->first];
										if (first_pass)
										  e.count[0] += i->second.count[0]; 
			        					if (l>pos && i->second.uffix)
			        						if ( (l-pos) < conf.max_ng-2)
			        							e. uffix = i->second.uffix->abstr[l-pos];
			        							else 
			        							e. uffix = i->second.uffix;
			        							
			        		}
		        		dstelem[l]->select->vara.push_back(pair<int,float>(srcit[l]->second.select->count,srcit[l]->second.select->confidence));

					}
  ++srcit[l];
}
} while (true);
	    first_pass = false;
}

int l = 0;
int card[conf.max_ng+1];
map<phon*,StatTreeNode>::iterator dstit[conf.max_ng+1],dstend[conf.max_ng+1];
dstit [0] = NGATRoot[a].nxt.begin();
dstend[0] = NGATRoot[a].nxt.end ();
card[0] = 0;

do {
	if (dstit[l] != dstend[l]) {
	   card[l] = (l>0? card[l-1] : 0) + dstit[l] -> first -> cardinality;
	   //descend
	   dstit   [l+1] = dstit  [l] -> second.nxt.begin();
	   dstend  [l+1] = dstit  [l] -> second.nxt.end();
		++l;
	} else {
		//ascend
		--l;
		if (l < 0) break;
		//action
			Selector* el = dstit[l]->second.select;
			if (el)
			{       int total = 0;
			        //int card = cardinality_sum(i->first);

			//take it out to be done once only
			        for (int k = 0; k < el->vara. size(); ++k)
			{
				        total  +=  el->vara[k].  second*el->vara[k].  first;
	        }
			        float rcb=.0;
			        if (total) {
			        double invMean = ((double)card[l])/total;
			        for (int k = 0; k < el->vara.size(); ++k) 
			{
				        rcb  += el->vara[k].second * log_approx(1+ el ->vara[k].  second* el->vara[k].  first*invMean);
	        }
        		}
	        
	    if (card[l] > 0) {
		        double confidence  =  range_factor(rcb/ /*log(2)/ */card[l]);
	        el->confidence  = confidence;
	        //cerr << "rcb= " << rcb <<", card = "<<card<<" Confidence  = " << confidence << endl;
	        el->conf_factor = conf.factor_by_confidence.get(confidence);
        } else
	        el -> confidence  = el -> conf_factor = 1.;
	    //for (map<phon*,Selection>::iterator it=el->nxt.begin(); it!=el->nxt.end(); ++it)
	      //    	if (it->second.uffix && it->second.uffix -> abstr[1]) it->second.uffix = it->second.uffix -> abstr[1]; 
    }
     ++dstit[l];
}
} while (true);
}

    inline void cascadeAbstract(const WGConfig& conf) {
	    int max_abstract = max(2,conf.max_ng) - 2;
	    int t = 0;

	    for (AbstractionVector a = 0U; a < 1<<max_abstract; ++a)
#if(O1)
			    abstractOneStep(a,conf);
#else
			    abstractOneStep1(a,conf);
#endif
  }

void statCount(const WGConfig& conf, vector<Lexeme>& lex, int v) {
  size_t word_length_sum = 0;
  vector <Gram> words;
  	for (int n=0; n<lex.size(); ++n)
  		phoneme_decompose(words, lex[n].lx.c_str(), true);
  	for (int n=0; n<words.size(); ++n)
  	{
  		//NGram stat count
		Gram&word = words[n];
			vector<phon*> gram;
		//	cout << w2u(lex[n].lx) << " => " << word << endl;
		if (word.size() > 2) {
#if(O1)
			for (int b = 0; b<word.size(); ++b) {
			//for (int e = b; e-b<MAX_NG && e<word.size(); ++e) {}
			if (gram.size()>=conf.max_ng) gram.erase(gram.begin());
			gram.push_back(word[b]);
			StatAtom & s = NGAStat[0U][gram];
			++s.count[0];
			s.confb = s.conf_factor = 1;
	    }
	    
#else
		Selection* p = 0;
		for (int e = 0, b = 0; e<word.size(); ++e,e-b>=conf.max_ng?++b:0)
		{
			Selector*& pp = NGATRoot[0U].treeNode(word,b,e)->select;
			if (!pp) pp = new Selector(Gram(word.begin()+b, word.begin()+e));
			if (p) p->uffix = pp;
			p = &pp->nxt[word[e]];
		    ++p->count[0];
		    ++pp->count;
		}	       
#endif

  dict_words.insert(Gram(word.begin()+1,word.end()-1));
  word_length_sum += word.size()-2;
  for (int i=1; i<word.size()-1; ++i) {
	  word[i]->freq ++;
	}
  }
  }
  
NGASelRoot = NGATRoot[0U].treeNode(Gram(1,phon::PP_BEGIN),0,1)->select;

	set<int> frq;
    for (map<wstring,phon*>::iterator i=phonemes.begin(); i!=phonemes.end(); ++i)
{

	if (i->second->isVowel()) frq.insert(i->second->freq);
}

	if (frq.size()>=2) {
		set<int>::reverse_iterator fi = frq.rbegin(); ++fi;
		threshold_vowel_freq = *(fi)*conf.freq_vowel_confidence;
        //cerr << "TVFR = " <<  threshold_vowel_freq << endl ;
	}
}

  void printStat(StatAtom& a) {
        for (int v=0; v<VARIANTS; ++v)
        	cout << "\t" << a.count[v];
        cout << "\t" << w2u(a.example);
}

  void printStat(PhonVecStat& s) {
	  for (PhonVecStat::iterator i=s.begin();
	  	i!=s.end(); ++i)
	  {
	  	wstring c1(g2w(i->first));
	  	cout << w2u(c1);
        printStat (i->second);
    }  	
  }

void printStat(map<wstring,StatAtom>& s) {
	  for (map<wstring,StatAtom>::iterator i=s.begin();
	  	i!=s.end(); ++i)
	  {
	  	cout << w2u(i->first);
        for (int v=0; v<VARIANTS; ++v)
        	cout << "\t" << i->second.count[v];
        cout << "\t" << w2u(i->second.example) << endl;
    }  	
  }

void printStat(map<int,StatAtom>& s) {
	  for (map<int,StatAtom>::iterator i=s.begin();
	  	i!=s.end(); ++i)
	  {
	  	cout << i->first;
        for (int v=0; v<VARIANTS; ++v)
        	cout << "\t" << i->second.count[v];
        cout << "\t" << w2u(i->second.example) << endl;
    }
  }
  
  void printStat()
  {
	  cout << "--- N-Grams: " << NGramStat.size() << "\n";
	  for (AbstractionVector a=0U; a<1<<(/*conf.max_ng-2*/ 3); ++a)
	  	printStat(NGAStat[a]);
  }

  inline string g2u(const Gram& s) {
	  stringbuf rcs;
	  for (int i=0; i<s.size(); ++i)
	    {
		    const string& cnv(w2u(s[i]->rep));
		    rcs.sputn(cnv.c_str(),cnv.length());
	    }
	  return rcs.str();
  }

  
  inline AbstractionVector abstrVec(const Gram& gra) {
  AbstractionVector a=0U;
  for (int i=0; i<gra.size(); ++i) if (gra[i]->cardinality) a|=1<<i;
  return a;
  }
  
	inline double factor(int nabs, double conf_factor) {
		return factors[nabs]* conf_factor;
	}
	
	template<class Tgt> static void fill_selector(const Gram& g, int nabs, Tgt& r) {
		int len = g.size();
		AbstractionVector av(abstrVec(g));
		map<Gram,StatAtom>::iterator it = NGAStat[av].lower_bound(g);
	      for (;it!=NGAStat[av].end() && vec_n_eq(g,it->first,len);++it) {
		      if (it->first.size()==len+1)
			{
				int score = it->second.count[0] * factor(nabs,it->second.conf_factor);
		        if (score > 0) r.insert(std::pair<double,phon*>(score, it->first[len]));
			}
		}
	}

	template<class Tgt> static void fill_selector(Selector* s, int nabs, Tgt& r) {
		for (map<phon*,Selection>::iterator it = s->nxt.begin();
	      it!=s->nxt.end();++it) {
				int score = it->second.count[0];
		        //cerr << "score is "<<score<<" for "<<w2u((it->first->rep)) << 
		        //	"(" << s->conf_factor << ")"
		        //<< " " << it->second.count[0] <<endl;
		        if (score > 0) r.insert(std::pair<int,phon*>(score, it->first));
		}
	}

  class NoOptions {};
  template <class E> struct Randomizer {
    map<int,E> scale;
    bool initialized;
    bool n2;
    int total_score;
    void insert(std::pair<int, E> x) {
	    if (x.first > 0) scale[total_score += x.first] = x.second;
    }
    pair<E,double> random() {
#if (O1)
	  if (total_score <= 0)
	  			throw NoOptions();
	  int sel = rand()%total_score;
#else
	  if (!scale.size())
	  			throw NoOptions();
	int sel = rand()%total_score;
#endif
	    typename map<int,E>::iterator seli = scale.upper_bound(sel);
		//cerr<<"Sel: "<<sel<<" / " << total_score << "\n";
	 if (seli==scale.end()) {
		cerr<<"Upper bound failure\n";
		exit(121);
		}
	int sel_score;
	if (seli==scale.begin()) sel_score = seli->first;
	else {
			 typename map<int,E>::iterator prev = seli;
			 prev--;
			 sel_score = seli->first-prev->first;
		 }
		 return pair<E,double>(seli->second,((double)sel_score)/total_score);
}
    Randomizer() : initialized (false), total_score(0), n2(false) {}
    };

  static void initFactors(const WGConfig& conf) {
	  int n = conf.max_abstracted;
//	  int L = conf.confidence.level;
	  factors = new double[n + 1];
	  factors[0] = 1.;
	  	if (n>0) factors[1] = 1;
	  for (int i=1 + 1; i<=n; ++i)
	    factors [i] = factors [i-1] * conf.alpha_factor;
#ifdef COMB_IN_FACTORS
	  if (n > 0) {
		  int combinations = n;
		  factors [1] /= combinations;
		//calculate C(i,n) combination count where n=n
		//and discount factors
		for (int i=2; i<=n; ++i)
		{
        combinations *= (n-i+1);
        combinations /= i;
	    factors [i] /= combinations;
      }
    }
#endif //COMB_IN_FACTORS
  }

  static void initIgnore() {
  	const wchar_t* ignore[] = {L"'",L"|",0};
  	for (const wchar_t** p = ignore; *p; ++p) charCombination[*p] = vector<TokType>(1,TokType(IGNORE_T,0,1));
  }

#include <string.h>
const string selng(Selector* s) {
	return s?w2u(g2w(s->ngram)).c_str():string(">0<");
}
const string selngxx(Selector* s) {
  if ( !s ) return string();
  stringbuf sbuf;
  	for (map<phon*,Selection> ::iterator it = s->nxt.begin();
	it != s->nxt.end(); ++it) {
		string ps = w2u( it->first->rep );
		for (int i = 0; i<ps.length(); ++i)
			sbuf.sputc(ps[i]);
		if (!it->second. uffix) sbuf.sputc('!');
		sbuf.sputc(',');
	}
	return sbuf.str();
}
#define IFDB if (0)


template <class A>
  void mkangs2(vector<phon*>& k, A& accu, int b, int N) {
	  if (b+1>=k.size()) {
		  IFDB cout << "Seek :" << k << " -> ";
		  accu.proc_ng(k,N);
  IFDB cout << "\n";
  }  else {
			mkangs2(k,accu,b+1,N);
		 
		  if (k[b]->abstractable())
		  {
		  phon* concrete = k[b];
			   k[b]=k[b]->abstracted();
		  	mkangs2(k,accu,b+1,N+1);
		  	k[b]=concrete;
	  	}
	  }
  }

// #define DEBUG_ADVANCE_SELECTORS

  void advance_selectors0(vector<Selector*>& v, phon* ph) {
#ifdef DEBUG_ADVANCE_SELECTORS
	  cerr << "Trying   " <<w2u(ph->rep)<<endl;
#endif //DEBUG_ADVANCE_SELECTORS
	  for (int i = (v.size() >> 1)-1; i>=0; --i)
		  if (!v[i] ||
		  	v[i]->nxt.find(ph)==v[i]->nxt.end()) {
		  v[i] = v[i + (v.size() >> 1 ) ];
	 }
	  
	  for (int i = (v.size() >> 1)-1; i>=0; --i) {
		  if (v[i]) {
			  map<phon*,Selection>::iterator it = v[i]->nxt.find(ph);
			  if (it!=v[i]->nxt.end()) {
				Selector* s = it->second.uffix;
#ifdef DEBUG_ADVANCE_SELECTORS
				cerr << "Advancing ["<<i<<"] "<<selng(v[i])<<":"<<selngxx(v[i]);
				cerr << " with " <<w2u(ph->rep) << " to [" << (i<<1) << "]";
				cerr<< selng(s)<<":"<<selngxx(s);
				cerr << " and [" << ((i<<1) | 1) << "]" <<selng(s? s -> abstr[0] : 0)
				<<":"<<selngxx(s? s -> abstr[0] : 0)<<endl;
#endif //DEBUG_ADVANCE_SELECTORS
				v[i<<1] = s;
				v[(i<<1) | 1] = s? s -> abstr[0] : 0;
			} else
				v[i<<1] = v[(i<<1) | 1] = 0;
		  } else
				v[i<<1] = v[(i<<1) | 1] = 0;
	  }
  }
  
void advance_selectors(vector<Selector*>& v, phon* ph) {
	vector<Selector*> tgt(v.size(),(Selector*)0);
#ifdef DEBUG_ADVANCE_SELECTORS
	  cerr << "Trying   " <<w2u(ph->rep)<<endl;
#endif //DEBUG_ADVANCE_SELECTORS
	  int tgti;
	  int sz = v.size();
	  assert(NumberOfSetBits(sz) == 1);
	  for (int i = 0; i<v.size(); ++i) {
		  if (! tgt[tgti = (i<<1) & ~sz] && v[i]) 
	  {
		map<phon*,Selection>::iterator it = 
			v[i]->nxt.find(ph); 
		if(it!=v[i]->nxt.end()) {
			tgt [tgti] = it->second. uffix;
#ifdef DEBUG_ADVANCE_SELECTORS
			cerr << "Advancing ["<<i<<"] "<<selng(v[i])<<":"<<selngxx(v[i]);
			cerr << " with " <<w2u(ph->rep) << " to [" << (tgti) << "]";
			cerr<< selng(tgt [tgti])<<":"<<selngxx(tgt [tgti])
					<< endl;
#endif
  //propagate abstracted
			unsigned int msk = tgti;
			if (tgt[tgti]) //ths should be  true unless ph==PP_END
			for (unsigned int j=masked_inc(tgti,msk); j < sz; j = masked_inc(j,msk))
			{
				int ap = ctz(j);
				Selector*abstr = tgt[tgti|(j&~(1<<ap))]->abstr[ap];
				if (abstr) {
					tgt[tgti|j] = abstr;
#ifdef DEBUG_ADVANCE_SELECTORS
	    			cerr << "Abstrcted target ["<<(tgti|(j&~(1<<ap)))<<"] sent to ["<<(tgti|j)<<"] ";
	    			cerr<< selng(tgt [tgti|j])<<":"<<selngxx(tgt [tgti|j])
					<< endl;				
#endif
				} else msk|=(1<<ap);
	}}}}
		swap(v,tgt);
	}
			  	

typedef Randomizer<Randomizer<phon*>* >  AbstrRandomizer;
typedef multimap<int,phon*,more<int> >	ProliferatorMap;
  struct Proliferator {
    ProliferatorMap m; //relative score -> suffix phoneme
    int total_score;
    void insert(std::pair<int,phon*> x) {
	    total_score += x.first;
	    m.insert(x);
    }
    Proliferator() : total_score(0) {}
  };

  struct Factory {
		map<Gram,Randomizer<phon*> > randomizerCache;
		map<Gram,Proliferator > proliferatorCache;
		Proliferator& proliferator(const Gram& g, int nabs) {
		  pair<map<Gram,Proliferator>::iterator,bool> r = 
		    proliferatorCache.insert(pair<Gram,Proliferator>(g,Proliferator()));
		  Proliferator& p = r.first->second;
		  if (r.second) fill_selector(g,factors[nabs],p);
		  return p;
		}
		Randomizer<phon*> & randomizer(const Gram& g, int nabs) {
		  Randomizer<phon*>& r = randomizerCache[g];
		  if (!r.initialized) {
			  fill_selector(g,factors[nabs],r);
		      r.initialized = true;
		  }
		  return r;
		}
		int total_score(const Gram& g, int nabs) {
					  map<Gram,Randomizer<phon*> >::iterator ri = randomizerCache.find(g);
					  if (ri != randomizerCache.end()) return ri->second.total_score;
					  map<Gram,Proliferator>::iterator pi = proliferatorCache.find(g);
					  if (pi != proliferatorCache.end()) return pi->second.total_score;
					  //Normally we should not be here..
					  cerr << "Warning: (Internal) score calculation overhead for "<<w2u(g2w(g))<<endl;
					  return randomizer(g, nabs).total_score;
		}
		
  } factory;
  
  class AbstrRandomizerBuilder {
	  static map<Gram,Randomizer<phon*> > cache;
	  
	  public:
	  AbstrRandomizer &target;
	  public:
	  void proc_ng(const Gram& g, int nabs) {
		  Randomizer<phon*>& r = factory.randomizer(g,nabs);
		int  ng_total_score = //round(1000*r.total_score*factors[nabs]);
								r.total_score;
		 //cerr << "Med add "<<ng_total_score<< " "<< " = " <<
		 //   r.total_score<< " * " << factors[nabs]<<" "<<nabs<< " " <<g<<endl;
		if (ng_total_score > 0) target.insert(std::pair<int,Randomizer<phon*>*>(ng_total_score,&r)); 
	  }
#define ABS_SCORE_GRANULARITY (128)
	  void proc_ng(Selector* s, int nabs) {
		  if (!s->r) {
			  s->r = new Randomizer<phon*>();
			  fill_selector(s,nabs,*s->r);
			  //cerr << "---"<<nabs<<"-"<<s->r->total_score<<endl;
		  }
		  Randomizer<phon*>& r = *s->r;
		int ng_total_score = s->factor()  * ABS_SCORE_GRANULARITY;
//		int  ng_total_score = //round(1000*r.total_score*factors[nabs]);
//								r.total_score;
		 //cerr << "Med add "<<ng_total_score<<" , " << s->count << " , " << nabs << " , "<<s->conf_factor<< endl;
		if (ng_total_score > 0) target.insert(std::pair<int,Randomizer<phon*>*>(ng_total_score,&r)); 
	  }
	  AbstrRandomizerBuilder(AbstrRandomizer& t) : target(t) {}
  };
  
  struct ScoreTracker {
	  double num, denom;
	  int nways;
	  phon* suf;
	  void proc_ng(Gram& g, int nabs) {
		denom += factory.total_score(g, nabs);
		  g.push_back(suf);
		AbstractionVector av(abstrVec(g));
	    map<Gram,StatAtom>::iterator it = NGAStat[av].find(g);
		  g.pop_back();
		  if (it != NGAStat[av].end())
            {num += it->second.count[0] * factor(nabs,it->second.conf_factor);
          if (it->second.count[0] * factor(nabs,it->second.conf_factor))
            ++nways;
        }
  }
		ScoreTracker(phon* _suf) : num(0), denom(0), suf(_suf),
		nways(0) {}
  };

  
  double honestScore(Gram w, const WGConfig& conf) {
	  Gram g;
	  double s = 1.0;
	  size_t i=0, l = w.size();
	  if (l > 0 && w[i] == phon::PP_BEGIN) ++i;	  
	  if (l > 0 && w[l-1] == phon::PP_DICT_MARKER) {w.pop_back();--l;}
	  if (l > 0 && w[l-1] != phon::PP_END) {w.push_back(phon::PP_END);++l;}
	  
	  g.push_back(phon::PP_BEGIN);
#if (O1)
#else
    vector <Selector*> selectors;  //check bounds?
    selectors.assign(1<<(conf.max_ng-2),(Selector*)0);
    selectors[0] = NGASelRoot;    
#endif
	  for (; i<l;++i) {
#if (O1)
		  g.push_back(w[i]);
		  if (g.size() > conf.max_ng - 1)
		  	g.erase(g.begin());//optimize?
		  ScoreTracker st(i+1<l? w[i+1] : phon::PP_END);
	      mkangs2(g,st,0,0);
	      (s *= st.num) /= st.denom;
	      //cout << "NWays: "<<st.nways<<" for " << w2u(g2w(w))<<"[.."<<i<<"]; "<<st.num<<" / " << st.denom << ": "<<s<< endl ;
#else
		int nways=0;
		double num=0., denom=0.;
		for (int j=0; j<selectors.size(); ++j)
			if (selectors[j]) {
				map <phon*, Selection>::iterator it = selectors[j]->nxt.find(w[i]);
				if (it != selectors[j]->nxt.end()) {
				int nabs = NumberOfSetBits(j);
				nways ++;
				double share =  selectors[j]->factor();
				denom += share;
       			num += share*it->second.count[0]/selectors[j]->count ;
  				}
  			}
				(s *= num) /= denom;
	    //  cout << "NWays: "<< nways<<" for " << w2u(w[i]->rep)<<"; "<<num<<" / " << denom << ": "<<s<< endl ;
		advance_selectors(selectors,w[i]);					
#endif
      }
      return s;
  }
  
  pair<phon*,double> select_random(Gram& prefix) {
	AbstrRandomizer abstr_r;
    AbstrRandomizerBuilder b(abstr_r);
	mkangs2(prefix,b,0,0);
    pair <Randomizer<phon*>*,double> r = abstr_r.random();
	pair <phon*,double> ans = r.first->random();
	//if (trueScore) {for all (abstr_r) s+=scoreof();}
	return pair<phon*,double>(ans.first,r.second*ans.second);	
  }

  pair<phon*,double> select_random(vector<Selector*>& v) {
	AbstrRandomizer abstr_r;
    AbstrRandomizerBuilder b(abstr_r);
	for (int i=0; i<v.size(); ++i) if (v[i]) b.proc_ng(v[i],NumberOfSetBits(i));
    pair <Randomizer<phon*>*,double> r = abstr_r.random();
    if (r.first->n2)  cerr << "n2!!!"<<endl;
	pair <phon*,double> ans = r.first->random();
	//if (trueScore) {for all (abstr_r) s+=scoreof();}
	return pair<phon*,double>(ans.first,r.second*ans.second);	
  }

  void printNG(WGConfig& conf) {
map<phon*,StatTreeNode>::iterator dstit[conf.max_ng+1],dstend[conf.max_ng+1];
for (int a =0; a < 1<<conf.max_ng-2; ++a){
	dstit [0] = NGATRoot[a].nxt.begin();
dstend[0] = NGATRoot[a].nxt.end ();
int l = 0;

do {
	if (dstit[l] != dstend[l]) {
	   //descend
	   dstit   [l+1] = dstit  [l] -> second.nxt.begin();
	   dstend  [l+1] = dstit  [l] -> second.nxt.end();
		++l;
	} else {
		//ascend
		--l;
		if (l < 0) break;
		//action
				cout << selng(dstit[l]->second. select)<<". ";
		for (int j=0; j<=l; ++j)
				cout <<w2u(dstit[j]->first->rep);
				cout<<endl;
		if (dstit[l]->second. select) {
			for (int j=0; j<conf.max_ng-2; j++)
				cout<< "\t"<<selng(dstit[l]->second. select->abstr[j]);
			cout<< "\t"<< "{" << dstit[l]->second. select->confidence<<"}"<< dstit[l]->second. select->factor();
#define OUT_CONF_EXPLANATION
#ifdef OUT_CONF_EXPLANATION
		  cout << "(conf = ";
		  
		  for (int i=0; i < dstit[l]->second. select->vara.size(); ++i)
		  	cout << dstit[l]->second. select->vara[i].first << "!" <<
		  	dstit[l]->second. select->vara[i].second<<";";
		  cout << ")";
#endif
		  cout<<endl;
					for (map<phon*,Selection>::iterator it = dstit[l]->second. select ->nxt.begin();
					it != dstit[l]->second. select ->nxt.end();   ++it) 
					cout << "\t|" << w2u(it->first->rep)<< "\t" << selng(it->second.uffix) <<endl;
					
				}
     ++dstit[l];
}
} while (true);
}
}
 
  pair<Gram,double> gen21(const WGConfig& conf) {
//    phonemes[L"$"]=phon::PP_END;//no hack
    Gram gram;
    string err;
    wstringbuf sbuf;
    pair<Gram,double> ws;
    wostream word(&sbuf);
	double sel_score = 1.0;
    gram.push_back(phon::PP_BEGIN);
    bool odd = false;
    vector <Selector*> selectors;  //check bounds?
    selectors.assign(1<<(conf.max_ng-2),(Selector*)0);
    selectors[0] = NGASelRoot;
    while (gram.back() != phon::PP_END) {     
     //keeping it at most max_ng-1 long
     if (gram.size()>=conf.max_ng) gram.erase(gram.begin());
	try {
#if (O1)
		  pair<phon*,double> sel = select_random(gram);
#else
		  pair<phon*,double> sel = select_random(selectors);
#endif
	sel_score*= sel.second;
	  if (sel.first == phon::PP_END) break;
	  gram.push_back(sel.first);
	  advance_selectors(selectors, sel.first);
	  ws.first.push_back(sel.first);
	  word << sel.first->rep;
	} catch (NoOptions noe) 
	  {
		  err = string("Unsuccessful at ")+w2u(sbuf.str());
		  //cerr << err <<endl;
		  return pair<Gram,double>(Gram(),0.0);
	  }
	}
	/*if (lexemes.find(sbuf.str())!=lexemes.end())
		if (!conf.allow_hits) {
			err = "Hit";
		    return pair<Gram,double>(Gram(),0.0);
		} else word << L" #";
	if (sel_score < conf.min_score) {
			err = "Low probability";
		   return pair<Gram,double>(Gram(),0.0);
		
	}
	if (conf.display_score) ((word<<L" (")<<sel_score)<<L")";
	*/
    ws.second = sel_score;
    return ws;
  }

 
void classifyPhonemes() {
	for (map<wstring,phon*>::iterator ph_i = phonemes.begin(); ph_i != phonemes.end(); ph_i++)
	{
		if (ph_i->second->abstractable())
			++ph_i->second->abstracted()->cardinality; 
	}
}

struct CmdArgs {
	std::vector<std::string> files, ifiles;
	bool http;
	bool defcc;
	const char* prog;
	WGConfig& config;
	vector<string> inputs;

	void usage_exit() {
		std::cerr<<"Usage: " << prog << " [-h] {-i init_file} {-l language} {-n max_ngram} {-a max_abstracted} {-s} {-f alpha_factor} {-c count} [-H] [-P]" << std::endl;
		exit(2);
	}
	void next (int& i, int argc) {if (++i>=argc) usage_exit();}
	CmdArgs   (int argc, char** argv, WGConfig& conf) : prog(argv[0]), config(conf), http(false), defcc(false)
	{
	  for (int i = 1; i<argc; ++i) {
		if (*argv[i]=='-') {
			if (argv[i][1]==0 || argv[i][2]!=0) usage_exit();
			switch (argv[i][1]) {
				case 'h': http = true;
				break;
				case 'i': next(i, argc); ifiles.push_back(argv[i]);
				break;
				case 'd': defcc = true;
				break;
				case 'l':  next(i, argc); config.lang = argv[i];
				break;
				case 'n':  next(i, argc); config.max_ng = strtol(argv[i],0,0);
				break;
				case 's':  next(i, argc); config.syl_max = strtol(argv[i],0,0);
				break;
				case 'P':  config.pr_stat=true;
				break;
				case 't':  config.top_list=true;
				break;
				case 'a':  next(i, argc); config.alpha_factor = strtod(argv[i],0);
				break;
				case 'f':  next(i, argc);
					try {config.factor_by_confidence.parse_insert(argv[i]);}
					catch (string e) {cerr<<e; exit(1);}
				break;
				case 'V':  next(i, argc); config.freq_vowel_confidence = strtod(argv[i],0);
				break;
				case 'r':  next(i, argc); config.random_seed = strtol(argv[i],0,0);
				break;
				case 'c':  next(i, argc); config.ngen = strtol(argv[i],0,0);
				break;
				case 'H':  config.allow_hits = true;
				break;
				case 'm':  next(i, argc); config.min_score = strtod(argv[i],0);
				break;
				case 'C':  config.checker = true;
				break;
				case 'v':  next(i, argc); config.  verbose = strtol(argv[i],0,0);
				break;
				case 'T':  config.sort_score = true;
				break;
				case '%':  config.display_score = true;
				break;
				default:
				usage_exit();
			}
		} else 
			inputs.push_back(argv[i]);	  
	  }
	}
};
  void   addHeadwordOnlyLexemes(vector<Lexeme> & lex) {
	  for (int i=0; i<headWord.size(); ++i)
	  {
	    lex.push_back(Lexeme());
	    lex.back().lx=headWord[i];
	  }
  }
  
  struct ProlIterBase;
  struct ProlIter {
	  ConstSmartPtr<ProlIterBase> b;
	  ProliferatorMap::iterator i, e;
	  double div_base_score; //divided by the total population of b.selector[my_inndex]
	  double score() {
		return div_base_score*i->first;
		}
	  ProlIter(ProlIterBase* base, double bs, ProliferatorMap& m)
	  :i(m.begin()), e(m.end()), div_base_score(bs), b(base) {}
  };

	struct ProlIterBase {
	  static map<Gram,Proliferator > cache;
	  multimap<double,ProlIter>& target_queue;
	  vector<Selector*> selectors;	  
	  double base_score;
	  Gram word; //full concrete prefix from the word beginning
	  int nuser;

	  public:
	  void fire() {
	  }
	  
	  void proc_ng(const Gram& g, int nabs) {
		  pair<map<Gram,Proliferator>::iterator,bool> r = 
		    cache.insert(pair<Gram,Proliferator>(g,Proliferator()));
		  Proliferator& p = factory.proliferator(g,nabs);

		//burst to target queue
		if (!p.m.empty()) {
			pair<double,ProlIter> entry(0.,ProlIter (this,base_score,p.m));
			entry.first = entry.second.score();
			target_queue.insert(entry);
	  		}
	  }
	  ProlIterBase(const Gram& w, vector<Selector*>& v,
	    double bs, multimap<double,ProlIter>& t /*obsolete */
	    ) : nuser(0), word(w), base_score(bs), selectors(v), target_queue(t) {}
	  void proc_ng(Selector* s, vector<Selector*>& v, int nabs, multimap<double,ProlIter>& target_queue,
	  	double share_score) {
			//if (share * ABS_SCORE_GRANULARITY< 1) return ;
			if (!s->p) {
			  s->p = new Proliferator();
			  fill_selector(s,nabs,*s->p);
			  //cerr << "---"<<nabs<<"-"<<s->r->total_score<<endl;
		  }
		//burst to target queue
		if (!s-> p->m.empty()) 
		{
			ProlIter entry(this, share_score/s->count, s->p->m);
			target_queue.insert(
		  		pair<double,ProlIter>(entry.score(),entry));
	  	}
	}
};
  
	  inline void burst(Gram word /*copy!*/, vector<Selector*> v/*copy!*/, phon* suff, double base_score, 
		multimap<double,ProlIter>& target_queue, const WGConfig& conf)
	  {
		word.push_back(suff);
		static set<Gram> bursten;
		if (bursten.insert(word).second) { 
		int len = min(conf.max_ng-1, (int)word.size());
#if (O1)
		Gram g(word.end()-len,word.end());
		mkangs2(g,ap,0,0);
#else
		double denom =  0.;
		advance_selectors(v, suff);
		for (int i=0; i<v.size(); ++i) if (v[i])
		{
			denom += v[i]->factor();
		}
		ProlIterBase* b = new ProlIterBase(word, v, base_score,  target_queue);
		for (int i=0; i<v.size(); ++i) if (v[i])
		{
			int nabs = NumberOfSetBits(i);
			double share = (v[i]->factor());
			b->proc_ng(v[i], v, nabs, target_queue, base_score*share/denom);
		}
#endif	
	  }
}
  const int max_gen = 65535; //hard limit of generated random word count
  const int max_burst = 4*max_gen; //hard limit of phoneme advance attempts while generating top-probability words

void genext(map<Gram,double,LexCompare>& out, const WGConfig& conf)
{
    multimap<double,ProlIter> queue;

    Gram start;
    vector<Selector*> v(1<<(max(conf.max_ng,2)-2), (Selector*)0);
    v[0] = NGATRoot[0]. select;
    start.push_back(phon::PP_BEGIN);
    burst(Gram(),v,phon::PP_BEGIN,1.,queue, conf);
    int iter_count = 0;
    while (!queue.empty() && out.size() < conf.ngen && ++iter_count < max_burst) {
	    pair<double,ProlIter> s = *(--queue.end());
	    queue.erase(--queue.end());
	    if (s.second.i->second == phon::PP_END) 
	    	{
		    	Gram word (s.second.b->word.begin()+1,s.second.b->word.end());  //removing '^'
		    	double score = (conf.min_score > 0.) ? honestScore(word,conf) : s.first;
		    	if (filter(out, conf, score, word)) out.insert(pair<const Gram&,double>(word, score));
    	    }
    	else burst(s.second.b->word,s.second.b->selectors,s.second.i->second,s.first,queue,conf);
	    if (++s.second.i != s.second.e)
	    
	      //re-insert
	      {
		      s.first = s.second.score();
		      if (s.first >= conf.min_score) queue.insert(s);
	      }
    }
}

	bool cmpScore(const pair<const Gram,double>* s1, const pair<const Gram,double>* s2) {
		//reverse compare for ranking
		return s1->second > s2->second;
	}
	
  inline void printScore(ostream& out, const Gram& w, double score, const WGConfig& conf )
  {
	  	if (conf.display_score) {
						out<< "(";
						if (conf.min_score > 0.)
							out << score; //honest score is already calculated
						else
						{
							if (conf.verbose > 2)
								out<<score<<"/";
							out  << honestScore(w, conf);
						}
						out << ")";
}}
  
  int main(int argc, char** argv) {
      WGConfig conf;
      CmdArgs args(argc,argv,conf);
      initFactors(conf);
      initIgnore();
      vector<Lexeme> lex;
	  for (int i=0; i<args.inputs.size();++i)
	    read_all_lines(args.inputs[i].c_str(),charCombFillMap(),&charCombination,headWord);
	  addHeadwordOnlyLexemes(lex);
	  time_t t;
#ifdef PROFILE
	  clock_t start_clock;
	  start_clock = clock();
#endif
	  statCount(conf,lex,0);
	  dictionaryWarnings();
	// for (double i = 0.; i <= 1.; i+=.1) cerr << "@ "<<i<<":"<<conf.factor_by_confidence.get(i)<<endl;
	classifyPhonemes();
#ifdef PROFILE
	cerr << "Stat ready: " << clock() - start_clock << " ticks" << endl;
#endif	
	    cascadeAbstract(conf);
#ifdef PROFILE
	cerr << "Abstracted: " << clock() - start_clock << " ticks" << endl;
#endif	
	  if (conf.pr_stat) {
		  printStat();
  	  }
	  if (conf.verbose > 11)
	     printNG( conf );

	if (dict_words. empty()) {
		cerr<<"Error: Empty dictionary."<<endl;
		exit(1);
	}
		  srand(conf.random_seed != -1L? conf.random_seed: time(&t));
		  map<Gram,double,LexCompare> rank;
	  if (conf.top_list) {
		  genext(rank, conf);
  	} else
		  for (int i=0, j=0; i < max_gen && j <conf.ngen; ++i) {
		  pair<Gram,double> ws (gen21(conf));
		  	if (conf.min_score > 0.) ws.second = honestScore(ws.first , conf);
		  if (!ws.first.empty() && filter(rank, conf, ws.second, ws.first) ) {
		  	rank.insert(ws); ++j;
	  }
  }
	  vector<const pair<const Gram,double>* > v;
	  		for (map<Gram,double>::iterator it = rank.begin(); it != rank.end(); ++it)
			{
				//it->second.erase(it->second.begin());
				wstring w(g2w(it->first) );
				if (gen_words.find(it->first) == gen_words.end()) 
				if (conf.sort_score)
				v.push_back(&*it);
				else{
					cout << w2u(w);
					printScore(cout , it->first, it->second,conf);
			  		cout <<endl;
				}
			gen_words.insert(it->first);
			}
			stable_sort(v.begin(),v.end(),cmpScore);
			for (int i=0; i<v.size(); ++i) {
				wstring w(g2w(v[i]->first) );
					cout << w2u(w);
					printScore(cout , v[i]->first, v[i]->second,conf);
					cout <<endl;
			}

#ifdef PROFILE
	cerr << "Generated: " << clock() - start_clock << " ticks" << endl;
#endif	
  if (conf.verbose > 0)
	report_rejected();
  exit(0);
	  return 0;
  }
  map<Gram,Proliferator>ProlIterBase::cache;//Check smartpointer delete!!

  map<Gram,Randomizer<phon*> >AbstrRandomizerBuilder::cache;
  Lexeme::M Lexeme::m;
  
   
  phon* phon::PP_BEGIN = new phon(L"^",1);
  phon* phon::PP_END   = new phon(L"$",2);
  phon* phon::PP_DICT_MARKER   = new phon(L"#",17);
  phon* phon::PP_VOWEL = new phon(L"V",3);
  phon* phon::PP_FREQUENT_VOWEL = new phon(L"W",4);
  phon* phon::PP_CONSONANT   = new phon(L"C",5);

