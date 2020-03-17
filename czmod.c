//=====================================================================
//
// czmod.c - c module to boost z.lua
//
// Created by skywind on 2020/03/11
// Last Modified: 2020/03/11 16:37:01
//
//=====================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <assert.h>

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
#include <windows.h>
#elif defined(__linux)
// #include <linux/limits.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#endif

#define IB_STRING_SSO 256

#include "system/iposix.h"
#include "system/imembase.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


//----------------------------------------------------------------------
// INLINE
//----------------------------------------------------------------------
#ifndef INLINE
#if defined(__GNUC__)

#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
#define INLINE         __inline__ __attribute__((always_inline))
#else
#define INLINE         __inline__
#endif

#elif (defined(_MSC_VER) || defined(__WATCOMC__))
#define INLINE __inline
#else
#define INLINE 
#endif
#endif

#if (!defined(__cplusplus)) && (!defined(inline))
#define inline INLINE
#endif


//---------------------------------------------------------------------
// internal functions
//---------------------------------------------------------------------
static const char *get_data_file(void);


//---------------------------------------------------------------------
// get environ
//---------------------------------------------------------------------
static ib_string* os_getenv(const char *name)
{
	char *p = getenv(name);
	if (p == NULL) {
		return NULL;
	}
	ib_string *text = ib_string_new();
	ib_string_assign(text, p);
	return text;
}


//---------------------------------------------------------------------
// get data file
//---------------------------------------------------------------------
static const char *get_data_file(void)
{
	static ib_string *text = NULL;
	if (text != NULL) {
		return text->ptr;
	}
	text = os_getenv("_ZL_DATA2");
	if (text) {
		return text->ptr;
	}
	text = os_getenv("HOME");
	if (text == NULL) {
		text = os_getenv("USERPROFILE");
	}
	if (text == NULL) {
		return NULL;
	}
	ib_string_append(text, "/.zlua");
	return text->ptr;
}


//---------------------------------------------------------------------
// load file content
//---------------------------------------------------------------------
ib_string *load_content(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		return NULL;
	}
	int fd = fileno(fp);
	flock(fd, LOCK_SH);
	ib_string *text = ib_string_new();
	size_t block = 65536;
	ib_string_resize(text, block);
	size_t pos = 0;
	while (feof(fp) == 0) {
		size_t avail = text->size - pos;
		if (avail < block) {
			ib_string_resize(text, text->size + block);
			avail = text->size - pos;
		}
		size_t hr = fread(&(text->ptr[pos]), 1, avail, fp);
		pos += hr;
	}
	flock(fd, LOCK_UN);
	fclose(fp);
	ib_string_resize(text, pos);
	return text;
}


//---------------------------------------------------------------------
// path item
//---------------------------------------------------------------------
typedef struct
{
	ib_string *path;
	int rank;
	uint32_t timestamp;
	double frecent;
}	PathItem;

static void item_delete(PathItem *item)
{
	if (item) {
		if (item->path) {
			ib_string_delete(item->path);
			item->path = NULL;
		}
		ikmem_free(item);
	}
};

PathItem* item_new(const char *path, int rank, uint32_t timestamp)
{
	PathItem* item = (PathItem*)ikmem_malloc(sizeof(PathItem));
	assert(item);
	item->path = ib_string_new_from(path);
	item->rank = rank;
	item->timestamp = timestamp;
	item->frecent = rank;
	return item;
};

// compare item
int item_compare(const void *p1, const void *p2)
{
	PathItem *n1 = (PathItem*)p1;
	PathItem *n2 = (PathItem*)p2;
	if (n1->frecent == n2->frecent) return 0;
	return (n1->frecent > n2->frecent)? 1 : -1;
}

ib_array* ib_array_new_items(void)
{
	return ib_array_new((void (*)(void*))item_delete);
}


//---------------------------------------------------------------------
// load data
//---------------------------------------------------------------------
ib_array* data_load(const char *filename)
{
	ib_string *content = load_content(filename);
	if (content == NULL) {
		return NULL;
	}
	else {
		ib_array *lines = ib_string_split_c(content, '\n');
		int size = ib_array_size(lines);
		int i;
		ib_array *items = ib_array_new_items();
		for (i = 0; i < size; i++) {
			ib_string *text = (ib_string*)ib_array_index(lines, i);
			int p1 = ib_string_find_c(text, '|', 0);
			if (p1 >= 0) {
				int p2 = ib_string_find_c(text, '|', p1 + 1);
				if (p2 >= 0) {
					uint32_t timestamp;
					int rank;
					text->ptr[p1] = 0;
					text->ptr[p2] = 0;
					rank = (int)atoi(text->ptr + p1 + 1);
					timestamp = (uint32_t)strtoul(text->ptr + p2 + 1, NULL, 10);
					PathItem *ni = item_new(text->ptr, rank, timestamp);
					ib_array_push(items, ni);
				}
			}
		}
		ib_array_delete(lines);
		return items;
	}
	return NULL;
}


//---------------------------------------------------------------------
// save data
//---------------------------------------------------------------------
void data_save(const char *filename, ib_array *items)
{
	ib_string *tmpname = ib_string_new_from(filename);
	FILE *fp;
	while (1) {
		char tmp[100];
		ib_string_assign(tmpname, filename);
		sprintf(tmp, ".%u%03u%d", (uint32_t)time(NULL), 
				(uint32_t)(clock() % 1000), rand() % 10000);
		ib_string_append(tmpname, tmp);
		if (iposix_path_isdir(tmpname->ptr) < 0) break;
	}
	fp = fopen(tmpname->ptr, "w");
	if (fp) {
		int size = ib_array_size(items);
		int i;
		for (i = 0; i < size; i++) {
			PathItem *item = (PathItem*)ib_array_index(items, i);
			fprintf(fp, "%s|%u|%u\n",
				item->path->ptr, item->rank, item->timestamp);
		}
		fclose(fp);
	#ifdef _WIN32
		ReplaceFileA(filename, tmpname->ptr, NULL, 2, NULL, NULL);
	#else
		rename(tmpname->ptr, filename);
	#endif
	}
	ib_string_delete(tmpname);
}


//---------------------------------------------------------------------
// save data
//---------------------------------------------------------------------
void data_write(const char *filename, ib_array *items)
{
	FILE *fp;
	int fd;
	fp = fopen(filename, "w+");
	if (fp) {
		int size = ib_array_size(items);
		int i;
		fd = fileno(fp);
		flock(fd, LOCK_EX);
		for (i = 0; i < size; i++) {
			PathItem *item = (PathItem*)ib_array_index(items, i);
			fprintf(fp, "%s|%u|%u\n",
				item->path->ptr, item->rank, item->timestamp);
		}
		fflush(fp);
		flock(fd, LOCK_UN);
		fclose(fp);
	}
}


//---------------------------------------------------------------------
// insert data
//---------------------------------------------------------------------
void data_add(ib_array *items, const char *path)
{
	ib_string *target = ib_string_new_from(path);
	int i = 0, size, found = 0;
#if defined(_WIN32)
	for (i = 0; i < target->size; i++) {
		if (target->ptr[i] == '/') target->ptr[i] = '\\';
		else {
			target->ptr[i] = (char)tolower(target->ptr[i]);
		}
	}
#endif
	size = ib_array_size(items);
	for (i = 0; i < size; i++) {
		PathItem *item = (PathItem*)ib_array_index(items, i);
		int equal = 0;
	#if defined(_WIN32)
		if (item->path->size == target->size) {
			char *src = item->path->ptr;
			char *dst = target->ptr;
			int avail = target->size;
			for (; avail > 0; src++, dst++, avail--) {
				if (tolower(src[0]) != dst[0]) break;
			}
			equal = (avail == 0)? 1 : 0;
		}
	#else
		if (ib_string_compare(item->path, target) == 0) {
			equal = 1;
		}
	#endif
		if (equal) {
			found = 1;
			item->rank++;
			item->timestamp = (uint32_t)time(NULL);
		}
	}
	if (!found) {
		PathItem *ni = item_new(target->ptr, 1, (uint32_t)time(NULL));
		ib_array_push(items, ni);
	}
	ib_string_delete(target);
}


//---------------------------------------------------------------------
// match string
//---------------------------------------------------------------------
int string_match(const char *text, int argc, const char *argv[])
{
	int enhanced = 1;
	int pos = 0;
	int i;
	if (argc == 0) return 0;
	for (i = 0; i < argc; i++) {
		const char *keyword = argv[i];
		const char *p = strstr(text + pos, keyword);
		if (p == NULL) {
			return -1;
		}
		pos = (int)(p - text) + ((int)strlen(keyword));
	}
	if (enhanced != 0) {
		const char *keyword = argv[argc - 1];
		const char *p1 = strrchr(text, '/');
		const char *p2;
		if (p1 == NULL) {
			p1 = strrchr(text, '\\');
			if (p1 == NULL) {
				return 0;
			}
		}
		else {
			p2 = strrchr(text, '\\');
			if (p2 != NULL) {
				if (p2 > p1) {
					p1 = p2;
				}
			}
		}
		p2 = strstr(p1, keyword);
		if (p2 == NULL) {
			return -2;
		}
	}
	return 0;
}


//---------------------------------------------------------------------
// score path
//---------------------------------------------------------------------
void data_score(ib_array *items, int mode)
{
	uint32_t current = (uint32_t)time(NULL);
	int count = (int)ib_array_size(items);
	int i;
	for (i = 0; i < count; i++) {
		PathItem* item = (PathItem*)ib_array_index(items, i);
		if (mode == 0) {
			uint32_t ts = current - item->timestamp;
			if (ts < 3600) {
				item->frecent = item->rank * 4;
			}
			else if (ts < 86400) {
				item->frecent = item->rank * 2;
			}
			else if (ts < 604800) {
				item->frecent = item->rank * 0.5;
			}
			else {
				item->frecent = item->rank * 0.25;
			}
		}
		else if (mode == 1) {
			item->frecent = item->rank;
		}
		else {
			uint32_t ts = current - item->timestamp;
			item->frecent = -((double)ts);
		}
	}
	ib_array_sort(items, item_compare);
	ib_array_reverse(items);
}


//---------------------------------------------------------------------
// display data
//---------------------------------------------------------------------
void data_print(ib_array *items)
{
	int count = (int)ib_array_size(items);
	int i;
	for (i = 0; i < count; i++) {
		PathItem *item = (PathItem*)ib_array_index(items, count - i - 1);
		printf("%.2f: %s\n", item->frecent, item->path->ptr);
	}
}


//---------------------------------------------------------------------
// match
//---------------------------------------------------------------------
ib_array* data_match(int argc, const char *argv[])
{
	const char *data = get_data_file();
	ib_array *items = data_load(data);
	ib_array *result = NULL;
	if (items == NULL) {
		return NULL;
	}
	result = ib_array_new_items();
	while (ib_array_size(items)) {
		PathItem* item = (PathItem*)ib_array_pop(items);
		int valid = 0;
		if (argc == 0) {
			valid = 1;
		}	else {
			int hr = string_match(item->path->ptr, argc, argv);
			if (hr == 0) {
				valid = 1;
			}
		}
		if (valid) {
			ib_array_push(result, item);
		}	else {
			item_delete(item);
		}
	}
	ib_array_delete(items);
	data_score(result, 0);
	return result;
}


//---------------------------------------------------------------------
// update database
//---------------------------------------------------------------------
void z_update(const char *newpath)
{
	const char *data = get_data_file();
	int fd = open(data, O_CREAT | O_RDWR, 0666);
	ib_array *items;
	if (fd < 0) {
		return;
	}
	flock(fd, LOCK_EX);
	off_t size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	items = ib_array_new_items();
	if (size > 0) {
		ib_string *content = ib_string_new();
		int avail = (int)size;
		int start = 0;
		ib_array *array;
		int i, count;
		ib_string_resize(content, (int)size);
		while (avail > 0) {
			int hr = read(fd, content->ptr + start, avail);
			if (hr <= 0) {
				break;
			}
			avail -= hr;
			start += hr;
		}
		lseek(fd, 0, SEEK_SET);
		array = ib_string_split(content, "\n", 1);
		ib_string_delete(content);
		for (count = ib_array_size(array), i = 0; i < count; i++) {
			ib_string *line = (ib_string*)ib_array_index(array, i);
			int p1 = ib_string_find_c(line, '|', 0);
			int p2 = 0;
			if (p1 < 0) {
				continue;
			}
			p2 = ib_string_find_c(line, '|', p1 + 1);
			if (p2 >= 0) { 
				uint32_t timestamp;
				int rank;
				line->ptr[p1] = 0;
				line->ptr[p2] = 0;
				rank = (int)atoi(line->ptr + p1 + 1);
				timestamp = (uint32_t)strtoul(line->ptr + p2 + 1, NULL, 10);
				PathItem *ni = item_new(line->ptr, rank, timestamp);
				ib_array_push(items, ni);
			}
		}
		ib_array_delete(array);
	}
	{
		int i, count = ib_array_size(items);
		int strsize = (int)strlen(newpath);
		int found = 0;
		uint64_t total = 0;
		for (i = 0; i < count; i++) {
			PathItem *item = (PathItem*)ib_array_index(items, i);
			total += item->rank;
		}
		if (total >= 5000) {
			ib_array *na = ib_array_new((void (*)(void*))item_delete);
			for (i = 0; i < count; i++) {
				PathItem *item = (PathItem*)ib_array_index(items, i);
				item->rank = (item->rank * 9) / 10;
			}
			while (ib_array_size(items) > 0) {
				PathItem *item = (PathItem*)ib_array_pop(items);
				if (item->rank == 0) {
					item_delete(item);
				}	else {
					ib_array_push(na, item);
				}
			}
			ib_array_delete(items);
			items = na;
			ib_array_reverse(items);
			count = (int)ib_array_size(items);
		}
		for (i = 0; i < count; i++) {
			PathItem *item = (PathItem*)ib_array_index(items, i);
			if (item->path->size == strsize) {
				if (memcmp(item->path->ptr, newpath, strsize) == 0) {
					item->rank += 1;
					item->timestamp = (uint32_t)time(NULL);
					found = 1;
					break;
				}
			}
		}
		if (found == 0) {
			PathItem *item = item_new(newpath, 1, (uint32_t)time(NULL));
			ib_array_push(items, item);
		}
	}
	{
		int i, count;
		count = (int)ib_array_size(items);
		static char text[PATH_MAX + 60];
		ib_string *content = ib_string_new();
		int avail, start;
		for (i = 0; i < count; i++) {
			PathItem *item = (PathItem*)ib_array_index(items, i);
			sprintf(text, "%s|%u|%u\n", item->path->ptr, item->rank, item->timestamp);
			ib_string_append(content, text);
		}
		start = 0;
		avail = content->size;
		while (avail > 0) {
			int hr = write(fd, content->ptr + start, avail);
			if (hr <= 0) {
				break;
			}
			start += hr;
			avail -= hr;
		}
		ftruncate(fd, content->size);
		ib_string_delete(content);
	}
	flock(fd, LOCK_UN);
	close(fd);
}


//---------------------------------------------------------------------
// add to database
//---------------------------------------------------------------------
void z_add(const char *newpath)
{
	const char *data = get_data_file();
	ib_array *items = data_load(data);
	if (items == NULL) {
		items = ib_array_new_items();
	}
	data_add(items, newpath);
#if 0
	data_save(data, items);
#else
	data_write(data, items);
#endif
	ib_array_delete(items);
}


//---------------------------------------------------------------------
// match and display
//---------------------------------------------------------------------
void z_echo(int argc, const char *argv[])
{
	ib_array *items = data_match(argc, argv);
	if (items == NULL) {
		return;
	}
	if (ib_array_size(items) > 0) {
		PathItem *item = ib_array_obj(items, PathItem*, 0);
		printf("%s\n", item->path->ptr);
	}
	ib_array_delete(items);
}


//---------------------------------------------------------------------
// main entry
//---------------------------------------------------------------------
int main(int argc, const char *argv[])
{
	if (argc <= 1) {
		return 0;
	}
	if (strcmp(argv[1], "--add") == 0) {
		if (argc >= 3) {
#if 0
			z_add(argv[2]);
#else
			z_update(argv[2]);
#endif
		}
	}
	else if (strcmp(argv[1], "-e") == 0) {
		z_echo(argc - 2, argv + 2);
	}
	return 0;
}


