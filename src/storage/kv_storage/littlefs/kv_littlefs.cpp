/**
 * This software is distributed under the terms of the MIT License.
 * Copyright (c) 2023 LXRobotics.
 * Author: Alexander Entinger <alexander.entinger@lxrobotics.com>
 * Contributors: https://github.com/107-systems/107-Arduino-Cyphal-Support/graphs/contributors.
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include "kv_littlefs.h"

#if __GNUC__ >= 11

#include <map>
#include <string>
#include <variant>
#include <sstream>

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

namespace cyphal::support::platform::storage
{

/**************************************************************************************
 * CONSTANTS
 **************************************************************************************/

static std::map<littlefs::Error, Error> const LITTLEFS_TO_STORAGE_ERROR_MAP =
{
  {littlefs::Error::IO          , Error::IO},
  {littlefs::Error::CORRUPT     , Error::Internal},
  {littlefs::Error::NOENT       , Error::Existence},
  {littlefs::Error::EXIST       , Error::Existence},
  {littlefs::Error::NOTDIR      , Error::Existence},
  {littlefs::Error::ISDIR       , Error::Existence},
  {littlefs::Error::NOTEMPTY    , Error::Existence},
  {littlefs::Error::BADF        , Error::Internal},
  {littlefs::Error::FBIG        , Error::Capacity},
  {littlefs::Error::INVAL       , Error::API},
  {littlefs::Error::NOSPC       , Error::Capacity},
  {littlefs::Error::NOMEM       , Error::Internal},
  {littlefs::Error::NOATTR      , Error::API},
  {littlefs::Error::NAMETOOLONG , Error::API},
  {littlefs::Error::NO_FD_ENTRY , Error::API},
};

/**************************************************************************************
 * CTOR/DTOR
 **************************************************************************************/

KeyValueStorage_littlefs::KeyValueStorage_littlefs(littlefs::Filesystem & filesystem)
: _filesystem{filesystem}
{ }

/**************************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************************/

auto KeyValueStorage_littlefs::get(const std::string_view key, const std::size_t size, void* const data) const
  -> std::variant<Error, std::size_t>
{
  auto const key_hash = std::hash<std::string_view>{}(key);
  std::stringstream key_filename;
  key_filename << key_hash;

  /* Open the file containing the registry value. */
  auto const rc_open = _filesystem.open(key_filename.str(), littlefs::OpenFlag::RDONLY);
  if (std::holds_alternative<littlefs::Error>(rc_open))
    return LITTLEFS_TO_STORAGE_ERROR_MAP.at(std::get<littlefs::Error>(rc_open));

  littlefs::FileHandle const file_hdl = std::get<littlefs::FileHandle>(rc_open);

  /* Read from the file. */
  auto const rc_read = _filesystem.read(file_hdl, data, size);
  if (std::holds_alternative<littlefs::Error>(rc_read))
  {
    (void)_filesystem.close(file_hdl);
    return LITTLEFS_TO_STORAGE_ERROR_MAP.at(std::get<littlefs::Error>(rc_read));
  }

  (void)_filesystem.close(file_hdl);

  return std::get<size_t>(rc_read);
}

auto KeyValueStorage_littlefs::put(const std::string_view key, const std::size_t size, const void* const data)
  -> std::optional<Error>
{
  auto const key_hash = std::hash<std::string_view>{}(key);
  std::stringstream key_filename;
  key_filename << key_hash;

  /* Open the file containing the registry value. */
  auto const rc_open = _filesystem.open(key_filename.str(), littlefs::OpenFlag::WRONLY | littlefs::OpenFlag::CREAT | littlefs::OpenFlag::TRUNC);
  if (std::holds_alternative<littlefs::Error>(rc_open))
    return LITTLEFS_TO_STORAGE_ERROR_MAP.at(std::get<littlefs::Error>(rc_open));

  littlefs::FileHandle const file_hdl = std::get<littlefs::FileHandle>(rc_open);

  /* Write to the file. */
  auto const rc_write = _filesystem.write(file_hdl, data, size);
  if (std::holds_alternative<littlefs::Error>(rc_write))
  {
    (void)_filesystem.close(file_hdl);
    return LITTLEFS_TO_STORAGE_ERROR_MAP.at(std::get<littlefs::Error>(rc_write));
  }

  (void)_filesystem.close(file_hdl);

  size_t const bytes_written = std::get<size_t>(rc_write);
  if (bytes_written != size)
  {
    (void)_filesystem.close(file_hdl);
    return Error::IO;
  }

  return std::nullopt;
}

auto KeyValueStorage_littlefs::drop(const std::string_view key) -> std::optional<Error>
{
  auto const key_hash = std::hash<std::string_view>{}(key);
  std::stringstream key_filename;
  key_filename << key_hash;

  auto const rc = _filesystem.remove(key_filename.str());
  if (rc != littlefs::Error::OK)
    return Error::IO;

  return std::nullopt;
}

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

} /* cyphal::support::platform::storage */

#endif /* __GNUC__ >= 11 */
