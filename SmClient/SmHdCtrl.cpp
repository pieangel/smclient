// VtHdCtrl.cpp : implementation file
//

#include "pch.h"
#include "SmMongoDBManager.h"
#include "SmClient.h"
#include "SmHdCtrl.h"
#include "afxdialogex.h"
#include "Log/loguru.hpp"
#include "Util/VtStringUtil.h"
#include "SmHdClient.h"
#include <ctime>
#include "SmSymbol.h"
#include "SmSymbolManager.h"
#include "SmTimeSeriesDBManager.h"
#include "SmTimeSeriesCollector.h"
#include "Json/json.hpp"
#include "SmQuoteDefine.h"
#include "SmHogaDefine.h"
#include "SmChartDataManager.h"
#include "SmChartData.h"
#include "SmTimeSeriesServiceManager.h"
#include "SmSessionManager.h"
#include <thread>
#include <algorithm>
#include <numeric>
#include <future>
#include <string>
#include <mutex>
#include "SmMarketManager.h"
#include "SmSymbolReader.h"

using namespace nlohmann;
// VtHdCtrl dialog

IMPLEMENT_DYNAMIC(SmHdCtrl, CDialogEx)

SmHdCtrl::SmHdCtrl(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HD_CTRL, pParent)
{

}

SmHdCtrl::~SmHdCtrl()
{
	if (m_CommAgent.GetSafeHwnd()) {
		if (m_CommAgent.CommGetConnectState() == 1) {
			if (m_sUserId != "")
				m_CommAgent.CommLogout(m_sUserId);
		}

		m_CommAgent.CommTerminate(TRUE);
		m_CommAgent.DestroyWindow();
	}
}

void SmHdCtrl::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

int SmHdCtrl::Init()
{
	if (!m_CommAgent.GetSafeHwnd()) {
		m_CommAgent.Create("HDF CommAgent", WS_CHILD, CRect(0, 0, 0, 0), this, 2286);
	}
	else
		return -1000;

	if (m_CommAgent.GetSafeHwnd()) {
		int nRet = m_CommAgent.CommInit(1);

		if (nRet < 0) {
			//AfxMessageBox("������α׷� ���� ����");
			LOG_F(INFO, _T("������α׷� ���� ����"));
		}
		else {
			//AfxMessageBox("������α׷� ���� ����");
			LOG_F(INFO, _T("������α׷� ���� ����"));
		}

		return nRet;
	}

	return -1000;
}

int SmHdCtrl::LogIn(CString id, CString pwd, CString cert)
{
	m_sUserId = id;
	m_sUserPw = pwd;
	m_sAuthPw = cert;
	const char* pLoginSuccess[] = { "�α��� ����"	, "Login Successful" };
	const char* pLoginFail[] = { "�α��� ����"	, "Login Failure" };

	int nRet = m_CommAgent.CommLogin(m_sUserId, m_sUserPw, m_sAuthPw);
	if (nRet > 0) {
		//AfxMessageBox(pLoginSuccess[0]);
		CString sTrCode = "v90001";
		CString sInput = "wopcode.cod";
		CString strNextKey = "";
		//int nRqID = m_CommAgent.CommRqData(sTrCode, sInput, sInput.GetLength(), strNextKey);

		//�α��� �ݵ�� ȣ��...
		m_CommAgent.CommAccInfo();
		LOG_F(INFO, pLoginSuccess[0]);
	}
	else
	{
		CString strRet;
		strRet.Format("[%d]", nRet);
		//AfxMessageBox(pLoginFail[0] + strRet);
		LOG_F(INFO, _T("%s"), strRet);
	}

	return nRet;
}

void SmHdCtrl::LogIn()
{
	int nRet = m_CommAgent.CommLogin(m_sUserId, m_sUserPw, m_sAuthPw);
	if (nRet > 0)
	{
		//AfxMessageBox("�α��� ����");

		// �α��� �ݵ�� ȣ��...
		m_CommAgent.CommAccInfo();
		//m_CommAgent.CommReqMakeCod("all", 0);

		//GetAccountInfo();
	}
	else
	{
		CString strRet;
		strRet.Format("[%d]", nRet);
		AfxMessageBox("�α��� ����" + strRet);
	}
}

int SmHdCtrl::LogOut(CString id)
{
	return m_CommAgent.CommLogout(id);
}

void SmHdCtrl::LogOut()
{
	// �α׾ƿ��Ѵ�.
	int nRet = m_CommAgent.CommLogout(m_sUserId);

	CString strRet;
	strRet.Format("[%d]", nRet);
	if (nRet < 0) {
		AfxMessageBox("�α׾ƿ� ����" + strRet);
	}
	else {
		AfxMessageBox("�α׾ƿ� ����" + strRet);
	}
}

void SmHdCtrl::RegisterProduct(std::string symCode)
{
	if (std::isdigit(symCode.at(2))) { // ��������
		int nRealType = 0;
		int nResult = 0;
		CString strKey = symCode.c_str();
		TCHAR first = symCode.at(0);
		CString prefix = symCode.substr(0,3).c_str();
		if (first == '1' || first == '4') {
			TCHAR second = symCode.at(1);
			if (second == '0') {
				nRealType = 51; // �ü�
				nResult = m_CommAgent.CommSetBroad(strKey, nRealType);
				nRealType = 65; // ȣ��
				nResult = m_CommAgent.CommSetBroad(strKey, nRealType);
				nRealType = 310; // ���� ü�ᰡ
				nResult = m_CommAgent.CommSetBroad(strKey, nRealType);
			}
			else {
				nRealType = 58; // �ü�
				nResult = m_CommAgent.CommSetBroad(strKey, nRealType);
				nRealType = 71; // ȣ��
				nResult = m_CommAgent.CommSetBroad(strKey, nRealType);
				nRealType = 310; // ����ü�ᰡ
				nResult = m_CommAgent.CommSetBroad(strKey, nRealType);
			}
		}
		else if (first == '2' || first == '3') {
			nRealType = 52;
			nResult = m_CommAgent.CommSetBroad(strKey, nRealType);
			nRealType = 66;
			nResult = m_CommAgent.CommSetBroad(strKey, nRealType);
			nRealType = 310; // ����ü�ᰡ
			nResult = m_CommAgent.CommSetBroad(strKey, nRealType);
		}
	}
	else { // �ؿܼ���
		std::string key = VtStringUtil::PadRight(symCode, ' ', 32);
		int nRealType = 76; // �ü�
		m_CommAgent.CommSetBroad(key.c_str(), nRealType);
		nRealType = 82; // ȣ��
		m_CommAgent.CommSetBroad(key.c_str(), nRealType);
	}
}

void SmHdCtrl::UnregisterProduct(std::string symCode)
{
	if (std::isdigit(symCode.at(2))) {
		int nRealType = 76;
		m_CommAgent.CommRemoveBroad(symCode.c_str(), nRealType);
		nRealType = 82;
		m_CommAgent.CommRemoveBroad(symCode.c_str(), nRealType);
	}
	else {
		int nRealType = 76;
		m_CommAgent.CommRemoveBroad(symCode.c_str(), nRealType);
		nRealType = 82;
		m_CommAgent.CommRemoveBroad(symCode.c_str(), nRealType);
	}
}


void SmHdCtrl::RequestChartData(SmChartDataRequest req)
{
	std::lock_guard<std::mutex> lock(_mutex);

	LOG_F(INFO, "RequestChartData %s", req.GetDataKey().c_str());
	_ChartDataReqQueue.push(req);

	if (_ChartDataReqQueue.size() > 1)
		return;

	GetChartData(_ChartDataReqQueue.front());
}

void SmHdCtrl::DownloadDomesticMasterFile(std::string file_name)
{
	if (_FileDownloading)
		return;
	_FileDownloading = true;
	CString sTrCode = "v90001";
	CString sInput = file_name.c_str();
	CString strNextKey = "";
	int nRqID = m_CommAgent.CommRqData(sTrCode, sInput, sInput.GetLength(), strNextKey);
}

void SmHdCtrl::DownloadDomesticMasterFiles()
{
	_DomesticMasterFileRemCount = SmSymbolReader::GetInstance()->DomesticSymbolMasterFileSet.size();
	if (_DomesticMasterFileRemCount > 0) {
		std::string file_name = *(SmSymbolReader::GetInstance()->DomesticSymbolMasterFileSet.begin());
		DownloadDomesticMasterFile(file_name);
	}
}

void SmHdCtrl::RequestChartDataFromQ()
{
	if (!_ChartDataReqQueue.empty()) {
		_ChartDataReqQueue.pop();
		GetChartData(_ChartDataReqQueue.front());
	}
}

void SmHdCtrl::GetChartData(SmChartDataRequest req)
{
	LOG_F(INFO, "GetChartData symbol Code: %s", req.symbolCode.c_str());
	LOG_F(INFO, "GetChartData : %s", req.GetDataKey().c_str());
	if (req.GetDataKey().length() < 8)
		return;
	if (std::isdigit(req.symbolCode.at(2))) {
		if (req.symbolCode.length() < 8)
			return;
		std::string prefix = req.symbolCode.substr(0, 3);
		// � ��Ͽ� �ִ� �͸� ��û�� �Ѵ�.
		if (SmMarketManager::GetInstance()->IsInRunList(prefix))
			GetChartDataForDomestic(req);
	} 
	else {

		if (req.symbolCode.length() < 4)
			return;
		// �ɺ� ���̺� �ִ� �͸� ��û�� �Ѵ�.
		SmSymbol* sym = SmSymbolManager::GetInstance()->FindSymbol(req.symbolCode);
		if (!sym)
			return;

		if (req.chartType == SmChartType::TICK)
			GetChartDataLongCycle(req);
		else if (req.chartType == SmChartType::MIN)
			GetChartDataShortCycle(req);
		else if (req.chartType == SmChartType::DAY)
			GetChartDataLongCycle(req);
		else if (req.chartType == SmChartType::WEEK)
			GetChartDataLongCycle(req);
		else if (req.chartType == SmChartType::MON)
			GetChartDataLongCycle(req);
		else
			GetChartDataShortCycle(req);
	}
}



void SmHdCtrl::GetChartDataShortCycle(SmChartDataRequest req)
{
	std::string temp;
	std::string reqString;
	// ���� �ڵ� 32 �ڸ�
	temp = VtStringUtil::PadRight(req.symbolCode, ' ', 32);
	reqString.append(temp);

	std::string str = VtStringUtil::getCurentDate();
	CString msg;
	msg.Format("%s \n", str.c_str());
	//TRACE(msg);
	reqString.append(str);
	reqString.append(str);
	reqString.append(_T("9999999999"));

	if (req.next == 0)
		reqString.append(_T("0"));
	else
		reqString.append(_T("1"));

	if (req.chartType == SmChartType::TICK)
		reqString.append("1");
	else if (req.chartType == SmChartType::MIN)
		reqString.append("2");
	else if (req.chartType == SmChartType::DAY)
		reqString.append("3");
	else if (req.chartType == SmChartType::WEEK)
		reqString.append("4");
	else if (req.chartType == SmChartType::MON)
		reqString.append("5");
	else
		reqString.append("2");

	temp = VtStringUtil::PadLeft(req.cycle, '0', 2);
	reqString.append(temp);

	temp = VtStringUtil::PadLeft(req.count, '0', 5);
	reqString.append(temp);

	CString sTrCode = "o51200";
	CString sInput = reqString.c_str();
	CString strNextKey = _T("");
	//int nRqID = m_CommAgent.CommRqData(sTrCode, sInput, sInput.GetLength(), "");

	CString sReqFidInput = "000001002003004005006007008009010011012013014015";
	int nRqID = m_CommAgent.CommFIDRqData(sTrCode, sInput, sReqFidInput, sInput.GetLength(), strNextKey);
	
	//TRACE(sInput);
	_ChartDataReqMap[nRqID] = req;
}

void SmHdCtrl::GetChartDataLongCycle(SmChartDataRequest req)
{
	std::string temp;
	std::string reqString;
	// ���� ��û�� 18�ڸ� ����
	reqString.append("                  ");

	temp = VtStringUtil::PadRight(req.symbolCode, ' ', 32);
	reqString.append(temp);

	time_t rawtime;
	struct tm* timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "%Y%m%d-%H:%M:%S", timeinfo);
	std::string str(buffer);
	CString msg;
	msg.Format("%s \n", str.c_str());
	//TRACE(msg);
	//reqString.append(curDate);
	//reqString.append(curDate);
	//reqString.append(_T("99999999"));
	reqString.append(_T("99999999"));
	reqString.append(_T("9999999999"));

	if (req.next == 0)
		reqString.append(_T("0"));
	else
		reqString.append(_T("1"));

	if (req.chartType == SmChartType::TICK)
		reqString.append("6");
	else if (req.chartType == SmChartType::MIN)
		reqString.append("2");
	else if (req.chartType == SmChartType::DAY)
		reqString.append("3");
	else if (req.chartType == SmChartType::WEEK)
		reqString.append("4");
	else if (req.chartType == SmChartType::MON)
		reqString.append("5");
	else
		reqString.append("2");

	temp = VtStringUtil::PadLeft(req.cycle, '0', 3);
	reqString.append(temp);

	temp = VtStringUtil::PadLeft(req.count, '0', 5);
	reqString.append(temp);

	reqString.append(_T("1"));
	reqString.append(_T("1"));


	CString sTrCode = "o44005";
	CString sInput = reqString.c_str();
	CString strNextKey = _T("");
	//int nRqID = m_CommAgent.CommRqData(sTrCode, sInput, sInput.GetLength(), "");

	CString sReqFidInput = "000001002003004005006007008009010011012013014015";
	//CString strNextKey = m_CommAgent.CommGetNextKey(nRqID, "");
	//int nRqID = m_CommAgent.CommFIDRqData(sTrCode, sInput, sReqFidInput, sInput.GetLength(), strNextKey);
	int nRqID = m_CommAgent.CommRqData(sTrCode, sInput, sInput.GetLength(), strNextKey);
	//TRACE(sInput);
	_ChartDataReqMap[nRqID] = req;
}

void SmHdCtrl::GetChartDataForDomestic(SmChartDataRequest req)
{
	std::string temp;
	std::string reqString;

	temp = VtStringUtil::PadRight(req.symbolCode, ' ', 15);
	reqString.append(temp);

	std::string str = VtStringUtil::getCurentDate();
	reqString.append(str);

	reqString.append(_T("999999"));

	temp = VtStringUtil::PadLeft(req.count, '0', 4);
	reqString.append(temp);

	temp = VtStringUtil::PadLeft(req.cycle, '0', 3);
	reqString.append(temp);

	if (req.chartType == SmChartType::TICK)
		reqString.append("0");
	else if (req.chartType == SmChartType::MIN)
		reqString.append("1");
	else if (req.chartType == SmChartType::DAY)
		reqString.append("2");
	else if (req.chartType == SmChartType::WEEK)
		reqString.append("3");
	else if (req.chartType == SmChartType::MON)
		reqString.append("4");
	else
		reqString.append("1");

	if (req.next == 0)
		reqString.append(_T("0"));
	else
		reqString.append(_T("1"));

	temp = VtStringUtil::PadRight(req.reqKey, ' ', 21);
	reqString.append(temp);

	reqString.append(_T("0"));
	reqString.append(_T("0"));
	reqString.append(_T("00"));
	reqString.append(_T("000000"));
	reqString.append(_T(" "));

	if (req.seq == 0)
		reqString.append(_T("0"));
	else
		reqString.append(_T("1"));

	CString sTrCode = "v90003";
	CString sInput = reqString.c_str();
	LOG_F(INFO, "GetChartDataDomestic %s", reqString.c_str());
	CString strNextKey = _T("");
	int nRqID = m_CommAgent.CommRqData(sTrCode, sInput, sInput.GetLength(), "");
	_ChartDataReqMap[nRqID] = req;
}

void SmHdCtrl::OnDomesticSymbolMaster(CString& sTrCode, LONG& nRqID)
{
	int nRepeatCnt = m_CommAgent.CommGetRepeatCnt(sTrCode, -1, "OutRec1");
	CString	strSymCode = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�����ڵ�");

	SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
	SmSymbol* sym = symMgr->FindSymbol((LPCTSTR)strSymCode.Trim());
	if (!sym)
		return;
	
	CString	fullCode = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�����ڵ�");
	
	CString strToPreDay = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "���ϴ��");
	CString strRatioToPreDay = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�����");

	CString	strTime = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "ü��ð�");
	CString	strClose = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "���簡");
	CString	strOpen = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ð�");
	CString	strHigh = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "��");
	CString	strLow = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "����");
	CString strPreClose = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "��������");
	CString strPreHigh = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "���ϰ�");
	CString strPreLow = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "��������");
	CString strSignToPreDay = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "���ϴ�񱸺�");
	CString strAccVol = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�����ŷ���");

	int preday_ratio = _ttoi(strRatioToPreDay);
	preday_ratio >= 0 ? strSignToPreDay = "+" : strSignToPreDay = "-";
	CString ratioToPreDay;
	ratioToPreDay.Format("%.2f", preday_ratio / 100.0);


	sym->Quote.Close = _ttoi(strClose);
	sym->Quote.Open = _ttoi(strOpen);
	sym->Quote.High = _ttoi(strHigh);
	sym->Quote.Low = _ttoi(strLow);
	sym->Quote.OriginTime = strTime;
	sym->Quote.GapFromPreDay = _ttoi(strToPreDay);
	sym->Quote.RatioToPreday = ratioToPreDay;
	sym->Quote.SignToPreDay = strSignToPreDay;
	sym->Quote.accVolume = _ttoi(strAccVol);

	SmQuote quoteItem;
	quoteItem.SymbolCode = strSymCode.Trim();
	quoteItem.OriginTime = strTime;
	quoteItem.SignToPreDay = strSignToPreDay.Trim();
	quoteItem.GapFromPreDay = _ttoi(strToPreDay);
	quoteItem.RatioToPreday = ratioToPreDay;
	quoteItem.Close = _ttoi(strClose);
	quoteItem.Open = _ttoi(strOpen);
	quoteItem.High = _ttoi(strHigh);
	quoteItem.Low = _ttoi(strLow);
	quoteItem.Volume = 0;
	quoteItem.Sign = "";
	quoteItem.accVolume = _ttoi(strAccVol);

	SmMongoDBManager* mongoMgr = SmMongoDBManager::GetInstance();
	mongoMgr->SaveSise(quoteItem);

	SmSessionManager* sessMgr = SmSessionManager::GetInstance();
	//sessMgr->SendReqUpdateQuote(quoteItem.SymbolCode);
	sessMgr->SendReqUpdateQuote(sym);

	CString msg;
	msg.Format(_T("symbol = %s, time = %s, h=%s, l=%s, o=%s, c=%s, ratio = %s, gap = %s\n"), strSymCode, strTime, strHigh, strLow, strOpen, strClose, strRatioToPreDay, strToPreDay);
	TRACE(msg);

	CString strHogaTime = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "ȣ�����Žð�");

	HdHoga hoga;
	hoga.strSymbolCode = strSymCode;
	hoga.Items[0].strBuyHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ��1");
	hoga.Items[0].strSellHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ��1");
	hoga.Items[0].strBuyHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ������1");
	hoga.Items[0].strSellHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ������1");
	hoga.Items[0].strBuyHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ���Ǽ�1");
	hoga.Items[0].strSellHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ���Ǽ�1");

	hoga.Items[1].strBuyHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ��2");
	hoga.Items[1].strSellHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ��2");
	hoga.Items[1].strBuyHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ������2");
	hoga.Items[1].strSellHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ������2");
	hoga.Items[1].strBuyHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ���Ǽ�2");
	hoga.Items[1].strSellHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ���Ǽ�2");

	hoga.Items[2].strBuyHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ��3");
	hoga.Items[2].strSellHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ��3");
	hoga.Items[2].strBuyHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ������3");
	hoga.Items[2].strSellHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ������3");
	hoga.Items[2].strBuyHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ���Ǽ�3");
	hoga.Items[2].strSellHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ���Ǽ�3");

	hoga.Items[3].strBuyHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ��4");
	hoga.Items[3].strSellHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ��4");
	hoga.Items[3].strBuyHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ������4");
	hoga.Items[3].strSellHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ������4");
	hoga.Items[3].strBuyHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ���Ǽ�4");
	hoga.Items[3].strSellHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ���Ǽ�4");

	hoga.Items[4].strBuyHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ��5");
	hoga.Items[4].strSellHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ��5");
	hoga.Items[4].strBuyHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ������5");
	hoga.Items[4].strSellHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ������5");
	hoga.Items[4].strBuyHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ���Ǽ�5");
	hoga.Items[4].strSellHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ���Ǽ�5");

	CString strTotBuyQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ���Ѽ���");
	CString strTotSellQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ���Ѽ���");
	CString strTotBuyCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ż�ȣ���ѰǼ�");
	CString strTotSellCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ŵ�ȣ���ѰǼ�");

	SmHoga hoga_data;
	for (int i = 0; i < 5; i++) {
		hoga_data.Ary[i].BuyPrice = sym->Hoga.Ary[i].BuyPrice = _ttoi(hoga.Items[i].strBuyHoga);
		hoga_data.Ary[i].BuyCnt = sym->Hoga.Ary[i].BuyCnt = _ttoi(hoga.Items[i].strBuyHogaCnt);
		hoga_data.Ary[i].BuyQty = sym->Hoga.Ary[i].BuyQty = _ttoi(hoga.Items[i].strBuyHogaQty);
		hoga_data.Ary[i].SellPrice = sym->Hoga.Ary[i].SellPrice = _ttoi(hoga.Items[i].strSellHoga);
		hoga_data.Ary[i].SellCnt = sym->Hoga.Ary[i].SellCnt = _ttoi(hoga.Items[i].strSellHogaCnt);
		hoga_data.Ary[i].SellQty = sym->Hoga.Ary[i].SellQty = _ttoi(hoga.Items[i].strSellHogaQty);
	}

	hoga_data.Time = sym->Hoga.Time = strHogaTime;
	hoga_data.TotBuyCnt = sym->Hoga.TotBuyCnt = _ttoi(strTotBuyCnt);
	hoga_data.TotBuyQty = sym->Hoga.TotBuyQty = _ttoi(strTotBuyQty);
	hoga_data.TotSellCnt = sym->Hoga.TotSellCnt = _ttoi(strTotSellCnt);
	hoga_data.TotSellQty = sym->Hoga.TotSellQty = _ttoi(strTotSellQty);

	
	mongoMgr->SaveHoga(hoga_data);

	
	sessMgr->SendReqUpdateHoga(sym);

	
	msg.Format(_T("hoga :: time = %s, tot_buy_cnt = %d\n"), sym->Hoga.SymbolCode.c_str(), sym->Hoga.TotBuyCnt);
	//TRACE(msg);
}

void SmHdCtrl::DownloadMasterFiles(std::string param)
{
	m_CommAgent.CommReqMakeCod(param.c_str(), 0);
}

void SmHdCtrl::GetSiseData(std::string symCode)
{
	if (std::isdigit(symCode.at(2))) {
		CString sInput = symCode.c_str();
		sInput.Append(_T("40001"));
		//�����ڵ�[000], ȣ�����Žð�[075], ���簡[051], �����ŷ���[057], ����������[040], ������[006], ��[053]  1�� ��ȸ��
		CString sReqFidInput = _T("000001002003004005006007008009010011012013014015016017018019020021022023024025026027028029030031032033034035036037038039040041042043044045046047048049050051052053054055056057058059060061062063064065066067068069070071072073074075076077078079080081082083084085086087088089090091092093094095096097098099100101102103104105106107108109110111112113114115116117118119120121122123124125126127128129130131132133134135136137138139140141142143144145146147148149150151152153154155156157158159160161162163164165166167168169170171172173174175176177178179180181182183184185186187188189190191192193194195196197198199200201202203204205206207208209210211212213214215216217218219220221222223224225226227228229230231232");
		//CString sReqFidInput = _T("000001002003004005049050051052053054075076078079080081082083084085086087089090091092093094095096097098099100101102103104105106107108109115");
		CString strNextKey = _T("");
		int nRqID = m_CommAgent.CommFIDRqData(DefSymbolMaster, sInput, sReqFidInput, sInput.GetLength(), strNextKey);
		_SiseDataReqMap[nRqID] = symCode;
	}
	else {
		CString sFidCode = "o51000";
		std::string sym_code = VtStringUtil::PadRight(symCode, ' ', 32);
		CString sInput = sym_code.c_str();
		CString sReqFidInput = "000001002003004005006007008009010011012013014015016017018019020021022023024025026027028029030031032033034035036037";
		CString strNextKey = "";
		int nRqID = m_CommAgent.CommFIDRqData(sFidCode, sInput, sReqFidInput, sInput.GetLength(), strNextKey);
		_SiseDataReqMap[nRqID] = symCode;
	}
}

void SmHdCtrl::GetHogaData(std::string symCode)
{
	if (std::isdigit(symCode.at(2))) {
		CString sInput = symCode.c_str();
		sInput.Append(_T("40001"));
		//�����ڵ�[000], ȣ�����Žð�[075], ���簡[051], �����ŷ���[057], ����������[040], ������[006], ��[053]  1�� ��ȸ��
		CString sReqFidInput = _T("000001002003004005006007008009010011012013014015016017018019020021022023024025026027028029030031032033034035036037038039040041042043044045046047048049050051052053054055056057058059060061062063064065066067068069070071072073074075076077078079080081082083084085086087088089090091092093094095096097098099100101102103104105106107108109110111112113114115116117118119120121122123124125126127128129130131132133134135136137138139140141142143144145146147148149150151152153154155156157158159160161162163164165166167168169170171172173174175176177178179180181182183184185186187188189190191192193194195196197198199200201202203204205206207208209210211212213214215216217218219220221222223224225226227228229230231232");
		//CString sReqFidInput = _T("000001002003004005049050051052053054075076078079080081082083084085086087089090091092093094095096097098099100101102103104105106107108109115");
		CString strNextKey = _T("");
		int nRqID = m_CommAgent.CommFIDRqData(DefSymbolMaster, sInput, sReqFidInput, sInput.GetLength(), strNextKey);
		_SiseDataReqMap[nRqID] = symCode;
	}
	else {
		CString sFidCode = "o51010";
		std::string sym_code = VtStringUtil::PadRight(symCode, ' ', 32);
		CString sInput = sym_code.c_str();
		CString sReqFidInput = "000001002003004005006007008009010011012013014015016017018019020021022023024025026027028029030031032033034035036037038039040041042043044045046";
		CString strNextKey = "";
		int nRqID = m_CommAgent.CommFIDRqData(sFidCode, sInput, sReqFidInput, sInput.GetLength(), strNextKey);
		_SiseDataReqMap[nRqID] = symCode;
	}
}

void SmHdCtrl::OnRcvdAbroadHoga(CString& strKey, LONG& nRealType)
{
	SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
	CString strSymCode = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�����ڵ�");
	SmSymbol* sym = symMgr->FindSymbol((LPCTSTR)strSymCode.Trim());
	if (!sym)
		return;
	CString strHogaTime = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ȣ���ð�");

	HdHoga hoga;
	hoga.Items[0].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ��1");
	hoga.Items[0].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ��1");
	hoga.Items[0].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ������1");
	hoga.Items[0].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ������1");
	hoga.Items[0].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ǽ�1");
	hoga.Items[0].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ǽ�1");

	hoga.Items[1].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ��2");
	hoga.Items[1].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ��2");
	hoga.Items[1].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ������2");
	hoga.Items[1].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ������2");
	hoga.Items[1].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ǽ�2");
	hoga.Items[1].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ǽ�2");

	hoga.Items[2].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ��3");
	hoga.Items[2].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ��3");
	hoga.Items[2].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ������3");
	hoga.Items[2].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ������3");
	hoga.Items[2].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ǽ�3");
	hoga.Items[2].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ǽ�3");

	hoga.Items[3].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ��4");
	hoga.Items[3].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ��4");
	hoga.Items[3].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ������4");
	hoga.Items[3].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ������4");
	hoga.Items[3].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ǽ�4");
	hoga.Items[3].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ǽ�4");

	hoga.Items[4].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ��5");
	hoga.Items[4].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ��5");
	hoga.Items[4].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ������5");
	hoga.Items[4].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ������5");
	hoga.Items[4].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ǽ�5");
	hoga.Items[4].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ǽ�5");

	CString strTotBuyQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ѽ���");
	CString strTotSellQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ѽ���");
	CString strTotBuyCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���ѰǼ�");
	CString strTotSellCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���ѰǼ�");
	CString strDomDate = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "��������");
	CString strDomTime = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�����ð�");
	SmHoga hoga_data;
	hoga_data.SymbolCode = strSymCode;
	for (int i = 0; i < 5; i++) {
		hoga_data.Ary[i].BuyPrice = sym->Hoga.Ary[i].BuyPrice = _ttoi(hoga.Items[i].strBuyHoga);
		hoga_data.Ary[i].BuyCnt = sym->Hoga.Ary[i].BuyCnt = _ttoi(hoga.Items[i].strBuyHogaCnt);
		hoga_data.Ary[i].BuyQty = sym->Hoga.Ary[i].BuyQty = _ttoi(hoga.Items[i].strBuyHogaQty);
		hoga_data.Ary[i].SellPrice = sym->Hoga.Ary[i].SellPrice = _ttoi(hoga.Items[i].strSellHoga);
		hoga_data.Ary[i].SellCnt = sym->Hoga.Ary[i].SellCnt = _ttoi(hoga.Items[i].strSellHogaCnt);
		hoga_data.Ary[i].SellQty = sym->Hoga.Ary[i].SellQty = _ttoi(hoga.Items[i].strSellHogaQty);
	}

	hoga_data.DomesticDate = sym->Hoga.DomesticDate = strDomDate;
	hoga_data.DomesticTime =  sym->Hoga.DomesticTime = strDomTime;
	hoga_data.Time = sym->Hoga.Time = strHogaTime;
	hoga_data.TotBuyCnt = sym->Hoga.TotBuyCnt = _ttoi(strTotBuyCnt);
	hoga_data.TotBuyQty = sym->Hoga.TotBuyQty = _ttoi(strTotBuyQty);
	hoga_data.TotSellCnt = sym->Hoga.TotSellCnt = _ttoi(strTotSellCnt);
	hoga_data.TotSellQty = sym->Hoga.TotSellQty = _ttoi(strTotSellQty);

	SmMongoDBManager* mongoMgr = SmMongoDBManager::GetInstance();
	mongoMgr->SaveHoga(hoga_data);

	SmSessionManager* sessMgr = SmSessionManager::GetInstance();
	sessMgr->SendReqUpdateHoga(sym);

	//TRACE(sym->Hoga.Time.c_str());

	SmTimeSeriesDBManager* dbMgr = SmTimeSeriesDBManager::GetInstance();
	//dbMgr->SaveHogaItem(std::move(hoga_data));

	CString msg;
	msg.Format(_T("hoga :: time = %s, tot_buy_cnt = %d\n"), sym->Hoga.SymbolCode.c_str(), sym->Hoga.TotBuyCnt);
	//TRACE(msg);
}

void SmHdCtrl::OnRcvdAbroadSise(CString& strKey, LONG& nRealType)
{
	CString strSymCode = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�����ڵ�");
	CString strTime = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ü��ð�");
	CString strSignToPreDay = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "���ϴ�񱸺�");
	CString strToPreDay = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "���ϴ��");
	CString strRatioToPreDay = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "���ϴ������");
	CString strClose = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ü�ᰡ");
	CString strOpen = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ð�");
	CString strHigh = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "��");
	CString strLow = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "����");
	CString strVolume = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ü�ᷮ");
	CString strSign = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ü�ᱸ��");
	CString strAccVol = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�����ŷ���");

	SmQuote quoteItem;
	quoteItem.SymbolCode = strSymCode.Trim();
	quoteItem.OriginTime = strTime;
	quoteItem.SignToPreDay = strSignToPreDay.Trim();
	quoteItem.GapFromPreDay = _ttoi(strToPreDay);
	quoteItem.RatioToPreday = strRatioToPreDay.Trim();
	quoteItem.Close = _ttoi(strClose);
	quoteItem.Open = _ttoi(strOpen);
	quoteItem.High = _ttoi(strHigh);
	quoteItem.Low = _ttoi(strLow);
	quoteItem.Volume= _ttoi(strVolume);
	quoteItem.Sign = strSign.Trim();
	quoteItem.accVolume = _ttoi(strAccVol);


	SmMongoDBManager* mongoMgr = SmMongoDBManager::GetInstance();
	mongoMgr->SaveSise(quoteItem);
	SmTimeSeriesDBManager* tsDBMgr = SmTimeSeriesDBManager::GetInstance();
	tsDBMgr->SaveQuoteItem(std::move(quoteItem));

	SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
	SmSymbol* sym = symMgr->FindSymbol((LPCTSTR)strSymCode.Trim());
	if (!sym)
		return;
	sym->Quote.Close = _ttoi(strClose);
	sym->Quote.Open = _ttoi(strOpen);
	sym->Quote.High = _ttoi(strHigh);
	sym->Quote.Low = _ttoi(strLow);
	sym->Quote.OriginTime = strTime;
	sym->Quote.GapFromPreDay = _ttoi(strToPreDay);
	sym->Quote.RatioToPreday = strRatioToPreDay.Trim();
	sym->Quote.SignToPreDay = strSignToPreDay;
	sym->Quote.accVolume = _ttoi(strAccVol);

	SmSessionManager* sessMgr = SmSessionManager::GetInstance();
	sessMgr->SendReqUpdateQuote(sym);

	CString msg;
	msg.Format(_T("symbol = %s, time = %s, h=%s, l=%s, o=%s, c=%s, v=%s, ratio = %s\n"), strSymCode, strTime, strHigh, strLow, strOpen, strClose, strVolume, strRatioToPreDay);
	//TRACE(msg);
}

void SmHdCtrl::OnRcvdDomesticChartData(CString& sTrCode, LONG& nRqID)
{
	int nRepeatCnt = m_CommAgent.CommGetRepeatCnt(sTrCode, -1, "OutRec2");
	//influxdb_cpp::server_info si("127.0.0.1", 8086, "abroad_future", "angelpie", "orion1");
	//influxdb_cpp::server_info si("127.0.0.1", 8086, "test_x", "test", "test");
	CString msg;

	auto it = _ChartDataReqMap.find(nRqID);
	if (it == _ChartDataReqMap.end())
		return;
	SmChartDataRequest req = it->second;
	SmTimeSeriesCollector * tsCol = SmTimeSeriesCollector::GetInstance();
	SmChartDataManager * chartDataMgr = SmChartDataManager::GetInstance();
	SmChartData * chart_data = chartDataMgr->AddChartData(req);
	int total_count = nRepeatCnt;
	int current_count = 1;
	std::vector<SmChartDataItem> chart_vec;
	// ���� �ֱٰ��� ���� ���� �´�. ���� ���� ������ �����͸� ���� �����´�.
	// Received the chart data first.
	for (int i = nRepeatCnt - 1; i >= 0; --i) {
		CString strDate = "";
		CString strTime = "";

		CString tempDate = m_CommAgent.CommGetData(sTrCode, -1, "OutRec2", i, "��¥�ð�");
		
		if (chart_data->ChartType() == SmChartType::MIN)
			tempDate.Append(_T("00"));
		else
			tempDate.Append(_T("000000"));
		
		
		strTime = tempDate.Right(6);
		strDate = tempDate.Left(8);
		
		CString strOpen = m_CommAgent.CommGetData(sTrCode, -1, "OutRec2", i, "�ð�");
		CString strHigh = m_CommAgent.CommGetData(sTrCode, -1, "OutRec2", i, "��");
		CString strLow = m_CommAgent.CommGetData(sTrCode, -1, "OutRec2", i, "����");
		CString strClose = m_CommAgent.CommGetData(sTrCode, -1, "OutRec2", i, "����");
		CString strVol = m_CommAgent.CommGetData(sTrCode, -1, "OutRec2", i, "�ŷ���");

		if (strDate.GetLength() == 0)
			continue;

		//msg.Format(_T("OnRcvdAbroadChartData ::code = %s, index = %d, date = %s, t = %s, o = %s, h = %s, l = %s, c = %s, v = %s\n"), req.symbolCode.c_str(), i, strDate, strTime, strOpen, strHigh, strLow, strClose, strVol);
		//TRACE(msg);

		SmChartDataItem data;
		data.symbolCode = req.symbolCode;
		data.chartType = req.chartType;
		data.cycle = req.cycle;
		data.date = strDate.Trim();
		data.time = strTime.Trim();
		data.h = _ttoi(strHigh);
		data.l = _ttoi(strLow);
		data.o = _ttoi(strOpen);
		data.c = _ttoi(strClose);
		data.v = _ttoi(strVol);

		chart_vec.push_back(data);

		if (req.reqType == SmChartDataReqestType::FIRST) {
			// ���⼭ �ٷ� �����͸� �����Ѵ�.
			SmTimeSeriesServiceManager::GetInstance()->SendChartData(req.session_id, total_count, current_count++, data);
		}
		else {
			SmTimeSeriesServiceManager::GetInstance()->SendCycleChartData(data);
		}
		if (req.reqType == SmChartDataReqestType::CYCLE) {
			// ���⼭ ������ ���̽��� ������Ʈ �Ѵ�.
			SmMongoDBManager::GetInstance()->SaveChartDataItem(data);
		}
	}

	LOG_F(INFO, "OnRcvdDomesticChartData %s", req.GetDataKey().c_str());

	// ���� ó������ ���� �����ʹ� ť�� ���ؼ� ó���Ѵ�.
	RequestChartDataFromQ();

	// ��Ʈ ������ ���� ��û ��Ͽ��� �����Ѵ�.
	_ChartDataReqMap.erase(it);
	// �ֱⵥ���Ͱ� ���������� �˸���.
	if (chart_data) {
		if (nRepeatCnt == chart_data->CycleDataSize()) {
			// ��Ʈ ������ ���� �ϷḦ �˸���.
			SmTimeSeriesServiceManager* tsSvcMgr = SmTimeSeriesServiceManager::GetInstance();
			tsSvcMgr->OnCompleteChartCycleData(req);
			
		}
		else {
			// ��Ʈ ������ ���� �ϷḦ �˸���.
			SmTimeSeriesServiceManager* tsSvcMgr = SmTimeSeriesServiceManager::GetInstance();
			tsSvcMgr->OnCompleteChartData(req, chart_data);

			try
			{
				if (_SaveChartData) {
					std::async(std::launch::async, [chart_vec] {
						for (auto it = chart_vec.begin(); it != chart_vec.end(); ++it) {
							SmChartDataItem item = *it;
							SmMongoDBManager::GetInstance()->SaveChartDataItem(item);
						}
						});
				}
			}
			catch (std::exception e)
			{
				std::string error = e.what();
				LOG_F(INFO, "%s", error);
			}

		}
	}
}

void SmHdCtrl::OnRcvdAbroadSiseByReq(CString& sTrCode, LONG& nRqID)
{
	int nRepeatCnt = m_CommAgent.CommGetRepeatCnt(sTrCode, -1, "OutRec1");
	for (int i = 0; i < nRepeatCnt; i++)
	{
		CString strData1 = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�����ڵ�");
		CString strData2 = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ѱ������");
		CString strData3 = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "���簡");
		CString msg;
		msg.Format(_T("code = %s, name = %s, close = %s\n"), strData1, strData2, strData3);
		TRACE(msg);

		CString strSymCode = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�����ڵ�");
		CString strTime = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�����ð�");
		CString strPrev = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "������񱸺�");
		CString strSignToPreDay = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "���ϴ�񱸺�");
		CString strToPreDay = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "���ϴ��");
		CString strRatioToPreDay = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "���ϴ������");
		CString strClose = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "���簡");
		CString strOpen = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�ð�");
		CString strHigh = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "��");
		CString strLow = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "����");
		CString strAccVol = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�����ŷ���");

		SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
		SmSymbol* sym = symMgr->FindSymbol((LPCTSTR)strSymCode.Trim());
		if (!sym)
			continue;
		sym->Quote.Close = _ttoi(strClose);
		sym->Quote.Open = _ttoi(strOpen);
		sym->Quote.High = _ttoi(strHigh);
		sym->Quote.Low = _ttoi(strLow);
		sym->Quote.OriginTime = strTime;
		sym->Quote.GapFromPreDay = _ttoi(strToPreDay);
		sym->Quote.RatioToPreday = strRatioToPreDay.Trim();
		sym->Quote.SignToPreDay = strSignToPreDay;
		sym->Quote.accVolume = _ttoi(strAccVol);

		SmQuote quoteItem;
		quoteItem.SymbolCode = strSymCode.Trim();
		quoteItem.OriginTime = strTime;
		quoteItem.SignToPreDay = strSignToPreDay.Trim();
		quoteItem.GapFromPreDay = _ttoi(strToPreDay);
		quoteItem.RatioToPreday = strRatioToPreDay.Trim();
		quoteItem.Close = _ttoi(strClose);
		quoteItem.Open = _ttoi(strOpen);
		quoteItem.High = _ttoi(strHigh);
		quoteItem.Low = _ttoi(strLow);
		quoteItem.Volume = 0;
		quoteItem.Sign = "";
		quoteItem.accVolume = _ttoi(strAccVol);

		SmMongoDBManager* mongoMgr = SmMongoDBManager::GetInstance();
		mongoMgr->SaveSise(quoteItem);

		SmSessionManager* sessMgr = SmSessionManager::GetInstance();
		//sessMgr->SendReqUpdateQuote(quoteItem.SymbolCode);
		sessMgr->SendReqUpdateQuote(sym);

		//CString msg;
		msg.Format(_T("symbol = %s, time = %s, h=%s, l=%s, o=%s, c=%s, ratio = %s\n"), strSymCode, strTime, strHigh, strLow, strOpen, strClose, strRatioToPreDay);
		TRACE(msg);
	}
}

void SmHdCtrl::OnRcvdAbroadHogaByReq(CString& sTrCode, LONG& nRqID)
{
	int nRepeatCnt = m_CommAgent.CommGetRepeatCnt(sTrCode, -1, "OutRec1");
	for (int i = 0; i < nRepeatCnt; i++)
	{
		SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
		CString strSymCode = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�����ڵ�");
		SmSymbol* sym = symMgr->FindSymbol((LPCTSTR)strSymCode.Trim());
		if (!sym)
			continue;
		CString strHogaTime = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "ȣ���ð�");

		HdHoga hoga;
		hoga.strSymbolCode = strSymCode;
		hoga.Items[0].strBuyHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ż�ȣ��1");
		hoga.Items[0].strSellHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ŵ�ȣ��1");
		hoga.Items[0].strBuyHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ż�ȣ������1");
		hoga.Items[0].strSellHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ŵ�ȣ������1");
		hoga.Items[0].strBuyHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ż�ȣ���Ǽ�1");
		hoga.Items[0].strSellHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ŵ�ȣ���Ǽ�1");

		hoga.Items[1].strBuyHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ż�ȣ��2");
		hoga.Items[1].strSellHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ŵ�ȣ��2");
		hoga.Items[1].strBuyHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ż�ȣ������2");
		hoga.Items[1].strSellHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ŵ�ȣ������2");
		hoga.Items[1].strBuyHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ż�ȣ���Ǽ�2");
		hoga.Items[1].strSellHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ŵ�ȣ���Ǽ�2");

		hoga.Items[2].strBuyHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ż�ȣ��3");
		hoga.Items[2].strSellHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ŵ�ȣ��3");
		hoga.Items[2].strBuyHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ż�ȣ������3");
		hoga.Items[2].strSellHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ŵ�ȣ������3");
		hoga.Items[2].strBuyHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ż�ȣ���Ǽ�3");
		hoga.Items[2].strSellHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ŵ�ȣ���Ǽ�3");

		hoga.Items[3].strBuyHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ż�ȣ��4");
		hoga.Items[3].strSellHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ŵ�ȣ��4");
		hoga.Items[3].strBuyHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ż�ȣ������4");
		hoga.Items[3].strSellHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ŵ�ȣ������4");
		hoga.Items[3].strBuyHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ż�ȣ���Ǽ�4");
		hoga.Items[3].strSellHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ŵ�ȣ���Ǽ�4");

		hoga.Items[4].strBuyHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ż�ȣ��5");
		hoga.Items[4].strSellHoga = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ŵ�ȣ��5");
		hoga.Items[4].strBuyHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ż�ȣ������5");
		hoga.Items[4].strSellHogaQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ŵ�ȣ������5");
		hoga.Items[4].strBuyHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ż�ȣ���Ǽ�5");
		hoga.Items[4].strSellHogaCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ŵ�ȣ���Ǽ�5");

		CString strTotBuyQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�Ѹż�ȣ���ܷ�");
		CString strTotSellQty = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�Ѹŵ�ȣ���ܷ�");
		CString strTotBuyCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�Ѹż�ȣ���Ǽ�");
		CString strTotSellCnt = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�Ѹŵ�ȣ���Ǽ�");
		
		SmHoga hoga_data;
		for (int i = 0; i < 5; i++) {
			hoga_data.Ary[i].BuyPrice = sym->Hoga.Ary[i].BuyPrice = _ttoi(hoga.Items[i].strBuyHoga);
			hoga_data.Ary[i].BuyCnt = sym->Hoga.Ary[i].BuyCnt = _ttoi(hoga.Items[i].strBuyHogaCnt);
			hoga_data.Ary[i].BuyQty = sym->Hoga.Ary[i].BuyQty = _ttoi(hoga.Items[i].strBuyHogaQty);
			hoga_data.Ary[i].SellPrice = sym->Hoga.Ary[i].SellPrice = _ttoi(hoga.Items[i].strSellHoga);
			hoga_data.Ary[i].SellCnt = sym->Hoga.Ary[i].SellCnt = _ttoi(hoga.Items[i].strSellHogaCnt);
			hoga_data.Ary[i].SellQty = sym->Hoga.Ary[i].SellQty = _ttoi(hoga.Items[i].strSellHogaQty);
		}

		hoga_data.Time = sym->Hoga.Time = strHogaTime;
		hoga_data.TotBuyCnt = sym->Hoga.TotBuyCnt = _ttoi(strTotBuyCnt);
		hoga_data.TotBuyQty = sym->Hoga.TotBuyQty = _ttoi(strTotBuyQty);
		hoga_data.TotSellCnt = sym->Hoga.TotSellCnt = _ttoi(strTotSellCnt);
		hoga_data.TotSellQty = sym->Hoga.TotSellQty = _ttoi(strTotSellQty);

		SmMongoDBManager* mongoMgr = SmMongoDBManager::GetInstance();
		mongoMgr->SaveHoga(hoga_data);

		SmSessionManager* sessMgr = SmSessionManager::GetInstance();
		sessMgr->SendReqUpdateHoga(sym);

		CString msg;
		msg.Format(_T("hoga :: time = %s, tot_buy_cnt = %d\n"), sym->Hoga.SymbolCode.c_str(), sym->Hoga.TotBuyCnt);
		//TRACE(msg);
	}
}

void SmHdCtrl::OnRcvdAbroadChartData(CString& sTrCode, LONG& nRqID)
{
	int nRepeatCnt = m_CommAgent.CommGetRepeatCnt(sTrCode, -1, "OutRec1");
	//influxdb_cpp::server_info si("127.0.0.1", 8086, "abroad_future", "angelpie", "orion1");
	//influxdb_cpp::server_info si("127.0.0.1", 8086, "test_x", "test", "test");
	CString msg;

	auto it = _ChartDataReqMap.find(nRqID);
	if (it == _ChartDataReqMap.end())
		return;
	SmChartDataRequest req = it->second;
	SmTimeSeriesCollector* tsCol = SmTimeSeriesCollector::GetInstance();
	SmChartDataManager* chartDataMgr = SmChartDataManager::GetInstance();
	SmChartData* chart_data = chartDataMgr->AddChartData(req);
	int total_count = nRepeatCnt;
	int current_count = 1;
	std::vector<SmChartDataItem> chart_vec;
	// ���� �ֱٰ��� ���� ���� �´�. ���� ���� ������ �����͸� ���� �����´�.
	// Received the chart data first.
	for (int i = nRepeatCnt - 1; i >= 0; --i) {
		CString strDate = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "��������");
		CString strTime = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�����ð�");
		CString strOpen = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "�ð�");
		CString strHigh = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "��");
		CString strLow = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "����");
		CString strClose = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "����");
		CString strVol = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", i, "ü�ᷮ");

		if (strDate.GetLength() == 0)
			continue;
		
		msg.Format(_T("OnRcvdAbroadChartData ::code = %s, index = %d, date = %s, t = %s, o = %s, h = %s, l = %s, c = %s, v = %s\n"), req.symbolCode.c_str(), i, strDate, strTime, strOpen, strHigh, strLow, strClose, strVol);
		TRACE(msg);

		SmChartDataItem data;
		data.symbolCode = req.symbolCode;
		data.chartType = req.chartType;
		data.cycle = req.cycle;
		data.date = strDate.Trim();
		data.time = strTime.Trim();
		data.h = _ttoi(strHigh);
		data.l = _ttoi(strLow);
		data.o = _ttoi(strOpen);
		data.c = _ttoi(strClose);
		data.v = _ttoi(strVol);

		chart_vec.push_back(data);
		
		if (req.reqType == SmChartDataReqestType::FIRST) {
			// ���⼭ �ٷ� �����͸� �����Ѵ�.
			SmTimeSeriesServiceManager::GetInstance()->SendChartData(req.session_id, total_count, current_count++, data);
		}
		else {
			SmTimeSeriesServiceManager::GetInstance()->SendCycleChartData(data);
		}
		if (req.reqType == SmChartDataReqestType::CYCLE) {
			// ���⼭ ������ ���̽��� ������Ʈ �Ѵ�.
			SmMongoDBManager::GetInstance()->SaveChartDataItem(data);
		}
	}


	LOG_F(INFO, "OnRcvdAbroadChartData %s", req.GetDataKey().c_str());

	// ���� ó������ ���� �����ʹ� ť�� ���ؼ� ó���Ѵ�.
	RequestChartDataFromQ();

	// ��Ʈ ������ ���� ��û ��Ͽ��� �����Ѵ�.
	_ChartDataReqMap.erase(it);
	// �ֱⵥ���Ͱ� ���������� �˸���.
	if (chart_data) {
		if (nRepeatCnt == chart_data->CycleDataSize()) {
			// ��Ʈ ������ ���� �ϷḦ �˸���.
			SmTimeSeriesServiceManager* tsSvcMgr = SmTimeSeriesServiceManager::GetInstance();
			tsSvcMgr->OnCompleteChartCycleData(req);
			
			
		}
		else {
			
			// ��Ʈ ������ ���� �ϷḦ �˸���.
			SmTimeSeriesServiceManager* tsSvcMgr = SmTimeSeriesServiceManager::GetInstance();
			tsSvcMgr->OnCompleteChartData(req, chart_data);
			
			try
			{
				if (_SaveChartData) {
					std::async(std::launch::async, [chart_vec] {
						for (auto it = chart_vec.begin(); it != chart_vec.end(); ++it) {
							SmChartDataItem item = *it;
							SmMongoDBManager::GetInstance()->SaveChartDataItem(item);
						}
						});
				}
			}
			catch (std::exception e)
			{
				std::string error = e.what();
				LOG_F(INFO, "%s", error);
			}
			
		}
	}
}


void SmHdCtrl::OnRcvdAbroadChartData2(CString& sTrCode, LONG& nRqID)
{
	int nRepeatCnt = m_CommAgent.CommGetRepeatCnt(sTrCode, -1, "OutRec2");
	//influxdb_cpp::server_info si("127.0.0.1", 8086, "abroad_future", "angelpie", "orion1");
	//influxdb_cpp::server_info si("127.0.0.1", 8086, "test_x", "test", "test");
	CString msg;

	auto it = _ChartDataReqMap.find(nRqID);
	if (it == _ChartDataReqMap.end())
		return;
	SmChartDataRequest req = it->second;
	SmTimeSeriesCollector* tsCol = SmTimeSeriesCollector::GetInstance();
	SmChartDataManager* chartDataMgr = SmChartDataManager::GetInstance();
	SmChartData* chart_data = chartDataMgr->AddChartData(req);
	int total_count = nRepeatCnt;
	int current_count = 1;
	std::vector<SmChartDataItem> chart_vec;
	// ���� �ֱٰ��� ���� ���� �´�. ���� ���� ������ �����͸� ���� �����´�.
	// Received the chart data first.
	for (int i = 0; i < nRepeatCnt; ++i) {
		CString strDate = m_CommAgent.CommGetData(sTrCode, -1, "OutRec2", i, "��������");
		CString strTime = m_CommAgent.CommGetData(sTrCode, -1, "OutRec2", i, "�����ð�");
		CString strOpen = m_CommAgent.CommGetData(sTrCode, -1, "OutRec2", i, "�ð�");
		CString strHigh = m_CommAgent.CommGetData(sTrCode, -1, "OutRec2", i, "��");
		CString strLow = m_CommAgent.CommGetData(sTrCode, -1, "OutRec2", i, "����");
		CString strClose = m_CommAgent.CommGetData(sTrCode, -1, "OutRec2", i, "����");
		CString strVol = m_CommAgent.CommGetData(sTrCode, -1, "OutRec2", i, "ü�ᷮ");
		CString strTotalVol = m_CommAgent.CommGetData(sTrCode, -1, "OutRec2", i, "�����ŷ���");

		
		if (strDate.GetLength() == 0)
			continue;

		msg.Format(_T("OnRcvdAbroadChartData :: index = %d, date = %s, t = %s, o = %s, h = %s, l = %s, c = %s, v = %s\n"), i, strDate, strTime, strOpen, strHigh, strLow, strClose, strVol);
		TRACE(msg);


		SmChartDataItem data;
		data.symbolCode = req.symbolCode;
		data.chartType = req.chartType;
		data.cycle = req.cycle;
		data.date = strDate.Trim();
		data.time = strTime.Trim();
		data.h = _ttoi(strHigh);
		data.l = _ttoi(strLow);
		data.o = _ttoi(strOpen);
		data.c = _ttoi(strClose);
		data.v = _ttoi(strVol);

		chart_vec.push_back(data);

		// ���⼭ �ٷ� �����͸� �����Ѵ�.
		SmTimeSeriesServiceManager::GetInstance()->SendChartData(req.session_id, total_count, current_count++, data);
		if (req.reqType == SmChartDataReqestType::CYCLE) {
			// ���⼭ ������ ���̽��� ������Ʈ �Ѵ�.
			SmMongoDBManager::GetInstance()->SaveChartDataItem(data);
		}
	}

	LOG_F(INFO, "OnRcvdAbroadChartData2 %s", req.GetDataKey().c_str());

	RequestChartDataFromQ();
	// ��Ʈ ������ ���� ��û ��Ͽ��� �����Ѵ�.
	_ChartDataReqMap.erase(it);
	// �ֱⵥ���Ͱ� ���������� �˸���.
	if (chart_data) {
		if (nRepeatCnt == chart_data->CycleDataSize()) {
			// ��Ʈ ������ ���� �ϷḦ �˸���.
			SmTimeSeriesServiceManager* tsSvcMgr = SmTimeSeriesServiceManager::GetInstance();
			tsSvcMgr->OnCompleteChartCycleData(req);
		}
		else {
			// ��Ʈ ������ ���� �ϷḦ �˸���.
			SmTimeSeriesServiceManager* tsSvcMgr = SmTimeSeriesServiceManager::GetInstance();
			tsSvcMgr->OnCompleteChartData(req, chart_data);

			try
			{
				if (_SaveChartData) {
					std::async(std::launch::async, [chart_vec] {
						for (auto it = chart_vec.begin(); it != chart_vec.end(); ++it) {
							SmChartDataItem item = *it;
							SmMongoDBManager::GetInstance()->SaveChartDataItem(item);
						}
						});
				}
			}
			catch (std::exception e)
			{
				std::string error = e.what();
				LOG_F(INFO, "%s", error);
			}
		}
	}
}


BEGIN_MESSAGE_MAP(SmHdCtrl, CDialogEx)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(SmHdCtrl, CDialogEx)
	ON_EVENT(SmHdCtrl, (UINT)-1, 3, OnDataRecv, VTS_BSTR VTS_I4)
	ON_EVENT(SmHdCtrl, (UINT)-1, 4, OnGetBroadData, VTS_BSTR VTS_I4)
	ON_EVENT(SmHdCtrl, (UINT)-1, 5, OnGetMsg, VTS_BSTR VTS_BSTR)
	ON_EVENT(SmHdCtrl, (UINT)-1, 6, OnGetMsgWithRqId, VTS_I4 VTS_BSTR VTS_BSTR)

END_EVENTSINK_MAP()

// VtHdCtrl message handlers

void SmHdCtrl::OnDataRecv(CString sTrCode, LONG nRqID)
{
	if (sTrCode == DefAbChartData) {
		OnRcvdAbroadChartData(sTrCode, nRqID);
	}
	else if (sTrCode == DefAbSiseData) {
		OnRcvdAbroadSiseByReq(sTrCode, nRqID);
	}
	else if (sTrCode == DefAbHogaData) {
		OnRcvdAbroadHogaByReq(sTrCode, nRqID);
	}
	else if (sTrCode == DefAbsChartData2) {
		OnRcvdAbroadChartData2(sTrCode, nRqID);
	}
	else if (sTrCode == DefChartData) {
		OnRcvdDomesticChartData(sTrCode, nRqID);
	}
	else if (sTrCode == DefSymbolMaster) {
		OnDomesticSymbolMaster(sTrCode, nRqID);
	}
	else if (sTrCode == "v90001")
	{
		// �������� ��û ���.
		long nFileSize = atol(m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "����ũ��"));
		CString strMsg;
		strMsg.Format("����ũ��[%d]", nFileSize);
		//WriteLog(strMsg);


		CString strFileNm = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "���ϸ�");
		CString strProcCd = m_CommAgent.CommGetData(sTrCode, -1, "OutRec1", 0, "�����ڵ�");

		if (strProcCd == "REOK")
		{
			TCHAR iniFileName[500];

			GetModuleFileNameA(NULL, iniFileName, MAX_PATH);
			CString strFileName;
			const CString strExeName = "SmClient.exe";
			strFileName.Format("%s", iniFileName);
			int nDel = strExeName.GetLength();
			int nFind = strFileName.Find(strExeName);
			strFileName.Delete(nFind, nDel);
			strFileName = strFileName + "mst/" + strFileNm;

			CString strCommonFileName = strFileName;

			CFile commonfile;
			// open file
			if (!commonfile.Open(strCommonFileName, CFile::modeWrite /*| CFile::typeBinary*/))
			{
				if (commonfile.Open(strCommonFileName, CFile::modeCreate | CFile::modeWrite /*| CFile::typeBinary*/) == FALSE)
				{
					CString strMsg;
					strMsg.Format("%sȭ�� ������ �����Ͽ����ϴ�. ", strCommonFileName);
					LOG_F(INFO, "symbol file download error :: file_name = %s", strMsg);
					return;
				}
			}

			CString strBuff = m_CommAgent.CommGetDataDirect(sTrCode, -1, 128 + 4 + 8, nFileSize, 0, "A");
			commonfile.Write(strBuff, nFileSize);
			commonfile.Close();

			LOG_F(INFO, "symbol file download success :: file_name = %s", strFileNm);

			auto it = SmSymbolReader::GetInstance()->DomesticSymbolMasterFileSet.find((LPCTSTR)strFileNm);
			if (it != SmSymbolReader::GetInstance()->DomesticSymbolMasterFileSet.end()) {
				SmSymbolReader::GetInstance()->DomesticSymbolMasterFileSet.erase(it);
			}

			_FileDownloading = false;
		}
	}
}

void SmHdCtrl::OnGetBroadData(CString strKey, LONG nRealType)
{
	if (!_Client)
		return;
	switch (nRealType)
	{
	case 76: // hoga
		OnRcvdAbroadHoga(strKey, nRealType);
		break;
	case 82: // sise
		OnRcvdAbroadSise(strKey, nRealType);
		break;
	case 51:	// ����ȣ��
	{
		OnFutureHoga(strKey, nRealType);
	}
	break;
	case 52:	// �ɼ�ȣ��
	{
		OnOptionHoga(strKey, nRealType);
	}
	break;
	case 58: //��ǰ����ȣ���ǽð�
	{
		OnProductHoga(strKey, nRealType);
	}
	break;
	case 65:	// ����ü��
	{
		OnRealFutureQuote(strKey, nRealType);
	}
	break;
	case 71:	// ��ǰ����ü��
	{
		OnRealProductQuote(strKey, nRealType);
	}
	break;
	case 66:	// �ɼ�ü��
	{
		OnRealOptionQuote(strKey, nRealType);
	}
	break;
	default:
		break;
	}
}


void SmHdCtrl::OnFutureHoga(CString& strKey, LONG& nRealType)
{
	SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
	CString strSymCode = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�����ڵ�");
	SmSymbol* sym = symMgr->FindSymbol((LPCTSTR)strSymCode.Trim());
	if (!sym)
		return;
	CString strHogaTime = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ȣ���ð�");

	HdHoga hoga;
	hoga.Items[0].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ��1");
	hoga.Items[0].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ��1");
	hoga.Items[0].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ������1");
	hoga.Items[0].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ������1");
	hoga.Items[0].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ǽ�1");
	hoga.Items[0].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ǽ�1");

	hoga.Items[1].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ��2");
	hoga.Items[1].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ��2");
	hoga.Items[1].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ������2");
	hoga.Items[1].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ������2");
	hoga.Items[1].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ǽ�2");
	hoga.Items[1].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ǽ�2");

	hoga.Items[2].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ��3");
	hoga.Items[2].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ��3");
	hoga.Items[2].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ������3");
	hoga.Items[2].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ������3");
	hoga.Items[2].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ǽ�3");
	hoga.Items[2].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ǽ�3");

	hoga.Items[3].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ��4");
	hoga.Items[3].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ��4");
	hoga.Items[3].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ������4");
	hoga.Items[3].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ������4");
	hoga.Items[3].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ǽ�4");
	hoga.Items[3].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ǽ�4");

	hoga.Items[4].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ��5");
	hoga.Items[4].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ��5");
	hoga.Items[4].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ������5");
	hoga.Items[4].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ������5");
	hoga.Items[4].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ǽ�5");
	hoga.Items[4].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ǽ�5");

	CString strTotBuyQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ѽ���");
	CString strTotSellQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ѽ���");
	CString strTotBuyCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���ѰǼ�");
	CString strTotSellCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���ѰǼ�");
	//CString strDomDate = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "��������");
	CString strDomTime = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ȣ���ð�");
	SmHoga hoga_data;
	hoga_data.SymbolCode = strSymCode;
	for (int i = 0; i < 5; i++) {
		hoga_data.Ary[i].BuyPrice = sym->Hoga.Ary[i].BuyPrice = _ttoi(hoga.Items[i].strBuyHoga);
		hoga_data.Ary[i].BuyCnt = sym->Hoga.Ary[i].BuyCnt = _ttoi(hoga.Items[i].strBuyHogaCnt);
		hoga_data.Ary[i].BuyQty = sym->Hoga.Ary[i].BuyQty = _ttoi(hoga.Items[i].strBuyHogaQty);
		hoga_data.Ary[i].SellPrice = sym->Hoga.Ary[i].SellPrice = _ttoi(hoga.Items[i].strSellHoga);
		hoga_data.Ary[i].SellCnt = sym->Hoga.Ary[i].SellCnt = _ttoi(hoga.Items[i].strSellHogaCnt);
		hoga_data.Ary[i].SellQty = sym->Hoga.Ary[i].SellQty = _ttoi(hoga.Items[i].strSellHogaQty);
	}

	//hoga_data.DomesticDate = sym->Hoga.DomesticDate = strDomDate;
	hoga_data.DomesticTime = sym->Hoga.DomesticTime = strDomTime;
	hoga_data.Time = sym->Hoga.Time = strHogaTime;
	hoga_data.TotBuyCnt = sym->Hoga.TotBuyCnt = _ttoi(strTotBuyCnt);
	hoga_data.TotBuyQty = sym->Hoga.TotBuyQty = _ttoi(strTotBuyQty);
	hoga_data.TotSellCnt = sym->Hoga.TotSellCnt = _ttoi(strTotSellCnt);
	hoga_data.TotSellQty = sym->Hoga.TotSellQty = _ttoi(strTotSellQty);

	SmMongoDBManager* mongoMgr = SmMongoDBManager::GetInstance();
	mongoMgr->SaveHoga(hoga_data);

	SmSessionManager* sessMgr = SmSessionManager::GetInstance();
	sessMgr->SendReqUpdateHoga(sym);

	//TRACE(sym->Hoga.Time.c_str());

	SmTimeSeriesDBManager* dbMgr = SmTimeSeriesDBManager::GetInstance();
	//dbMgr->SaveHogaItem(std::move(hoga_data));

	CString msg;
	msg.Format(_T("hoga :: time = %s, tot_buy_cnt = %d\n"), sym->Hoga.SymbolCode.c_str(), sym->Hoga.TotBuyCnt);
	//TRACE(msg);
}

void SmHdCtrl::OnOptionHoga(CString& strKey, LONG& nRealType)
{
	SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
	CString strSymCode = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�����ڵ�");
	SmSymbol* sym = symMgr->FindSymbol((LPCTSTR)strSymCode.Trim());
	if (!sym)
		return;
	CString strHogaTime = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ȣ���ð�");

	HdHoga hoga;
	hoga.Items[0].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ��1");
	hoga.Items[0].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ��1");
	hoga.Items[0].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ������1");
	hoga.Items[0].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ������1");
	hoga.Items[0].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ǽ�1");
	hoga.Items[0].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ǽ�1");

	hoga.Items[1].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ��2");
	hoga.Items[1].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ��2");
	hoga.Items[1].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ������2");
	hoga.Items[1].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ������2");
	hoga.Items[1].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ǽ�2");
	hoga.Items[1].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ǽ�2");

	hoga.Items[2].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ��3");
	hoga.Items[2].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ��3");
	hoga.Items[2].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ������3");
	hoga.Items[2].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ������3");
	hoga.Items[2].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ǽ�3");
	hoga.Items[2].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ǽ�3");

	hoga.Items[3].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ��4");
	hoga.Items[3].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ��4");
	hoga.Items[3].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ������4");
	hoga.Items[3].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ������4");
	hoga.Items[3].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ǽ�4");
	hoga.Items[3].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ǽ�4");

	hoga.Items[4].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ��5");
	hoga.Items[4].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ��5");
	hoga.Items[4].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ������5");
	hoga.Items[4].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ������5");
	hoga.Items[4].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ǽ�5");
	hoga.Items[4].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ǽ�5");

	CString strTotBuyQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���Ѽ���");
	CString strTotSellQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���Ѽ���");
	CString strTotBuyCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ż�ȣ���ѰǼ�");
	CString strTotSellCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ŵ�ȣ���ѰǼ�");
	//CString strDomDate = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "��������");
	CString strDomTime = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ȣ���ð�");
	SmHoga hoga_data;
	hoga_data.SymbolCode = strSymCode;
	for (int i = 0; i < 5; i++) {
		hoga_data.Ary[i].BuyPrice = sym->Hoga.Ary[i].BuyPrice = _ttoi(hoga.Items[i].strBuyHoga);
		hoga_data.Ary[i].BuyCnt = sym->Hoga.Ary[i].BuyCnt = _ttoi(hoga.Items[i].strBuyHogaCnt);
		hoga_data.Ary[i].BuyQty = sym->Hoga.Ary[i].BuyQty = _ttoi(hoga.Items[i].strBuyHogaQty);
		hoga_data.Ary[i].SellPrice = sym->Hoga.Ary[i].SellPrice = _ttoi(hoga.Items[i].strSellHoga);
		hoga_data.Ary[i].SellCnt = sym->Hoga.Ary[i].SellCnt = _ttoi(hoga.Items[i].strSellHogaCnt);
		hoga_data.Ary[i].SellQty = sym->Hoga.Ary[i].SellQty = _ttoi(hoga.Items[i].strSellHogaQty);
	}

	//hoga_data.DomesticDate = sym->Hoga.DomesticDate = strDomDate;
	hoga_data.DomesticTime = sym->Hoga.DomesticTime = strDomTime;
	hoga_data.Time = sym->Hoga.Time = strHogaTime;
	hoga_data.TotBuyCnt = sym->Hoga.TotBuyCnt = _ttoi(strTotBuyCnt);
	hoga_data.TotBuyQty = sym->Hoga.TotBuyQty = _ttoi(strTotBuyQty);
	hoga_data.TotSellCnt = sym->Hoga.TotSellCnt = _ttoi(strTotSellCnt);
	hoga_data.TotSellQty = sym->Hoga.TotSellQty = _ttoi(strTotSellQty);

	SmMongoDBManager* mongoMgr = SmMongoDBManager::GetInstance();
	mongoMgr->SaveHoga(hoga_data);

	SmSessionManager* sessMgr = SmSessionManager::GetInstance();
	sessMgr->SendReqUpdateHoga(sym);

	//TRACE(sym->Hoga.Time.c_str());

	SmTimeSeriesDBManager* dbMgr = SmTimeSeriesDBManager::GetInstance();
	//dbMgr->SaveHogaItem(std::move(hoga_data));

	CString msg;
	msg.Format(_T("hoga :: time = %s, tot_buy_cnt = %d\n"), sym->Hoga.SymbolCode.c_str(), sym->Hoga.TotBuyCnt);
	//TRACE(msg);
}

void SmHdCtrl::OnProductHoga(CString& strKey, LONG& nRealType)
{
	CString strSymCode = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "kfutcode");

	SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
	SmSymbol* sym = symMgr->FindSymbol((LPCTSTR)strSymCode.Trim());
	if (!sym)
		return;

	CString strHogaTime = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "hotime");

	HdHoga hoga;
	hoga.Items[0].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "bidho1");
	hoga.Items[0].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "offerho1");
	hoga.Items[0].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "bidrem1");
	hoga.Items[0].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "offerrem1");
	hoga.Items[0].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "bidcnt1");
	hoga.Items[0].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "offercnt1");

	hoga.Items[1].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "bidho2");
	hoga.Items[1].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "offerho2");
	hoga.Items[1].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "bidrem2");
	hoga.Items[1].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "offerrem2");
	hoga.Items[1].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "bidcnt2");
	hoga.Items[1].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "offercnt2");

	hoga.Items[2].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "bidho3");
	hoga.Items[2].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "offerho3");
	hoga.Items[2].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "bidrem3");
	hoga.Items[2].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "offerrem3");
	hoga.Items[2].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "bidcnt3");
	hoga.Items[2].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "offercnt3");

	hoga.Items[3].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "bidho4");
	hoga.Items[3].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "offerho4");
	hoga.Items[3].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "bidrem4");
	hoga.Items[3].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "offerrem4");
	hoga.Items[3].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "bidcnt4");
	hoga.Items[3].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "offercnt4");

	hoga.Items[4].strBuyHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "bidho5");
	hoga.Items[4].strSellHoga = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "offerho5");
	hoga.Items[4].strBuyHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "bidrem5");
	hoga.Items[4].strSellHogaQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "offerrem5");
	hoga.Items[4].strBuyHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "bidcnt5");
	hoga.Items[4].strSellHogaCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "offercnt5");


	CString strTotBuyQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "totbidrem");
	CString strTotSellQty = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "totofferrem");
	CString strTotBuyCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "totbidcnt");
	CString strTotSellCnt = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "totoffercnt");

	SmHoga hoga_data;
	hoga_data.SymbolCode = strSymCode;
	for (int i = 0; i < 5; i++) {
		hoga_data.Ary[i].BuyPrice = sym->Hoga.Ary[i].BuyPrice = _ttoi(hoga.Items[i].strBuyHoga);
		hoga_data.Ary[i].BuyCnt = sym->Hoga.Ary[i].BuyCnt = _ttoi(hoga.Items[i].strBuyHogaCnt);
		hoga_data.Ary[i].BuyQty = sym->Hoga.Ary[i].BuyQty = _ttoi(hoga.Items[i].strBuyHogaQty);
		hoga_data.Ary[i].SellPrice = sym->Hoga.Ary[i].SellPrice = _ttoi(hoga.Items[i].strSellHoga);
		hoga_data.Ary[i].SellCnt = sym->Hoga.Ary[i].SellCnt = _ttoi(hoga.Items[i].strSellHogaCnt);
		hoga_data.Ary[i].SellQty = sym->Hoga.Ary[i].SellQty = _ttoi(hoga.Items[i].strSellHogaQty);
	}

	//hoga_data.DomesticDate = sym->Hoga.DomesticDate = strDomDate;
	hoga_data.DomesticTime = sym->Hoga.DomesticTime = strHogaTime;
	hoga_data.Time = sym->Hoga.Time = strHogaTime;
	hoga_data.TotBuyCnt = sym->Hoga.TotBuyCnt = _ttoi(strTotBuyCnt);
	hoga_data.TotBuyQty = sym->Hoga.TotBuyQty = _ttoi(strTotBuyQty);
	hoga_data.TotSellCnt = sym->Hoga.TotSellCnt = _ttoi(strTotSellCnt);
	hoga_data.TotSellQty = sym->Hoga.TotSellQty = _ttoi(strTotSellQty);

	SmMongoDBManager* mongoMgr = SmMongoDBManager::GetInstance();
	mongoMgr->SaveHoga(hoga_data);

	SmSessionManager* sessMgr = SmSessionManager::GetInstance();
	sessMgr->SendReqUpdateHoga(sym);

	//TRACE(sym->Hoga.Time.c_str());

	SmTimeSeriesDBManager* dbMgr = SmTimeSeriesDBManager::GetInstance();
	//dbMgr->SaveHogaItem(std::move(hoga_data));

	CString msg;
	msg.Format(_T("hoga :: code = %s, tot_buy_cnt = %d\n"), strSymCode, sym->Hoga.Ary[0].BuyPrice);
	TRACE(msg);
}


void SmHdCtrl::OnRealFutureQuote(CString& strKey, LONG& nRealType)
{
	CString strSymCode = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�����ڵ�");
	CString strTime = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ü��ð�");
	CString strSignToPreDay = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "����");
	CString strToPreDay = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "���ϴ��");
	CString strRatioToPreDay = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�����");
	CString strClose = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "���簡");
	CString strOpen = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ð�");
	CString strHigh = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "��");
	CString strLow = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "����");
	CString strVolume = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ü�ᷮ");
	CString strSign = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ü�ᱸ��");
	CString strAccVol = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�����ŷ���");

	int preDayRatio = _ttoi(strRatioToPreDay);

	SmQuote quoteItem;
	quoteItem.SymbolCode = strSymCode.Trim();
	quoteItem.OriginTime = strTime;
	if (preDayRatio > 0)
		quoteItem.SignToPreDay = "+";
	else if (preDayRatio < 0)
		quoteItem.SignToPreDay = "-";
	else
		quoteItem.SignToPreDay = "";
	strSignToPreDay = quoteItem.SignToPreDay.c_str();

	// ���� �׳� ������.
	quoteItem.GapFromPreDay = _ttoi(strToPreDay);
	// ������� �ۼ�Ʈ�� �ٲ㼭 ������.
	CString tempRatio;
	tempRatio.Format("%.2f", preDayRatio / 100.0);
	strRatioToPreDay = tempRatio;
	quoteItem.RatioToPreday = strRatioToPreDay;
	quoteItem.Close = _ttoi(strClose);
	quoteItem.Open = _ttoi(strOpen);
	quoteItem.High = _ttoi(strHigh);
	quoteItem.Low = _ttoi(strLow);
	quoteItem.Volume = _ttoi(strVolume);
	quoteItem.Sign = strSign.Trim();
	quoteItem.accVolume = _ttoi(strAccVol);


	SmMongoDBManager* mongoMgr = SmMongoDBManager::GetInstance();
	mongoMgr->SaveSise(quoteItem);
	SmTimeSeriesDBManager* tsDBMgr = SmTimeSeriesDBManager::GetInstance();
	tsDBMgr->SaveQuoteItem(std::move(quoteItem));

	SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
	SmSymbol* sym = symMgr->FindSymbol((LPCTSTR)strSymCode.Trim());
	if (!sym)
		return;
	sym->Quote.Close = _ttoi(strClose);
	sym->Quote.Open = _ttoi(strOpen);
	sym->Quote.High = _ttoi(strHigh);
	sym->Quote.Low = _ttoi(strLow);
	sym->Quote.OriginTime = strTime;
	sym->Quote.GapFromPreDay = _ttoi(strToPreDay);
	sym->Quote.RatioToPreday = strRatioToPreDay;
	sym->Quote.SignToPreDay = strSignToPreDay;
	sym->Quote.accVolume = _ttoi(strAccVol);

	SmSessionManager* sessMgr = SmSessionManager::GetInstance();
	sessMgr->SendReqUpdateQuote(sym);

	CString msg;
	//msg.Format(_T("symbol = %s, time = %s, h=%s, l=%s, o=%s, c=%s, v=%s, ratio = %s\n"), strSymCode, strTime, strHigh, strLow, strOpen, strClose, strVolume, strRatioToPreDay);
	//TRACE(msg);
}

void SmHdCtrl::OnRealOptionQuote(CString& strKey, LONG& nRealType)
{
	CString strSymCode = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�����ڵ�");
	CString strTime = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ü��ð�");
	CString strSignToPreDay = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "����");
	CString strToPreDay = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "���ϴ��");
	CString strRatioToPreDay = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�����");
	CString strClose = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "���簡");
	CString strOpen = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�ð�");
	CString strHigh = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "��");
	CString strLow = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "����");
	CString strVolume = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ü�ᷮ");
	CString strSign = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ü�ᱸ��");
	CString strAccVol = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�����ŷ���");

	int signToPreDay = _ttoi(strSignToPreDay);

	SmQuote quoteItem;
	quoteItem.SymbolCode = strSymCode.Trim();
	quoteItem.OriginTime = strTime;
	if (signToPreDay == 2)
		quoteItem.SignToPreDay = "+";
	else if (signToPreDay == 5)
		quoteItem.SignToPreDay = "-";
	else
		quoteItem.SignToPreDay = "";
	quoteItem.GapFromPreDay = _ttoi(strToPreDay);
	quoteItem.RatioToPreday = strRatioToPreDay.Trim();
	quoteItem.Close = _ttoi(strClose);
	quoteItem.Open = _ttoi(strOpen);
	quoteItem.High = _ttoi(strHigh);
	quoteItem.Low = _ttoi(strLow);
	quoteItem.Volume = _ttoi(strVolume);
	quoteItem.Sign = strSign.Trim();
	quoteItem.accVolume = _ttoi(strAccVol);


	SmMongoDBManager * mongoMgr = SmMongoDBManager::GetInstance();
	mongoMgr->SaveSise(quoteItem);
	SmTimeSeriesDBManager * tsDBMgr = SmTimeSeriesDBManager::GetInstance();
	tsDBMgr->SaveQuoteItem(std::move(quoteItem));

	SmSymbolManager * symMgr = SmSymbolManager::GetInstance();
	SmSymbol * sym = symMgr->FindSymbol((LPCTSTR)strSymCode.Trim());
	if (!sym)
		return;
	sym->Quote.Close = _ttoi(strClose);
	sym->Quote.Open = _ttoi(strOpen);
	sym->Quote.High = _ttoi(strHigh);
	sym->Quote.Low = _ttoi(strLow);
	sym->Quote.OriginTime = strTime;
	sym->Quote.GapFromPreDay = _ttoi(strToPreDay);
	sym->Quote.RatioToPreday = strRatioToPreDay.Trim();
	sym->Quote.SignToPreDay = strSignToPreDay;
	sym->Quote.accVolume = _ttoi(strAccVol);

	SmSessionManager * sessMgr = SmSessionManager::GetInstance();
	sessMgr->SendReqUpdateQuote(sym);

	CString msg;
	//msg.Format(_T("symbol = %s, time = %s, h=%s, l=%s, o=%s, c=%s, v=%s, ratio = %s\n"), strSymCode, strTime, strHigh, strLow, strOpen, strClose, strVolume, strRatioToPreDay);
	//TRACE(msg);
}

void SmHdCtrl::OnRealProductQuote(CString& strKey, LONG& nRealType)
{
	CString strSymCode = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�����ڵ�");
	CString strTime = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "ü��ð�");
	CString strSignToPreDay = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "sign");
	CString strToPreDay = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "change");
	CString strRatioToPreDay = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "drate");
	CString strClose = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "���簡");
	CString strOpen = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "open");
	CString strHigh = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "high");
	CString strLow = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "low");
	CString strVolume = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "cvolume");
	CString strSign = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "cgubun");
	CString strAccVol = m_CommAgent.CommGetData(strKey, nRealType, "OutRec1", 0, "�����ŷ���");

	int preday_ratio = _ttoi(strRatioToPreDay);
	SmQuote quoteItem;
	quoteItem.SymbolCode = strSymCode.Trim();
	quoteItem.OriginTime = strTime;
	if (preday_ratio > 0)
		quoteItem.SignToPreDay = "+";
	else if (preday_ratio < 0)
		quoteItem.SignToPreDay = "-";
	else
		quoteItem.SignToPreDay = "";
	// ���� �׳� ������.
	quoteItem.GapFromPreDay = _ttoi(strToPreDay);
	// ������� �ۼ�Ʈ�� �ٲ㼭 ������.
	CString tempRatio;
	tempRatio.Format("%.2f", preday_ratio / 100.0);
	strRatioToPreDay = tempRatio;
	quoteItem.RatioToPreday = strRatioToPreDay;
	quoteItem.Close = _ttoi(strClose);
	quoteItem.Open = _ttoi(strOpen);
	quoteItem.High = _ttoi(strHigh);
	quoteItem.Low = _ttoi(strLow);
	quoteItem.Volume = _ttoi(strVolume);
	quoteItem.Sign = strSign.Trim();
	quoteItem.accVolume = _ttoi(strAccVol);


	SmMongoDBManager * mongoMgr = SmMongoDBManager::GetInstance();
	mongoMgr->SaveSise(quoteItem);
	SmTimeSeriesDBManager * tsDBMgr = SmTimeSeriesDBManager::GetInstance();
	tsDBMgr->SaveQuoteItem(std::move(quoteItem));

	SmSymbolManager * symMgr = SmSymbolManager::GetInstance();
	SmSymbol * sym = symMgr->FindSymbol((LPCTSTR)strSymCode.Trim());
	if (!sym)
		return;
	sym->Quote.Close = _ttoi(strClose);
	sym->Quote.Open = _ttoi(strOpen);
	sym->Quote.High = _ttoi(strHigh);
	sym->Quote.Low = _ttoi(strLow);
	sym->Quote.OriginTime = strTime;
	sym->Quote.GapFromPreDay = _ttoi(strToPreDay);
	sym->Quote.RatioToPreday = strRatioToPreDay;
	sym->Quote.SignToPreDay = strSignToPreDay;
	sym->Quote.accVolume = _ttoi(strAccVol);

	SmSessionManager * sessMgr = SmSessionManager::GetInstance();
	sessMgr->SendReqUpdateQuote(sym);

	CString msg;
	//msg.Format(_T("symbol = %s, time = %s, h=%s, l=%s, o=%s, c=%s, v=%s, ratio = %s\n"), strSymCode, strTime, strHigh, strLow, strOpen, strClose, strVolume, strRatioToPreDay);
	//TRACE(msg);
}


void SmHdCtrl::OnGetMsg(CString strCode, CString strMsg)
{
	int i = 0;
}

void SmHdCtrl::OnGetMsgWithRqId(int nRqId, CString strCode, CString strMsg)
{
	try
	{
		CString msg;
		msg.Format(_T("req_id = %d, hd_server_code = %s, hd_server_msg = %s\n"), nRqId, strCode, strMsg);
		TRACE(msg);

		LOG_F(INFO, "OnGetMsgWithRqId %s", msg);

		if (strCode.CompareNoCase(_T("0332")) == 0) {
			//AfxMessageBox(_T("���� �ٿ�ε� �Ϸ�"));
			SmMongoDBManager* mongo = SmMongoDBManager::GetInstance();
			mongo->ReadSymbol();
		}
	}
	catch (std::exception e)
	{
		std::string error_msg = e.what();
		LOG_F(INFO, "Exception OnGetMsgWithRqId %s", error_msg.c_str());
	}
}