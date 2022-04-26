/*********************************************************************************
 *     File Name           :     multipartparser.h
 *     Created By          :     Ye Yangang
 *     Creation Date       :     [2017-02-20 16:50]
 *     Last Modified       :     [AUTO_UPDATE_BEFORE_SAVE]
 *     Description         :     Generate multipart/form-data POST body
 **********************************************************************************/

#pragma once

#include <string>
#include <tuple>
#include <vector>

class MultipartParser {
 public:
  MultipartParser();
  inline const std::string &body_content() { return body_content_; }
  inline const std::string &boundary() { return boundary_; }
  inline void AddParameter(const std::string &name, const std::string &value) {
    params_.push_back(std::pair<std::string, std::string>(name, value));
  }
  inline void AddFile(const std::string &name, const std::string &value) {
    files_.push_back(std::pair<std::string, std::string>(name, value));
  }
  const std::string &GenBodyContent();

 private:
  void _get_file_name_type(const std::string &file_path, std::string *filenae,
                           std::string *content_type);

 private:
  static const std::string boundary_prefix_;
  static const std::string rand_chars_;
  std::string boundary_;
  std::string body_content_;
  std::vector<std::pair<std::string, std::string>> params_;
  std::vector<std::pair<std::string, std::string>> files_;
};
