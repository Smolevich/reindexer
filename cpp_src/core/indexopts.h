#pragma once

#include "sortingprioritiestable.h"

struct CollateOpts {
	explicit CollateOpts(uint8_t mode = CollateNone);
	explicit CollateOpts(const std::string& sortOrderUTF8);

	uint8_t mode = CollateNone;
	SortingPrioritiesTable sortOrderTable;
};

/// Cpp version of IndexOpts: includes
/// sort order table which is not possible
/// to link in C-GO version because of templates
/// in memory.h and unordered_map.h
struct IndexOpts {
	IndexOpts(const IndexOptsC& indexOptsC);
	explicit IndexOpts(uint8_t flags = 0, CollateMode mode = CollateNone);
	explicit IndexOpts(const std::string& sortOrderUTF8, uint8_t flags = 0);

	bool IsPK() const;
	bool IsArray() const;
	bool IsDense() const;
	bool IsAppendable() const;

	IndexOpts& PK(bool value = true);
	IndexOpts& Array(bool value = true);
	IndexOpts& Dense(bool value = true);
	IndexOpts& Appendable(bool value = true);
	IndexOpts& SetCollateMode(CollateMode mode);
	CollateMode GetCollateMode() const;

	uint8_t options;
	CollateOpts collateOpts_;
};
