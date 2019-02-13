#include <storages/postgres/pool.hpp>

#include <storages/postgres/detail/pool_impl.hpp>

namespace storages {
namespace postgres {

ConnectionPool::ConnectionPool(const std::string& dsn,
                               engine::TaskProcessor& bg_task_processor,
                               size_t initial_size, size_t max_size,
                               CommandControl default_cmd_ctl) {
  pimpl_ = detail::ConnectionPoolImpl::Create(
      dsn, bg_task_processor, initial_size, max_size, default_cmd_ctl);
}

ConnectionPool::~ConnectionPool() = default;

ConnectionPool::ConnectionPool(ConnectionPool&&) noexcept = default;

ConnectionPool& ConnectionPool::operator=(ConnectionPool&&) = default;

std::string const& ConnectionPool::GetDsn() const { return pimpl_->GetDsn(); }

detail::ConnectionPtr ConnectionPool::GetConnection() {
  return pimpl_->Acquire();
}

const InstanceStatistics& ConnectionPool::GetStatistics() const {
  return pimpl_->GetStatistics();
}

Transaction ConnectionPool::Begin(const TransactionOptions& options,
                                  OptionalCommandControl cmd_ctl) {
  return pimpl_->Begin(options, cmd_ctl);
}

detail::NonTransaction ConnectionPool::Start() { return pimpl_->Start(); }

void ConnectionPool::SetDefaultCommandControl(CommandControl cmd_ctl) {
  pimpl_->SetDefaultCommandControl(cmd_ctl);
}

}  // namespace postgres
}  // namespace storages
