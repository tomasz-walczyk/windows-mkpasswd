/**
 * Copyright (C) 2019 Tomasz Walczyk
 *
 * This file is subject to the terms and conditions defined
 * in 'LICENSE' file which is part of this source code package.
 *
 */

#include <windows-mkpasswd/windows-mkpasswd.hpp>
#include <stdlib.h>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////

int main()
{
  std::string password;
  while (std::getline(std::cin, password)) {
    const auto hash = sha512_crypt(password);
    secure_clear_string(password);
    if (hash.empty()) {
      std::cerr << "Cannot encrypt a given password!\n";
      return EXIT_FAILURE;
    } else {
      std::cout << hash << "\n";
    }
  }
  return EXIT_SUCCESS;
}
