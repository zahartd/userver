#pragma once

/// @file userver/storages/sqlite/component.hpp
/// @brief @copybrief components::SQLite

#include <userver/components/component_base.hpp>

USERVER_NAMESPACE_BEGIN

namespace components {

class SQLite : public ComponentBase {
 public:

  /// Component constructor
  SQLite(const ComponentConfig&, const ComponentContext&);
  /// Component destructor
  ~SQLite() override = default;

 private:
  std::string name_;
};

template <>
inline constexpr bool kHasValidate<SQLite> = true;

}  // namespace components

USERVER_NAMESPACE_END
