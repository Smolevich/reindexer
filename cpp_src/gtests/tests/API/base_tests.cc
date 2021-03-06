#include <fstream>
#include <vector>
#include "reindexer_api.h"
#include "tools/errors.h"

#include "core/item.h"
#include "core/keyvalue/key_string.h"
#include "core/keyvalue/keyvalue.h"
#include "core/reindexer.h"
#include "tools/stringstools.h"

#include <deque>

using reindexer::Reindexer;

TEST_F(ReindexerApi, AddNamespace) {
	auto err = reindexer->OpenNamespace(default_namespace, StorageOpts().Enabled(false));
	ASSERT_EQ(true, err.ok());
}

TEST_F(ReindexerApi, AddExistingNamespace) {
	CreateNamespace(default_namespace);

	auto err = reindexer->AddNamespace(reindexer::NamespaceDef(default_namespace, StorageOpts().Enabled(false)));
	ASSERT_FALSE(err.ok()) << err.what();
}

TEST_F(ReindexerApi, AddIndex) {
	CreateNamespace(default_namespace);

	auto err = reindexer->AddIndex(default_namespace, {"id", "", "hash", "int", IndexOpts().PK()});
	ASSERT_TRUE(err.ok()) << err.what();
}

TEST_F(ReindexerApi, AddExistingIndex) {
	auto err = reindexer->OpenNamespace(default_namespace, StorageOpts().Enabled(false));
	ASSERT_TRUE(err.ok()) << err.what();

	err = reindexer->AddIndex(default_namespace, {"id", "", "hash", "int", IndexOpts().PK()});
	ASSERT_TRUE(err.ok()) << err.what();

	err = reindexer->AddIndex(default_namespace, {"id", "", "hash", "int", IndexOpts().PK()});
	ASSERT_TRUE(err.ok()) << err.what() << err.what();
}

TEST_F(ReindexerApi, AddExistingIndexWithDiffType) {
	auto err = reindexer->OpenNamespace(default_namespace, StorageOpts().Enabled(false));
	ASSERT_TRUE(err.ok()) << err.what();

	err = reindexer->AddIndex(default_namespace, {"id", "", "hash", "int", IndexOpts().PK()});
	ASSERT_TRUE(err.ok()) << err.what();

	err = reindexer->AddIndex(default_namespace, {"id", "", "hash", "int64", IndexOpts().PK()});
	ASSERT_FALSE(err.ok());
}

TEST_F(ReindexerApi, DeleteNamespace) {
	CreateNamespace(default_namespace);

	auto err = reindexer->CloseNamespace(default_namespace);
	ASSERT_TRUE(err.ok()) << err.what();
}

TEST_F(ReindexerApi, DeleteNonExistingNamespace) {
	auto err = reindexer->CloseNamespace(default_namespace);
	ASSERT_FALSE(err.ok()) << "Error: unexpected result of delete non-existing namespace.";
}

TEST_F(ReindexerApi, NewItem) {
	auto err = reindexer->OpenNamespace(default_namespace, StorageOpts().Enabled());

	ASSERT_TRUE(err.ok()) << err.what();
	err = reindexer->AddIndex(default_namespace, {"id", "", "hash", "int", IndexOpts().PK()});
	ASSERT_TRUE(err.ok()) << err.what();
	err = reindexer->AddIndex(default_namespace, {"value", "", "text", "string", IndexOpts()});
	ASSERT_TRUE(err.ok()) << err.what();
	Item item(reindexer->NewItem(default_namespace));
	ASSERT_TRUE(item);
	ASSERT_TRUE(item.Status().ok()) << item.Status().what();
}

TEST_F(ReindexerApi, Insert) {
	auto err = reindexer->OpenNamespace(default_namespace, StorageOpts().Enabled(false));

	ASSERT_TRUE(err.ok()) << err.what();
	err = reindexer->AddIndex(default_namespace, {"id", "", "hash", "int", IndexOpts().PK()});
	ASSERT_TRUE(err.ok()) << err.what();
	err = reindexer->AddIndex(default_namespace, {"value", "", "text", "string", IndexOpts()});
	ASSERT_TRUE(err.ok()) << err.what();
	Item item(reindexer->NewItem(default_namespace));
	ASSERT_TRUE(item);
	ASSERT_TRUE(item.Status().ok()) << item.Status().what();

	// Set field 'id'
	item["id"] = 1;
	// Set field 'value'
	item["value"] = "value of item";
	ASSERT_TRUE(err.ok()) << err.what();

	err = reindexer->Insert(default_namespace, item);
	ASSERT_TRUE(err.ok()) << err.what();
}

TEST_F(ReindexerApi, DISABLED_DslSetOrder) {
	auto err = reindexer->OpenNamespace(default_namespace, StorageOpts().Enabled(false));

	ASSERT_TRUE(err.ok()) << err.what();
	err = reindexer->AddIndex(default_namespace, {"id", "", "tree", "int", IndexOpts().PK()});
	ASSERT_TRUE(err.ok()) << err.what();
	err = reindexer->AddIndex(default_namespace, {"value", "", "hash", "string", IndexOpts()});
	ASSERT_TRUE(err.ok()) << err.what();

	//	DefineNamespaceDataset(default_namespace, {
	//		{"id", IndexIntBTree, &opts},
	//		{"value", IndexStrHash, &arrOpts}
	//	});

	{
		Item item(reindexer->NewItem(default_namespace));
		ASSERT_TRUE(item);
		ASSERT_TRUE(item.Status().ok()) << item.Status().what();

		item["id"] = 3;
		item["value"] = "val3";
		err = reindexer->Upsert(default_namespace, item);
		ASSERT_TRUE(err.ok()) << err.what();
	}

	{
		Item item(reindexer->NewItem(default_namespace));
		ASSERT_TRUE(item);
		ASSERT_TRUE(item.Status().ok()) << item.Status().what();

		item["id"] = 2;
		item["value"] = "val2";

		std::cout << item.GetJSON().data() << std::endl;

		err = reindexer->Upsert(default_namespace, item);
		ASSERT_TRUE(err.ok()) << err.what();
	}

	{
		Item item(reindexer->NewItem(default_namespace));
		ASSERT_TRUE(item);
		ASSERT_TRUE(item.Status().ok()) << item.Status().what();

		item["id"] = 1;
		item["value"] = "val1";

		std::cout << item.GetJSON().data() << std::endl;

		err = reindexer->Upsert(default_namespace, item);
		ASSERT_TRUE(err.ok()) << err.what();
	}

	err = reindexer->Commit(default_namespace);
	ASSERT_TRUE(err.ok()) << err.what();

	std::ifstream t("/Users/viktor/Desktop/test_dsl_set_order.json");

	ASSERT_TRUE(t.is_open());

	std::string json1((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

	reindexer::Query q;
	err = q.ParseJson(json1);
	ASSERT_TRUE(err.ok()) << err.what();

	reindexer::QueryResults r;

	err = reindexer->Select(q, r);
	ASSERT_TRUE(err.ok()) << err.what();

	PrintQueryResults(default_namespace, r);
}

template <int collateMode>
struct CollateComparer {
	bool operator()(const string& lhs, const string& rhs) const {
		reindexer::Slice sl1(lhs.c_str(), lhs.length());
		reindexer::Slice sl2(rhs.c_str(), rhs.length());
		CollateOpts opts(collateMode);
		return collateCompare(sl1, sl2, opts) < 0;
	}
};

TEST_F(ReindexerApi, SortByUnorderedIndexes) {
	auto err = reindexer->OpenNamespace(default_namespace, StorageOpts().Enabled(false));
	EXPECT_TRUE(err.ok()) << err.what();

	err = reindexer->AddIndex(default_namespace, {"id", "", "hash", "int", IndexOpts().PK()});
	EXPECT_TRUE(err.ok()) << err.what();

	err = reindexer->AddIndex(default_namespace, {"valueInt", "", "hash", "int", IndexOpts()});
	EXPECT_TRUE(err.ok()) << err.what();

	err = reindexer->AddIndex(default_namespace, {"valueString", "", "hash", "string", IndexOpts()});
	EXPECT_TRUE(err.ok()) << err.what();

	err = reindexer->AddIndex(default_namespace, {"valueStringASCII", "", "hash", "string", IndexOpts().SetCollateMode(CollateASCII)});
	EXPECT_TRUE(err.ok()) << err.what();

	err = reindexer->AddIndex(default_namespace, {"valueStringNumeric", "", "hash", "string", IndexOpts().SetCollateMode(CollateNumeric)});
	EXPECT_TRUE(err.ok()) << err.what();

	err = reindexer->AddIndex(default_namespace, {"valueStringUTF8", "", "hash", "string", IndexOpts().SetCollateMode(CollateUTF8)});
	EXPECT_TRUE(err.ok()) << err.what();

	deque<int> allIntValues;
	std::set<string> allStrValues;
	std::set<string, CollateComparer<CollateASCII>> allStrValuesASCII;
	std::set<string, CollateComparer<CollateNumeric>> allStrValuesNumeric;
	std::set<string, CollateComparer<CollateUTF8>> allStrValuesUTF8;

	for (int i = 0; i < 100; ++i) {
		Item item(reindexer->NewItem(default_namespace));
		EXPECT_TRUE(item);
		EXPECT_TRUE(item.Status().ok()) << item.Status().what();

		item["id"] = i;
		item["valueInt"] = i;
		allIntValues.push_front(i);

		string strCollateNone = RandString().c_str();
		allStrValues.insert(strCollateNone);
		item["valueString"] = strCollateNone;

		string strASCII(strCollateNone + "ASCII");
		allStrValuesASCII.insert(strASCII);
		item["valueStringASCII"] = strASCII;

		string strNumeric(std::to_string(i + 1));
		allStrValuesNumeric.insert(strNumeric);
		item["valueStringNumeric"] = strNumeric;

		allStrValuesUTF8.insert(strCollateNone);
		item["valueStringUTF8"] = strCollateNone;

		err = reindexer->Upsert(default_namespace, item);
		EXPECT_TRUE(err.ok()) << err.what();
	}

	err = reindexer->Commit(default_namespace);
	EXPECT_TRUE(err.ok()) << err.what();

	bool descending = true;
	const unsigned offset = 5;
	const unsigned limit = 30;

	QueryResults sortByIntQr;
	Query sortByIntQuery = Query(default_namespace, offset, limit).Sort("valueInt", descending);
	err = reindexer->Select(sortByIntQuery, sortByIntQr);
	EXPECT_TRUE(err.ok()) << err.what();

	deque<int> selectedIntValues;
	for (size_t i = 0; i < sortByIntQr.size(); ++i) {
		Item item(sortByIntQr.GetItem(static_cast<int>(i)));
		int value = item["valueInt"].Get<int>();
		selectedIntValues.push_back(value);
	}

	EXPECT_TRUE(std::equal(allIntValues.begin() + offset, allIntValues.begin() + offset + limit, selectedIntValues.begin()));

	QueryResults sortByStrQr, sortByASCIIStrQr, sortByNumericStrQr, sortByUTF8StrQr;
	Query sortByStrQuery = Query(default_namespace).Sort("valueString", !descending);
	Query sortByASSCIIStrQuery = Query(default_namespace).Sort("valueStringASCII", !descending);
	Query sortByNumericStrQuery = Query(default_namespace).Sort("valueStringNumeric", !descending);
	Query sortByUTF8StrQuery = Query(default_namespace).Sort("valueStringUTF8", !descending);

	err = reindexer->Select(sortByStrQuery, sortByStrQr);
	EXPECT_TRUE(err.ok()) << err.what();

	err = reindexer->Select(sortByASSCIIStrQuery, sortByASCIIStrQr);
	EXPECT_TRUE(err.ok()) << err.what();

	err = reindexer->Select(sortByNumericStrQuery, sortByNumericStrQr);
	EXPECT_TRUE(err.ok()) << err.what();

	err = reindexer->Select(sortByUTF8StrQuery, sortByUTF8StrQr);
	EXPECT_TRUE(err.ok()) << err.what();

	auto collectQrStringFieldValues = [](const QueryResults& qr, const char* fieldName, vector<string>& selectedStrValues) {
		selectedStrValues.clear();
		for (size_t i = 0; i < qr.size(); ++i) {
			Item item(qr.GetItem(int(i)));
			selectedStrValues.push_back(item[fieldName].As<string>());
		}
	};

	vector<string> selectedStrValues;
	auto itSortedStr(allStrValues.begin());
	collectQrStringFieldValues(sortByStrQr, "valueString", selectedStrValues);
	for (auto it = selectedStrValues.begin(); it != selectedStrValues.end(); ++it) {
		EXPECT_EQ(*it, *itSortedStr++);
	}

	itSortedStr = allStrValuesASCII.begin();
	collectQrStringFieldValues(sortByASCIIStrQr, "valueStringASCII", selectedStrValues);
	for (auto it = selectedStrValues.begin(); it != selectedStrValues.end(); ++it) {
		EXPECT_EQ(*it, *itSortedStr++);
	}

	auto itSortedNumericStr = allStrValuesNumeric.cbegin();
	collectQrStringFieldValues(sortByNumericStrQr, "valueStringNumeric", selectedStrValues);
	for (auto it = selectedStrValues.begin(); it != selectedStrValues.end(); ++it) {
		EXPECT_EQ(*it, *itSortedNumericStr++);
	}

	itSortedStr = allStrValuesUTF8.cbegin();
	collectQrStringFieldValues(sortByUTF8StrQr, "valueStringUTF8", selectedStrValues);
	for (auto it = selectedStrValues.begin(); it != selectedStrValues.end(); ++it) {
		EXPECT_EQ(*it, *itSortedStr++);
	}
}

TEST_F(ReindexerApi, SortByUnorderedIndexWithJoins) {
	const string secondNamespace = "test_namespace_2";
	vector<int> secondNamespacePKs;

	auto err = reindexer->OpenNamespace(default_namespace, StorageOpts().Enabled(false));
	EXPECT_TRUE(err.ok()) << err.what();

	err = reindexer->AddIndex(default_namespace, {"id", "", "hash", "int", IndexOpts().PK()});
	EXPECT_TRUE(err.ok()) << err.what();

	err = reindexer->AddIndex(default_namespace, {"fk", "", "hash", "int", IndexOpts()});
	EXPECT_TRUE(err.ok()) << err.what();

	{
		err = reindexer->OpenNamespace(secondNamespace, StorageOpts().Enabled(false));
		EXPECT_TRUE(err.ok()) << err.what();

		err = reindexer->AddIndex(secondNamespace, {"pk", "", "hash", "int", IndexOpts().PK()});
		EXPECT_TRUE(err.ok()) << err.what();

		for (int i = 0; i < 50; ++i) {
			Item item(reindexer->NewItem(secondNamespace));
			EXPECT_TRUE(item);
			EXPECT_TRUE(item.Status().ok()) << item.Status().what();

			secondNamespacePKs.push_back(i);
			item["pk"] = i;

			err = reindexer->Upsert(secondNamespace, item);
			EXPECT_TRUE(err.ok()) << err.what();
		}

		err = reindexer->Commit(secondNamespace);
		EXPECT_TRUE(err.ok()) << err.what();
	}

	for (int i = 0; i < 100; ++i) {
		Item item(reindexer->NewItem(default_namespace));
		EXPECT_TRUE(item);
		EXPECT_TRUE(item.Status().ok()) << item.Status().what();

		item["id"] = i;

		int fk = secondNamespacePKs[rand() % (secondNamespacePKs.size() - 1)];
		item["fk"] = fk;

		err = reindexer->Upsert(default_namespace, item);
		EXPECT_TRUE(err.ok()) << err.what();
	}

	err = reindexer->Commit(default_namespace);
	EXPECT_TRUE(err.ok()) << err.what();

	bool descending = true;
	const unsigned offset = 10;
	const unsigned limit = 40;

	Query querySecondNamespace = Query(secondNamespace);
	Query sortQuery = Query(default_namespace, offset, limit).Sort("id", descending);
	Query joinQuery = sortQuery.InnerJoin("fk", "pk", CondEq, querySecondNamespace);

	QueryResults queryResult;
	err = reindexer->Select(joinQuery, queryResult);
	EXPECT_TRUE(err.ok()) << err.what();

	for (size_t i = 0; i < queryResult.size(); ++i) {
		const reindexer::ItemRef& itemRef = queryResult[i];
		auto itFind(queryResult.joined_->find(itemRef.id));
		EXPECT_TRUE(itFind != queryResult.joined_->end());
	}
}

void TestDSLParseCorrectness(const string& testDsl) {
	Query query;
	Error err = query.ParseJson(testDsl);
	ASSERT_TRUE(err.ok());
}

TEST_F(ReindexerApi, DslFieldsTest) {
	TestDSLParseCorrectness(R"xxx({"join_queries": [{
                                    "type": "inner",
                                    "op": "AND",
                                    "namespace": "test1",
                                    "filters": [{
                                        "Op": "",
                                        "Field": "id",
                                        "Cond": "SET",
                                        "Value": [81204872, 101326571, 101326882]
                                    }],
                                    "sort": {
                                        "field": "test1",
                                        "desc": true
                                    },
                                    "limit": 3,
                                    "offset": 0,
                                    "on": [{
                                            "left_field": "joined",
                                            "right_field": "joined",
                                            "cond": "lt",
                                            "op": "OR"
                                        },
                                        {
                                            "left_field": "joined2",
                                            "right_field": "joined2",
                                            "cond": "gt",
                                            "op": "AND"
                                        }
                                    ]
                                },
                                {
                                    "type": "left",
                                    "op": "OR",
                                    "namespace": "test2",
                                    "filters": [{
                                        "Op": "",
                                        "Field": "id2",
                                        "Cond": "SET",
                                        "Value": [81204872, 101326571, 101326882]
                                    }],
                                    "sort": {
                                        "field": "test2",
                                        "desc": true
                                    },
                                    "limit": 4,
                                    "offset": 5,
                                    "on": [{
                                            "left_field": "joined1",
                                            "right_field": "joined1",
                                            "cond": "le",
                                            "op": "AND"
                                        },
                                        {
                                            "left_field": "joined2",
                                            "right_field": "joined2",
                                            "cond": "ge",
                                            "op": "OR"
                                        }
                                    ]
                                }
                            ]
                        })xxx");

	TestDSLParseCorrectness(R"xxx({"merge_queries": [{
                                    "namespace": "services",
                                    "offset": 0,
                                    "limit": 3,
                                    "distinct": "",
                                    "sort": {
                                        "field": "",
                                        "desc": true
                                    },
                                    "filters": [{
                                        "Op": "",
                                        "Field": "id",
                                        "Cond": "SET",
                                        "Value": [81204872, 101326571, 101326882]
                                    }]
                                },
                                {
                                    "namespace": "services",
                                    "offset": 1,
                                    "limit": 5,
                                    "distinct": "",
                                    "sort": {
                                        "field": "field1",
                                        "desc": false
                                    },
                                    "filters": [{
                                        "Op": "not",
                                        "Field": "id",
                                        "Cond": "ge",
                                        "Value": 81204872
                                    }]
                                }
                            ]
                        })xxx");
	TestDSLParseCorrectness(R"xxx({"select_filter": ["f1", "f2", "f3", "f4", "f5"]})xxx");
	TestDSLParseCorrectness(R"xxx({"select_functions": ["f1()", "f2()", "f3()", "f4()", "f5()"]})xxx");
	TestDSLParseCorrectness(R"xxx({"req_total":"cached"})xxx");
	TestDSLParseCorrectness(R"xxx({"req_total":"enabled"})xxx");
	TestDSLParseCorrectness(R"xxx({"req_total":"disabled"})xxx");
	TestDSLParseCorrectness(R"xxx({"aggregations":[{"field":"field1", "type":"sum"}, {"field":"field2", "type":"avg"}]})xxx");
}
