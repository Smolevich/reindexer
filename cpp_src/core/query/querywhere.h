#pragma once

#include <memory>
#include <string>
#include <vector>
#include "core/keyvalue/keyvalue.h"
#include "estl/h_vector.h"

namespace reindexer {

using std::string;
using std::vector;

class QueryWhere;
class tokenizer;

struct QueryEntry {
	QueryEntry(OpType o, CondType cond, const string &idx, int idxN, bool dist = false)
		: op(o), condition(cond), index(idx), idxNo(idxN), distinct(dist) {}
	QueryEntry() = default;

	bool operator==(const QueryEntry &) const;
	bool operator!=(const QueryEntry &) const;

	OpType op = OpAnd;
	CondType condition = CondType::CondAny;
	string index;
	int idxNo = -1;
	bool distinct = false;
	KeyValues values;

	string Dump() const;
};

struct QueryJoinEntry {
	bool operator==(const QueryJoinEntry &) const;
	OpType op_;
	CondType condition_;
	string index_;
	string joinIndex_;
	int idxNo = -1;
};

struct QueryEntries : public h_vector<QueryEntry, 4> {};

struct AggregateEntry {
	bool operator==(const AggregateEntry &) const;
	bool operator!=(const AggregateEntry &) const;
	string index_;
	AggType type_;
};

class QueryWhere {
public:
	QueryWhere() {}

	bool operator==(const QueryWhere &) const;

protected:
	int ParseWhere(tokenizer &tok);
	string toString() const;
	static CondType getCondType(const string &cond);

public:
	QueryEntries entries;
	h_vector<AggregateEntry, 1> aggregations_;
	// Condition for join. Filled in each subqueries, empty in  root query
	vector<QueryJoinEntry> joinEntries_;
};

}  // namespace reindexer
