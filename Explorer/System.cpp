#include "System.h"
#pragma comment(lib, "Shlwapi.lib")

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW; 		
	wcex.lpfnWndProc = (WNDPROC)WndProc; 		
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance; 			
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW); 	
	wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOW + 1); 
	wcex.lpszMenuName = NULL; 				
	wcex.lpszClassName = class_name;
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex); 
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;
	HWND hWnd;
	hWnd = CreateWindow(class_name,
		"Проводник по Рикардо", 				
		WS_OVERLAPPEDWINDOW,		
		CW_USEDEFAULT, 			
		CW_USEDEFAULT,			
		CW_USEDEFAULT, 			
		CW_USEDEFAULT, 			
		NULL, 					
		LoadMenu(hInstance, MAKEINTRESOURCE(MAIN_MENU)),
		hInstance,
		NULL); 				

	if (!hWnd) 	
	{
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow); 		
	UpdateWindow(hWnd); 				
	return TRUE;
}

HWND listview_create(HWND hWnd, RECT rt, UINT _id)
{
	INITCOMMONCONTROLSEX init;
	init.dwSize = sizeof(INITCOMMONCONTROLSEX);
	init.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&init);

	HWND ListView = CreateWindow(WC_LISTVIEW,
		NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT |
		LVS_EDITLABELS | LVS_SINGLESEL,
		rt.left, rt.top, rt.right, rt.bottom,
		hWnd,
		(HMENU)_id,
		GetModuleHandle(NULL),
		NULL);

	ListView_SetExtendedListViewStyleEx(ListView, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	ShowWindow(ListView, SW_SHOWDEFAULT);
	return ListView;
}

void add_to_lv(HWND ListView, WIN32_FIND_DATA _file)
{
	std::string buffer = file_info(_file, 0);
	int last_index = ListView_GetItemCount(ListView);
	LVITEM lv_item;
	lv_item.mask = LVIF_TEXT;
	lv_item.cchTextMax = 256;
	lv_item.iItem = last_index;
	lv_item.pszText = (LPSTR)buffer.c_str();
	lv_item.iSubItem = 0;
	ListView_InsertItem(ListView, &lv_item);

	for (int i = 1; i < 5; i++) {
		buffer = file_info(_file, i);
		ListView_SetItemText(ListView, last_index, i, (char*)buffer.c_str());
	}
}

HWND treeviev_create(HWND hWnd, RECT rt)
{
	InitCommonControls();

	HWND Tree = CreateWindowEx(0,
		WC_TREEVIEW,
		TEXT("Tree View"),
		WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | WS_VSCROLL | WS_TABSTOP |
		TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS,
		0, 0, rt.left + 350, rt.bottom,
		hWnd,
		(HMENU)TREEVIEW,
		GetModuleHandle(NULL),
		NULL);

	ShowWindow(Tree, SW_SHOWDEFAULT);
	init_tree(Tree);
	return Tree;
}

void set_lv_colum(HWND ListView)
{
	RECT rt;
	GetClientRect(ListView, &rt);

	LVCOLUMN lv_colum;
	lv_colum.mask = LVCF_TEXT | LVCF_WIDTH;
	lv_colum.cx = (rt.right - rt.left) / 5;
	lv_colum.cchTextMax = 256;

	for (int i = 0; i < 5; ++i) {
		lv_colum.pszText = (LPSTR)make_colum_names(i);
		ListView_InsertColumn(ListView, i, &lv_colum);
	}
}

const char * make_colum_names(int _id)
{
	switch (_id)
	{
	case 0:
		return "Имя";
	
	case 1:
		return "Тип";

	case 2:
		return "Размер";

	case 3:
		return "Дата изменения";

	case 4:
		return "Дата создания";
	}
	return "";
}

const char * make_colum_names_dialog(int _id)
{
	switch (_id)
	{
	case 0:
		return "Имя";

	case 1:
		return "Тип";

	case 2:
		return "Путь";
	}
	return "";
}

void show_tree_item(HWND Tree, LPARAM lParam)
{
	NMHDR *ex = (NMHDR*)lParam;
	LPNMTREEVIEW tv_item = (LPNMTREEVIEW)ex;
	HTREEITEM item = tv_item->itemNew.hItem;
	std::string child_path;
	HTREEITEM child = TreeView_GetChild(Tree, item);

	if (child) {
		do {
			if (!TreeView_GetChild(Tree, child)) {
				child_path = full_path(Tree, child) + "\\*";
				load_tree(Tree, child, child_path);
			}
		} while (child = TreeView_GetNextSibling(Tree, child));
	}
}

HMENU init_context_menu(bool _paste)
{
	HMENU c_menu = CreatePopupMenu();
	HMENU a_menu = CreatePopupMenu();
	UINT paste;

	if (_paste)
		paste = MF_ENABLED;
	else
		paste = MF_DISABLED;

	AppendMenu(c_menu, MFT_STRING, OPEN, "Открыть");
	AppendMenu(c_menu, MFT_STRING | MF_POPUP, (UINT)a_menu, "Создать");
		AppendMenu(a_menu, MFT_STRING, CREATE_TXT, "Текстовый файл");
		AppendMenu(a_menu, MFT_STRING, CREATE_FOLDER, "Папку");
	AppendMenu(c_menu, MFT_STRING, CUT, "Вырезать");
	AppendMenu(c_menu, MFT_STRING, COPY, "Копировать");
	AppendMenu(c_menu, MFT_STRING | paste, PASTE, "Вставить");
	AppendMenu(c_menu, MFT_STRING, DELETE_F, "Удалить");
	AppendMenu(c_menu, MFT_STRING, RENAME, "Переименовать");
	AppendMenu(c_menu, MFT_STRING, INFO, "Свойства");

	return c_menu;
}

std::string name_item(HWND Tree, HTREEITEM item, char * _text)
{
	TV_ITEM tv_item;

	tv_item.mask = TVIF_TEXT | TVIF_HANDLE;
	tv_item.hItem = item;
	tv_item.pszText = _text;
	tv_item.cchTextMax = 63;
	TreeView_GetItem(Tree, &tv_item);

	return (char*)tv_item.pszText;
}

std::string full_path(HWND Tree, HTREEITEM _item)
{
	char *buffer;
	std::string path;
	HTREEITEM root = TreeView_GetRoot(Tree);

	while (_item != root && _item) {
		buffer = new char[128];
		path.insert(0, (name_item(Tree, _item, buffer) + '\\'));
		_item = TreeView_GetParent(Tree, _item);
		delete[] buffer;
	}
	return path;
}

char ** disk_list(int & amount)
{
	char *all = new char[512], *disk, **list;
	int i = 0;
	amount = GetLogicalDriveStrings(511, all) / 4;

	disk = all;
	list = new char*[amount];

	while (*disk != 0) {
		list[i] = new char[4];
		strcpy(list[i++], disk);
		disk += 4;
	}
	delete[] all;
	return list;
}

std::string file_info(WIN32_FIND_DATA file, int _switch)
{
	std::string _return;
	char *buffer;
	SYSTEMTIME st;

	switch (_switch)
	{
	case 0:
		_return = file.cFileName;
		break;

	case 1:
		if (file.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
			_return = file.cFileName;
			_return = _return.substr(_return.rfind('.') + 1, _return.size()).c_str();
		}
		else {
			_return = "Папка с файлами";
		}
		break;

	case 2:
		if (file.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
			_return = std::to_string((file.nFileSizeHigh * MAXDWORD) + file.nFileSizeLow) + " КБ";
		}
		else {
			_return = " ";
		}
		break;

	case 3:
		FileTimeToSystemTime(&file.ftLastWriteTime, &st);
		buffer = new char[16];

		sprintf(buffer,
			"%02d.%02d.%d",
			st.wDay,
			st.wMonth,
			st.wYear);

		_return = buffer;
		delete[] buffer;
		break;

	case 4:
		FileTimeToSystemTime(&file.ftCreationTime, &st);
		buffer = new char[16];

		sprintf(buffer,
			"%02d.%02d.%d",
			st.wDay,
			st.wMonth,
			st.wYear);

		_return = buffer;
		delete[] buffer;
		break;

	}
	return _return;
}

void update_listview(HWND ListView, const Path & _path)
{
	std::string buffer;
	Finder file;

	ListView_DeleteAllItems(ListView);
	if (file.find(_path.current + "*")) {
		do {
			if (file.not_hidden()) {
				add_to_lv(ListView, file.get());
			}
		} while (file.go_next());
	}
}

void select_item(HWND ListView, Path & _path)
{
	char *buffer = new char[128];
	int index = ListView_GetNextItem(ListView,
		-1, LVNI_ALL | LVNI_SELECTED);
	ListView_GetItemText(ListView, index, 0, buffer, 127);

	if (index != -1) {
		_path.selected = _path.current + buffer;
		_path.selected_index = index;
	}
	delete[] buffer;
}

void init_tree(HWND Tree)
{
	TV_INSERTSTRUCT insert;
	insert.item.mask = TVIF_TEXT | TVIF_PARAM;
	insert.hParent = NULL;
	insert.hInsertAfter = TVI_LAST;
	insert.item.pszText = (char*)"Мой компьютер";

	HTREEITEM root = TreeView_InsertItem(Tree, &insert);
	HTREEITEM item;
	int disk_amount;
	char **disks = disk_list(disk_amount), *temp;

	for (int i = 0; i < disk_amount; i++) {
		temp = new char[64];
		insert.hParent = root;
		insert.item.pszText = disks[i];
		item = TreeView_InsertItem(Tree, &insert);
		load_tree(Tree, item, name_item(Tree, item, temp));
		delete[] temp;
	}

	TreeView_Expand(Tree, root, TVE_EXPAND);
	TreeView_SelectItem(Tree, root);

	for (int i = 0; i < disk_amount; ++i)
		delete[] disks[i];
	delete[] disks;
}

void load_tree(HWND Tree, HTREEITEM _item, const std::string & _path)
{
	Finder file;
	if (file.find(_path + '*')) {

		TV_INSERTSTRUCT _insert;
		_insert.hParent = _item;
		_insert.item.mask = TVIF_TEXT;

		do {
			if (!file.is_file() && file.not_hidden()) {
				_insert.item.pszText = file.get().cFileName;
				TreeView_InsertItem(Tree, &_insert);
			}
		} while (file.go_next());
	}
}

void open(HWND hWnd, HWND ListView, Path & _path)
{
	if (_path.selected.size()) {
		Finder file;
		if (file.find(_path.selected)) {
			if (file.is_file()) {
				SHELLEXECUTEINFO sei;
				sei.cbSize = sizeof(sei);
				sei.fMask = SEE_MASK_NOCLOSEPROCESS;
				sei.lpVerb = "open";
				sei.hwnd = hWnd;
				sei.nShow = SW_SHOWNORMAL;
				sei.lpFile = _path.selected.c_str();
				sei.lpParameters = NULL;
				sei.lpDirectory = NULL;
				ShellExecuteEx(&sei);
			}
			else {
				_path.current = _path.selected + '\\';
				update_listview(ListView, _path);
			}
		}
	}
}

std::string same_name(const std::string &_name)
{
	int i = 1;
	Finder file;
	std::string buffer = _name;

	while (file.find(buffer)) {
		if (file.is_file()) {
			buffer = _name;
			buffer.insert(buffer.rfind('.'), " (" + std::to_string(i++) + ")");
		}
		else {
			buffer = _name;
			buffer += " (" + std::to_string(i++) + ")";
		}
	}
	return buffer;
}

void _delete(const std::string &_path)
{
	Finder file;
	if (file.find(_path)) {
		try {
			if (file.is_file()) {
				std::tr2::sys::remove(_path);
			}
			else {
				std::tr2::sys::remove_all(_path);
			}
		}
		catch (...) {
			MessageBox(NULL, "Не удалось удалить объект!\nВозможно вам не хватает прав!", "Ошибка!", MB_OK);
		}
	}
}

void find_files(std::string _start, const char * _name, HWND ListView)
{
	Finder file;
	if (file.find(_start + "*")) {
		do {
			if (!strcmp(file.get().cFileName, _name)) {
				add_to_dlg_lv(ListView, new Find(file.get(), _start));
			}
			if (!file.is_file() && file.not_hidden()) {
				find_files(_start + file.get().cFileName + "\\", _name, ListView);
			}
		} while (file.go_next());
	}
}

void add_to_dlg_lv(HWND ListView, Find * _find)
{
	int last_index = ListView_GetItemCount(ListView);
	LVITEM lv_item;
	lv_item.mask = LVIF_TEXT;
	lv_item.cchTextMax = 128;
	lv_item.iItem = last_index;
	lv_item.pszText = (LPSTR)_find->name;
	lv_item.iSubItem = 0;
	ListView_InsertItem(ListView, &lv_item);
	ListView_SetItemText(ListView, last_index, 1, (char*)_find->type);
	ListView_SetItemText(ListView, last_index, 2, (char*)_find->path.c_str());
	delete _find;
}

void go_back(HWND ListView, Path & _path)
{
	if (_path.current.size() > 4) {
		int index = _path.current.size() - 3;
		while (_path.current[--index] != '\\');
		_path.current = _path.current.substr(0, index + 1);
		update_listview(ListView, _path);
	}
}

void first_page(HWND ListView, Path & _path)
{
	_path.current = "C:\\";
	update_listview(ListView, _path);
}

Find::Find(WIN32_FIND_DATA _file, const std::string & _path) :
	name(new char[32]), type(new char[32])
{
	strcpy(name,_file.cFileName);
	if (_file.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
		path = name;
		strcpy(type, path.substr(path.rfind('.') + 1, path.size()).c_str());
	}
	else {
		strcpy(type,"Папка с файлами");
	}
	path = _path;
}

Find::~Find()
{
	delete[] name;
	delete[] type;
}

Path::Buffer::Buffer(const std::string & _path, std::string _name, bool _cut) :
	path(_path), file(_name), cut(_cut)
{
}

Path::Buffer::operator bool()
{
	return path.size() && file.size();
}

void make_pic(HWND & hDlg, HDC &hDC, int _id, RECT & rt)
{
	HANDLE hBitmap, hOldBitmap;
	BITMAP Bitmap;
	HDC hCompatibleDC;
	hBitmap = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(_id), IMAGE_BITMAP, 0, 0, NULL);
	GetObject(hBitmap, sizeof(BITMAP), &Bitmap);
	hCompatibleDC = CreateCompatibleDC(hDC);
	hOldBitmap = SelectObject(hCompatibleDC, hBitmap);
	StretchBlt(hDC, 0, 0, rt.right, rt.bottom, hCompatibleDC, 0, 0, Bitmap.bmWidth,
		Bitmap.bmHeight, SRCCOPY);
	SelectObject(hCompatibleDC, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hCompatibleDC);
}
