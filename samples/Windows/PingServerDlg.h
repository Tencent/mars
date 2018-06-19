#pragma once
#include "atlframe.h"
#include "atlctrls.h"
#include "atlctrlx.h"
#include "resource.h"

#include "Wrapper/NetworkObserver.h"
#include "Wrapper/CGITask.h"
#include "Business/HelloCGITask.h"
#include "Business/GetConvListCGITask.h"
# include "mars/boost/enable_shared_from_this.hpp"
#include <string>

class CMainDlg;
class CPingServerDlg : public CDialogImpl<CPingServerDlg>, public CUpdateUI<CPingServerDlg>,
	public CMessageFilter, public CIdleHandler, public boost::enable_shared_from_this<CPingServerDlg>, public HelloCGICallback, public GetConvListCGICallback
{
public:
	enum { IDD = IDD_DIALOGPINGSERVER };
	CPingServerDlg();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CPingServerDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CPingServerDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_HANDLER(IDC_BTNPINGSERVER, BN_CLICKED, OnBnClickedTestmars)
		COMMAND_HANDLER(IDC_BTNCHATROOM, BN_CLICKED, OnBnClickedBtnchatroom)
	END_MSG_MAP()

	virtual void OnResponse(HelloCGITask* task, const com::tencent::mars::sample::proto::HelloResponse& response);
	virtual void OnResponse(GetConvListCGITask* task, const com::tencent::mars::sample::proto::ConversationListResponse& response);

	void SetHostWnd(CMainDlg* _host);
	std::wstring GetUserName();
protected:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedTestmars(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnchatroom(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

protected:
	CEdit m_editInfo;
	CMainDlg* m_host;
};
