
// masterDlg.h : 头文件
//

#pragma once

#include <string>
//opencv 库
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include "afxwin.h"

// CmasterDlg 对话框
class CmasterDlg : public CDialogEx
{
// 构造
public:
	CmasterDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MASTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	int Mat2CImage(cv::Mat &mat, CImage &cImage);
	void DisplayImage(HDC hDC, CRect rc, const CImage& image);
	void ReSize();

	POINT old;
	DWORD m_IP;
	int m_Port;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CEdit putkeyname;
};

DWORD WINAPI RealProc(LPVOID lpParameter);

DWORD WINAPI EyeProc(LPVOID lpParameter);

DWORD WINAPI PptProc(LPVOID lpParameter);

DWORD WINAPI KeyProc(LPVOID lpParameter);

struct ThreadParameter{

	CmasterDlg *ceye;
	std::string Realip;
	std::string Eyeip;
	std::string Pptip;
	std::string Keyip;

};