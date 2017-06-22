// PingServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ChatDlg.h"
#include "Business/MarsWrapper.h"
#include <comutil.h>

CChatDlg::CChatDlg()
{
	::LoadLibrary(CRichEditCtrl::GetLibraryName());
}
CChatDlg::~CChatDlg()
{
	MarsWrapper::Instance().setChatMsgObserver(NULL);
}
BOOL CChatDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CChatDlg::OnIdle()
{
	UIUpdateChildWindows();
	return FALSE;
}
LRESULT CChatDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_edit = GetDlgItem(IDC_EDITMSG);
	m_editMsgFlow = GetDlgItem(IDC_RICHEDITMSGFLOW);

	m_cfSelfAccount.cbSize = sizeof(CHARFORMAT);
	m_cfSelfAccount.dwMask = CFM_COLOR;
	m_cfSelfAccount.dwEffects = 0;
	m_cfSelfAccount.crTextColor = RGB(250, 100, 0);
	
	m_cfRecvAccount.cbSize = sizeof(CHARFORMAT);
	m_cfRecvAccount.dwMask = CFM_COLOR;
	m_cfRecvAccount.dwEffects = 0;
	m_cfRecvAccount.crTextColor = RGB(0, 100, 200);

	m_editMsgFlow.GetDefaultCharFormat(m_cfDefaultAccount);

	ZeroMemory(&m_pfAccountIndent, sizeof(PARAFORMAT2));
	m_pfAccountIndent.cbSize = sizeof(PARAFORMAT2);
	m_pfAccountIndent.dwMask = PFM_STARTINDENT;
	m_pfAccountIndent.dxStartIndent = 0;
	ZeroMemory(&m_pfContentIndent, sizeof(PARAFORMAT2));
	m_pfContentIndent.cbSize = sizeof(PARAFORMAT2);
	m_pfContentIndent.dwMask = PFM_STARTINDENT;
	m_pfContentIndent.dxStartIndent = 200;


	return true;
}


LRESULT CChatDlg::OnBnClickedBtnsendmsg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int len = m_edit.GetWindowTextLength();
	wchar_t* buf = new wchar_t[len + 3 ];
	m_edit.GetWindowText(buf, len + 1);
	buf[len] = L'\r';
	buf[len + 1] = L'\n';
	buf[len + 2] = L'\0';

	int msgFlowTextLen = m_editMsgFlow.GetWindowTextLength();
	m_editMsgFlow.SetSel(msgFlowTextLen, msgFlowTextLen);

	std::string pre_text = m_name + " :\r\n";
	m_editMsgFlow.SetParaFormat(m_pfAccountIndent);
	m_editMsgFlow.SetSelectionCharFormat(m_cfSelfAccount);
	m_editMsgFlow.AppendText((wchar_t*)(_bstr_t)pre_text.c_str());
	m_editMsgFlow.SetParaFormat(m_pfContentIndent);
	m_editMsgFlow.SetSelectionCharFormat(m_cfDefaultAccount);
	m_editMsgFlow.AppendText(buf);
	m_editMsgFlow.PostMessage(WM_VSCROLL, SB_BOTTOM);
	_bstr_t bstr_buf = (buf);
	ChatMsg msg;
	msg.from_ = m_name;
	msg.topic_ = m_topic;
	msg.content_ = (char*)bstr_buf;
	MarsWrapper::Instance().sendChatMsg(msg);

	delete[]buf;

	m_edit.SetWindowText(L"");

	return 0;
}
void CChatDlg::OnRecvChatMsg(const ChatMsg& msg)
{
	m_editMsgFlow.SetSelNone();

	std::string text_account = msg.from_ + " :\r\n    ";
	m_editMsgFlow.SetParaFormat(m_pfAccountIndent);
	m_editMsgFlow.SetSelectionCharFormat(m_cfRecvAccount);
	m_editMsgFlow.AppendText((wchar_t*)(_bstr_t)text_account.c_str());
	m_editMsgFlow.SetParaFormat(m_pfContentIndent);
	m_editMsgFlow.SetSelectionCharFormat(m_cfDefaultAccount);
	std::string content = msg.content_ + "\r\n";
	m_editMsgFlow.AppendText((wchar_t*)(_bstr_t)content.c_str());


	m_editMsgFlow.PostMessage(WM_VSCROLL, SB_BOTTOM);
}


void CChatDlg::SetChatInfo(const std::string& name, const std::string& topic)
{
	m_topic = topic;
	m_name = name;
}