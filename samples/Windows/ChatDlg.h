#pragma once
#include "atlframe.h"
#include "atlctrls.h"
#include "atlctrlx.h"
#include "resource.h"
#include "Business/MarsWrapper.h"

class CChatDlg : public CDialogImpl<CChatDlg>, public CUpdateUI<CChatDlg>,
	public CMessageFilter, public CIdleHandler
{
public:
	enum { IDD = IDD_DIALOGCHAT };

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CChatDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CChatDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_BTNSENDMSG, BN_CLICKED, OnBnClickedBtnsendmsg)
	END_MSG_MAP()

	CChatDlg();
	~CChatDlg();


	void OnRecvChatMsg(const ChatMsg& msg);
	void SetChatInfo(const std::string& name, const std::string& topic);
protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnsendmsg(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
protected:
	CListViewCtrl m_conversationList;
	CEdit m_edit;
	CRichEditCtrl m_editMsgFlow;
	CHARFORMAT m_cfSelfAccount;
	CHARFORMAT m_cfRecvAccount;
	CHARFORMAT m_cfDefaultAccount;
	PARAFORMAT2 m_pfAccountIndent;
	PARAFORMAT2 m_pfContentIndent;

	std::string m_topic;
	std::string m_name;
};
