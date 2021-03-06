#pragma once

#include "base_fixture.h"

class JoinItems : protected BaseFixture {
public:
	virtual ~JoinItems() {}

	JoinItems(Reindexer* db, size_t maxItems, size_t idStart = 7000) : BaseFixture(db, "JoinItems", maxItems, idStart) {
		AddIndex("id", "id", "hash", "int", IndexOpts().PK())
			.AddIndex("name", "name", "tree", "string", IndexOpts())
			.AddIndex("location", "location", "hash", "string", IndexOpts())
			.AddIndex("device", "device", "hash", "string", IndexOpts());
	}

	virtual Error Initialize();
	virtual void RegisterAllCases();

protected:
	virtual Item MakeItem();

	string randomString(const string& prefix);

private:
	vector<string> adjectives_;
	vector<string> devices_;
	vector<string> locations_;
	vector<string> names_;
};
