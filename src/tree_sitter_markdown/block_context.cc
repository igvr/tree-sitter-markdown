#include "./block_context.h"

namespace tree_sitter_markdown {

bool BlockContext::has_fst_ctn() const { return has_fst_ctn_; }
Symbol BlockContext::sym() const { return sym_; }
LexedLength BlockContext::len() const { return len_; }
LexedColumn BlockContext::ind() const { return ind_; }
ParseState BlockContext::pst() const { assert(pst_ != PST_INVALID); return pst_; }

void BlockContext::mrk_has_fst_ctn() { has_fst_ctn_ = true; }

BlockContext::BlockContext(): has_fst_ctn_(false), sym_(SYM_TXT), len_(LEXED_LENGTH_MAX), ind_(LEXED_COLUMN_MAX), pst_(PST_INVALID) {}
BlockContext::BlockContext(const Symbol sym, const LexedLength len, const LexedColumn ind): has_fst_ctn_(false), sym_(sym), len_(len), ind_(ind), pst_(blk_sym_pst(sym)) {}

unsigned BlockContext::serialize(unsigned char *buffer) const {
  assert(is_blk_sym(sym_));
  assert(sym_ <= 0b1111111);
  assert(len_ <= 0b11111111);
  assert(ind_ <= 0b11111111);
  buffer[0] = (sym_ << 1) | has_fst_ctn_;
  buffer[1] = len_;
  buffer[2] = ind_;
  return 3;
}
unsigned BlockContext::deserialize(const unsigned char *buffer) {
  has_fst_ctn_ = buffer[0] & 1;
  sym_ = static_cast<Symbol>(buffer[0] >> 1);
  len_ = buffer[1];
  ind_ = buffer[2];
  pst_ = blk_sym_pst(sym_);
  return 3;
}

void BlockContext::print() const {
  fprintf(stderr, "(%s h=%d l=%d n=%d)", sym_nam(sym_), has_fst_ctn_, len_, ind_);
}

bool BlockContextStack::empty() const { return stk_.empty(); }
BlockContext &BlockContextStack::back() { return stk_.back(); }
const BlockContext &BlockContextStack::back() const { return stk_.back(); }
BlockContextStack::ConstIterator BlockContextStack::begin() const { return stk_.begin(); }
BlockContextStack::ConstIterator BlockContextStack::end() const { return stk_.end(); }
BlockContextStack::ConstReverseIterator BlockContextStack::rbegin() const { return stk_.rbegin(); }
BlockContextStack::ConstReverseIterator BlockContextStack::rend() const { return stk_.rend(); }

void BlockContextStack::mrk_has_fst_ctn() {
  ReverseIterator itr = stk_.rbegin();
  ReverseIterator end = stk_.rend();
  for (; itr != end; itr++) {
    if (itr->has_fst_ctn()) break;
    itr->mrk_has_fst_ctn();
  }
}

void BlockContextStack::clear() { stk_.clear(); }
unsigned BlockContextStack::serialize(unsigned char *buffer) const {
  size_t i = 0;
  buffer[i++] = stk_.size();
  ConstIterator itr = stk_.begin();
  ConstIterator end = stk_.end();
  for (; itr != end; itr++) {
    i += itr->serialize(&buffer[i]);
  }
  return i;
}
unsigned BlockContextStack::deserialize(const unsigned char *buffer) {
  size_t i = 0;
  stk_.resize(buffer[i++]);
  Iterator itr = stk_.begin();
  Iterator end = stk_.end();
  for (; itr != end; itr++) {
    i += itr->deserialize(&buffer[i]);
  }
  return i;
}

void BlockContextStack::push(const BlockContext &ctx) {
  stk_.push_back(ctx);
}
void BlockContextStack::pop() {
  assert(!empty());
  stk_.pop_back();
}

void BlockContextStack::print() const {
  ConstIterator itr = stk_.begin();
  ConstIterator bgn = stk_.begin();
  ConstIterator end = stk_.end();
  for (; itr != end; itr++) {
    if (itr != bgn) fprintf(stderr, ", ");
    itr->print();
  }
}

}
