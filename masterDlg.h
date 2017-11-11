
// masterDlg.h : ͷ�ļ�
//
#if !defined(AFX_PMLXDLG_H__1E3222DD_01BF_4358_81DE_CAD831CE39EB__INCLUDED_)
#define AFX_PMLXDLG_H__1E3222DD_01BF_4358_81DE_CAD831CE39EB__INCLUDED_

#if _MSC_VER > 1000
#endif
#pragma once

#include <string>
//opencv ��
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include "afxwin.h"
#include "afxcmn.h"

// CmasterDlg �Ի���
class CmasterDlg : public CDialogEx
{
	// ����
public:
	CmasterDlg(CWnd* pParent = NULL);	// ��׼���캯��

	// �Ի�������
	enum { IDD = IDD_MASTER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


	// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
	int m_PointNum;
	BOOL lButtonDownNotUp = FALSE;
	CPoint regionLeftTopTemp;
	CPoint regionRightBottomTemp;
};
#endif
DWORD WINAPI RealProc(LPVOID lpParameter);

DWORD WINAPI EyeProc(LPVOID lpParameter);

DWORD WINAPI PptProc(LPVOID lpParameter);

//DWORD WINAPI KeyProc(LPVOID lpParameter);

DWORD WINAPI ScreenSizeProc(LPVOID lpParameter);


struct ThreadParameter{

	CmasterDlg *ceye;
	std::string Realip;
	std::string Eyeip;
	std::string Pptip;
	std::string Keyip;
};