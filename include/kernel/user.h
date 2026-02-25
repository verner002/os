/**
 * @file user.h
 * @author verner002
 * @date 20/02/2026
*/

#pragma once

typedef int uid_t;
typedef int gid_t;

int user_add(char const *login, char const *pwd);
int user_init(void);
int user_login(char const *login, char const *pwd);
