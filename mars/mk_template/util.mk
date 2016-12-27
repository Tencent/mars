
enum-files-subdir0 = $(wildcard $1/$2)
enum-files-subdir1 = $(wildcard $1/*/$2)
enum-files-subdir2 = $(wildcard $1/*/*/$2)
enum-files-subdir3 = $(wildcard $1/*/*/*/$2)
enum-files-subdir4 = $(wildcard $1/*/*/*/*/$2)
enum-files-subdir5 = $(wildcard $1/*/*/*/*/*/$2)
enum-files-subdir6 = $(wildcard $1/*/*/*/*/*/*/$2)
enum-files-subdir7 = $(wildcard $1/*/*/*/*/*/*/*/$2)
enum-files-subdir8 = $(wildcard $1/*/*/*/*/*/*/*/*/$2)
enum-files-subdir9 = $(wildcard $1/*/*/*/*/*/*/*/*/*$2)

enum-files-subdir-all = $(foreach n,0 1 2 3 4 5 6 7 8 9,$(call enum-files-subdir$(n),$1,$2))
enum-files-subdir-all-src = $(foreach n,0 1 2 3 4 5 6 7 8 9,$(call enum-files-subdir$(n),$1,*.c) $(call enum-files-subdir$(n),$1,*.cc) $(call enum-files-subdir$(n),$1,*.cxx)$(call enum-files-subdir$(n),$1,*.cpp))
