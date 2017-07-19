// PingServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PingServerDlg.h"
#include <comutil.h>

#include "Business/MarsWrapper.h"
#include "MainDlg.h"
CPingServerDlg::CPingServerDlg()
	: m_host(nullptr)
{}
BOOL CPingServerDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CPingServerDlg::OnIdle()
{
	UIUpdateChildWindows();
	return FALSE;
}

LRESULT CPingServerDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_editInfo = GetDlgItem(IDC_EDITINFO);
	return true;
}
LRESULT CPingServerDlg::OnBnClickedTestmars(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	std::wstring strName = GetUserName();
	MarsWrapper::Instance().pingServer((char*)(_bstr_t)strName.c_str(), "Hello World", shared_from_this());
	std::wstring info = L"Start Send Message: User=" + strName + L", Message=Hello World !\r\n";
	m_editInfo.AppendText(info.c_str());
	return 0;
}

LRESULT CPingServerDlg::OnBnClickedBtnchatroom(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	MarsWrapper::Instance().getConversationList(shared_from_this());
	std::wstring info = L"Start GetConversationList!\r\n";
	m_editInfo.AppendText(info.c_str());
	return 0;
}
void CPingServerDlg::SetHostWnd(CMainDlg* _host)
{
	m_host = _host;
}

void CPingServerDlg::OnResponse(HelloCGITask* task, const com::tencent::mars::sample::proto::HelloResponse& response)
{
	std::wstring info = std::wstring(L"Recv Hello Response: ") + (wchar_t*)(_bstr_t)response.errmsg().c_str() + L"\r\n";
	m_editInfo.AppendText(info.c_str());
}

void CPingServerDlg::OnResponse(GetConvListCGITask* task, const com::tencent::mars::sample::proto::ConversationListResponse& response)
{
	std::string info = "Recv GetConvsationList Response: \r\n";
	std::vector<ConversationInfo>* conversationList = new std::vector<ConversationInfo>();
	int size = response.list_size();
	for (int i = 0; i < size; ++i)
	{
		const com::tencent::mars::sample::proto::Conversation& item = response.list(i);
		info += "    topic=" +item.topic();
		info += ", notice=" + item.notice();
		info += ", name=" + item.name();
		info += "\r\n";
		
		ConversationInfo conversationItem;
		conversationItem.topic_ = item.topic();
		conversationItem.notice_ = item.notice();
		conversationItem.name_ = item.name();
		conversationList->push_back(conversationItem);
	}
	m_editInfo.AppendText((wchar_t*)(_bstr_t)info.c_str());

	if (m_host)m_host->PostMessage(WM_GETCONVERSATIONLIST, (WPARAM)conversationList);
}
LRESULT CPingServerDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

std::wstring CPingServerDlg::GetUserName()
{
	wchar_t name[1024] = L"\0";
	GetDlgItemText(IDC_EDITNAME, name, sizeof(name) / sizeof(name[0]));
	std::wstring strName = name;
	if (strName.empty())strName = L"Anonymous";
	return strName;
}