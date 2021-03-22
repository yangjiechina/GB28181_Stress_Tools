
// GB28181_Stress_ToolsDlg.cpp : implementation file
//
#include "pch.h"
#include "framework.h"
#include "GB28181_Stress_Tools.h"
#include "GB28181_Stress_ToolsDlg.h"
#include "afxdialogex.h"

#include "Device.h"
#include "LoadH264.h"
#include "NaluProvider.h"
#include <io.h> 
#include <fcntl.h>  
#include <iostream>
#include <thread>
#include <vector>
#include "pugixml.hpp"
#include <sstream>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CGB28181StressToolsDlg dialog



CGB28181StressToolsDlg::CGB28181StressToolsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GB28181_STRESS_TOOLS_DIALOG, pParent)
	, m_edit_server_sip_id(_T(""))
	, m_edit_server_ip(_T(""))
	, m_edit_server_port(0)
	, m_edit_device_count(0)
	, m_edit_password(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGB28181StressToolsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_device_list);
	DDX_Control(pDX, IDC_BUTTON1, m_btn_start);
	DDX_Text(pDX, IDC_EDIT1, m_edit_server_sip_id);
	DDX_Text(pDX, IDC_EDIT2, m_edit_server_ip);
	DDX_Text(pDX, IDC_EDIT3, m_edit_server_port);
	DDX_Text(pDX, IDC_EDIT4, m_edit_device_count);
	DDX_Text(pDX, IDC_EDIT5, m_edit_password);
}

BEGIN_MESSAGE_MAP(CGB28181StressToolsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CGB28181StressToolsDlg::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_EDIT1, &CGB28181StressToolsDlg::OnEnChangeEdit1)
END_MESSAGE_MAP()

//控制台初始化
void InitConsoleWindow()
{
	int nRet = 0;
	FILE* fp;
	AllocConsole();
	nRet = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
	fp = _fdopen(nRet, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);
}
// CGB28181StressToolsDlg message handlers
bool is_started;
vector<std::shared_ptr<Device>> m_device_vector;
vector<NaluProvider*> nalu_vector_vector;
pugi::xml_document config_file;

BOOL CGB28181StressToolsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	InitConsoleWindow();
	//加载xml配置文件
	config_file.load_file("config.xml");
	pugi::xml_node config = config_file.first_child();
	if (!config) {
		//config_file.append_child("config");
		stringstream ss;
		ss << "<config>";
		ss << "<serverId>34020000002000000001</serverId>";
		ss << "<serverIp></serverIp>";
		ss << "<serverPort>5060</serverPort>";
		ss << "<password>12345678</password>";
		ss << "<count></count>";
		ss << "</config>";
		std::string buffer = ss.str();
		config_file.append_buffer(buffer.c_str(), buffer.length());
		config = config_file.first_child();
		config_file.save_file("config.xml");
	}

	// TODO: Add extra initialization here
	//set_initial_config_params
	m_edit_server_sip_id = config.child("serverId").child_value();
	m_edit_server_ip = config.child("serverIp").child_value();
	m_edit_server_port = atoi(config.child("serverPort").child_value());
	m_edit_password = config.child("password").child_value();
	m_edit_device_count = atoi(config.child("count").child_value());
	UpdateData(false);

	CRect list_rect;
	m_device_list.GetClientRect(&list_rect);

	m_device_list.SetExtendedStyle(m_device_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//set_title
	// 为列表视图控件添加三列   
	m_device_list.InsertColumn(0, _T("序号"), LVCFMT_CENTER, list_rect.Width() / 10, 0);

	m_device_list.InsertColumn(1, _T("DeviceId"), LVCFMT_CENTER, list_rect.Width() / 5, 0);
	m_device_list.InsertColumn(2, _T("VideoChannelId"), LVCFMT_CENTER, list_rect.Width() / 5, 1);
	m_device_list.InsertColumn(3, _T("信令监听端口"), LVCFMT_CENTER, list_rect.Width() / 8, 3);
	m_device_list.InsertColumn(4, _T("推流监听端口"), LVCFMT_CENTER, list_rect.Width() / 8, 4);
	m_device_list.InsertColumn(5, _T("推流协议"), LVCFMT_CENTER, list_rect.Width() / 8, 5);
	m_device_list.InsertColumn(6, _T("状态"), LVCFMT_CENTER, list_rect.Width() / 8, 6);
	//ServerId
	//ServerIp
	//ServerPort
	//Password
	//count
	//pugi::xml_node serverId = config.child("serverId");
	//pugi::xml_node serverIp = config.child("serverIp");
	//pugi::xml_node serverPort = config.child("serverPort");
	//pugi::xml_node password = config.child("password");
	//pugi::xml_node count = config.child("count");

	//std::string serverId = config.child("serverId").child_value();
	//std::string serverIp = config.child("serverIp").child_value();
	//std::string serverPort = config.child("serverPort").child_value();
	//std::string password = config.child("password").child_value();
	//std::string count = config.child("count").child_value();
	//config_file.save_file("config.xml");


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGB28181StressToolsDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGB28181StressToolsDlg::OnPaint()
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
HCURSOR CGB28181StressToolsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CGB28181StressToolsDlg::update_item(int index, Message msg) {

	int item_count = m_device_list.GetItemCount();

	if (index + 1 > item_count) {
		MessageBox(_T("index out range"));
		return;
	}
	if (STATUS_TYPE == msg.type) {
		m_device_list.SetItemText(index, 6, CString(msg.content));
	}
	else if (PULL_STREAM_PROTOCOL_TYPE== msg.type) {
		m_device_list.SetItemText(index, 5, CString(msg.content));
	}
	else if (PULL_STREAM_PORT_TYPE == msg.type) {
		m_device_list.SetItemText(index, 4, CString(msg.content));

	}
}
void CGB28181StressToolsDlg::Start() {
	m_device_list.DeleteAllItems();

	int device_count = m_edit_device_count;
	//多少Deivce对应一个队列
	int rate = 20;

	int start_port = 50000;

	extern vector<Nalu*> nalu_vector;

	callback = std::bind(&CGB28181StressToolsDlg::update_item,this, std::placeholders::_1, std::placeholders::_2);

	for (int i = 0; i < device_count; i++) {

		this_thread::sleep_for(chrono::seconds(1));
		
		NaluProvider* provider = nullptr;

		int index = i / rate;
		
		if (nalu_vector_vector.size() >= index + 1) {
			provider = nalu_vector_vector.at(index);
		}else {
			//拷贝h264队列
			if (i % rate == 0) {
				size_t size = nalu_vector.size();
				vector<Nalu*>* current_nalu_vector = new vector<Nalu*>();
				for (int i = 0; i < size; i++) {
					Nalu * nalu = nalu_vector.at(i);

					char * copy_packet = (char *)malloc(nalu->length * sizeof(char));

					memcpy(copy_packet, nalu->packet, nalu->length);

					Nalu * copy_nalu = new Nalu();
					copy_nalu->packet = copy_packet;
					copy_nalu->length = nalu->length;
					copy_nalu->type = nalu->type;
					current_nalu_vector->push_back(copy_nalu);
				}
				 provider = new NaluProvider(current_nalu_vector);
				nalu_vector_vector.push_back(provider);
			}
		}
		USES_CONVERSION;

		m_edit_password.GetBuffer();
		string prefix = "3402000000132000000";

		string deviceId = prefix.append(to_string(i));
		const char * sip_Id = T2A(m_edit_server_sip_id);
		if(is_started){
			start_port++;
			m_device_list.InsertItem(i, CString(to_string(i).c_str()));
			m_device_list.SetItemText(i, 1,CString(deviceId.c_str()));
			m_device_list.SetItemText(i, 2, CString(deviceId.c_str()));
			m_device_list.SetItemText(i, 3, CString(to_string(start_port).c_str()));
			m_device_list.SetItemText(i, 4, _T(""));
			m_device_list.SetItemText(i, 5, _T(""));
			m_device_list.SetItemText(i, 6, _T(""));

			//Device * device = new Device(deviceId.c_str(), T2A(m_edit_server_sip_id), T2A(m_edit_server_ip), m_edit_server_port,
			//	T2A(m_edit_password), provider);
			
			std::shared_ptr<Device> device_ptr =  std::make_shared<Device>(deviceId.c_str(), T2A(m_edit_server_sip_id), T2A(m_edit_server_ip), m_edit_server_port,
				T2A(m_edit_password), provider);
			device_ptr->list_index = i;
			device_ptr->set_callback(callback);
			device_ptr->start_sip_client(start_port);
			m_device_vector.push_back(device_ptr);

		}
		//如果在启动的过程中点击“结束”，跳出循环
		else {
			return;
		}

	}
}
void CGB28181StressToolsDlg::Stop() {
	/*while (!m_device_vector.empty()) {
		Device* device =  m_device_vector.back();
		m_device_vector.pop_back();
		device->stop_sip_client();
		m_device_list.SetItemText(device->list_index, 6, _T("释放设备"));
	}*/
	m_device_vector.clear();
}
bool CGB28181StressToolsDlg::CheckParams() {
	if (m_edit_server_sip_id.GetLength() < 10) {
		MessageBox(_T("Server Sip Id 必须大于10"));
		return false;
	}

	if (m_edit_server_ip.IsEmpty()) {
		MessageBox(_T("请输入 Server Ip"));
		return false;
	}
	int port = m_edit_server_port;
	if (port <= 0) {
		MessageBox(_T("port 必须大于1"));
		return false;
	}

	if (m_edit_device_count <= 0) {
		MessageBox(_T("模拟数量 必须大于1"));
		return false;
	}
	if (m_edit_password.IsEmpty()) {
		MessageBox(_T("密码不能为空"));
		return false;
	}
	USES_CONVERSION;
	/*pugi::xml_node config = config_file.first_child();
	config.child("serverId").first_child().set_value(T2A(m_edit_server_sip_id));
	config.child("serverIp").first_child().set_value(T2A(m_edit_server_ip));
	config.child("serverPort").first_child().set_value(to_string(m_edit_server_port).c_str());
	config.child("password").first_child().set_value(T2A(m_edit_password));
	config.child("count").first_child().set_value(to_string(m_edit_device_count).c_str());*/
	stringstream ss;
	ss << "<config>";
	ss << "<serverId>"<< T2A(m_edit_server_sip_id)<<"</serverId>";
	ss << "<serverIp>"<< T2A(m_edit_server_ip)<< "</serverIp>";
	ss << "<serverPort>"<< m_edit_server_port<<"</serverPort>";
	ss << "<password>"<< T2A(m_edit_password) <<"</password>";
	ss << "<count>"<< m_edit_device_count <<"</count>";
	ss << "</config>";
	config_file.reset();
	std::string buffer =  ss.str();
	config_file.append_buffer(buffer.c_str(), buffer.length());
	config_file.save_file("config.xml");
	return true;
}
void CGB28181StressToolsDlg::OnBnClickedButton1()
{
	//1.读取h264视频源
	//获取参数，创建设备，开始通信
	if (!is_started) {
		UpdateData(TRUE);
	
		if (!CheckParams()) {
			return;
		}

		extern vector<Nalu*> nalu_vector;
		if(nalu_vector.empty()){
			const char * h264_path = "bigbuckbunnynoB_480x272.h264";
			if (load(h264_path) < 0) {
				MessageBox(_T("读取h264源文件失败"));
				return;
			}
		}
		is_started = true;
		thread t(&CGB28181StressToolsDlg::Start,this);
		t.detach();
		m_btn_start.SetWindowTextW(_T("结束"));
	}
	//释放所有设备
	else {
		is_started = false;
		thread t(&CGB28181StressToolsDlg::Stop, this);
		t.detach();
		m_btn_start.SetWindowTextW(_T("开始"));

	}
	// TODO: Add your control notification handler code here
}


void CGB28181StressToolsDlg::OnEnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

