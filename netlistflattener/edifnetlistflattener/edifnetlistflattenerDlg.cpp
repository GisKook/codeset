// edifnetlistflattenerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "edifnetlistflattener.h"
#include "edifnetlistflattenerDlg.h"
extern "C"{
#include "edifheader.h"
};

#include <iosfwd>
#include <fstream>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CedifnetlistflattenerDlg dialog




CedifnetlistflattenerDlg::CedifnetlistflattenerDlg(CWnd* pParent /*=NULL*/)
: CDialog(CedifnetlistflattenerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CedifnetlistflattenerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INFILE, m_infile);
	DDX_Control(pDX, IDC_EDIT_OUTFILE, m_outfile);
	DDX_Control(pDX, IDC_STATIC_TIPS, m_tips);
}

BEGIN_MESSAGE_MAP(CedifnetlistflattenerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_INFILE, &CedifnetlistflattenerDlg::OnBnClickedButtonInfile)
	ON_BN_CLICKED(IDC_BUTTON_OUTFILE, &CedifnetlistflattenerDlg::OnBnClickedButtonOutfile)
	ON_BN_CLICKED(IDOK, &CedifnetlistflattenerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CedifnetlistflattenerDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CedifnetlistflattenerDlg message handlers

BOOL CedifnetlistflattenerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_conf = "./conf.ini";

	std::ifstream conf;
	conf.open(m_conf.GetBuffer());
	int count = conf.gcount(); 
	char * filepath = (char *)malloc(count + 1);
	filepath[count] = 0;
	conf.read(filepath, count);
	m_strfilepath = filepath;
	free(filepath);
	conf.close();
	m_conf.ReleaseBuffer();

    char directory[256] = {0};
	CString logdir;
	::GetCurrentDirectory(255, directory);
	logdir = directory;
	m_logfilename = logdir+"\\netlistflattener.log";	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CedifnetlistflattenerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CedifnetlistflattenerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CedifnetlistflattenerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CedifnetlistflattenerDlg::OnBnClickedButtonInfile()
{
	// TODO: Add your control notification handler code here
	TCHAR szFilters[]= _T("EDF Files (*.edn)|*edn|EDN Files (*.edf)|*.edf|All Files (*.*)|*.*||");
	CFileDialog fileDlg(TRUE, _T("Open EDIF File"), _T("*.edn"),
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters);
	///	if(m_strfilepath.IsEmpty()){
	///		fileDlg.GetOFN().lpstrInitialDir = "c:";
	///	}else{
	///		fileDlg.GetOFN().lpstrInitialDir = m_strfilepath;
	///	}
	char * buffer = (char *)malloc(m_strfilepath.GetLength()+1);
	memset(buffer, 0, m_strfilepath.GetLength() + 1);
	int count = m_strfilepath.ReverseFind('\\');
	if(count != -1){
		memcpy(buffer, m_strfilepath.GetBuffer(), count);
		fileDlg.GetOFN().lpstrInitialDir = buffer;
	}

	fileDlg.GetOFN().lpstrTitle = "open netlist file";
	if(fileDlg.DoModal() == IDOK)
	{
		m_strInFileName = fileDlg.GetPathName();
		///		CString fileName = fileDlg.GetFileTitle();
		///		SetWindowText(fileName);
		m_infile.SetWindowText(m_strInFileName);
		m_strfilepath = m_strInFileName;
	}
	free(buffer);
}

void CedifnetlistflattenerDlg::OnBnClickedButtonOutfile()
{
	// TODO: Add your control notification handler code here
	TCHAR szFilters[]= _T("EDF Files (*.edf)|*edf|EDN Files (*.edn)|*.edn|All Files (*.*)|*.*||");
	CFileDialog fileDlg(FALSE, _T(""), _T("*.edf"),
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters);
	fileDlg.GetOFN().lpstrTitle = "save netlist file";
	if(fileDlg.DoModal() == IDOK)
	{
		m_strOutFileName = fileDlg.GetPathName();
		///		CString fileName = fileDlg.GetFileTitle();
		///		SetWindowText(fileName);
		m_outfile.SetWindowText(m_strOutFileName);
	}
}

void CedifnetlistflattenerDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	// OnOK();
	m_outfile.GetWindowText(m_strOutFileName);
	m_infile.GetWindowText(m_strInFileName);
	int result = ParseEDIF(m_strInFileName.GetBuffer(), m_logfilename.GetBuffer(), m_strOutFileName.GetBuffer());
	time_t rawtime;
	struct tm * timeinfo;
	char comment[256] = {0};
	char * curtime = NULL;
	char sztime[128] = {0};

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	curtime = asctime(timeinfo);
	memcpy(sztime, curtime, strlen(curtime) - 1);
	CString strtips;
	if(result == 0){ 
		if(IsLogicalerror() == 1){ 
			strtips.Format("[%s] Parse successfully. but some nets are not correct.see log file for details", sztime);
		}else{
			strtips.Format("[%s] Parse successfully", sztime);
		}
	}else{
		strtips.Format("[%s] Parse error, The input file have syntax error. see log file for details", sztime);
	}
	m_tips.SetWindowText(strtips);
	CloseEDIF();
	m_strfilepath = m_strInFileName;
}


void CedifnetlistflattenerDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码

	if(!m_strfilepath.IsEmpty()){
		std::ofstream conf;
		conf.open(m_conf.GetBuffer(), std::ios_base::trunc | std::ios_base::out);
		conf.write(m_strfilepath.GetBuffer(), m_strfilepath.GetLength());
		conf.flush();
		conf.close();
		m_strfilepath.ReleaseBuffer();
		m_conf.ReleaseBuffer();
	}
	OnCancel();
}
