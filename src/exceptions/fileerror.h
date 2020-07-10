// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef FILEERROR_H
#define FILEERROR_H

#include <QFileDevice>
#include <stdexcept>
#include <string>

class FileError : public std::runtime_error {
 public:
  static FileError mkError(std::string msg, std::string path, QFileDevice::FileError err) {
    std::string suberror;
    switch (err) {
      case QFileDevice::ReadError:
        suberror = "Error reading file";
        break;
      case QFileDevice::WriteError:
        suberror = "Error writing file";
        break;
      case QFileDevice::FatalError:
        suberror = "Fatal error occurred";
        break;
      case QFileDevice::ResourceError:
        suberror = "Insufficient resources available";
        break;
      case QFileDevice::OpenError:
        suberror = "Could not open file";
        break;
      case QFileDevice::AbortError:
        suberror = "Operation was aborted";
        break;
      case QFileDevice::TimeOutError:
        suberror = "Operation timed out";
        break;
      case QFileDevice::UnspecifiedError:
        suberror = "Unknown Error";
        break;
      case QFileDevice::RemoveError:
        suberror = "Unable to remove file";
        break;
      case QFileDevice::RenameError:
        suberror = "Unable to rename/move file";
        break;
      case QFileDevice::PositionError:
        suberror = "Position error";  // I don't think we'll ever enounter this error
        break;
      case QFileDevice::ResizeError:
        suberror = "Unable to resize file";
        break;
      case QFileDevice::PermissionsError:
        suberror = "Unable to access file";
        break;
      case QFileDevice::CopyError:
        suberror = "Unable to copy file";
        break;
      case QFileDevice::NoError:
        suberror = "Actually, no error occurred -- just bad programming.";
        break;
    }
    return msg + " (path: " + path + "): " + suberror;
  }

 private:
  FileError(std::string msg) : std::runtime_error(msg) {}
};

#endif  // FILEERROR_H
