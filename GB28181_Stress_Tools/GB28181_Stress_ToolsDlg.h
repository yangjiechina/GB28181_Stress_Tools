
// GB28181_Stress_ToolsDlg.h : header file
//

#pragma once
#include "Message.h"
#include <functional>

// CGB28181StressToolsDlg dialog
class CGB28181StressToolsDlg : public CDialogEx
{
// Construction
public:
	CGB28181StressToolsDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GB28181_STRESS_TOOLS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_device_list;
	afx_msg void OnBnClickedButton1();
private:
	CButton m_btn_start;
	CString m_edit_server_sip_id;
	CString m_edit_server_ip;
	int m_edit_server_port;
	int m_edit_device_count;

	bool CheckParams();

	void Start();

	void Stop();

	void update_item(int index, Message msg);

	std::function<void(int index, Message msg)> callback;
public:
	afx_msg void OnEnChangeEdit1();
private:
	CString m_edit_password;
};
