#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS
#define  _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS 
#define  _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS 
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <hash_set>
#include <queue>
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")
using namespace std;

queue<string> URL;
hash_set<string> visitedurl;
hash_set<string> visitedimg;
int g_ImgCnt = 1;

#define DEFAULT_PAGE_BUF_SIZE 1048576;//Ĭ��ҳ���С

//     1. http://      2. jandan.net/    3.ooxx
//     http://jandan.net/ooxx/page-15#comments
bool ParseURL(const string &url, string &host, string &resource)
{
	
	size_t found = url.find("http://");   //1
	if (found == string::npos)
		return false;
	found += strlen("http://");
	size_t found1 = url.find_first_of('/', found);
	if (found1 == string::npos)
		return false;
	host = url.substr(found, found1 - found);    //2

	resource = url.substr(found1, url.size() - found1);    //3

	
	return true;
}

bool gethttpresponse(const string &host, const string &resource, string &response, int &bytes)
{
	struct hostent *hp = gethostbyname(host.c_str());
	if (hp == NULL) {
		cout << "���ܽ�����������ַ��" << endl;
		return false;
	}
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int nNetTimeout = 1000;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));
	if (sock == -1 || sock == -2) {
		cout << "���ܴ���socket��" << endl;
		return false;
	}
	//������������ַ
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(80);
	memcpy(&sa.sin_addr, hp->h_addr, 4);
	//���ӷ�����
	if (connect(sock, (SOCKADDR*)&sa, sizeof(sa)) != 0) {
		cout << "�������ӷ�������" << endl;
		return false;
	}
	//׼����������
	string request = "GET " + resource + " HTTP/1.1\r\nHost:" + host + "\r\nConnection:Close\r\n\r\n";
	if (SOCKET_ERROR == send(sock, request.c_str(), request.size(), 0)) {
		cout << "�������ݴ���" << endl;
		return false;
	}
	//��������
	int m_page_bufsize = DEFAULT_PAGE_BUF_SIZE;
	char * buf = new char[m_page_bufsize];
	memset(buf, 0, m_page_bufsize);

	int bytesread = 0;
	int ret = 1;
	cout << "��ȡ��";
	while (ret > 0) {
		ret = recv(sock, buf + bytesread, m_page_bufsize - bytesread, 0);
		if (ret > 0) {
			bytesread += ret;
		}
		if (m_page_bufsize - bytesread < 100) {
			cout << endl << "���·���ռ䣡" << endl;
			char * mbuf = new char[2 * m_page_bufsize];
			strcpy(mbuf, buf);
			delete[] buf;
			buf = mbuf;
		}
		cout << ret << " ";
	}
	cout << endl;
	buf[bytesread] = '\0';
	response.assign(buf, bytesread);
	delete[] buf;
	bytes = bytesread;
	closesocket(sock);
	return true;
}

bool ParseHtml(const string& response, vector<string> &imgurls)
{
	string resource;
	string url;
	size_t found;
	//string http = "href=\"http://";
	string img = "img src=";
	found = response.find(img);
	while (found != string::npos) {
		found += strlen("img src=");
		SIZE_T found1 = response.find('"', found );
		SIZE_T found11 = response.find('"', found1+1 );
		

		//<img src="//wx1.sinaimg.cn/mw600/0076BSS5ly1g5avpxj2baj30iz0sgtce.jpg" referrerPolicy="no-referrer"
		//   imgurl  ww3.sinaimg.cn/mw600/0073ob6Pgy1g59qgmtp8hj30rs0kuacs.jpg
		string imgurl = "http:"+ response.substr(found1+1, found11 - found1-1);
		cout << imgurl << endl;
		

			found = response.find(img, found1 + imgurl.size());
			
		
		if (visitedimg.find(imgurl) == visitedimg.end()) {
			visitedimg.insert(visitedimg.end(), imgurl);
			if (visitedimg.size() > 100000)
				visitedimg.clear();
			imgurls.push_back(imgurl);

			found = response.find(img, found1 + imgurl.size());
		}
		
	}
	cout << "�������������ҳ" << endl;

	return true;
}
	
//ȥ���������
bool Tofilename(const string url, string &filename)
{
	int size = url.size();
	for (int i = 0; i < size; i++) {
		if (url[i] != '*' && url[i] != '\\'&& url[i] != '/'
			&& url[i] != ':'&& url[i] != '?'&& url[i] != '<'
			&& url[i] != '>'&& url[i] != '|'&& url[i] != '"'
			&& url[i] != '.' && url[i] != '-' && url[i] != ' ') {
			filename += url[i];
		}
	}
	filename += ".txt";
	return true;
}

void Downloads(const vector<string> &imgurls, const string &url)
{
	int size = imgurls.size();
	string filename;
	if (Tofilename(url, filename) == 0) {
		cout << "ת�����ִ���" << endl;
		return;
	}

	filename = "./img";
	for (int i = 0; i < size; i++) {
		string str = imgurls[i];
		
		string host;
		string resource;
		if (!ParseURL(imgurls[i], host, resource)) {
			cout << "��ַ����!" << endl;
			return;
		}
		
		string image;
		int bytes = 0;
		if (gethttpresponse(host,resource,image,bytes)) {
			if (image.size() == 0) {
				cout << "�������ݴ���" << endl;
				continue;
				
			}
			size_t found = image.find("\r\n\r\n");
			if (found == string::npos) {
				cout << "�������ݴ���" << endl;
				continue;
			}
			found += strlen("\r\n\r\n");
			if (found == bytes) {
				cout << "�������ݴ���" << endl;
				continue;
			}
			int index = imgurls[i].find_last_of("/");
			if (index != string::npos) {
				string imgname = imgurls[i].substr(index, imgurls[i].size() - index);
				ofstream ofile(filename + imgname, ios::binary);
				if (!ofile.is_open())
					continue;
				cout << g_ImgCnt++ << filename + imgname << endl;
				ofile.write(&image[found], bytes - found - 1);
				ofile.close();
			}
		}
	}
}


//�������
void BFS(string url)
{
	string host;
	string resource;
	
	if (!ParseURL(url, host, resource)) {
		cout << "��ַ����!" << endl;
		return;
	}
	string response = "";
	int bytes = 0;
	if (!gethttpresponse(host,resource,response,bytes)) {
		cout << "û�еõ���ҳ��Ӧ��" << endl;
		return;
	}
	if (response.size() == 0) {
		cout << "�������������ݴ���" << endl;
		return;
	}
	//�洢��ҳ������Ϣ
	string filename;
	if (Tofilename(url, filename) == 0) {
		cout << "ת�����ִ���!" << endl;
		return;
	}
	ofstream ofile("./html/" + filename);
	if (ofile.is_open()) {
		ofile << response << endl;
		ofile.close();
	}
	else {
		cout << "���ļ�����" << endl;
		return;
	}

	vector<string> imgurls;
	ParseHtml(response, imgurls);
	printf("hahahah");
	Downloads(imgurls, url);
}

int main()
{
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
		cout << "����winsockt��̬��ʧ��" << endl;
		return 1;
	}
	CreateDirectoryA("./img", 0);
	CreateDirectoryA("./html", 0);
	//��ȡ�����ļ�URL
	string urlstart;
	int i;

	for (i = 1;i <= 17;i++)
	{
		urlstart = "http://jandan.net/ooxx/page-" + to_string(i) + "#comments";
		/*fstream ifile("HrefURL.txt");


			if (!ifile.is_open())
			{
				cout << "���ļ�����" << endl;
				return 1;
			} */


		
		
			URL.push(urlstart);
			visitedurl.insert(urlstart);
		
		



		while (URL.size() > 0)
		{
			string str = URL.front();
			cout << str << endl;
			//������ȣ�Ѱ�������ļ���������url
			BFS(str);
			URL.pop();
			cout << "URL��Ŀ��" << URL.size() << endl;
			if (g_ImgCnt > 200)
				break;
		}
	}
	WSACleanup(); 
	system("pause");
	return 0;
}