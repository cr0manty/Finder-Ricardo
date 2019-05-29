#include "System.h"
#pragma comment (lib, "winmm.lib")

struct For_Threead
{
	//передача переменных в поток
	HWND lv;
	HWND button;
	char * buffer;
	For_Threead(HWND, HWND); //инициализазиця
	~For_Threead();
};

struct For_Search
{
	HWND ListView;
	Path *path;
	For_Search(HWND, Path *);
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND ListView, TreeView, rename; //объекты на окне
	static Path path;//
	RECT rt;
	char * buffer;
	LPNMHDR _switch;

	switch (message)
	{
	case WM_CREATE: {
		path.current = "C:\\";
		GetClientRect(hWnd, &rt);
		ListView = listview_create(hWnd, { rt.left + 350, 0, rt.right - 290, rt.bottom }, LISTVIEW);
		set_lv_colum(ListView);
		update_listview(ListView, path);
		TreeView = treeviev_create(hWnd, rt);
		RegisterHotKey(hWnd, BACK_HK, NULL, VK_LEFT);
		RegisterHotKey(hWnd, FIRST_PAGE_HK, NULL, VK_ESCAPE);
		RegisterHotKey(hWnd, REFRESH_HK, MOD_CONTROL, 0x52); 
		break;
	}

	case WM_NOTIFY:
		_switch = (LPNMHDR)lParam;
		switch (_switch->code)
		{
		case NM_DBLCLK:
			open(hWnd, ListView, path);
			break;

		case NM_CLICK: 
			select_item(ListView, path);
			break;

		case LVN_ENDLABELEDIT: {
			if (rename) {
				buffer = new char[256];
				if (!GetWindowText(rename, buffer, 255)) {
					MessageBox(NULL, "Пустое название!", "Ошибка!", MB_OK);
					delete[] buffer;
					break;
				}
				if (MoveFile(path.selected.c_str(), (path.current + buffer).c_str()))
					update_listview(ListView, path);
				delete[] buffer;
				break;
			}
		}

		case TVN_SELCHANGED: {
			HTREEITEM item = TreeView_GetSelection(TreeView);
			TreeView_Expand(TreeView, item, TVE_EXPAND);
			path.selected = full_path(TreeView, item);

			if (path.selected.size()) {
				Finder file;
				file.find(path.selected);
				if (!file.is_file()) {
					if (!TreeView_GetChild(TreeView, item)) {
						path.selected += '\\';
					}
					path.current = path.selected;
					update_listview(ListView, path);
				}
			}
			break;
		}

		case TVN_ITEMEXPANDED:
			show_tree_item(TreeView, lParam);
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case COPY: {
			Finder file;
			if (file.find(path.selected)) {
				path.buffer = new Path::Buffer(path.selected, file.get().cFileName, false);
			}
			break;
		}

		case CUT: {
			Finder file;
			if (file.find(path.selected)) {
				path.buffer = new Path::Buffer(path.selected, file.get().cFileName, true);
			}
			break;
		}

		case OPEN:
			open(hWnd, ListView, path);
			break;

		case PASTE: {
			try {
				Finder check_file;
				std::string temp = path.current + path.buffer->file;
				if (check_file.find(temp)) {
					temp = same_name(temp);
				}
				if (path.buffer->cut) {
					if (!MoveFile(path.buffer->path.c_str(), temp.c_str()))
						throw "Move Error!";
				}
				else {
					std::tr2::sys::copy(path.buffer->path.c_str(), temp.c_str());
				}
			}		
			catch (...) {
				MessageBox(NULL, "Не удалось вставить объект!", "Ошибка", MB_OK);
				return false;
			}
			delete path.buffer;
			update_listview(ListView, path);
			break;
		}

		case RENAME:
			rename = ListView_EditLabel(ListView, path.selected_index);
			break;

		case CREATE_FOLDER: {
			std::string temp = same_name(path.current + "Новая папка");
			try {
				CreateDirectory(temp.c_str(), NULL);
				path.selected = temp;
				update_listview(ListView, path);
			}
			catch (...) {
				break;
			}
			break;
		}

		case CREATE_TXT: {
			std::string name = same_name(path.current + "Новый текстовый документ" + ".txt");
			try {
				std::ofstream create_file(name);
				create_file.close();
				path.selected = name;
				update_listview(ListView, path);
			}
			catch (...) {
				break;
			}
			break;
		}
	
		case FIRST_PAGE:
			first_page(ListView, path);
			break;

		case INFO:
			DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(INFO_DIALOG), hWnd, (DLGPROC)Info, (LPARAM)&path);
			break;

		case ABOUT:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ABOUT_DIALOG), hWnd, (DLGPROC)About);
			break;

		case SEARCH:
			DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(SEARCH_DIALOG), NULL, (DLGPROC)Search, (LPARAM)new For_Search(ListView, &path));
			break;

		case BACK: 
			go_back(ListView, path);
			break;

		case CLOSE_PROGRAM:
			PostQuitMessage(0);
			break;

		case DELETE_F: {
			if (MessageBox(NULL, "Вы уверены что хотите удалить этот файл?",
				"Удалить файл?", MB_ICONQUESTION | MB_YESNO) == IDYES) {
				_delete(path.selected);
				update_listview(ListView, path);
			}
			break;
		}
		}
		break;

	case WM_HOTKEY:
		switch (wParam)
		{
		case REFRESH_HK:
			update_listview(ListView, path);
			break;

		case BACK_HK:
			go_back(ListView, path);
			break;

		case FIRST_PAGE_HK:
			first_page(ListView, path);
			break;
		}
		break;

	case WM_CONTEXTMENU: {
		select_item(ListView, path);
		buffer = new char[128];
		HMENU context_menu = init_context_menu(path.buffer);
		TrackPopupMenu(context_menu, TPM_RIGHTBUTTON |
			TPM_TOPALIGN |
			TPM_LEFTALIGN,
			LOWORD(lParam), HIWORD(lParam), 0, hWnd, NULL);

		int index = ListView_GetNextItem(ListView, -1, LVNI_ALL | LVNI_SELECTED);
		ListView_GetItemText(ListView, index, 0, buffer, 128);
		path.selected = path.current + buffer;
		delete[] buffer;
		DestroyMenu(context_menu);
		break;
	}

	case WM_DESTROY: 				
		PostQuitMessage(0);
		break;

	case WM_SIZE: {
		HWND temp = NULL;
		GetClientRect(hWnd, &rt);

		SetWindowPos(ListView, temp, rt.left + 300, rt.top, rt.right, rt.bottom, NULL);
		SetWindowPos(TreeView, temp, rt.left, rt.top, rt.left + 300, rt.bottom, NULL);
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

BOOL CALLBACK Info(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND *Object;
	Finder file;
	Path *path;

	switch (msg)
	{
	case WM_INITDIALOG:
		path = (Path*)lParam;
		if (file.find(path->selected) && path->selected.size() > 3) {
			Object = new HWND[6];

			for (int i = 0; i < 5; i++) {
				Object[i] = GetDlgItem(hDlg, OBJECT1 + i);
				SendMessage(Object[i], WM_SETTEXT, (WPARAM)255, (LPARAM)file_info(file.get(), i).c_str());
			}
			Object[5] = GetDlgItem(hDlg, OBJECT6);
			SendMessage(Object[5], WM_SETTEXT, (WPARAM)255, (LPARAM)path->current.c_str());

			delete[] Object;
			return true;
		}

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return true;
		}
		break;
	}
	return false;
}

BOOL CALLBACK About(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static RECT rt;
	static int i = IDB_BITMAP3;
	static int timer = 111;
	switch (msg)
	{
	case WM_INITDIALOG:
		PlaySound(MAKEINTRESOURCE(IDR_WAVE2), 0, SND_RESOURCE | SND_ASYNC | SND_MEMORY);
		SetTimer(hDlg, timer, 70, NULL);
		GetClientRect(hDlg, &rt);
		return true;

	case WM_TIMER:
		i = i == IDB_BITMAP114 ? i = IDB_BITMAP3 : i + 1;
		InvalidateRect(hDlg, NULL, TRUE);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		make_pic(hDlg, hdc, i, rt);
		EndPaint(hDlg, &ps);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			KillTimer(hDlg, timer);
			EndDialog(hDlg, LOWORD(wParam));
			PlaySound(NULL, NULL, NULL);
			return true;
		}
		break;
	}
	return false;
}

BOOL CALLBACK Search(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND *Object;
	static DWORD thread_id;
	static For_Search * _search;
	For_Threead *th;
	LPNMHDR _switch;

	switch (msg)
	{
	case WM_INITDIALOG: {
		_search = (For_Search*)lParam;
		Object = new HWND[3];
		Object[0] = GetDlgItem(hDlg, SEARCH_EDIT);
		Object[2] = listview_create(hDlg, { 30, 60, 410, 200 }, SEARCH_LISTVIEW);

		LVCOLUMN lv_colum;
		lv_colum.mask = LVCF_TEXT | LVCF_WIDTH;
		lv_colum.cx = 136;
		lv_colum.cchTextMax = 256;

		for (int i = 0; i < 3; ++i) {
			lv_colum.pszText = (LPSTR)make_colum_names_dialog(i);
			ListView_InsertColumn(Object[2], i, &lv_colum);
		}
		return true;
	}

	case WM_NOTIFY: {
		_switch = (LPNMHDR)lParam;
		switch (_switch->code) 
		{
		case NM_DBLCLK: {
			int index = ListView_GetNextItem(Object[2],
				-1, LVNI_ALL | LVNI_SELECTED);

			if (index == -1)
				return false;

			char *buffer = new char[256];
			ListView_GetItemText(Object[2], index, 2, buffer, 255);
			_search->path->current = buffer;
			delete[] buffer;
			update_listview(_search->ListView, *_search->path);
			return true;
		}
		return false;
		}
	}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case SEARCH_SEARCH: {
			Object[1] = GetDlgItem(hDlg, SEARCH_SEARCH);
			ListView_DeleteAllItems(Object[2]);
			th = new For_Threead(Object[2], Object[1]);
			GetWindowText(Object[0], th->buffer, 127);

			if (!strlen(th->buffer)) {
				MessageBox(hDlg, "Строка не может быть пустой!", "Ошибка!", MB_OK);
				return 0;
			}

			SendMessage(Object[0], WM_KILLFOCUS, 0, 0);
			CreateThread(NULL, 0, thread_func, th, 0, &thread_id);
			EnableWindow(Object[1], FALSE);
			return true;
		}

		case IDCANCEL:
			PostThreadMessage(thread_id, WM_QUIT, 0, 0);
			delete[] Object;
			EndDialog(hDlg, LOWORD(wParam));
			return true;
		}
		return false;
	}
	return false;
}

DWORD WINAPI thread_func(LPVOID _item)
{
	int disk_amount;
	For_Threead * item = (For_Threead*)_item;
	char **list = disk_list(disk_amount);
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	for (int i = 0; i < disk_amount; ++i) {
		if (PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE))
			break;

		find_files(list[i], item->buffer, item->lv);
		delete[] list[i];
	}
	EnableWindow(item->button, TRUE);

	delete[] list;
	delete item;
	ExitThread(0);
}

For_Threead::For_Threead(HWND _lv, HWND _button) :
	buffer(new char[128]), lv(_lv), button(_button)
{
}

For_Threead::~For_Threead()
{
	delete[] buffer;
}

For_Search::For_Search(HWND _ListView, Path *_path) :
	ListView(_ListView), path(_path)
{
}
