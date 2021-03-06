// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include <Strsafe.h>
#include <Shlobj.h>  
#pragma comment(lib,"Shell32.lib")  
	
#ifdef DWMBLUR	//win7毛玻璃开关
#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")
#endif
	
CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited = FALSE;
}

CMainDlg::~CMainDlg()
{
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	#ifdef DWMBLUR	//win7毛玻璃开关
	MARGINS mar = {5,5,30,5};
	DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
	#endif

	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	m_bLayoutInited = TRUE;
	return 0;
}

HRESULT CMainDlg::OnChooseFolder()
{
	TCHAR szBuffer[MAX_PATH] = {0};
	BROWSEINFO bi = {0};
	bi.hwndOwner = NULL;					// 拥有着窗口句柄，为NULL表示对话框是非模态的，实际应用中一般都要有这个句柄  
	bi.pszDisplayName = szBuffer;			// 接收文件夹的缓冲区  
	bi.lpszTitle = TEXT("选择一个文件夹");	// 标题  
	bi.ulFlags = BIF_NEWDIALOGSTYLE;
	LPITEMIDLIST idl = SHBrowseForFolder(&bi);
	if (SHGetPathFromIDList(idl, szBuffer)) {
		SOUI::SWindow *pText = FindChildByName2<SOUI::SWindow>(L"text_folder_path");
		assert(pText);
		pText->SetWindowText(szBuffer);
	} else {
		SMessageBox(NULL, TEXT("请选择一个文件夹"), NULL, MB_ICONERROR);
	}

	return S_OK;
}

HRESULT CMainDlg::OnDeleteFolder()
{
	SOUI::SWindow *pText = FindChildByName2<SOUI::SWindow>(L"text_folder_path");
	assert(pText);
	SStringW lpPath = pText->GetWindowText();
	if (S_OK == DeleteFolder((LPWSTR)(LPCWSTR)lpPath)) {
		SMessageBox(GetActiveWindow(), L"删除成功", L"提示", MB_OK);
	} else {
		SMessageBox(GetActiveWindow(), L"删除失败", L"提示", MB_OK);
	}

	return S_OK;
}

HRESULT CMainDlg::DeleteFolder(wchar_t Dir[MAX_PATH])
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	wchar_t DirSpec[MAX_PATH];						 //定义要遍历的文件夹的目录  
	DWORD dwError;
	StringCchCopy(DirSpec, MAX_PATH, Dir);
	StringCchCat(DirSpec, MAX_PATH, TEXT("\\*"));	 //定义要遍历的文件夹的完整路径\*  

	hFind = FindFirstFile(DirSpec, &FindFileData);   //找到文件夹中的第一个文件  

	if (hFind == INVALID_HANDLE_VALUE)               //如果hFind句柄创建失败，输出错误信息  
	{
		FindClose(hFind);
		return S_FALSE;
	} else {
		while (FindNextFile(hFind, &FindFileData) != 0)//当文件或者文件夹存在时  
		{
			if ((FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) != 0 && wcscmp(FindFileData.cFileName, L".") == 0 || wcscmp(FindFileData.cFileName, L"..") == 0)        //判断是文件夹&&表示为"."||表示为"."  
			{
				continue;
			}
			if ((FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) != 0)      //判断如果是文件夹  
			{
				wchar_t DirAdd[MAX_PATH];
				StringCchCopy(DirAdd, MAX_PATH, Dir);
				StringCchCat(DirAdd, MAX_PATH, TEXT("\\"));
				StringCchCat(DirAdd, MAX_PATH, FindFileData.cFileName);				//拼接得到此文件夹的完整路径  
				DeleteFolder(DirAdd);												//实现递归调用  
			}
			if ((FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == 0)		//如果不是文件夹  
			{
				//拼出完整路径  
				if (0 != wcscmp(FindFileData.cFileName, L"test_delete_file.txt")) {
					SStringW strFileName;
					strFileName.Format(L"%s\\%s", Dir, FindFileData.cFileName);
					/*DeleteFile(strFileName);*/

					// 重启后删除指定的文件
					MoveFileEx(strFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				}
			}
		}
		FindClose(hFind);
	}

	return S_OK;
}

//TODO:消息映射
void CMainDlg::OnClose()
{
	CSimpleWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;
	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}

