/**
 * Copyright (C) 2019 Tomasz Walczyk
 *
 * This file is subject to the terms and conditions defined
 * in 'LICENSE' file which is part of this source code package.
 *
 */

#include <string>

extern const std::size_t PASSWORD_LEN_MIN;
extern const std::size_t PASSWORD_LEN_MAX;
extern const std::size_t SALT_LEN_MIN;
extern const std::size_t SALT_LEN_MAX;
extern const std::size_t ROUNDS_MIN;
extern const std::size_t ROUNDS_MAX;
extern const std::size_t ROUNDS_DEFAULT;

std::string sha512_crypt(const std::string& password);
std::string sha512_crypt(const std::string& password, std::size_t rounds);
std::string sha512_crypt(const std::string& password, std::size_t rounds, const std::string& salt);

void secure_clear_string(std::string& string);
