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
  std::variant <Error, std::size_t> result;

  auto const key_hash = std::hash<std::string_view>{}(key);
  std::stringstream key_filename;
  key_filename << key_hash;

  /* Open the file containing the registry value. */
  auto const opt_file_hdl = _filesystem.open(key_filename.str(), static_cast<int>(littlefs::OpenFlag::RDONLY));
  if (!opt_file_hdl.has_value())
  {
    auto const lfs_err = _filesystem.last_error();

    if (lfs_err == littlefs::Error::NOENT)
      result = Error::Existence;
    else
      result = Error::IO;

    return result;
  }
  littlefs::FileHandle const file_hdl = opt_file_hdl.value();

  /* Read from the file. */
  auto const opt_bytes_read = _filesystem.read(file_hdl, data, size);
  if (!opt_bytes_read.has_value())
  {
    result = Error::IO;
    return result;
  }

  (void)_filesystem.close(file_hdl);

  result = opt_bytes_read.value();
  return result;
}

auto KeyValueStorage_littlefs::put(const std::string_view key, const std::size_t size, const void* const data)
  -> std::optional<Error>
{
  auto const key_hash = std::hash<std::string_view>{}(key);
  std::stringstream key_filename;
  key_filename << key_hash;

  /* Open the file containing the registry value. */
  auto const opt_file_hdl = _filesystem.open(key_filename.str(),
                                             static_cast<int>(littlefs::OpenFlag::RDWR) | static_cast<int>(littlefs::OpenFlag::CREAT) | static_cast<int>(littlefs::OpenFlag::TRUNC));
  if (!opt_file_hdl.has_value())
    return Error::IO;

  littlefs::FileHandle const file_hdl = opt_file_hdl.value();

  /* Write to the file. */
  auto const opt_bytes_written = _filesystem.write(file_hdl, data, size);
  if (!opt_bytes_written.has_value())
  {
    (void)_filesystem.close(file_hdl);
    return Error::IO;
  }

  size_t const bytes_written = opt_bytes_written.value();
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
