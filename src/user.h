#pragma once

#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

class User_info {
public:
  User_info(std::string username, std::string password)
      : _username{std::move(username)}, _password{std::move(password)}
  {
  }

  [[nodiscard]] auto username() const -> std::string const &
  {
    return _username;
  }

  [[nodiscard]] auto password() const -> std::string const &
  {
    return _password;
  }

private:
  std::string _username;
  std::string _password;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(User_info, _username, _password)
};
