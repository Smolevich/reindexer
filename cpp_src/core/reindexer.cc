#include "core/reindexer.h"
#include "core/reindexerimpl.h"

namespace reindexer {

Reindexer::Reindexer() { impl_ = new ReindexerImpl(); }
Reindexer::~Reindexer() { delete impl_; }
Error Reindexer::EnableStorage(const string& storagePath, bool skipPlaceholderCheck) {
	return impl_->EnableStorage(storagePath, skipPlaceholderCheck);
}
Error Reindexer::AddNamespace(const NamespaceDef& nsDef) { return impl_->AddNamespace(nsDef); }
Error Reindexer::OpenNamespace(const string& name, const StorageOpts& storage) { return impl_->OpenNamespace(name, storage); }
Error Reindexer::DropNamespace(const string& _namespace) { return impl_->DropNamespace(_namespace); }
Error Reindexer::CloseNamespace(const string& _namespace) { return impl_->CloseNamespace(_namespace); }
Error Reindexer::Insert(const string& _namespace, Item& item) { return impl_->Insert(_namespace, item); }
Error Reindexer::Update(const string& _namespace, Item& item) { return impl_->Update(_namespace, item); }
Error Reindexer::Upsert(const string& _namespace, Item& item) { return impl_->Upsert(_namespace, item); }
Error Reindexer::Delete(const string& _namespace, Item& item) { return impl_->Delete(_namespace, item); }
Item Reindexer::NewItem(const string& _namespace) { return impl_->NewItem(_namespace); }
Error Reindexer::GetMeta(const string& _namespace, const string& key, string& data) { return impl_->GetMeta(_namespace, key, data); }
Error Reindexer::PutMeta(const string& _namespace, const string& key, const Slice& data) { return impl_->PutMeta(_namespace, key, data); }
Error Reindexer::EnumMeta(const string& _namespace, vector<string>& keys) { return impl_->EnumMeta(_namespace, keys); }
Error Reindexer::Delete(const Query& q, QueryResults& result) { return impl_->Delete(q, result); }
Error Reindexer::Select(const string& query, QueryResults& result) { return impl_->Select(query, result); }
Error Reindexer::Select(const Query& q, QueryResults& result) { return impl_->Select(q, result); }
Error Reindexer::Commit(const string& _namespace) { return impl_->Commit(_namespace); }
Error Reindexer::ConfigureIndex(const string& _namespace, const string& index, const string& config) {
	return impl_->ConfigureIndex(_namespace, index, config);
}
Error Reindexer::ResetStats() { return impl_->ResetStats(); }
Error Reindexer::GetStats(reindexer_stat& stat) { return impl_->GetStats(stat); }
Error Reindexer::AddIndex(const string& _namespace, const IndexDef& idx) { return impl_->AddIndex(_namespace, idx); }
Error Reindexer::DropIndex(const string& _namespace, const string& index) { return impl_->DropIndex(_namespace, index); }
Error Reindexer::EnumNamespaces(vector<NamespaceDef>& defs, bool bEnumAll) { return impl_->EnumNamespaces(defs, bEnumAll); }

}  // namespace reindexer
