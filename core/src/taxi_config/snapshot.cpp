#include <userver/taxi_config/snapshot.hpp>

#include <userver/rcu/rcu.hpp>
#include <userver/taxi_config/storage_mock.hpp>

#include <taxi_config/storage_data.hpp>

USERVER_NAMESPACE_BEGIN

namespace taxi_config {

struct Snapshot::Impl final {
  explicit Impl(const impl::StorageData& storage)
      : data_ptr(storage.config.Read()) {}

  rcu::ReadablePtr<impl::SnapshotData> data_ptr;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
Snapshot::Snapshot(const Snapshot&) = default;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
Snapshot::Snapshot(Snapshot&&) noexcept = default;

Snapshot& Snapshot::operator=(const Snapshot&) = default;

Snapshot& Snapshot::operator=(Snapshot&&) noexcept = default;

Snapshot::~Snapshot() = default;

Snapshot::Snapshot(const impl::StorageData& storage) : impl_(storage) {}

const impl::SnapshotData& Snapshot::GetData() const { return *impl_->data_ptr; }

}  // namespace taxi_config

USERVER_NAMESPACE_END
