#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_TR2_SYS_NAMESPACE_DEPRECATION_WARNING
#include <filesystem>
#include <string>
#include <vector>
#include <windows.h>
#include <fstream>
#include <Shellapi.h>
#include <commctrl.h>
#include "resource.h"
#include "Search.h"
#pragma comment(lib, "ComCtl32.Lib")
#pragma comment(lib, "Shlwapi.lib")

static HINSTANCE hInst;
static LPCSTR class_name = "ricardo";

struct Path
{
	std::string current;
	std::string selected;
	int selected_index;

	struct Buffer {
		std::string path;
		std::string file;
		bool cut;
		Buffer(const std::string &, std::string, bool);
		operator bool();
	} *buffer;
};

struct Find
{
	std::string path;
	char * name;
	char * type;
	Find(WIN32_FIND_DATA, const std::string &);
	~Find();
};

//Главные функции
ATOM MyRegisterClass(HINSTANCE hInstance);
DWORD WINAPI thread_func(LPVOID);
BOOL InitInstance(HINSTANCE, int);
BOOL CALLBACK Info(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK About(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK Search(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void find_files(std::string, const char *, HWND);

//Создание и инициализация объектов окна
HWND listview_create(HWND, RECT, UINT);
HWND treeviev_create(HWND, RECT);
HMENU init_context_menu(bool);
void add_to_lv(HWND, WIN32_FIND_DATA);
void init_tree(HWND);
void load_tree(HWND, HTREEITEM, const std::string &);
void set_lv_colum(HWND);
void show_tree_item(HWND, LPARAM);
const char * make_colum_names(int);
const char * make_colum_names_dialog(int);
void add_to_dlg_lv(HWND, Find *);
void make_pic(HWND &, HDC &, int, RECT &);

//вспомогательные 
char ** disk_list(int &);
std::string name_item(HWND, HTREEITEM, char *);
std::string full_path(HWND, HTREEITEM);
std::string file_info(WIN32_FIND_DATA, int);
std::string same_name(const std::string &);
void update_listview(HWND, const Path &);
void select_item(HWND, Path &);
void open(HWND, HWND, Path &);
void _delete(const std::string &);
void go_back(HWND, Path &);
void first_page(HWND, Path &);
