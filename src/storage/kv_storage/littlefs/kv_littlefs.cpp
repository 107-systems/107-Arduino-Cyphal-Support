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

#include <string>
#include <variant>
#include <sstream>

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

namespace cyphal::support::platform::storage
{

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
  {
    if (std::get<littlefs::Error>(rc_open) == littlefs::Error::NOENT)
      return Error::Existence;
    else
      return Error::IO;
  }
  littlefs::FileHandle const file_hdl = std::get<littlefs::FileHandle>(rc_open);

  /* Read from the file. */
  auto const rc_read = _filesystem.read(file_hdl, data, size);
  if (std::holds_alternative<littlefs::Error>(rc_read))
  {
    (void)_filesystem.close(file_hdl);
    return Error::IO;
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
    return Error::IO;

  littlefs::FileHandle const file_hdl = std::get<littlefs::FileHandle>(rc_open);

  /* Write to the file. */
  auto const rc_write = _filesystem.write(file_hdl, data, size);
  if (std::holds_alternative<littlefs::Error>(rc_write))
  {
    (void)_filesystem.close(file_hdl);
    return Error::IO;
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
