/**
 * This software is distributed under the terms of the MIT License.
 * Copyright (c) 2023 LXRobotics.
 * Author: Alexander Entinger <alexander.entinger@lxrobotics.com>
 * Contributors: https://github.com/107-systems/107-Arduino-Cyphal-Support/graphs/contributors.
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include "storage.h"

#include <string>
#include <variant>
#include <sstream>

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

namespace cyphal::support
{

/**************************************************************************************
 * CTOR/DTOR
 **************************************************************************************/

KeyValueLittleFs::KeyValueLittleFs(littlefs::Filesystem & filesystem)
: _filesystem{filesystem}
{ }

/**************************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************************/

auto KeyValueLittleFs::get(const std::string_view key, const std::size_t size, void* const data) const
  -> std::variant<platform::storage::Error, std::size_t>
{
  std::variant <platform::storage::Error, std::size_t> result;

  auto const key_hash = std::hash<std::string_view>{}(key);
  std::stringstream key_filename;
  key_filename << key_hash;

  /* Open the file containing the registry value. */
  auto const opt_file_hdl = _filesystem.open(key_filename.str(), static_cast<int>(littlefs::OpenFlag::RDONLY));
  if (!opt_file_hdl.has_value())
  {
    result = platform::storage::Error::IO;
    return result;
  }
  littlefs::FileHandle const file_hdl = opt_file_hdl.value();

  /* Read from the file. */
  auto const opt_bytes_read = _filesystem.read(file_hdl, data, size);
  if (!opt_bytes_read.has_value())
  {
    result = platform::storage::Error::IO;
    return result;
  }

  (void)_filesystem.close(file_hdl);

  result = opt_bytes_read.value();
  return result;
}

auto KeyValueLittleFs::put(const std::string_view key, const std::size_t size, const void* const data)
  -> std::optional<platform::storage::Error>
{
  auto const key_hash = std::hash<std::string_view>{}(key);
  std::stringstream key_filename;
  key_filename << key_hash;

  /* Open the file containing the registry value. */
  auto const opt_file_hdl = _filesystem.open(key_filename.str(),
                                             static_cast<int>(littlefs::OpenFlag::RDWR) | static_cast<int>(littlefs::OpenFlag::CREAT) | static_cast<int>(littlefs::OpenFlag::TRUNC));
  if (!opt_file_hdl.has_value())
    return platform::storage::Error::IO;

  littlefs::FileHandle const file_hdl = opt_file_hdl.value();

  /* Write to the file. */
  auto const opt_bytes_written = _filesystem.write(file_hdl, data, size);
  if (!opt_bytes_written.has_value())
  {
    (void)_filesystem.close(file_hdl);
    return platform::storage::Error::IO;
  }

  size_t const bytes_written = opt_bytes_written.value();
  if (bytes_written != size)
  {
    (void)_filesystem.close(file_hdl);
    return platform::storage::Error::IO;
  }

  return std::nullopt;
}

auto KeyValueLittleFs::drop(const std::string_view key) -> std::optional<platform::storage::Error>
{
  auto const key_hash = std::hash<std::string_view>{}(key);
  std::stringstream key_filename;
  key_filename << key_hash;

  auto const rc = _filesystem.remove(key_filename.str());
  if (rc != littlefs::Error::OK)
    return platform::storage::Error::IO;

  return std::nullopt;
}

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

} /* cyphal::support */
