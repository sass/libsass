#include "ast.hpp"
#include "context.hpp"
#include "contextualize.hpp"
#include "node.hpp"
#include "sass_util.hpp"
#include "extend.hpp"
#include "to_string.hpp"
#include <set>
#include <algorithm>
#include <iostream>

namespace Sass {
  using namespace std;

  bool Compound_Selector::operator<(const Compound_Selector& rhs) const
  {
    To_String to_string;
    // ugly
    return const_cast<Compound_Selector*>(this)->perform(&to_string) <
           const_cast<Compound_Selector&>(rhs).perform(&to_string);
  }

  bool Complex_Selector::operator<(const Complex_Selector& rhs) const
  {
    To_String to_string;
    return const_cast<Complex_Selector*>(this)->perform(&to_string) <
           const_cast<Complex_Selector&>(rhs).perform(&to_string);
  }

  bool Complex_Selector::operator==(const Complex_Selector& rhs) const {
  	// TODO: We have to access the tail directly using tail_ since ADD_PROPERTY doesn't provide a const version.

  	const Complex_Selector* pOne = this;
    const Complex_Selector* pTwo = &rhs;

    // Consume any empty references at the beginning of the Complex_Selector
    if (pOne->combinator() == Complex_Selector::ANCESTOR_OF && pOne->head()->is_empty_reference()) {
    	pOne = pOne->tail_;
    }
    if (pTwo->combinator() == Complex_Selector::ANCESTOR_OF && pTwo->head()->is_empty_reference()) {
    	pTwo = pTwo->tail_;
    }

    while (pOne && pTwo) {
    	if (pOne->combinator() != pTwo->combinator()) {
      	return false;
      }

      if (*(pOne->head()) != *(pTwo->head())) {
      	return false;
      }

    	pOne = pOne->tail_;
      pTwo = pTwo->tail_;
    }

    return pOne == NULL && pTwo == NULL;
  }

  Compound_Selector* Compound_Selector::unify_with(Compound_Selector* rhs, Context& ctx)
  {
    Compound_Selector* unified = rhs;
    for (size_t i = 0, L = length(); i < L; ++i)
    {
      if (!unified) break;
      else          unified = (*this)[i]->unify_with(unified, ctx);
    }
    return unified;
  }

  bool Simple_Selector::operator==(const Simple_Selector& rhs) const
  {
  	// Compare the string representations for equality.

  	// Cast away const here. To_String should take a const object, but it doesn't.
  	Simple_Selector* pLHS = const_cast<Simple_Selector*>(this);
    Simple_Selector* pRHS = const_cast<Simple_Selector*>(&rhs);

    To_String to_string;
    return pLHS->perform(&to_string) == pRHS->perform(&to_string);
  }

  bool Simple_Selector::operator<(const Simple_Selector& rhs) const {
		// Use the string representation for ordering.

  	// Cast away const here. To_String should take a const object, but it doesn't.
  	Simple_Selector* pLHS = const_cast<Simple_Selector*>(this);
    Simple_Selector* pRHS = const_cast<Simple_Selector*>(&rhs);

    To_String to_string;
    return pLHS->perform(&to_string) < pRHS->perform(&to_string);
  }

  Compound_Selector* Simple_Selector::unify_with(Compound_Selector* rhs, Context& ctx)
  {
    To_String to_string(&ctx);
    for (size_t i = 0, L = rhs->length(); i < L; ++i)
    { if (perform(&to_string) == (*rhs)[i]->perform(&to_string)) return rhs; }

    // check for pseudo elements because they need to come last
    size_t i, L;
    bool found = false;
    if (typeid(*this) == typeid(Pseudo_Selector) || typeid(*this) == typeid(Wrapped_Selector))
    {
      for (i = 0, L = rhs->length(); i < L; ++i)
      {
        if ((typeid(*(*rhs)[i]) == typeid(Pseudo_Selector) || typeid(*(*rhs)[i]) == typeid(Wrapped_Selector)) && (*rhs)[L-1]->is_pseudo_element())
        { found = true; break; }
      }
    }
    else
    {
      for (i = 0, L = rhs->length(); i < L; ++i)
      {
        if (typeid(*(*rhs)[i]) == typeid(Pseudo_Selector) || typeid(*(*rhs)[i]) == typeid(Wrapped_Selector))
        { found = true; break; }
      }
    }
    if (!found)
    {
      Compound_Selector* cpy = new (ctx.mem) Compound_Selector(*rhs);
      (*cpy) << this;
      return cpy;
    }
    Compound_Selector* cpy = new (ctx.mem) Compound_Selector(rhs->pstate());
    for (size_t j = 0; j < i; ++j)
    { (*cpy) << (*rhs)[j]; }
    (*cpy) << this;
    for (size_t j = i; j < L; ++j)
    { (*cpy) << (*rhs)[j]; }
    return cpy;
  }

  Compound_Selector* Type_Selector::unify_with(Compound_Selector* rhs, Context& ctx)
  {
    // TODO: handle namespaces

    // if the rhs is empty, just return a copy of this
    if (rhs->length() == 0) {
      Compound_Selector* cpy = new (ctx.mem) Compound_Selector(rhs->pstate());
      (*cpy) << this;
      return cpy;
    }

    // if this is a universal selector and rhs is not empty, just return the rhs
    if (name() == "*")
    { return new (ctx.mem) Compound_Selector(*rhs); }


    Simple_Selector* rhs_0 = (*rhs)[0];
    // otherwise, this is a tag name
    if (typeid(*rhs_0) == typeid(Type_Selector))
    {
      // if rhs is universal, just return this tagname + rhs's qualifiers
      if (static_cast<Type_Selector*>(rhs_0)->name() == "*")
      {
        Compound_Selector* cpy = new (ctx.mem) Compound_Selector(rhs->pstate());
        (*cpy) << this;
        for (size_t i = 1, L = rhs->length(); i < L; ++i)
        { (*cpy) << (*rhs)[i]; }
        return cpy;
      }
      // if rhs is another tag name and it matches this, return rhs
      else if (static_cast<Type_Selector*>(rhs_0)->name() == name())
      { return new (ctx.mem) Compound_Selector(*rhs); }
      // else the tag names don't match; return nil
      else
      { return 0; }
    }
    // else it's a tag name and a bunch of qualifiers -- just append them
    Compound_Selector* cpy = new (ctx.mem) Compound_Selector(rhs->pstate());
    (*cpy) << this;
    (*cpy) += rhs;
    return cpy;
  }

  Compound_Selector* Selector_Qualifier::unify_with(Compound_Selector* rhs, Context& ctx)
  {
    if (name()[0] == '#')
    {
      for (size_t i = 0, L = rhs->length(); i < L; ++i)
      {
        Simple_Selector* rhs_i = (*rhs)[i];
        if (typeid(*rhs_i) == typeid(Selector_Qualifier) &&
            static_cast<Selector_Qualifier*>(rhs_i)->name()[0] == '#' &&
            static_cast<Selector_Qualifier*>(rhs_i)->name() != name())
          return 0;
      }
    }
    rhs->has_line_break(has_line_break());
    return Simple_Selector::unify_with(rhs, ctx);
  }

  Compound_Selector* Pseudo_Selector::unify_with(Compound_Selector* rhs, Context& ctx)
  {
    if (is_pseudo_element())
    {
      for (size_t i = 0, L = rhs->length(); i < L; ++i)
      {
        Simple_Selector* rhs_i = (*rhs)[i];
        if (typeid(*rhs_i) == typeid(Pseudo_Selector) &&
            static_cast<Pseudo_Selector*>(rhs_i)->is_pseudo_element() &&
            static_cast<Pseudo_Selector*>(rhs_i)->name() != name())
        { return 0; }
      }
    }
    return Simple_Selector::unify_with(rhs, ctx);
  }

  bool Compound_Selector::is_superselector_of(Compound_Selector* rhs)
  {
    To_String to_string;

    Simple_Selector* lbase = base();
    Simple_Selector* rbase = rhs->base();

    // Check if pseudo-elements are the same between the selectors

    set<string> lpsuedoset, rpsuedoset;
    for (size_t i = 0, L = length(); i < L; ++i)
    {
    	if ((*this)[i]->is_pseudo_element()) {
      	string pseudo((*this)[i]->perform(&to_string));
        pseudo = pseudo.substr(pseudo.find_first_not_of(":")); // strip off colons to ensure :after matches ::after since ruby sass is forgiving
      	lpsuedoset.insert(pseudo);
      }
    }
    for (size_t i = 0, L = rhs->length(); i < L; ++i)
    {
    	if ((*rhs)[i]->is_pseudo_element()) {
      	string pseudo((*rhs)[i]->perform(&to_string));
        pseudo = pseudo.substr(pseudo.find_first_not_of(":")); // strip off colons to ensure :after matches ::after since ruby sass is forgiving
	    	rpsuedoset.insert(pseudo);
      }
    }
  	if (lpsuedoset != rpsuedoset) {
      return false;
    }

		// Check the Simple_Selectors

    set<string> lset, rset;

    if (!lbase) // no lbase; just see if the left-hand qualifiers are a subset of the right-hand selector
    {
      for (size_t i = 0, L = length(); i < L; ++i)
      { lset.insert((*this)[i]->perform(&to_string)); }
      for (size_t i = 0, L = rhs->length(); i < L; ++i)
      { rset.insert((*rhs)[i]->perform(&to_string)); }
      return includes(rset.begin(), rset.end(), lset.begin(), lset.end());
    }
    else { // there's an lbase
      for (size_t i = 1, L = length(); i < L; ++i)
      { lset.insert((*this)[i]->perform(&to_string)); }
      if (rbase)
      {
        if (lbase->perform(&to_string) != rbase->perform(&to_string)) // if there's an rbase, make sure they match
        { return false; }
        else // the bases do match, so compare qualifiers
        {
          for (size_t i = 1, L = rhs->length(); i < L; ++i)
          { rset.insert((*rhs)[i]->perform(&to_string)); }
          return includes(rset.begin(), rset.end(), lset.begin(), lset.end());
        }
      }
    }
    // catch-all
    return false;
  }

  bool Compound_Selector::operator==(const Compound_Selector& rhs) const {
    To_String to_string;

    // Check if pseudo-elements are the same between the selectors

    set<string> lpsuedoset, rpsuedoset;
    for (size_t i = 0, L = length(); i < L; ++i)
    {
    	if ((*this)[i]->is_pseudo_element()) {
      	string pseudo((*this)[i]->perform(&to_string));
        pseudo = pseudo.substr(pseudo.find_first_not_of(":")); // strip off colons to ensure :after matches ::after since ruby sass is forgiving
      	lpsuedoset.insert(pseudo);
      }
    }
    for (size_t i = 0, L = rhs.length(); i < L; ++i)
    {
    	if (rhs[i]->is_pseudo_element()) {
      	string pseudo(rhs[i]->perform(&to_string));
        pseudo = pseudo.substr(pseudo.find_first_not_of(":")); // strip off colons to ensure :after matches ::after since ruby sass is forgiving
	    	rpsuedoset.insert(pseudo);
      }
    }
  	if (lpsuedoset != rpsuedoset) {
      return false;
    }

		// Check the base

    const Simple_Selector* const lbase = base();
    const Simple_Selector* const rbase = rhs.base();

    if ((lbase && !rbase) ||
    	(!lbase && rbase) ||
      ((lbase && rbase) && (*lbase != *rbase))) {
			return false;
    }


    // Check the rest of the SimpleSelectors
    // Use string representations. We can't create a set of Simple_Selector pointers because std::set == std::set is going to call ==
    // on the pointers to determine equality. I don't know of a way to pass in a comparison object. The one you can specify as part of
    // the template type is used for ordering, but not equality. We also can't just put in non-pointer Simple_Selectors because the
    // class is intended to be subclassed, and we'd get splicing.

    set<string> lset, rset;

    for (size_t i = 0, L = length(); i < L; ++i)
    { lset.insert((*this)[i]->perform(&to_string)); }
    for (size_t i = 0, L = rhs.length(); i < L; ++i)
    { rset.insert(rhs[i]->perform(&to_string)); }

    return lset == rset;
  }

  bool Complex_Selector_Pointer_Compare::operator() (const Complex_Selector* const pLeft, const Complex_Selector* const pRight) const {
    return *pLeft < *pRight;
  }

  bool Complex_Selector::is_superselector_of(Compound_Selector* rhs)
  {
    if (length() != 1)
    { return false; }
    return base()->is_superselector_of(rhs);
  }

  bool Complex_Selector::is_superselector_of(Complex_Selector* rhs)
  {
    Complex_Selector* lhs = this;
    To_String to_string;
    // check for selectors with leading or trailing combinators
    if (!lhs->head() || !rhs->head())
    { return false; }
    Complex_Selector* l_innermost = lhs->innermost();
    if (l_innermost->combinator() != Complex_Selector::ANCESTOR_OF && !l_innermost->tail())
    { return false; }
    Complex_Selector* r_innermost = rhs->innermost();
    if (r_innermost->combinator() != Complex_Selector::ANCESTOR_OF && !r_innermost->tail())
    { return false; }
    // more complex (i.e., longer) selectors are always more specific
    size_t l_len = lhs->length(), r_len = rhs->length();
    if (l_len > r_len)
    { return false; }

    if (l_len == 1)
    { return lhs->head()->is_superselector_of(rhs->base()); }

    bool found = false;
    Complex_Selector* marker = rhs;
    for (size_t i = 0, L = rhs->length(); i < L; ++i) {
      if (i == L-1)
      { return false; }
      if (lhs->head()->is_superselector_of(marker->head()))
      { found = true; break; }
      marker = marker->tail();
    }
    if (!found)
    { return false; }

    /*
      Hmm, I hope I have the logic right:

      if lhs has a combinator:
        if !(marker has a combinator) return false
        if !(lhs.combinator == '~' ? marker.combinator != '>' : lhs.combinator == marker.combinator) return false
        return lhs.tail-without-innermost.is_superselector_of(marker.tail-without-innermost)
      else if marker has a combinator:
        if !(marker.combinator == ">") return false
        return lhs.tail.is_superselector_of(marker.tail)
      else
        return lhs.tail.is_superselector_of(marker.tail)
    */
    if (lhs->combinator() != Complex_Selector::ANCESTOR_OF)
    {
      if (marker->combinator() == Complex_Selector::ANCESTOR_OF)
      { return false; }
      if (!(lhs->combinator() == Complex_Selector::PRECEDES ? marker->combinator() != Complex_Selector::PARENT_OF : lhs->combinator() == marker->combinator()))
      { return false; }
      return lhs->tail()->is_superselector_of(marker->tail());
    }
    else if (marker->combinator() != Complex_Selector::ANCESTOR_OF)
    {
      if (marker->combinator() != Complex_Selector::PARENT_OF)
      { return false; }
      return lhs->tail()->is_superselector_of(marker->tail());
    }
    else
    {
      return lhs->tail()->is_superselector_of(marker->tail());
    }
    // catch-all
    return false;
  }

  size_t Complex_Selector::length()
  {
    // TODO: make this iterative
    if (!tail()) return 1;
    return 1 + tail()->length();
  }

  Compound_Selector* Complex_Selector::base()
  {
    if (!tail()) return head();
    else return tail()->base();
  }

  Complex_Selector* Complex_Selector::context(Context& ctx)
  {
    if (!tail()) return 0;
    if (!head()) return tail()->context(ctx);
    Complex_Selector* cpy = new (ctx.mem) Complex_Selector(pstate(), combinator(), head(), tail()->context(ctx));
    cpy->media_block(media_block());
    cpy->last_block(last_block());
    return cpy;
  }

  Complex_Selector* Complex_Selector::innermost()
  {
    if (!tail()) return this;
    else         return tail()->innermost();
  }

  Complex_Selector::Combinator Complex_Selector::clear_innermost()
  {
    Combinator c;
    if (!tail() || tail()->length() == 1)
    { c = combinator(); combinator(ANCESTOR_OF); tail(0); }
    else
    { c = tail()->clear_innermost(); }
    return c;
  }

  void Complex_Selector::set_innermost(Complex_Selector* val, Combinator c)
  {
    if (!tail())
    { tail(val); combinator(c); }
    else
    { tail()->set_innermost(val, c); }
  }

  Complex_Selector* Complex_Selector::clone(Context& ctx) const
  {
    Complex_Selector* cpy = new (ctx.mem) Complex_Selector(*this);
    if (tail()) cpy->tail(tail()->clone(ctx));
    return cpy;
  }

  Complex_Selector* Complex_Selector::cloneFully(Context& ctx) const
  {
    Complex_Selector* cpy = new (ctx.mem) Complex_Selector(*this);

    if (head()) {
    	cpy->head(head()->clone(ctx));
    }

    if (tail()) {
      cpy->tail(tail()->cloneFully(ctx));
    }

    return cpy;
  }

  Compound_Selector* Compound_Selector::clone(Context& ctx) const
  {
    Compound_Selector* cpy = new (ctx.mem) Compound_Selector(*this);
    return cpy;
  }



  /* not used anymore - remove?
  Selector_Placeholder* Selector::find_placeholder()
  {
    return 0;
  }*/

  void Selector_List::adjust_after_pushing(Complex_Selector* c)
  {
    if (c->has_reference())   has_reference(true);
    if (c->has_placeholder()) has_placeholder(true);

#ifdef DEBUG
    To_String to_string;
    this->mCachedSelector(this->perform(&to_string));
#endif
  }
  
  

  // For every selector in RHS, see if we have /any/ selectors which are a super-selector of it
  bool Selector_List::is_superselector_of(Sass::Selector_List *rhs) {

#ifdef DEBUG
    To_String to_string;
#endif

    // For every selector in RHS, see if it matches /any/ of our selectors
    for(size_t rhs_i = 0, rhs_L = rhs->length(); rhs_i < rhs_L; ++rhs_i) {
      Complex_Selector* seq1 = (*rhs)[rhs_i];
#ifdef DEBUG
      string seq1_string = seq1->perform(&to_string);
#endif

      bool any = false;
      for (size_t lhs_i = 0, lhs_L = length(); lhs_i < lhs_L; ++lhs_i) {
        Complex_Selector* seq2 = (*this)[lhs_i];
#ifdef DEBUG
        string seq2_string = seq2->perform(&to_string);
#endif
        bool is_superselector = seq2->is_superselector_of(seq1);
        if( is_superselector ) {
          any = true;
          break;
        }
      }
      
      // Seq1 did not match any of our selectors - whole thing is no good, abort
      if(!any) {
        return false;
      }
    }
    return true;
  }
  
  Selector_List* Selector_List::unify_with(Selector_List* rhs, Context& ctx) {


#ifdef DEBUG
    To_String to_string;
    string lhs_string = perform(&to_string);
    string rhs_string = rhs->perform(&to_string);
 
    auto counter = 0;

    std::cout << "\n\n\n---------------------\n\n\n" << std::endl;
    std::cout << "Unifying " << this->perform(&to_string) << " with:" << rhs->perform(&to_string) << std::endl;
    std::cout << "\n\n\n---------------------\n\n\n" << std::endl;
#endif
    
// Store only unique Selector_List returned by Complex_Selector::unify_with
//    std::set< Selector_List*, std::function< bool(Selector_List*, Selector_List*) > > unique_selector_list([] ( Selector_List* lhs, Selector_List* rhs ) {
//      return *lhs == *rhs;
//    } );

    vector<Complex_Selector*> unified_complex_selectors;
    // Unify all of children with RHS's children, storing the results in `unified_complex_selectors`
    for (size_t lhs_i = 0, lhs_L = length(); lhs_i < lhs_L; ++lhs_i) {
      Complex_Selector* seq1 = (*this)[lhs_i];
      for(size_t rhs_i = 0, rhs_L = rhs->length(); rhs_i < rhs_L; ++rhs_i) {
        Complex_Selector* seq2 = (*rhs)[rhs_i];
        
#ifdef DEBUG
        string seq1_string = seq1->perform(&to_string);
        string seq2_string = seq2->perform(&to_string);
        counter++;
#endif
        Selector_List* result = seq1->unify_with(seq2, ctx);
        if( result ) {
          for(size_t i = 0, L = result->length(); i < L; ++i) {
#ifdef DEBUG
            std::cout << "Counter:" << counter << " result:" << (*result)[i]->perform(&to_string) << std::endl;
#endif
            unified_complex_selectors.push_back( (*result)[i] );
          }
        }
      }
    }
    
    // Creates the final Selector_List by combining all the complex selectors
    Selector_List* final_result = new (ctx.mem) Selector_List(pstate());
    for (auto itr = unified_complex_selectors.begin(); itr != unified_complex_selectors.end(); ++itr) {
      *final_result << *itr;
    }

    return final_result;
  }
  
  void Selector_List::populate_extends(Selector_List* extendee, Context& ctx, ExtensionSubsetMap& extends) {
    To_String to_string;
    
    Selector_List* extender = this;
    for (auto complex_sel : extendee->elements()) {
      Complex_Selector* c = complex_sel;

      
      // Ignore any parent selectors, until we find the first non Selector_Reference head
      Compound_Selector* compound_sel = c->head();
      Complex_Selector* pIter = complex_sel;
      while (pIter) {
        Compound_Selector* pHead = pIter->head();
        if (pHead && dynamic_cast<Selector_Reference*>(pHead->elements()[0]) == NULL) {
          compound_sel = pHead;
          break;
        }
        
        pIter = pIter->tail();
      }
      
      if (!pIter->head() || pIter->tail()) {
        error("nested selectors may not be extended", c->pstate());
      }
      
      compound_sel->is_optional(extendee->is_optional());
      
      for (size_t i = 0, L = extender->length(); i < L; ++i) {
        // let's test this out
        cerr << "REGISTERING EXTENSION REQUEST: " << (*extender)[i]->perform(&to_string) << " <- " << compound_sel->perform(&to_string) << endl;
        extends.put(compound_sel->to_str_vec(), make_pair((*extender)[i], compound_sel));
      }
    }
  };
  
  
  Selector_List* Complex_Selector::unify_with(Complex_Selector* other, Context& ctx) {
    To_String to_string;

    Compound_Selector* thisBase = base();
    Compound_Selector* rhsBase = other->base();
    
    if( thisBase == 0 || rhsBase == 0 ) return 0;

    // Not sure about this check, but closest way I could check to see if this is a ruby 'SimpleSequence' equivalent
    if(  tail()->combinator() != Combinator::ANCESTOR_OF || other->tail()->combinator() != Combinator::ANCESTOR_OF ) return 0;
  
    Compound_Selector* unified = rhsBase->unify_with(thisBase, ctx);
    if( unified == 0 ) return 0;
    
    Node lhsNode = complexSelectorToNode(this, ctx);
    Node rhsNode = complexSelectorToNode(other, ctx);
    
    // Create a temp Complex_Selector, turn it into a Node, and combine it with the existing RHS node
    Complex_Selector* fakeComplexSelector = new (ctx.mem) Complex_Selector(ParserState("[NODE]"), Complex_Selector::ANCESTOR_OF, unified, NULL);
    Node unifiedNode = complexSelectorToNode(fakeComplexSelector, ctx);
    rhsNode.plus(unifiedNode);
    
    Node node = Extend::StaticSubweave(lhsNode, rhsNode, ctx);

#ifdef DEBUG
    std::cout << "Node:" << node << std::endl;
#endif
    
    Selector_List* result = new (ctx.mem) Selector_List(pstate());
    for (NodeDeque::iterator iter = node.collection()->begin(), iterEnd = node.collection()->end(); iter != iterEnd; iter++) {
      Node childNode = *iter;
      childNode = Node::naiveTrim(childNode, ctx);
      
      Complex_Selector* childNodeAsComplexSelector = nodeToComplexSelector(childNode, ctx);
      if( childNodeAsComplexSelector ) { (*result) << childNodeAsComplexSelector; }
    }
    
#ifdef DEBUG
//    To_String to_string;
    string lhs_string = result->perform(&to_string);
#endif

    return result->length() ? result : 0;
  }

  

  /* not used anymore - remove?
  Selector_Placeholder* Selector_List::find_placeholder()
  {
    if (has_placeholder()) {
      for (size_t i = 0, L = length(); i < L; ++i) {
        if ((*this)[i]->has_placeholder()) return (*this)[i]->find_placeholder();
      }
    }
    return 0;
  }*/

  /* not used anymore - remove?
  Selector_Placeholder* Complex_Selector::find_placeholder()
  {
    if (has_placeholder()) {
      if (head() && head()->has_placeholder()) return head()->find_placeholder();
      else if (tail() && tail()->has_placeholder()) return tail()->find_placeholder();
    }
    return 0;
  }*/

  /* not used anymore - remove?
  Selector_Placeholder* Compound_Selector::find_placeholder()
  {
    if (has_placeholder()) {
      for (size_t i = 0, L = length(); i < L; ++i) {
        if ((*this)[i]->has_placeholder()) return (*this)[i]->find_placeholder();
      }
      // return this;
    }
    return 0;
  }*/

  /* not used anymore - remove?
  Selector_Placeholder* Selector_Placeholder::find_placeholder()
  {
    return this;
  }*/

  vector<string> Compound_Selector::to_str_vec()
  {
    To_String to_string;
    vector<string> result;
    result.reserve(length());
    for (size_t i = 0, L = length(); i < L; ++i)
    { result.push_back((*this)[i]->perform(&to_string)); }
    return result;
  }

  Compound_Selector* Compound_Selector::minus(Compound_Selector* rhs, Context& ctx)
  {
    To_String to_string(&ctx);
    Compound_Selector* result = new (ctx.mem) Compound_Selector(pstate());

    // not very efficient because it needs to preserve order
    for (size_t i = 0, L = length(); i < L; ++i)
    {
      bool found = false;
      string thisSelector((*this)[i]->perform(&to_string));
      for (size_t j = 0, M = rhs->length(); j < M; ++j)
      {
        if (thisSelector == (*rhs)[j]->perform(&to_string))
        {
          found = true;
          break;
        }
      }
      if (!found) (*result) << (*this)[i];
    }

    return result;
  }

  void Compound_Selector::mergeSources(SourcesSet& sources, Context& ctx)
  {
    for (SourcesSet::iterator iterator = sources.begin(), endIterator = sources.end(); iterator != endIterator; ++iterator) {
      this->sources_.insert((*iterator)->clone(ctx));
    }
  }

  /* not used anymore - remove?
  vector<Compound_Selector*> Complex_Selector::to_vector()
  {
    vector<Compound_Selector*> result;
    Compound_Selector* h = head();
    Complex_Selector* t = tail();
    if (h) result.push_back(h);
    while (t)
    {
      h = t->head();
      t = t->tail();
      if (h) result.push_back(h);
    }
    return result;
  }*/

}

