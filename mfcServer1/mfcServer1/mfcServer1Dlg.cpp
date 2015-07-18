
// mfcServer1Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "mfcServer1.h"
#include "mfcServer1Dlg.h"
#include "afxdialogex.h"
#include "iostream"
#include <string>
#include "md5.h"
using namespace std;

char packageData[MAX_PACKAGE_NUM][2*PACKAGE_SIZE+1];	//������Ҫ���͵��ļ�����

CString TYPE[30] = {TYPE_ChatMsg , TYPE_Server_is_closed , TYPE_UserList , TYPE_OnlineState , TYPE_FileSend , TYPE_FileData , TYPE_AskFileData , TYPE_File_NO , TYPE_File_Over , TYPE_File_Fail , TYPE_LoginFail , TYPE_UserIsOnline , TYPE_OfflineMsg , TYPE_AllUser , TYPE_AddUserList , TYPE_I_am_online , TYPE_Logout , TYPE_Login , TYPE_Register , TYPE_Status};
CString STR[5] = {"@@@","<<<",">>>","&&&","###"};//�û���+����+����+ȥ��+����+����

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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


// CmfcServer1Dlg �Ի���

CmfcServer1Dlg::CmfcServer1Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CmfcServer1Dlg::IDD, pParent)
	, m_port(22783)
{
	m_connect = 0;
	//���û���Ϣ�����û���
	fstream file(DATASRC, ios::out | ios::_Noreplace);
	file.close();
	CreateDirectory(OLMSG,0);
	fstream src(DATASRC,ios::in);
	userNum = 0;
	userList = "";
	while (!src.eof()){
		src >> userInfo[userNum].User >> userInfo[userNum].Pw;
		if(0==userInfo[userNum].User[0] || 0==userInfo[userNum].Pw[0])
			break;
		userInfo[userNum].Online = userInfo[userNum].refuse = 0;
		userList += userInfo[userNum].User;
		userList += ";";
		CStdioFile ( OLMSG+"//"+CString(userInfo[userNum].User),CFile::modeCreate|CFile::modeNoTruncate);
		CreateFile(userInfo[userNum].User,0,0,0,0,0,0);
		userNum ++;
	}
	src.close();
	GetOnlineNum();
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);//(IDR_MAINFRAME);
	DeleteFile("ServerMsg.txt");
	sft.fileSendOver = 1;	//Ĭ��û����Ҫת�����ļ�
}

CmfcServer1Dlg::~CmfcServer1Dlg(){
	Shell_NotifyIcon(NIM_DELETE, &nd);
}
void CmfcServer1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PortId, m_port);
	DDX_Text(pDX, IDC_userC, m_userOnlineCount);
	DDX_Control(pDX, IDC_EDIT2, m_event);
}

#pragma region MessageMap
BEGIN_MESSAGE_MAP(CmfcServer1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OpenServer, &CmfcServer1Dlg::OnOpenServer)
	ON_MESSAGE(WM_NOTIFYICONMSG, OnNotifyIconMsg)
	ON_COMMAND(ID_hide, &CmfcServer1Dlg::OnHide)
	ON_COMMAND(ID_exit, &CmfcServer1Dlg::OnExit)
	ON_COMMAND(ID_show, &CmfcServer1Dlg::OnShow)
	ON_WM_CLOSE()
	ON_WM_DROPFILES()
	ON_WM_TIMER()
END_MESSAGE_MAP()
#pragma endregion

//�������IP
#include <afxinet.h>
void GetOutIP() 
{
    CInternetSession session;
    CInternetFile * httpfile=(CInternetFile *)session.OpenURL("http://1111.ip138.com/ic.asp");
    CString content;
    CString Data;      //��ŷ��ص���ҳ����
    while(httpfile->ReadString(content))
    {
        Data+=content;
    }
    int start=Data.Find("[");
    int end=Data.Find("]");
    int count=end-start;
    Data=Data.Mid(start+1,count-1);
    MessageBox(0,"����ip: "+Data,"",0);
    httpfile->Close();
    session.Close();    
}
// CmfcServer1Dlg ��Ϣ��������

BOOL CmfcServer1Dlg::OnInitDialog()
{
	//GetOutIP();
	HANDLE hmutex = ::CreateMutex(NULL, true, "myMFCServer1");	//����������󲢷�������
	if (hmutex) {
		if (ERROR_ALREADY_EXISTS == GetLastError()) {
			MessageBox("��ǰ�������������У�ֻ����һ���������������У�", "���г���",MB_SETFOREGROUND);
			exit(-1);
		}
	}
	ReleaseMutex(hmutex);

	CDialogEx::OnInitDialog();

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, ID__clear, "��ռ�¼");
		/*
		// ��������...���˵������ӵ�ϵͳ�˵��С�
		// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�

		ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
		ASSERT(IDM_ABOUTBOX < 0xF000);
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
		*/
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ����Ӷ���ĳ�ʼ������
	LONG style = ::GetWindowLong(this->GetSafeHwnd(), GWL_STYLE);
	style &= ~WS_THICKFRAME;//ʹ���ڲ��������ı��С
	::SetWindowLong(this->GetSafeHwnd(), GWL_STYLE, style);

	HWND h = ::CreateStatusWindow(WS_CHILD | WS_VISIBLE, "��ӭʹ�ñ�����!", this->m_hWnd, 0);
	::SendMessage(h, SB_SETBKCOLOR, 1, RGB(0, 120, 200));
	nd.cbSize = sizeof(NOTIFYICONDATA);
	nd.hWnd = m_hWnd;
	nd.uID = IDI_ICON1;
	nd.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nd.uCallbackMessage = WM_NOTIFYICONMSG;
	nd.hIcon = m_hIcon;
	sprintf_s(nd.szTip, "������");
	Shell_NotifyIcon(NIM_ADD, &nd);

	OnOpenServer();
	SetForegroundWindow();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CmfcServer1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if (nID == ID__clear){
		OnClearLog();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի���������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CmfcServer1Dlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CmfcServer1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CmfcServer1Dlg::OnOpenServer()
{
	static bool first = 1;
	for (int i = 0; i<userNum; ++i){
		userInfo[i].Online = 0;
	}
	GetOnlineNum();
	if (m_connect)
	{
		SendCloseMsg();
		sprintf_s(nd.szTip, "������ - �ѹر�");	//�޸�֪ͨ��ͼ����ʾ����
		Shell_NotifyIcon(NIM_MODIFY, &nd);
		UpdateData(false);
		return;
	}
	listenSocket = new CServerSocket();
	listenSocket->m_pDlg = this;// ָ���Ի���Ϊ���Ի��򣬲����������
	UpdateData(true);
	if (!listenSocket->Create(m_port, SOCK_STREAM))	// �������������׽���
	{
		if(!first)
			AfxMessageBox(_T("�����׽��ִ���"));
		first = 0;	//���ܻ�Ϊelse,������������ʱ���Ὣfirst����Ϊ0
		listenSocket->Close();
		return;
	}
	//�ڷ������ݵ�ʱ����ִ����ϵͳ��������socket�������Ŀ���������߳��������
	int nSize = DATA_BUF_SIZE;//���û�������С
	setsockopt(*listenSocket,SOL_SOCKET,SO_SNDBUF,(const char*)&nSize,sizeof(int));
	setsockopt(*listenSocket,SOL_SOCKET,SO_RCVBUF,(const char*)&nSize,sizeof(int));
	////��ֹNagle�㷨����ͨ����δȷ�ϵ����ݴ��뻺����ֱ������һ����һ���͵ķ������������������͵�����С���ݰ�����Ŀ��
	/*bool b_noDely = 1;
	setsockopt( *listenSocket, SOL_SOCKET, TCP_NODELAY, ( char * )&b_noDely, sizeof( b_noDely ) );*/
	if (!listenSocket->Listen(UserNumMax))
	{
		CString err;
		err.Format("������룺%d",GetLastError());
		if(!first)
			AfxMessageBox(_T("����ʧ�ܣ�"+err));
		first = 0;
		listenSocket->Close();
		return;
	}
	first = 0;
	m_connect = true;
	GetDlgItem(IDC_PortId)->EnableWindow(0);
	GetDlgItem(IDC_IPADDRESS1)->EnableWindow(0);
	UpdateData(false);
	SetDlgItemText(IDC_OpenServer, _T("�رշ�����"));
	UpdateEvent(_T("ϵͳ�򿪷�����."));
	sprintf_s(nd.szTip, "������ - �Ѵ�\r\n�˿ںţ�%d", m_port);
	Shell_NotifyIcon(NIM_MODIFY, &nd);
}

//���������û�����Ӧ�û�����
/*˵������������CServerSocket���е�OnAccept��Ϣ�е��ã�������Ӧ�û�
	���ӷ�����������	��Ҫ����ΪAccept�������ӳɹ���ͨ������
	m_clientList�������û���������־�������û����͡�Hello����ʾ��ӭ��*/
void CmfcServer1Dlg::AddClient()
{
	CServerSocket* pSocket = new CServerSocket;
	pSocket->m_pDlg = this;
	listenSocket->Accept(*pSocket);
	pSocket->AsyncSelect(FD_READ | FD_WRITE | FD_CLOSE);
	m_clientList.AddTail(pSocket);
}

//�����Ƴ����ߵ��û�
/*˵������������CServerSocket���е�OnClose��Ϣ�е��ã��õ�POSITION�ṹ��
	���Ҵ洢�û�����λ�û������ˣ��������û��ͷţ���������ɾ������������־��*/
void CmfcServer1Dlg::RemoveClient(CServerSocket* pSocket)
{
	POSITION nPos = m_clientList.GetHeadPosition();
	POSITION nTmpPos = nPos;
	while (nPos)
	{
		CServerSocket* pSockItem = (CServerSocket*)m_clientList.GetNext(nPos);
		if (pSockItem->m_hSocket == pSocket->m_hSocket)
		{
			pSockItem->Close();
			delete pSockItem;
			m_clientList.RemoveAt(nTmpPos);
			break;
		}
		nTmpPos = nPos;
	}
}

// ��֤�û����������Ƿ���Ч,CheckOnlineΪ1ʱ��ʾ�������״̬,onlyUserΪ1ʱ��ʾֻ�����û��Ƿ����,����֤����
int CmfcServer1Dlg::UserInfoValid(bool CheckOnline,bool onlyUser, CString checkUser)
{
	if(checkUser=="")
		checkUser = mymsg.userId;
	for (int i = 0; i < userNum; ++i)
	{
		if (checkUser == userInfo[i].User && (onlyUser || mymsg.pw == userInfo[i].Pw)){	//�ҵ����û�����Ϣ
			if (CheckOnline){	//����Ƿ�����
				if (userInfo[i].Online)
					return i;
				else
					return -1;
			}
			else
				return i;
		}
	}
	return -1;
}

bool CmfcServer1Dlg::isOnline(CString _user)
{
	if(_user=="������" || _user=="����������" || _user==TYPE[AllUser])
		return 1;
	for (int i = 0; i < userNum && i<UserNumMax; ++i)
	{
		if(userInfo[i].User == _user){
			if (userInfo[i].Online)
				return 1;
			else
				return 0;
		}
	}
	return 0;
}

//���ڻ�ȡ����
void CmfcServer1Dlg::ReceData(CServerSocket* pSocket)
{
	static char pData[DATA_BUF_SIZE];	//������յ�������
	memset(pData,0,sizeof(pData));
	static int saveMsg = atoi(AfxGetApp()->GetProfileString("ServerSetting","saveMsgLog","-1"));//���洫����־
	if(saveMsg==-1){
		AfxGetApp()->WriteProfileString("ServerSetting","saveMsgLog","0");
		saveMsg = 0;
	}
	static ofstream save_MSG("ServerMsg.txt");	//������Ϣ��¼���ļ�
	if (pSocket->Receive(pData, DATA_BUF_SIZE) != SOCKET_ERROR)
	{
		///MessageBox(pData, "����������");
		mymsg.load(pData);
		MyMsg msg = mymsg;
		if(saveMsg==1 && msg.type!=TYPE[I_am_online])
			save_MSG<<pData<<endl;
		if(msg.userId!="")
		{
#pragma region ���벻Ϊ��:��¼��ע����ע��
			if(msg.pw!="") //���벻Ϊ��
			{
				if(0){}
				elif(msg.type == TYPE[Login])
				{
					if(msg.userId=="CAPTURE")
					{
						SendMSG(msg.join(userList,TYPE[UserList]));
						UpdateEvent("_CAPUTURE_ come");
					}else
					{
						int re = UserInfoValid();	//��֤�û�����
						if (re!=-1){	//�û�����������Ч
							if (userInfo[re].Online){	//���û��Ѿ�����
								SendMSG(msg.join("",TYPE[UserIsOnline]));
								RemoveClient(pSocket);
							}else if(userInfo[re].refuse){
								UpdateEvent(_T("�û�[" + msg.userId + "]�������ߣ����ܾ�."));
								return;
							}else{
								SendMSG(msg.join(userList,TYPE[UserList]));
								userInfo[re].Online = 1;
								GetOnlineNum();
								UpdateData(false);
								CString ss;
								ss.Format("%d",m_userOnlineCount);
								UpdateEvent(_T("�û�[" + msg.userId + "]����. �����û�����" + ss));
								char olMsg[1024];
								string _str;
								CString Msg;		//��������Ϣ����
								ifstream in(OLMSG+"//"+msg.userId,ios::in);
								while(!in.eof() && in.is_open()){
									getline(in,_str);
									strncpy_s(olMsg,_str.c_str(),1024);
									Msg += olMsg;
								}
								in.close();
								if(Msg!=""){
									Sleep(100);
									SendMSG(msg.join(Msg,TYPE[OfflineMsg]));
									DeleteFile(OLMSG+"//"+msg.userId);
								}
							}
						}
						else{
							SendMSG(msg.join("",TYPE[LoginFail]));
							RemoveClient(pSocket);
						}
					}
				}
				elif(msg.type == TYPE[Logout]){			//�û�ע��[user]@[pw]###logout
					POSITION nPos = m_clientList.GetHeadPosition();
					POSITION nTmpPos = nPos;
					while (nPos)
					{
						CServerSocket* pSockItem = (CServerSocket*)m_clientList.GetNext(nPos);
						if (pSockItem->m_hSocket == pSocket->m_hSocket)
						{
							pSockItem->Close();
							delete pSockItem;
							m_clientList.RemoveAt(nTmpPos);
							int re = UserInfoValid();
							if (re>=0)
								userInfo[re].Online = 0;	//����״̬��Ϊ0
							GetOnlineNum();
							UpdateData(false);
							UpdateEvent(_T("�û�[" + msg.userId + "]�뿪."));
							break;
						}
						nTmpPos = nPos;
					}
				}
				elif(msg.type == TYPE[Register])
				{			//�û�ע��[user]@[pw]###register
					if (UserInfoValid(1,1)>=0)	//���û���ǰ���ߣ��������ٴ�ע��
						return;
					int re = UserInfoValid(0,1);
					if (re>=0){					//���û������û��б��е������ߣ���д����
						strncpy_s(userInfo[re].Pw,msg.pw,17);
						UpdateEvent(_T("�û�[" + msg.userId + "]�޸�����."));
					}else{						//���û������ڣ����ӵ��û��б���
						strncpy_s(userInfo[userNum].User ,msg.userId,17);
						strncpy_s(userInfo[userNum].Pw ,msg.pw,17);
						userInfo[userNum].Online = userInfo[userNum].refuse = 0;
						userList += userInfo[userNum].User;
						userList += ";";
						CStdioFile ( OLMSG+"//"+CString(userInfo[userNum].User),CFile::modeCreate|CFile::modeNoTruncate);
						SendMSG(msg.join(userInfo[userNum].User,TYPE[AddUserList],TYPE[AllUser]));
						userNum ++;
						UpdateEvent(_T("�û�[" + msg.userId + "]ע��."));
					}
					fstream src(DATASRC,ios::out);	//�����û��б�д������Դ
					for (int i = 0; i<userNum; ++i)
						src << userInfo[i].User <<"\t"<< userInfo[i].Pw<<"\n";
					src.close();
				}
				else{
					//MessageBox("��Ϣ����ʧ�ܣ�����δ֪֪ͨ����"+msg,"��ܰ��ʾ");
					UpdateEvent("����δ֪��Ϣ����:" + msg.type);
				}
			}
#pragma endregion
			else //û�����벿��
			{
				if(0){}
				elif(msg.type==TYPE[OnlineState]){
					if(msg.data=="������" || msg.data=="����������")
						SendMSG(msg.join("1",TYPE[OnlineState]));
					elif (isOnline(msg.data))
						SendMSG(msg.join("1",TYPE[OnlineState]));
					else
						SendMSG(msg.join("0",TYPE[OnlineState]));
				}
				elif(msg.type==TYPE[ChatMsg]){
					if(msg.toUser=="����������"){
						SendMSG( msg.join(msg.data,TYPE[ChatMsg],TYPE[AllUser],"������-"+msg.userId) );
						UpdateEvent(msg.data, "�û�[" + msg.userId + "]��[������]\t");
						return;
					}
					UpdateEvent(msg.data, "�û�[" + msg.userId + "]��["+msg.toUser+"]\t");
					if(isOnline(msg.toUser)){
						SendMSG( msg.join(msg.data,TYPE[ChatMsg],msg.toUser,msg.userId) ); // ת�����ݸ�Ŀ���û�
						///Sleep(120);
						///if (msg.data.GetLength() > 12)
						///	msg.data = msg.data.Left(8) + "..." + msg.data.Right(8);
						///SendMSG(user + "@ [" + toUser + "]���յ���Ϣ��" + msg + CHAR6); // �������ݸ��������ݵ��û�
					}else{
						SendMSG( msg.join("["+msg.toUser+"]��ǰ�����ߣ���תΪ������Ϣ",TYPE[Status]) );// תΪ������Ϣ����
						CTime time = CTime::GetCurrentTime();	// ��ȡϵͳ��ǰʱ��
						ofstream out(OLMSG+"\\"+msg.toUser,ios::out|ios::app);
						out<<msg.join(msg.data,time.Format("%m-%d %H:%M:%S"),"$",msg.userId)<<endl;
						out.close();
					}
				}
				elif(msg.type==TYPE[I_am_online]) //�������Ʒ��͵���֤����
				{
					//�������⴦��
				}
				elif(msg.type==TYPE[FileSend] || msg.type==TYPE[AskFileData] || msg.type==TYPE[FileData])
				{
					FileTransfer(msg,pData);
				}
			}
			return;
		}
		UpdateEvent(pData,"unknown\t");
	}
}
//�����ļ�����Ϣ,����1��ʾ�����ɹ�
int  CmfcServer1Dlg::FileTransfer(MyMsg& msg,const char* pData)
{
	static int saveTrans = atoi(AfxGetApp()->GetProfileString("ServerSetting","saveTransLog","-1"));//���洫����־
	if(saveTrans==-1){
		AfxGetApp()->WriteProfileString("ServerSetting","saveTransLog","0");
		saveTrans = 0;
	}
	if(0){}
	elif(msg.type==TYPE[FileSend]){		//�����ļ����󣬿����Ƿ����������Ļ����Ǹ���һ���û���
		if(msg.toUser=="������")
		{
			SetForegroundWindow();
			fileSend(msg,1);
		}
		elif(isOnline(msg.toUser)){	//����Ƿ����ߡ������ߣ��ȴ浽����������ת�������û�
			sft.set(msg.toUser,msg.userId,0,msg.data);
			fileUser = msg.userId;
			int i = msg.data.Find('|');
			CString name = msg.data.Left(i);
			CString size = rightN(msg.data,i+1);
			i = size.Find('|');
			CString fileMD5 = rightN(size,i+1);
			size = size.Left(i);
			//Sleep(20);
			SendMSG( msg.join("0",TYPE[AskFileData],msg.userId,"������") );
			CreateDirectory("ServerFile",0);
			rf.init("ServerFile\\"+name,atoi(size),fileMD5);
			SetTimer(2,2000,0);
			fileSendName = name;
			//SendMSG( msg.join(msg.data,msg.type,msg.toUser,msg.userId) );
		}
	}
	elif(msg.type==TYPE[AskFileData])
	{
		if(msg.toUser!="������")
		{
			/*//����1��ת�������û��������������⣩
			if(isOnline(msg.toUser))
			{
				Sleep(10);
				SendMSG( msg.join(msg.data,msg.type,msg.toUser,msg.userId) );
			}*/
			//����2���ļ��������ȱ��浽�����������ڷ��͸����û�
			if(msg.toUser==sft.fileFromUser)
			{
				msg.toUser = "������";
			}
		}
		if(msg.toUser=="������") //����������
		{
			static fstream dataTrans;
			if(msg.data==TYPE[File_NO]){		//û�оܾ������ļ�
				UpdateEvent("["+msg.userId+"]�ܾ������ļ���"+fileSendName);
			}elif(msg.data==TYPE[File_Over]){
				UpdateEvent("["+msg.userId+"]�ѽ����ļ���"+fileSendName);
				if(saveTrans) dataTrans.close();
			}
			elif(msg.data==TYPE[File_Fail]){
				UpdateEvent("["+msg.userId+"]δ�ܳɹ������ļ���"+fileSendName);
				if(saveTrans) dataTrans.close();
			}
			else{			//���������ݰ�
				int dataIndex = atoi(msg.data);
				while(0==dataIndex && readFileEnd==0)
				{
					Sleep(10);
					static MSG msg1;
					if (PeekMessage(&msg1,(HWND)NULL, 0, 0, PM_REMOVE))  
					{  
						::SendMessage(msg1.hwnd, msg1.message, msg1.wParam, msg1.lParam);    
					}
				}	//�ȴ���ȡ�ļ����ݽ���
				if(dataIndex>=0 && dataIndex<MAX_PACKAGE_NUM)
				{
					if(saveTrans)
					{
						if(0==dataIndex)
							dataTrans.open("TransLog.txt",ios::out);
						dataTrans<<"�������ݰ� "<<dataIndex<<"\t  "<<strlen(packageData[dataIndex])<<endl;
					}
					SendMSG(msg.join(packageData[dataIndex],TYPE[FileData],msg.userId,"������"));
				}
			}
			if(sft.fileSendOver==0 && (msg.data==TYPE[File_NO] || msg.data==TYPE[File_Over] || msg.data==TYPE[File_Fail]))
				sendFileToOthers();
		}
	}
	elif(msg.type==TYPE[FileData]){		//�ļ�������Ҫ����
		if(msg.toUser=="������")		//��������������
		{
			if(rf.isRecving()){
				KillTimer(2);
				rf.addPacketage(strstr(pData,STR[4])+3);
				SetTimer(2,2000,0);
			}
		}else //ת����Ŀ���û��������ݴ浽������
		{
			//SendMSG( msg.join(strstr(pData,STR[4])+3,msg.type,msg.toUser,msg.userId) );
		}
	}
	else{
		MBox2(msg.type+":"+msg.data,"unknown");
		return 0;
	}
	return 1;
}
//���ڸ����¼���־
/*	˵������������������Ҫ������־�ĵط����е��ã�
	�����������¼�û��ĵ�¼���˳��¼���*/
void CmfcServer1Dlg::UpdateEvent(CString str,CString from)
{
	static bool firstEvent = 1;
	CString cstr;
	CTime time = CTime::GetCurrentTime();	// ��ȡϵͳ��ǰʱ��
	str += _T("\r\n");		// ���ڻ�����ʾ��־
	if (firstEvent){
		cstr = from + time.Format(_T("%Y/%m/%d %H:%M:%S  ")) + str;	// ��ʽ����ǰ���ں�ʱ��
		firstEvent = 0;
	}else
		cstr = from + time.Format(_T("%H:%M:%S  ")) + str;	// ��ʽ����ǰʱ��
	int lastLine = m_event.LineIndex(m_event.GetLineCount() - 1);//��ȡ�༭�����һ������
	m_event.SetSel(lastLine + 1, lastLine + 2, 0);	//ѡ��༭�����һ��
	m_event.ReplaceSel(cstr);                     //�滻��ѡ��һ�е�����
	ofstream history("history.txt",ios::out|ios::app);
	history<<cstr.GetBuffer();
	history.close();
	GetDlgItemText(IDC_EDIT2,cstr);
	if(cstr.GetLength()>10000){
		cstr = cstr.Right(1000);
		m_event.SetSel(lastLine + 1, lastLine + 2, 0);	//ѡ��༭�����һ��
		m_event.ReplaceSel(cstr);
	}
}

/*	���ڷ�����Ϣ�������ͻ���
	˵�������ͺ��������ڷ�����Ϣ�������û�����Ҫ����ΪSend����AddClient��
	RecvData�ж��е��ã�������ʱ���÷���Ϣ���û���*/
void CmfcServer1Dlg::SendMSG(CString str)
{
	POSITION nPos = m_clientList.GetHeadPosition();
	while (nPos)
	{
		CServerSocket* pTemp = (CServerSocket*)m_clientList.GetNext(nPos);
		pTemp->Send(str, str.GetLength()+1);
	}
}

/*	��дControlPC���������ڴ������յ���ָ����Ƶ��ԣ���Ҫ��Ϊ��ʵ���ֻ����ƶ�д��
	˵�������ƹ��ܿ����Լ��������ӣ�����ֻ�����ֲ���Ϊ������˵����ShellExecute
	�������ڵ�������Ӧ�ó��򣬹رս��̱Ƚ��鷳һ�㣬�����Ȼ�ȡӦ�ó��򴰿ڵ�ID��
	ͨ��OpenProcess��TerminateProcess��ֹ���̡�*/
void CmfcServer1Dlg::ControlPC(CString ControlMsg)
{
	if (ControlMsg == "open kugou") {            //�򿪲�����
		ShellExecute(NULL, _T("open"), _T("E:\\Program Files\\KuGou\\KuGou.exe"), NULL, NULL, SW_SHOWNORMAL);
		UpdateEvent("�û�[" + mymsg.userId + "]����򿪿ṷ������" );
	}
	else if (ControlMsg == "open qq"){
		ShellExecute(NULL, _T("open"), "E:\\Program Files\\Tencent\\QQ\\Bin\\QQ.exe", NULL, NULL, SW_SHOWNORMAL);
		UpdateEvent("�û�[" + mymsg.userId + "]�����QQӦ��");
	}
	else if (ControlMsg == "open chrome"){
		ShellExecute(NULL, _T("open"), "chrome.exe", NULL, NULL, SW_SHOWNORMAL);//C:\\Program Files (x86)\\Google\\Chrome\\Application
		UpdateEvent("�û�[" + mymsg.userId + "]����򿪹ȸ������");
	}
	else if (ControlMsg == "close kugou")         //�رղ�����
	{
		HWND hWnd = ::FindWindow(_T("kugou_ui"), NULL);
		DWORD id_num;
		GetWindowThreadProcessId(hWnd, &id_num);
		HANDLE hd = OpenProcess(PROCESS_ALL_ACCESS, FALSE, id_num);
		TerminateProcess(hd, 0);
		UpdateEvent("�û�[" + mymsg.userId + "]����رտṷ������");
	}
}

void CmfcServer1Dlg::OnOK()
{
	OnOpenServer();
	//CDialogEx::OnOK();
}
//����������Ϣ
LRESULT CmfcServer1Dlg::OnNotifyIconMsg(WPARAM wParam, LPARAM lParam)
{
	CPoint Point;
	switch (lParam)
	{
	case WM_RBUTTONDOWN://�����������ҽ�
	{
		CMenu pMenu;//���ز˵�
		if (pMenu.LoadMenu(IDR_MENU1))
		{
			CMenu* pPopup = pMenu.GetSubMenu(0);
			GetCursorPos(&Point);
			SetForegroundWindow();
			pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,Point.x, Point.y, this);
		}
	}
	break;
	case WM_LBUTTONDOWN:
		this->ShowWindow(SW_SHOW);
		break;
	default:
		break;
	}
	return 0;
}

void CmfcServer1Dlg::OnHide()
{
	this->ShowWindow(SW_HIDE);
}

void CmfcServer1Dlg::OnExit()
{
	SendCloseMsg();
	this->PostMessage(WM_CLOSE);
}

void CmfcServer1Dlg::OnShow()
{
	this->ShowWindow(SW_SHOW);
	SetForegroundWindow();
}

void CmfcServer1Dlg::OnClose()
{
	SendCloseMsg();
	CString str;
	GetDlgItem(IDC_EDIT2)->GetWindowText(str);
	ofstream history("history.txt",ios::out|ios::app);
	history<<"\n";
	history.close();
	Shell_NotifyIcon(NIM_DELETE, &nd);
	CDialogEx::OnClose();
}
 // ���͹رշ���������Ϣ
void CmfcServer1Dlg::SendCloseMsg(){
	if (m_connect)
	{
		SendMSG( mymsg.join("",TYPE[Server_is_closed],TYPE[AllUser]));
		delete listenSocket;
		listenSocket = NULL;
		m_connect = false;
		m_clientList.RemoveAll();	//�����������
		SetDlgItemText(IDC_userC, _T("0"));
		GetDlgItem(IDC_PortId)->EnableWindow(1);	//ʹ�ؼ������޸�
		GetDlgItem(IDC_IPADDRESS1)->EnableWindow(1);
		SetDlgItemText(IDC_OpenServer, _T("�򿪷�����"));
		UpdateEvent(_T("ϵͳ�رշ�����."));
	}
}

void CmfcServer1Dlg::OnClearLog()
{
	SetDlgItemText(IDC_EDIT2, _T(""));
}

int CmfcServer1Dlg::GetOnlineNum()
{
	m_userOnlineCount = 0;
	for (int i = 0; i < userNum && i<UserNumMax; ++i)
	{
		if (userInfo[i].Online!=0)
			m_userOnlineCount++;
	}
	return m_userOnlineCount;
}

void CmfcServer1Dlg::OnDropFiles(HDROP hDropInfo)
{
	// ��ȡ�ļ��������ļ��������û�
	SetForegroundWindow();		//���ô����ö���ʾ
	int  nFileCount = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 256);   //��ȡ������ļ�����
	char filepath[1024]="";
	DragQueryFile(hDropInfo,0,filepath,1024);	// ��ȡ�Ϸŵ�1���ļ��������ļ���
	if(GetFileAttributes(filepath)!=FILE_ATTRIBUTE_DIRECTORY){
		CFile fileSend(filepath,CFile::modeRead);
		long size = (long)fileSend.GetLength();
		fileSend.Close();
		CString s;
		s.Format("��ֻ֧�ִ��� %dMB ���ڵ��ļ���",MAX_PACKAGE_NUM*PACKAGE_SIZE/1024/1024);
		if(size>MAX_PACKAGE_NUM*PACKAGE_SIZE){
			MessageBox(s,"��ܰ��ʾ");
			return;
		}
		fileSendName = filepath;
		SetTimer(1,2,0);	//���ú�̨�̶߳�ȡ�ļ�����
		CString name = filepath;
		name = name.Right(name.GetLength()-name.ReverseFind('\\')-1);
		static MD5 md5;
		char szMD5[33] = "";
		md5.fileMd5(szMD5,filepath);
		s.Format("%s|%d|%s",name,size,szMD5);
		SendMSG( mymsg.join(s,TYPE[FileSend],TYPE[AllUser],"������"));
	}else
		MessageBox("����Ĳ���һ����Ч�ļ�","��ܰ��ʾ");

	CDialogEx::OnDropFiles(hDropInfo);
}

void CmfcServer1Dlg::OnTimer(UINT_PTR nIDEvent)
{
	if(0==nIDEvent){

	}elif(1==nIDEvent){ //��̨��ȡ�ļ����ݲ�����
		readFileEnd = 0;
		KillTimer(1);
		memset(packageData,0,sizeof(packageData));
		CXXFStream fileSend(fileSendName,ios::in | ios::binary);
		long fileSize = (long)fileSend.getSize(),readSize = 0;
		for(unsigned i=0; readSize<fileSize; i++){
			if(fileSize-readSize>PACKAGE_SIZE){		//���������İ�û�ж�ȡ
				fileSend.readString(packageData[i],PACKAGE_SIZE);
				readSize += PACKAGE_SIZE;
			}else{
				fileSend.readString(packageData[i],fileSize-readSize);
				readSize = fileSize;
			}
		}
		readFileEnd = 1;
		fileSend.close();
		fileSendName = fileSendName.Right(fileSendName.GetLength()-fileSendName.ReverseFind('\\')-1);
	}
	elif(nIDEvent==2){		//�����ļ����
		KillTimer(2);
		if(rf.isRecving()){
			SendMSG( mymsg.join(TYPE[File_Fail],TYPE[AskFileData],fileUser,"������") );
			UpdateEvent("�����ļ���ʱ�����Ժ����ԣ�","��ܰ��ʾ");
			//::MessageBox(GetSafeHwnd(),"�����ļ���ʱ�����Ժ����ԣ�","��ܰ��ʾ",0);
			modifyStatus("�ļ����ճ�ʱ",0);
			rf.recvEnd();
		}
	}
	CDialog::OnTimer(nIDEvent);
}

void CmfcServer1Dlg::fileSend(MyMsg& msg,bool NoAsk/*=0*/)
{
	int i = msg.data.Find('|');
	CString name = msg.data.Left(i);
	CString size = rightN(msg.data,i+1);
	i = size.Find('|');
	CString fileMD5 = rightN(size,i+1);
	size = size.Left(i);
	int fileSize = atoi(size);
	if(fileSize>1024*1024)
		size.Format("%.2f MB",fileSize/1024.0/1024.0);
	else if(fileSize>1024)
		size.Format("%.2f KB",fileSize/1024.0);
	else
		size.Format("%d �ֽ�",fileSize);
	if(rf.isRecving()){
		if(!NoAsk) MessageBox('['+msg.userId+"] ���㷢���ļ���\n�ļ�����"+name+"\n�ļ���С��"+size+"\n��ǰ���ڽ����ļ�����ȴ���ǰ�ļ��������ٽ��������ļ�����ֹ��ǰ�ļ�����","��ܰ��ʾ",0);
		return;
	}
	if(NoAsk || MessageBox('['+msg.userId+"] ���㷢���ļ���\n�ļ�����"+name+"\n�ļ���С��"+size+"\n�Ƿ�ͬ����գ�",
		"��ܰ��ʾ",MB_YESNO | MB_ICONQUESTION)==IDYES)
	{
		CString fmt = "*" + name.Right(name.GetLength()-name.ReverseFind('.'));
		CFileDialog dlg(false,0,name,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,fmt+"|"+fmt+"|All Files(*.*)|*.*||");
		if(NoAsk || dlg.DoModal()==IDOK){
			fileUser = msg.userId;
			SendMSG( msg.join("0",TYPE[AskFileData],msg.userId,"������") );
			CreateDirectory("CaptureFile",0);
			CString saveFileAt = NoAsk ? ("CaptureFile\\"+name) : dlg.GetPathName();
			rf.init(saveFileAt,fileSize,fileMD5);
			SetTimer(2,2000,0);
		}else{
			SendMSG( msg.join(TYPE[File_NO],TYPE[AskFileData],msg.userId,"������") );
		}
	}else
		SendMSG( msg.join(TYPE[File_NO],TYPE[AskFileData],msg.userId,"������") );
}

void CmfcServer1Dlg::sendFileToOthers(bool first)
{
	if(sft.fileSendOver==0)
	{
		if(first) //��ȡ�ļ�����׼������
		{
			fileSendName = "ServerFile\\"+fileSendName;
			SetTimer(1,2,0);
		}
		static int sendAt = 0;
		if(isOnline(sft.fileToUser))
		{
			if(sft.fileToUser=="����������" || sft.fileToUser==TYPE[AllUser])
			{
				if(sendAt<userNum) //��û�з������
				{
					while(!(userInfo[sendAt].Online) || userInfo[sendAt].User==sft.fileFromUser) ++sendAt;
					if(sendAt<userNum)
					{
						Sleep(20);
						SendMSG( mymsg.join(sft.fileInfo,TYPE[FileSend],userInfo[sendAt].User,sft.fileFromUser) );
						++sendAt;
					}
				}
			}else
			{
				SendMSG( mymsg.join(sft.fileInfo,TYPE[FileSend],sft.fileToUser,"������") );
				sft.fileSendOver = 1;
			}
		}
		if(sendAt>=userNum || !isOnline(sft.fileToUser))
		{
			sendAt = 0;
			sft.fileSendOver = 1;
		}
	}
}

void CmfcServer1Dlg::modifyStatus(CString sta,bool _sleep)
{
	HWND h = ::CreateStatusWindow(WS_CHILD | WS_VISIBLE, sta, this->m_hWnd, 0);
	if(_sleep)
		Sleep(10 * _sleep);
	::SendMessage(h, SB_SETBKCOLOR, 1, RGB(0, 125, 205));
}

//����������ʹ����theApp,��˲��ܷ�����������
void RecvFile::addPacketage(const char *data){
	static CmfcServer1Dlg* pDlg = (CmfcServer1Dlg*)theApp.GetMainWnd();
	if(packageRecv<packageNum){
		packageRecv ++;
		if(packageRecv<packageNum)		//�������ݰ�û�н���
			pDlg->SendMSG( pDlg->mymsg.join(getPackRecv(),TYPE[AskFileData],pDlg->fileUser,"������") );//������һ�����ݰ�
		strcpy_s(packageData[packageRecv-1],data);
	}
	static long timeNow=timeStart;
	if(pDlg!=0){
		if(clock()-timeNow>400){
			timeNow=clock();
			CString str;
			str.Format("�ļ��ѽ��� %.1f%%��    ��ʱ %.1fs   ƽ���ٶ� %.1fk/s",100.0 * packageRecv / packageNum
				,(clock()-timeStart)/1000.0, 1.0*packageRecv/packageNum*fileLength/((clock()-timeStart)));
			pDlg->modifyStatus(str,0);
		}
	}
	if(packageNum==packageRecv)
		saveFile(clock() - timeStart);
}
void RecvFile::saveFile(int useTime){
	static CmfcServer1Dlg* pDlg = (CmfcServer1Dlg*)theApp.GetMainWnd();
	static HWND hWnd = pDlg->GetSafeHwnd();
	bool clear = 1;	//�Ƿ�������ݰ�����
	if(packageNum==packageRecv){
		pDlg->modifyStatus("����У���ļ����ݣ�",0);
		CXXFStream out(fileNewName,ios::out | ios::binary);
		for(int i=0;i<packageRecv-1;i++){
			out.writeString(packageData[i],PACKAGE_SIZE);
		}
		if(fileLength % PACKAGE_SIZE==0)	//���һ����������
			out.writeString(packageData[packageNum-1],PACKAGE_SIZE);
		else								//���һ��û�������
			out.writeString(packageData[packageNum-1],fileLength % PACKAGE_SIZE);
		out.close();
		static MD5 md5;
		char newFileMD5[33] = "";
		md5.fileMd5(newFileMD5,(LPCTSTR)fileNewName);
		if(fileMD5==CString(newFileMD5)){
			pDlg->SendMSG( pDlg->mymsg.join(TYPE[File_Over],TYPE[AskFileData],pDlg->fileUser,"������") );
			timeMsg.Format("    ��ʱ %.1fs   ƽ���ٶ� %.1fk/s",useTime/1000.0, 1.0*fileLength/useTime);
			pDlg->modifyStatus("�ļ��ѽ�����ϣ�"+timeMsg,0);
			pDlg->sendFileToOthers(1);
			clear = 0;
			pDlg->UpdateEvent("���ղ������ļ��ɹ�","��ܰ��ʾ");
			//MessageBox(hWnd,"���ղ������ļ��ɹ�","��ܰ��ʾ",0);
		}else{
			pDlg->SendMSG( pDlg->mymsg.join(TYPE[File_Fail],TYPE[AskFileData],pDlg->fileUser,"������") );
			//MessageBox(hWnd,"���ջ򱣴��ļ�ʧ�ܣ����Ժ����ԣ���������룺0x00041)","��ܰ��ʾ",0);
			pDlg->UpdateEvent("���ջ򱣴��ļ�ʧ�ܣ����Ժ����ԣ���������룺0x00041)","��ܰ��ʾ");
			pDlg->modifyStatus("�ļ�����ʧ��",0);
			DeleteFile(fileNewName);
		}
	}
	recvEnd(clear);
}