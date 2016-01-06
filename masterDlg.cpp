
// masterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "master.h"
#include "masterDlg.h"
#include "afxdialogex.h"

#include <iostream>
#include <string>
#include <zmq.h>
#include <zhelpers.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

#define realsize 816000
#define mathight    340
#define matwidth    600

#define pptsize  3888000
#define ppthight 900
#define pptwidth 1440

#define pptsize2  612000
#define ppthight2 340
#define pptwidth2 600

//视线图片
cv::Mat eyeImage;

//传入线程的参数结构体
ThreadParameter tp;

//加锁
CCriticalSection m_crit;

//线程入口

//接收学生实时画面数据，展示

DWORD WINAPI EyeProc(LPVOID lpParameter)
{

	ThreadParameter *tp = static_cast<ThreadParameter *>(lpParameter);
	CmasterDlg *pWnd = tp->ceye;
	string serveripport = tp->Eyeip;

	void *context = zmq_init(1);
	void *subscriber = zmq_socket(context, ZMQ_SUB);
	zmq_connect(subscriber, serveripport.c_str());

	//  设置订阅信息，默认为空字符串，接受全部消息，不过滤
	string empty;
	zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, empty.c_str(), empty.length());
	//int sndhwm = 0;
	//zmq_setsockopt(subscriber, ZMQ_RCVHWM, &sndhwm, sizeof(int));
	//cv::Mat i = cv::imread("D:\me.jpg");
	////cv::resize(i,eyeImage,cv::Size(1440,900));
	//i.copyTo(eyeImage);
	CSingleLock lock(&m_crit);
	int innum = 0;
	vector<int> inx(10);
	vector<int> iny(10);
	while (true)
	{
		int gx=0, gy=0;

		char *re = s_recv(subscriber);

		sscanf(re, "%d %d", &gx, &gy);
		TRACE("gx: %d\n", gx);
		free(re);

		inx.push_back(gx);
		iny.push_back(gy);

		innum++;
		TRACE("innum: %d\r\n", innum);

		if (innum == 10)
		{
			TRACE("innum == 10");
			innum = 0;

			int sumx = 0;
			int sumy = 0;
			for (size_t i = 0; i < 10; i++)
			{
				sumx += inx[i];
				sumy += iny[i];
			}

			inx.clear();
			iny.clear();

			gx = sumx / 10;
			gy = sumy / 10;

			//获得pic设备相关类
			CDC* pEyeDC = pWnd->GetDlgItem(IDC_STATIC_PPT)->GetDC();
			//获得设备句柄
			HDC	 hEyeDC = pEyeDC->GetSafeHdc();

			//获得pic Rect
			CRect Eyerc;
			pWnd->GetDlgItem(IDC_STATIC_PPT)->GetWindowRect(&Eyerc);

			//在Mat上画屏幕点
			//DrawAttentionPicture(eyeImage, pWnd->screendc.screenW, pWnd->screendc.screenH);
			cv::Point eyeAttentionTmp(gx, gy);

			CImage eyeCimg;

			lock.Lock();
			if (lock.IsLocked())
			{
				circle(eyeImage, eyeAttentionTmp, 20, cv::Scalar(255, 0, 0), 6);

				pWnd->Mat2CImage(eyeImage, eyeCimg);
				lock.Unlock();
			}

			if (!eyeCimg.IsNull())
			{
				//Mat输出到pic
				pWnd->DisplayImage(hEyeDC, Eyerc, eyeCimg);
			}

			//释放Mat和设备相关类
			eyeCimg.Destroy();
			pWnd->ReleaseDC(pEyeDC);
		}

	}

	zmq_close(subscriber);
	zmq_term(context);

}

DWORD WINAPI PptProc(LPVOID lpParameter)
{

	ThreadParameter *tp = static_cast<ThreadParameter *>(lpParameter);
	CmasterDlg *pWnd = tp->ceye;
	string serveripport = tp->Pptip;

	void *context = zmq_init(1);
	void *subscriber = zmq_socket(context, ZMQ_SUB);
	zmq_connect(subscriber, serveripport.c_str());

	//  设置订阅信息，默认为空字符串，接受全部消息，不过滤
	string empty;
	zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, empty.c_str(), empty.length());
	//int sndhwm = 0;
	//zmq_setsockopt(subscriber, ZMQ_RCVHWM, &sndhwm, sizeof(int));

	CSingleLock lock(&m_crit);
	while (true)
	{
		char *re = new char[pptsize2];
		int len = zmq_recv(subscriber, re, pptsize2, 0);
		//*(re + 3888000) = 0;

		cv::Mat data_mat = cv::Mat(ppthight2, pptwidth2, CV_8UC3, (void *)re);

		lock.Lock();

		if (lock.IsLocked())
		{
			//data_mat.copyTo(eyeImage);
			cv::resize(data_mat,eyeImage,cv::Size(1440,900));
			lock.Unlock();
		}
	}

	zmq_close(subscriber);
	zmq_term(context);

	return 0;
}

DWORD WINAPI KeyProc(LPVOID lpParameter)
{
	ThreadParameter *tp = static_cast<ThreadParameter *>(lpParameter);
	CmasterDlg *pWnd = tp->ceye;
	string serveripport = tp->Keyip;

	void *context = zmq_init(1);
	void *subscriber = zmq_socket(context, ZMQ_SUB);
	zmq_connect(subscriber, serveripport.c_str());

	//  设置订阅信息，默认为空字符串，接受全部消息，不过滤
	string empty;
	zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, empty.c_str(), empty.length());
	//int sndhwm = 0;
	//zmq_setsockopt(subscriber, ZMQ_RCVHWM, &sndhwm, sizeof(int));

	while (true)
	{
		char str[20];
		int key=0;
		char *re = s_recv(subscriber);

		sscanf(re, "%d %s", &key, &str);
		CString keyname(str);
		if (key == -1)
		{
			pWnd->putkeyname.SetWindowTextW(keyname);
			//pWnd->UpdateData(FALSE);
			TRACE("%s\r\n", str);
		}
		
		free(re);
	}

	zmq_close(subscriber);
	zmq_term(context);

	return 0;
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CmasterDlg 对话框

CmasterDlg::CmasterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CmasterDlg::IDD, pParent)
	, m_IP(3232252517)
	, m_Port(5556)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CmasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS1, m_IP);
	DDX_Text(pDX, IDC_EDIT1, m_Port);
	DDX_Control(pDX, IDC_EDIT2, putkeyname);
}

BEGIN_MESSAGE_MAP(CmasterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CmasterDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CmasterDlg::OnBnClickedCancel)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CmasterDlg 消息处理程序

BOOL CmasterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

	//获得原始对话框大小
	CRect rect;
	GetClientRect(&rect);     //取客户区大小    
	old.x = rect.right - rect.left;
	old.y = rect.bottom - rect.top;

	//创建tobii屏幕img
	cv::Mat mat(900, 1440, CV_8UC3);

	//cv::imshow("s", mat);
	mat.copyTo(eyeImage);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CmasterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CmasterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CmasterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CmasterDlg::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码

	UpdateData(TRUE);

	unsigned short usIPa, usIPb, usIPc, usIPd;
	usIPa = (m_IP & (0xff << 24)) >> 24;
	usIPb = (m_IP & (0xff << 16)) >> 16;
	usIPc = (m_IP & (0xff << 8)) >> 8;
	usIPd = m_IP & 0xff;

	//"tcp://localhost:5556"
	string sIPa = std::to_string(usIPa);
	string sIPb = std::to_string(usIPb);
	string sIPc = std::to_string(usIPc);
	string sIPd = std::to_string(usIPd);
	//string Realport = std::to_string(m_Port);
	string Eyeport = std::to_string(m_Port + 1);
	string Pptport = std::to_string(m_Port + 2);

	//string serveripRealprot = "tcp://" + sIPa + "." + sIPb + "." + sIPc + "." + sIPd + ":" + Realport;
	string serveripEyeprot = "tcp://" + sIPa + "." + sIPb + "." + sIPc + "." + sIPd + ":" + Eyeport;
	string serveripPptprot = "tcp://" + sIPa + "." + sIPb + "." + sIPc + "." + sIPd + ":" + Pptport;
	string serveripKeyprot = "tcp://" + sIPa + "." + sIPb + "." + sIPc + "." + sIPd + ":5555";

	//将对话框的对象指针CeyeDlg作为参数传入给线程
	tp.ceye = this;

	//在这里创建4个线程，分别接收key, real，eye和ppt数据，注意参数tp成员不能共享，只用一个ip是错的。
	//tp.Realip = serveripRealprot;
	//HANDLE hThread_1 = CreateThread(NULL, 0, RealProc, &tp, 0, NULL);

	tp.Eyeip = serveripEyeprot;
	HANDLE hThread_2 = CreateThread(NULL, 0, EyeProc, &tp, 0, NULL);

	tp.Pptip = serveripPptprot;
	HANDLE hThread_3 = CreateThread(NULL, 0, PptProc, &tp, 0, NULL);

	tp.Keyip = serveripKeyprot;
	HANDLE hThread_4 = CreateThread(NULL, 0, KeyProc, &tp, 0, NULL);

	GetDlgItem(IDOK)->EnableWindow(FALSE);

	//CDialogEx::OnOK();
}


void CmasterDlg::OnBnClickedCancel()
{
	// TODO:  在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}

int CmasterDlg::Mat2CImage(cv::Mat &mat, CImage &cImage)
{
	int width = mat.cols;
	int height = mat.rows;
	int channels = mat.channels();

	cImage.Destroy();
	cImage.Create(width,
		height,
		8 * channels);

	uchar* ps;
	uchar* pimg = (uchar*)cImage.GetBits();
	int step = cImage.GetPitch();

	for (int i = 0; i < height; ++i)
	{
		ps = (mat.ptr<uchar>(i));
		for (int j = 0; j < width; ++j)
		{
			if (channels == 1) //gray  
			{
				*(pimg + i*step + j) = ps[j];
			}
			else if (channels == 3) //color  
			{
				for (int k = 0; k < 3; ++k)
				{
					*(pimg + i*step + j * 3 + k) = ps[j * 3 + k];
				}
			}
			else if (4 == channels)
			{
				*(pimg + i*step + j * 4) = ps[j * 4];
				*(pimg + i*step + j * 4 + 1) = ps[j * 4 + 1];
				*(pimg + i*step + j * 4 + 2) = ps[j * 4 + 2];
				*(pimg + i*step + j * 4 + 3) = ps[j * 4 + 3];
			}
		}
	}

	return 0;
}

void CmasterDlg::DisplayImage(HDC hDC, CRect rc, const CImage& image)
{
	int nwidth = rc.Width();
	int nheight = rc.Height();
	SetStretchBltMode(hDC, COLORONCOLOR);//设置位图的伸缩模式  
	image.StretchBlt(hDC, 0, 0, nwidth, nheight, 0, 0, image.GetWidth(), image.GetHeight(), SRCCOPY);
}


void CmasterDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here  
	if (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED)
	{
		ReSize();
	}
}

void CmasterDlg::ReSize()
{
	float fsp[2];
	POINT Newp; //获取现在对话框的大小  
	CRect recta;
	GetClientRect(&recta);     //取客户区大小    
	Newp.x = recta.right - recta.left;
	Newp.y = recta.bottom - recta.top;
	fsp[0] = (float)Newp.x / old.x;
	fsp[1] = (float)Newp.y / old.y;
	CRect Rect;
	int woc;
	CPoint OldTLPoint, TLPoint; //左上角  
	CPoint OldBRPoint, BRPoint; //右下角  
	HWND  hwndChild = ::GetWindow(m_hWnd, GW_CHILD);  //列出所有控件    
	while (hwndChild)
	{
		woc = ::GetDlgCtrlID(hwndChild);//取得ID  
		GetDlgItem(woc)->GetWindowRect(Rect);
		ScreenToClient(Rect);
		OldTLPoint = Rect.TopLeft();
		TLPoint.x = long(OldTLPoint.x*fsp[0]);
		TLPoint.y = long(OldTLPoint.y*fsp[1]);
		OldBRPoint = Rect.BottomRight();
		BRPoint.x = long(OldBRPoint.x *fsp[0]);
		BRPoint.y = long(OldBRPoint.y *fsp[1]);
		Rect.SetRect(TLPoint, BRPoint);
		GetDlgItem(woc)->MoveWindow(Rect, TRUE);
		hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
	}
	old = Newp;
}
