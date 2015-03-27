﻿
// MFCApplication1Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
#include "cvheader.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
cv::CascadeClassifier face_cascade;
cv::CascadeClassifier fist_cascade;
CvCapture* capture;
CRect rect;
IplImage* mybackground=NULL;
CDC *pDC;
HDC hDC;
CWnd *pwnd;
int gamestartcounting = 0;
int computergesture = 0;
int usergesture = -1;//-1-undetected
// CMFCApplication1Dlg dialog
#ifndef HISTORYQUEUE
#define HISTORYQUEUE
class historyqueue //To avoid error
{
public:
	historyqueue()
	{
		int i;
		for (i = 0; i < 5; i++) this->p[i] = -1;
		this->current = 0;
	}
	void push(int x)
	{
		this->p[this->current] = x;
		this->current++;
		this->current = this->current % 5;
	}
	int query()
	{
		int j;
		int q[3];
		for (j = 0; j < 3; j++) q[j] = 0;
		for (j = 0; j < 5; j++) if (this->p[j] != -1) q[this->p[j]]++;
		for (j = 0; j < 3; j++) if (q[j] >= 3) return j;
		return -1;
	}
private:
	int p[5];
	int current;
};
#endif
historyqueue* history = new historyqueue();
CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMFCApplication1Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication1Dlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMFCApplication1Dlg::OnBnClickedButton2)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BUTTON3, &CMFCApplication1Dlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CMFCApplication1Dlg message handlers

BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	pwnd = GetDlgItem(IDC_STATIC);
	//pwnd->MoveWindow(35,30,352,288);  
	pDC = pwnd->GetDC();
	//pDC =GetDC();  
	hDC = pDC->GetSafeHdc();
	pwnd->GetClientRect(&rect);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFCApplication1Dlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




void CMFCApplication1Dlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == 1){
		IplImage* myframe;
		myframe = cvQueryFrame(capture);
		cv::Mat m = myframe;
		CvvImage m_CvvImage;
		m_CvvImage.CopyOf(myframe, 1);
		m_CvvImage.DrawToHDC(hDC, &rect);
		m_CvvImage.Destroy();
		usergesture = mygesturedetect(m);
		history->push(usergesture);
		switch (history->query())
		{
		case -1: GetDlgItem(IDC_STATICUSER)->SetWindowTextW(_T("You : Gesture Not Detected")); break;
		case 0: GetDlgItem(IDC_STATICUSER)->SetWindowTextW(_T("You : Scissor")); break;
		case 1: GetDlgItem(IDC_STATICUSER)->SetWindowTextW(_T("You : Rock")); break;
		case 2: GetDlgItem(IDC_STATICUSER)->SetWindowTextW(_T("You : Paper")); break;
		}
		
	}
	if (nIDEvent == 2){
		TCHAR buf[50];
		_itow_s(gamestartcounting--, buf,10);
		GetDlgItem(IDC_STATICCOUNT)->SetWindowTextW(buf);
		if (gamestartcounting < 0)
		{
			int u = history->query();
			switch (computergesture)
			{
			case 0: GetDlgItem(IDC_STATICCOUNT)->SetWindowTextW(_T("Computer: Scissor")); break;
			case 1: GetDlgItem(IDC_STATICCOUNT)->SetWindowTextW(_T("Computer: Rock")); break;
			case 2: GetDlgItem(IDC_STATICCOUNT)->SetWindowTextW(_T("Computer: Paper")); break;
			}
			if (u == computergesture)
			{

				GetDlgItem(IDC_STATICRESULT)->SetWindowTextW(_T("Tie"));
			}
			else if (u - computergesture == 1 || (u == 0 && computergesture == 2))//user win
			{
				CString tmp;
				GetDlgItem(IDC_EDIT1)->GetWindowText(tmp);
				_itow_s(_tstoi(tmp) + 1, buf, 30);
				GetDlgItem(IDC_EDIT1)->SetWindowTextW(buf);

				GetDlgItem(IDC_STATICRESULT)->SetWindowTextW(_T("You Win"));
			}
			else
			{
				CString tmp;
				GetDlgItem(IDC_EDIT2)->GetWindowText(tmp);
				_itow_s(_tstoi(tmp) + 1, buf, 30);
				GetDlgItem(IDC_EDIT2)->SetWindowTextW(buf);
				GetDlgItem(IDC_STATICRESULT)->SetWindowTextW(_T("You Lose"));
			}
			CString tmp,tmp2;
			switch (u)
			{
			case -1:tmp = "Undetected"; break;
			case 0:tmp = "Scissor"; break;
			case 1:tmp = "Rock"; break;
			case 2:tmp = "Paper"; break;
			}
			GetDlgItem(IDC_STATICRESULT)->GetWindowText(tmp2);
			GetDlgItem(IDC_STATICRESULT)->SetWindowText(tmp2+"  (You:"+tmp+")");
			KillTimer(2);
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CMFCApplication1Dlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	KillTimer(2);
	gamestartcounting = 3;
	srand((unsigned)time(NULL));
	computergesture = rand() % 3; //0-scissor, 1-rock, 2-paper
	SetTimer(2, 1000, NULL);
	char buf[30];
	_itoa_s(gamestartcounting--, buf, 2,10);
	GetDlgItem(IDC_STATICCOUNT)->SetWindowText(CString(buf));

}


void CMFCApplication1Dlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_EDIT1)->SetWindowTextW(_T("0"));
	GetDlgItem(IDC_EDIT2)->SetWindowTextW(_T("0"));
	
}


int CMFCApplication1Dlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	if (!capture)
	{
		capture = cvCaptureFromCAM(0);
		  
	}

	if (!capture)
	{
		MessageBox(_T("FAIL TO START CAMERA"));
		return 0;
	}
	if (!face_cascade.load("D:\haarcascade_frontalface_alt.xml")){ MessageBox(_T("--(!)Error loading haarcascade_frontalface_alt.xml\n")); };
	//if (!fist_cascade.load("fist.xml")){ MessageBox(_T("--(!)Error loading fist.xml\n")); };
	IplImage* m_Frame;
	m_Frame = cvQueryFrame(capture);
	CvvImage m_CvvImage;
	m_CvvImage.CopyOf(m_Frame, 1);
	if (true)
	{
		m_CvvImage.DrawToHDC(hDC, &rect);
		//cvWaitKey(10);  
	}

	SetTimer(1, 11, NULL);
	return 0;
}


void CMFCApplication1Dlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	cvReleaseCapture(&capture);
}


void CMFCApplication1Dlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	GetDlgItem(IDC_EDIT1)->SetWindowTextW(_T("0"));
	GetDlgItem(IDC_EDIT2)->SetWindowTextW(_T("0"));
	CFont *m_Font1 = new CFont;
	m_Font1->CreatePointFont(160, _T("Arial Bold"));
	CStatic * m_Label = (CStatic *)GetDlgItem(IDC_STATICCOUNT);
	m_Label->SetFont(m_Font1);
}


void CMFCApplication1Dlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	if (mybackground) cvReleaseImage(&mybackground);
	mybackground = cvCloneImage(cvQueryFrame(capture));

}
