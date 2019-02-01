#include <storages/mongo_ng/pool_impl.hpp>

#include <cassert>

#include <bson/bson.h>

#include <formats/bson.hpp>
#include <logging/log.hpp>
#include <storages/mongo_ng/exception.hpp>

#include <formats/bson/wrappers.hpp>
#include <storages/mongo_ng/async_stream.hpp>

namespace storages::mongo_ng::impl {
namespace {

UriPtr MakeUri(const std::string& pool_id, const std::string& uri_string,
               int conn_timeout_ms, int so_timeout_ms) {
  bson_error_t parse_error;
  UriPtr uri(mongoc_uri_new_with_error(uri_string.c_str(), &parse_error));
  if (!uri) {
    throw InvalidConfigException(
        pool_id, std::string("bad uri: ") + parse_error.message);
  }
  mongoc_uri_set_option_as_int32(uri.get(), MONGOC_URI_CONNECTTIMEOUTMS,
                                 conn_timeout_ms);
  mongoc_uri_set_option_as_int32(uri.get(), MONGOC_URI_SOCKETTIMEOUTMS,
                                 so_timeout_ms);
  return uri;
}

mongoc_ssl_opt_t MakeSslOpt(const mongoc_uri_t* uri) {
  mongoc_ssl_opt_t ssl_opt = *mongoc_ssl_opt_get_default();
  ssl_opt.pem_file = mongoc_uri_get_option_as_utf8(
      uri, MONGOC_URI_SSLCLIENTCERTIFICATEKEYFILE, NULL);
  ssl_opt.pem_pwd = mongoc_uri_get_option_as_utf8(
      uri, MONGOC_URI_SSLCLIENTCERTIFICATEKEYPASSWORD, NULL);
  ssl_opt.ca_file = mongoc_uri_get_option_as_utf8(
      uri, MONGOC_URI_SSLCERTIFICATEAUTHORITYFILE, NULL);
  ssl_opt.weak_cert_validation = mongoc_uri_get_option_as_bool(
      uri, MONGOC_URI_SSLALLOWINVALIDCERTIFICATES, false);
  ssl_opt.allow_invalid_hostname = mongoc_uri_get_option_as_bool(
      uri, MONGOC_URI_SSLALLOWINVALIDHOSTNAMES, false);
  return ssl_opt;
}

}  // namespace

PoolImpl::PoolImpl(std::string id, const std::string& uri_string,
                   const PoolConfig& config)
    : id_(std::move(id)),
      app_name_(config.app_name),
      queue_(config.idle_limit),
      size_(0) {
  static const GlobalInitializer kInitMongoc;
  CheckAsyncStreamCompatible();

  uri_ = MakeUri(id_, uri_string, config.conn_timeout_ms, config.so_timeout_ms);
  ssl_opt_ = MakeSslOpt(uri_.get());

  try {
    LOG_INFO() << "Creating " << config.initial_size << " mongo connections";
    for (size_t i = 0; i < config.initial_size; ++i) Push(Create());
  } catch (const std::exception& ex) {
    LOG_ERROR() << "Mongo pool was not fully prepopulated: " << ex.what();
  }
}

PoolImpl::~PoolImpl() {
  const ClientDeleter deleter;
  mongoc_client_t* client = nullptr;
  while (queue_.pop(client)) deleter(client);
}

PoolImpl::BoundClientPtr PoolImpl::Acquire() {
  return {Pop(), ClientPusher(*this)};
}

void PoolImpl::Push(mongoc_client_t* client) noexcept {
  assert(client);
  if (queue_.bounded_push(client)) return;
  mongoc_client_destroy(client);
  --size_;
}

mongoc_client_t* PoolImpl::Pop() {
  mongoc_client_t* client = nullptr;
  if (queue_.pop(client)) return client;
  return Create();
}

mongoc_client_t* PoolImpl::Create() {
  // "admin" is an internal mongodb database and always exists/accessible
  static const char* kPingDatabase = "admin";
  static const auto kPingCommand = formats::bson::MakeDoc("ping", 1);
  static const ReadPrefsPtr kPingReadPrefs(
      mongoc_read_prefs_new(MONGOC_READ_SECONDARY_PREFERRED));

  if (size_ >= PoolConfig::kMaxSize) {
    throw PoolException(
        id_, "Mongo pool reached size limit: " + std::to_string(size_));
  }
  LOG_INFO() << "Creating mongo connection, current pool size: "
             << size_.load();

  UnboundClientPtr client(mongoc_client_new_from_uri(uri_.get()));
  mongoc_client_set_error_api(client.get(), MONGOC_ERROR_API_VERSION_2);
  mongoc_client_set_stream_initiator(client.get(), &MakeAsyncStream, &ssl_opt_);

  if (!app_name_.empty()) {
    mongoc_client_set_appname(client.get(), app_name_.c_str());
  }

  // force topology refresh
  // XXX: Periodic task forcing topology refresh?
  bson_error_t error;
  if (!mongoc_client_command_simple(client.get(), kPingDatabase,
                                    kPingCommand.GetBson().get(),
                                    kPingReadPrefs.get(), nullptr, &error)) {
    throw PoolException(
        id_, std::string("Connection to MongoDB failed: ") + error.message);
  }

  ++size_;
  return client.release();
}

}  // namespace storages::mongo_ng::impl
