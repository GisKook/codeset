// edifnetlistflattenerDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CedifnetlistflattenerDlg dialog
class CedifnetlistflattenerDlg : public CDialog
{
// Construction
public:
	CedifnetlistflattenerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EDIFNETLISTFLATTENER_DIALOG };

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
	afx_msg void OnBnClickedButtonInfile();

private:
	CString m_strInFileName;
	CString m_strOutFileName;
public:
	CEdit m_infile;
	afx_msg void OnBnClickedButtonOutfile();
	CEdit m_outfile;
	afx_msg void OnBnClickedOk();
	CStatic m_tips;
private:
	CString m_strfilepath;
private:
	CString m_conf;
    CString m_logfilename;
public:
	afx_msg void OnBnClickedCancel();
};
