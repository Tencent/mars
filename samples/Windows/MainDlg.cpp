#include "stdafx.h"
#include "MainDlg.h"
#include "AboutDlg.h"
#include "Business/MarsWrapper.h"
#include <comutil.h>


BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	UIUpdateChildWindows();
	return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	RECT rc;
	GetClientRect(&rc);
	m_tabView.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE, WS_EX_STATICEDGE);
	m_pingServerDlg = boost::shared_ptr<CPingServerDlg>(new CPingServerDlg());
	m_pingServerDlg->Create(m_tabView.m_hWnd);
	m_tabView.AddPage(m_pingServerDlg->m_hWnd, _T("Ping Server"));
	m_pingServerDlg->SetHostWnd(this);
		
	/*HWND hTabCtrl = GetDlgItem(IDC_TAB1);
	TCITEM item;
	item.mask = TCIF_TEXT;
	item.iImage = 0;
	item.lParam = 0;
	item.pszText = _T("PingServer");
	item.cchTextMax = 10;
	SendMessage(hTabCtrl, TCM_INSERTITEM, 0, (LPARAM)&item);

	item.pszText = _T("Chat");
	item.cchTextMax = 10;
	SendMessage(hTabCtrl, TCM_INSERTITEM, 1, (LPARAM)&item);
	
	HWND hChildWnd = CreateDialogParam(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDD_DIALOGPINGSERVER), hTabCtrl, NULL, 0);
	::ShowWindow(hChildWnd, SW_SHOWDEFAULT);*/


	//HWND hChildWnd = CreateDialogParam(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDD_DIALOGPINGSERVER), hTabCtrl, NULL, 0);
	//ShowWindow(hChildWnd, SW_SHOWDEFAULT);

	MarsWrapper::Instance().start();

	MarsWrapper::Instance().setChatMsgObserver(this);
	return TRUE;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);


	MarsWrapper::Instance().setChatMsgObserver(nullptr);
	m_pingServerDlg->SetHostWnd(nullptr);
	DestoryAllChatTab();
	return 0;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add validation code 
	CloseDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(wID);
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}
LRESULT CMainDlg::OnGetConversationList(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DestoryAllChatTab();
	std::vector<ConversationInfo>* conversationList = (std::vector<ConversationInfo>*)wParam;
	int size = conversationList->size();
	if (size > 5)size = 5;

	std::string name = (char*)(_bstr_t)m_pingServerDlg->GetUserName().c_str();
	for (int i = 0; i < size; ++i)
	{
		const ConversationInfo& item = (*conversationList)[i];
		CChatDlg* dlg = new CChatDlg();
		dlg->Create(m_tabView.m_hWnd);
		dlg->SetChatInfo(name, item.topic_);
		m_tabView.AddPage(dlg->m_hWnd, (wchar_t*)(_bstr_t)item.notice_.c_str());
		m_chatDlgList[item.topic_] = dlg;
	}
	delete conversationList;
	return 0;
}
void CMainDlg::DestoryAllChatTab()
{
	int size = m_tabView.GetPageCount();
	for (int i = size - 1; i > 0; i--)m_tabView.RemovePage(i);
	for (auto it = m_chatDlgList.begin(); it != m_chatDlgList.end(); ++it)
	{
		delete it->second;
	}
	m_chatDlgList.clear();
}
void CMainDlg::OnRecvChatMsg(const ChatMsg& msg)
{
	auto it = m_chatDlgList.find(msg.topic_);
	if (it != m_chatDlgList.end()) it->second->OnRecvChatMsg(msg);
}