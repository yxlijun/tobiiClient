
// masterDlg.cpp : ʵ���ļ�
//
static char THIS_FILE[] = __FILE__;
#undef THIS_FILE
#include "stdafx.h"
#include "master.h"
#include "masterDlg.h"
#include  "resource.h"
#include "afxdialogex.h"
#include "CvvImage.h"
#include <iostream>
#include <string>
#include <zmq.h>
#include <zhelpers.h>
#include<math.h>//��ѧ�������ͷ�ļ�
#include<Vfw.h>//����AVI��Ƶ��������ͷ�ļ�
#pragma comment(lib,"Vfw32.lib")//���ӿ�Vfw32.lib
#pragma comment(lib,"Winmm.lib")//���ӿ�
//����AVI��Ƶ����������Ľṹ�����
using namespace std;
HRESULT hr;
CImage eyeCimg;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif
HANDLE hThread_2;
HANDLE hThread_3;
HANDLE hThread_4;
HANDLE hThread_5;
HANDLE hThread_6;
using namespace std;

#define realsize 816000
#define mathight    340
#define matwidth    600

#define pptsize  3888000
//#define ppthight 1080
//#define pptwidth 1920
int pptheight = 1080;
int pptwidth = 1920;

//#define pptsize2  612000
//#define ppthight2 340
//#define pptwidth2 600

#define pptsize2  6220800
#define ppthight2 1080
#define pptwidth2 1920

const int MouseMoveMessege = 512;
const int MouseLeftDownMessege = 513;
const int MouseLeftUpMessege = 514;
const int MouseRightDownMessege = 516;
const int MouseRightUpMessege = 517;
const int KeyDownMessege = 256;

//����ͼƬ
cv::Mat eyeImage;

//�����̵߳Ĳ����ṹ��
ThreadParameter tp;

//����
CCriticalSection m_crit;

//����ͷ
void drawArrow(cv::Mat& img, cv::Point pStart, cv::Point pEnd, int len, int alpha,
	cv::Scalar& color, int thickness, int lineType)
{
	const double PI = 3.1415926;
	cv::Point arrow;
	//���� �� �ǣ���򵥵�һ�����������ͼʾ���Ѿ�չʾ���ؼ����� atan2 ��������������棩   
	double angle = atan2((double)(pStart.y - pEnd.y), (double)(pStart.x - pEnd.x));
	line(img, pStart, pEnd, color, thickness, lineType);
	//������Ǳߵ���һ�˵Ķ˵�λ�ã�����Ļ��������Ҫ����ͷ��ָ��Ҳ����pStart��pEnd��λ�ã� 

	//����ͷ�����м䣬ȡstart��end��ֵ
	cv::Point center((pStart.x + pEnd.x) / 2, (pStart.y + pEnd.y) / 2);

	arrow.x = center.x + len * cos(angle + PI * alpha / 180);
	arrow.y = center.y + len * sin(angle + PI * alpha / 180);
	line(img, center, arrow, color, thickness, lineType);
	arrow.x = center.x + len * cos(angle - PI * alpha / 180);
	arrow.y = center.y + len * sin(angle - PI * alpha / 180);
	line(img, center, arrow, color, thickness, lineType);

}

//�����ΰ�͸����
void DrawTransRec(IplImage* img, int x, int y, int width, int height, CvScalar color, double alpha)
{
	IplImage * rec;
	try
	{
		TRACE("begin cvCreateImage\r\n");  //��ӡ���൱��print
		rec = cvCreateImage(cvSize(width, height), img->depth, img->nChannels);
		TRACE("begin cvRectangle\r\n");
		cvRectangle(rec, cvPoint(0, 0), cvPoint(width, height), color, -1);
		TRACE("begin cvSetImageROI\r\n");
		cvSetImageROI(img, cvRect(x, y, width, height));
		TRACE("begin cvAddWeighted\r\n");
		//ʹ��addWeighted����ʱ������ͼ��ߴ�Ҫһ�£�����ᱨ�ڴ����
		cvAddWeighted(img, alpha, rec, 1 - alpha, 0.0, img);
		TRACE("begin cvResetImageROI\r\n");
		cvResetImageROI(img);
	}
	catch (...)
	{
		return;
	}

}


//�߳����

//����ѧ��ʵʱ�������ݣ�չʾ

DWORD WINAPI EyeProc(LPVOID lpParameter)
{
	CmasterDlg *pDlg = (CmasterDlg*)(AfxGetApp()->GetMainWnd());
	CRect rect1;
	pDlg->GetDlgItem(IDC_STATIC_PPT)->GetWindowRect(&rect1);//��ȡ�ؼ��������Ļ��λ��
	pDlg->ScreenToClient(rect1);//ת��Ϊ�Ի����ϵ����λ��

	CClientDC dc(pDlg);
	dc.SelectStockObject(NULL_BRUSH);
	CPen pen(PS_SOLID, 1, RGB(255, 0, 0));
	DeleteObject(dc.SelectObject(pen));
	//dc.SetROP2(R2_XORPEN);
	float rwidth = (float)rect1.Width();
	float rheight = (float)rect1.Height();
	float xscale = 1440 / rwidth;
	float yscale = 900 / rheight;
	ThreadParameter *tp = static_cast<ThreadParameter *>(lpParameter);
	CmasterDlg *pWnd = tp->ceye;
	string serveripport = tp->Eyeip;

	void *context = zmq_init(1);
	void *subscriber = zmq_socket(context, ZMQ_SUB);
	zmq_connect(subscriber, serveripport.c_str());

	//  ���ö�����Ϣ��Ĭ��Ϊ���ַ���������ȫ����Ϣ��������
	string empty;
	zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, empty.c_str(), empty.length());

	CSingleLock lock(&m_crit);
	int innum = 0;
	vector<int> inx;
	vector<int> iny;
	vector<pair<int, int>> inxy;
	while (true)
	{
		int gx = 0, gy = 0;

		char *re = s_recv(subscriber);

		sscanf(re, "%d %d", &gx, &gy);
		//TRACE("gx: %d\n", gx);
		free(re);

		inx.push_back(gx);
		iny.push_back(gy);


		//���Եĸ�tobii����ƽ����ȡn����ľ�ֵ
		innum++;
		if (innum == 10)
		{

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
			int gx1 = (int)gx / xscale;
			int gy1 = (int)gy / yscale;

			//TRACE("gx: %d,%d\n", gx,gy);
			//��Ļ���ֻ����m���㣬���һ��ʱ��ǰͼ�������µ�tobii���ߡ�
			if (inxy.size() == pWnd->m_PointNum)
			{
				inxy.erase(inxy.begin());
			}

			inxy.push_back(make_pair(gx, gy));

			//���pic�豸�����
			CDC* pEyeDC = pWnd->GetDlgItem(IDC_STATIC_PPT)->GetDC();
			//����豸���
			HDC	 hEyeDC = pEyeDC->GetSafeHdc();

			//���pic Rect
			CRect Eyerc;
			pWnd->GetDlgItem(IDC_STATIC_PPT)->GetWindowRect(&Eyerc);

			CImage eyeCimg;
			cv::Mat tempimg;

			//��Ҫ���ʵ�ԭʼ��Ļͼ�������һ��socket�������ݺ��дMat���������������ٽ���
			lock.Lock();
			if (lock.IsLocked())
			{

				eyeImage.copyTo(tempimg);
				lock.Unlock();
			}


			//���۾���
			for (size_t i = 0; i < inxy.size(); i++)
			{
				//����
				cv::Point eyeAttentionTmp(inxy[i].first, inxy[i].second);
				circle(tempimg, eyeAttentionTmp, 40, cv::Scalar(190, 190, 190), 2);
				//������
				//DrawTransRec(&(IplImage)(tempimg), eyeAttentionTmp.x - 20, eyeAttentionTmp.y - 20, 40, 40, cv::Scalar(255, 0, 0), 0.85);
				//DrawTransRec((tempimg), eyeAttentionTmp.x - 20, eyeAttentionTmp.y - 20, 40, 40, cv::Scalar(255, 0, 0), 0.85);
				//DrawTransRec(pImage, 150, 150, 150, 150, CV_RGB(255, 0, 0), 0.5);
			}

			//�������֮��ֱ��
			for (size_t i = 0; i < inxy.size() - 1; i++)
			{
				cv::Point pt1(inxy[i].first, inxy[i].second);
				cv::Point pt2(inxy[i + 1].first, inxy[i + 1].second);

				drawArrow(tempimg, pt1, pt2, 17, 35, cv::Scalar(255, 0, 0), 2, 8);
				/*cv::line(tempimg, pt1, pt2, cv::Scalar(255, 0, 0), 10);
				arrowedLine(tempimg, pt1, pt2, cv::Scalar(255, 0, 0), 10);*/

			}


			pWnd->Mat2CImage(tempimg, eyeCimg);
			if (!eyeCimg.IsNull())
			{
				//Mat�����pic
				pWnd->DisplayImage(hEyeDC, Eyerc, eyeCimg);
			}

			//�ͷ�Mat���豸�����
			eyeCimg.Destroy();
			pWnd->ReleaseDC(pEyeDC);

		}

	}

	zmq_close(subscriber);
	zmq_term(context);

}

DWORD WINAPI PptProc(LPVOID lpParameter)
{
	CmasterDlg *pDlg = (CmasterDlg*)(AfxGetApp()->GetMainWnd());
	CRect rect1;
	pDlg->GetDlgItem(IDC_STATIC_PPT)->GetWindowRect(&rect1);//��ȡ�ؼ��������Ļ��λ��
	pDlg->ScreenToClient(rect1);//ת��Ϊ�Ի����ϵ����λ��

	CClientDC dc(pDlg);
	dc.SelectStockObject(NULL_BRUSH);
	CPen pen(PS_SOLID, 1, RGB(255, 0, 0));
	DeleteObject(dc.SelectObject(pen));
	//dc.SetROP2(R2_XORPEN);

	ThreadParameter *tp = static_cast<ThreadParameter *>(lpParameter);
	CmasterDlg *pWnd = tp->ceye;
	string serveripport = tp->Pptip;

	void *context = zmq_init(1);
	void *subscriber = zmq_socket(context, ZMQ_SUB);
	zmq_connect(subscriber, serveripport.c_str());

	//  ���ö�����Ϣ��Ĭ��Ϊ���ַ���������ȫ����Ϣ��������
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
		//TRACE("%d", len);
		lock.Lock();

		if (lock.IsLocked())
		{
			//data_mat.copyTo(eyeImage);
			cv::resize(data_mat, eyeImage, cv::Size(pptwidth,pptheight));
			lock.Unlock();
		}
	}

	zmq_close(subscriber);
	zmq_term(context);

	return 0;
}

//DWORD WINAPI KeyProc(LPVOID lpParameter)
//{
//	ThreadParameter *tp = static_cast<ThreadParameter *>(lpParameter);
//	CmasterDlg *pWnd = tp->ceye;
//	string serveripport = tp->Keyip;
//
//	void *context = zmq_init(1);
//	void *subscriber = zmq_socket(context, ZMQ_SUB);
//	zmq_connect(subscriber, serveripport.c_str());
//
//	//  ���ö�����Ϣ��Ĭ��Ϊ���ַ���������ȫ����Ϣ��������
//	string empty;
//	zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, empty.c_str(), empty.length());
//	//int sndhwm = 0;
//	//zmq_setsockopt(subscriber, ZMQ_RCVHWM, &sndhwm, sizeof(int));
//
//	while (true)
//	{
//		char str[20];
//		int key1 = 0;
//		int key2 = 0;
//		char *re = s_recv(subscriber);
//
//		sscanf(re, "%d %d %s", &key1, &key2, &str);
//		CString keyname(str);
//		if (key1 == KeyDownMessege)
//		{
//			pWnd->putkeyname.SetWindowTextW(keyname);
//			//pWnd->UpdateData(FALSE);
//			TRACE("%s\r\n", str);
//		}
//
//		free(re);
//	}
//
//	zmq_close(subscriber);
//	zmq_term(context);
//
//	return 0;
//}

DWORD WINAPI ScreenSizeProc(LPVOID lpParameter)
{
	ThreadParameter *tp = static_cast<ThreadParameter *>(lpParameter);
	CmasterDlg *pWnd = tp->ceye;
	string serveripport = tp->Keyip;

	void *context = zmq_init(1);
	void *subscriber = zmq_socket(context, ZMQ_SUB);
	zmq_connect(subscriber, serveripport.c_str());

	//  ���ö�����Ϣ��Ĭ��Ϊ���ַ���������ȫ����Ϣ��������
	string empty;
	zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, empty.c_str(), empty.length());
	//int sndhwm = 0;
	//zmq_setsockopt(subscriber, ZMQ_RCVHWM, &sndhwm, sizeof(int));

	while (true)
	{
		int Width = 0;
		int Height = 0;
		char *re = s_recv(subscriber);

		sscanf(re, "%d %d", &Width, &Height);
		
		TRACE("%d%d", Width, Height);

		pptwidth = Width;
		pptheight = Height;
		free(re);
	}

	zmq_close(subscriber);
	zmq_term(context);

	return 0;
}



// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	// ʵ��
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


// CmasterDlg �Ի���

CmasterDlg::CmasterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CmasterDlg::IDD, pParent)
	, m_IP(2130706433)
	, m_Port(5556)
	, m_PointNum(3)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CmasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS1, m_IP);
	//DDX_Text(pDX, IDC_EDIT1, m_Port);
	DDX_Control(pDX, IDC_EDIT2, putkeyname);
	DDX_Text(pDX, IDC_EDIT1, m_PointNum);
	DDV_MinMaxInt(pDX, m_PointNum, 1, 10000);
}

BEGIN_MESSAGE_MAP(CmasterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CmasterDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CmasterDlg::OnBnClickedCancel)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CmasterDlg ��Ϣ�������

BOOL CmasterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������

	//���ԭʼ�Ի����С
	CRect rect;
	GetClientRect(&rect);     //ȡ�ͻ�����С    
	old.x = rect.right - rect.left;
	old.y = rect.bottom - rect.top;

	//����tobii��Ļimg
	cv::Mat mat(900, 1440, CV_8UC3);

	//cv::imshow("s", mat);
	mat.copyTo(eyeImage);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CmasterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CClientDC dc(this);
		dc.SetROP2(R2_NOTXORPEN);
		dc.SelectStockObject(NULL_BRUSH);
		dc.Rectangle(CRect(regionLeftTopTemp, regionRightBottomTemp));
		CDialogEx::OnPaint();
	}

}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CmasterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CmasterDlg::OnBnClickedOk()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
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
	string Screenport = std::to_string(m_Port + 3);

	//string Screenport = std::to_string(m_Port + 3);
	//string serveripRealprot = "tcp://" + sIPa + "." + sIPb + "." + sIPc + "." + sIPd + ":" + Realport;
	string serveripEyeprot = "tcp://" + sIPa + "." + sIPb + "." + sIPc + "." + sIPd + ":" + Eyeport;
	string serveripPptprot = "tcp://" + sIPa + "." + sIPb + "." + sIPc + "." + sIPd + ":" + Pptport;
	string serveripKeyprot = "tcp://" + sIPa + "." + sIPb + "." + sIPc + "." + sIPd +":"+ Screenport;
	//���Ի���Ķ���ָ��CeyeDlg��Ϊ����������߳�
	tp.ceye = this;

	//�����ﴴ��4���̣߳��ֱ����key, real��eye��ppt���ݣ�ע�����tp��Ա���ܹ���ֻ��һ��ip�Ǵ�ġ�
	//tp.Realip = serveripRealprot;
	//HANDLE hThread_1 = CreateThread(NULL, 0, RealProc, &tp, 0, NULL);

	tp.Eyeip = serveripEyeprot;
	hThread_2 = CreateThread(NULL, 0, EyeProc, &tp, 0, NULL);

	tp.Pptip = serveripPptprot;
	hThread_3 = CreateThread(NULL, 0, PptProc, &tp, 0, NULL);

	/*tp.Keyip = serveripKeyprot;
	hThread_4 = CreateThread(NULL, 0, KeyProc, &tp, 0, NULL);*/
	tp.Keyip = serveripKeyprot;
	hThread_4 = CreateThread(NULL, 0, ScreenSizeProc, &tp, 0, NULL);

	//GetDlgItem(IDOK)->EnableWindow(FALSE);

	//CDialogEx::OnOK();
}


void CmasterDlg::OnBnClickedCancel()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
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
	SetStretchBltMode(hDC, COLORONCOLOR);//����λͼ������ģʽ  
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
	Invalidate();
}

void CmasterDlg::ReSize()
{
	float fsp[2];
	POINT Newp; //��ȡ���ڶԻ���Ĵ�С  
	CRect recta;
	GetClientRect(&recta);     //ȡ�ͻ�����С    
	Newp.x = recta.right - recta.left;
	Newp.y = recta.bottom - recta.top;
	fsp[0] = (float)Newp.x / old.x;
	fsp[1] = (float)Newp.y / old.y;
	CRect Rect;
	int woc;
	CPoint OldTLPoint, TLPoint; //���Ͻ�  
	CPoint OldBRPoint, BRPoint; //���½�  
	HWND  hwndChild = ::GetWindow(m_hWnd, GW_CHILD);  //�г����пؼ�    
	while (hwndChild)
	{
		woc = ::GetDlgCtrlID(hwndChild);//ȡ��ID  
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



