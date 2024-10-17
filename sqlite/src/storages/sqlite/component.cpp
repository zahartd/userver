#include <userver/storages/sqlite/component.hpp>

#include <userver/components/component.hpp>

USERVER_NAMESPACE_BEGIN

namespace components {

SQLite::SQLite(const ComponentConfig& config,
                   const ComponentContext& context)
    : ComponentBase(config, context),
      name_{config.Name()} {}

}  // namespace components

USERVER_NAMESPACE_END
