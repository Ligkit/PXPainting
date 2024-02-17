#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <Windows.h>
#include <graphics.h>

#define	LIGHTMOD		1
#define	DARKMOD			0
#define	BACKCOLOR		RGB(30, 30, 30)		//黑色

int					winX{ 80 };				//窗口宽80格
int					winY{ 60 };				//窗口高60格
int					cellSize{ 10 };			//每格10像素
int					mode{ DARKMOD };		//设置深浅模式
bool				enableGetColor{ false };//是否可以取色
MOUSEMSG			mouseMsg{ };			//鼠标消息
HWND				hwnd{ };				//窗口句柄
struct				COLOR					//RGB颜色
{
	COLOR(int rr, int gg, int bb);
	COLOR() = default;
	int r;
	int g;
	int b;
};
struct				POINT2					//封装二位点
{
	POINT2(int xx, int yy);
	int x;
	int y;
};
struct				PXDATA					//像素信息
{
	PXDATA(int xx, int yy, COLOR cc);
	PXDATA(int xx, int yy, COLORREF cc);
	PXDATA(std::string data);
	PXDATA() = default;
	int x;
	int y;
	COLORREF c;
};
std::vector<PXDATA> openmap;		//打开图片内容
std::vector<PXDATA> savemap;		//保存图片内容
std::string			split			(std::string str, int h, int e);	//裁剪字符串
std::string			bSplit			(std::string str, char c = ' ');	//截取后半段
std::string			fSplit			(std::string str, char c = ' ');	//截取前半段
bool				openImageDialog	(HWND hwnd, std::wstring& filePath);//打开图片
bool				isIn			(POINT2 lt, POINT2 rb);				//判断鼠标是否在一定区域
void				init			();									//初始化窗体
void				update			();									//更新显示
void				clear			();									//清空画布
void				drawRect		(POINT2 p, int w, int h, COLOR c);	//绘制矩形
void				save			();									//保存图片
void				open			();									//打开图片

int main()
{
	init();
	while (true)
	{
		update();
	}
	closegraph();
	return 0;
}

COLOR::COLOR(int rr, int gg, int bb)
{
	r = rr;
	g = gg;
	b = bb;
}

POINT2::POINT2(int xx, int yy)
{
	x = xx;
	y = yy;
}

PXDATA::PXDATA(int xx, int yy, COLOR cc)
{
	x = xx;
	y = yy;
	c = (cc.r << 16) + (cc.g << 8) + (cc.b);
}

PXDATA::PXDATA(int xx, int yy, COLORREF cc)
{
	x = xx;
	y = yy;
	c = cc;
}

PXDATA::PXDATA(std::string data)
{
	std::istringstream iss(data); //data的内容: xxx xxx xxxxx
	iss >> x >> y >> c;
}

bool openImageDialog(HWND hwnd, std::wstring& filePath)
{
	OPENFILENAMEW ofn;
	WCHAR szFile[MAX_PATH] = { 0 };
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"TMG Files\0*.tmg;\0All Files\0*.*\0";
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;

	if (GetOpenFileNameW(&ofn))
	{
		filePath = ofn.lpstrFile;
		return true;
	}

	return false;
}

std::string split(std::string str, int h, int e)
{
	for (int i = 0; i < h; i++)
		str[i] = '\0';
	for (int j = e; j < str.length(); j++)
		str[j] = '\0';
	return str;
}

std::string bSplit(std::string str, char c)
{
	for (int i = 0; i < str.length(); i++)
	{
		if (str[i] == c)
		{
			str = split(str, i, str.length());
			break;
		}
	}
	return str;
}

std::string fSplit(std::string str, char c)
{
	for (int i = 0; i < str.length(); i++)
	{
		if (str[i] == c)
		{
			str = split(str, 0, i);
			break;
		}
	}
	return str;
}

void init()
{
	hwnd = initgraph(winX * cellSize, winY * cellSize);	//初始化窗体
	SetWindowTextA(hwnd, "Drawing");					//设置窗体标题
	setbkcolor(mode == LIGHTMOD ? WHITE : BACKCOLOR);	//设置背景颜色 依主题深浅而定
	setbkmode(TRANSPARENT);								//设置透明化
	cleardevice();										//刷新(清除)
	setlinecolor(LIGHTGRAY);							//设置线条颜色为浅灰色
	setlinestyle(PS_SOLID, 1);							//设置线条样式: 实线, 1像素宽
	line(0, 100, winX * cellSize, 100);					//绘制线条

	for (int x = 0; x < winX * cellSize; x += cellSize)	//绘制网格
	{
		line(x, 100, x, winY * cellSize);
		for (int y = 100; y < winY * cellSize; y += cellSize)
		{
			line(0, y, winX * cellSize, y);
		}
			
	}
	clear();

	float H{ 0 };					// 色相
	float S{ 1 };					// 饱和度
	float L{ 0.5f };				// 亮度

	for (int x = 250; x < 650; x++)	//绘制彩色条
	{
		H += 1;
		setfillcolor(HSLtoRGB(H, S, L));
		setlinecolor(HSLtoRGB(H, S, L));
		fillrectangle(x, 15, x + 1, 35);
	}
	setlinecolor(WHITE);
	setfillcolor(BACKCOLOR);
	fillrectangle(650, 15, 660, 35);
	setlinecolor(BACKCOLOR);
	setfillcolor(WHITE);
	fillrectangle(660, 15, 670, 35);
	
	drawRect(POINT2(30, 10), 60, 25, mode == LIGHTMOD ? COLOR(200, 200, 200) : COLOR(63, 63, 70));	//保存
	drawRect(POINT2(30, 50), 60, 25, mode == LIGHTMOD ? COLOR(200, 200, 200) : COLOR(63, 63, 70));	//清屏
	drawRect(POINT2(110, 10), 60, 25, mode == LIGHTMOD ? COLOR(200, 200, 200) : COLOR(63, 63, 70));	//关于
	drawRect(POINT2(110, 50), 60, 25, mode == LIGHTMOD ? COLOR(200, 200, 200) : COLOR(63, 63, 70));	//退出
	drawRect(POINT2(680, 10), 100, 25, mode == LIGHTMOD ? COLOR(200, 200, 200) : COLOR(63, 63, 70));//切换
	drawRect(POINT2(190, 50), 60, 25, mode == LIGHTMOD ? COLOR(200, 200, 200) : COLOR(63, 63, 70)); //取色
	drawRect(POINT2(270, 50), 60, 25, mode == LIGHTMOD ? COLOR(200, 200, 200) : COLOR(63, 63, 70)); //打开

	settextcolor(mode == LIGHTMOD ? BLACK : WHITE);
	outtextxy(45, 15, L"保存");
	outtextxy(45, 55, L"清屏");
	outtextxy(125, 15, L"关于");
	outtextxy(125, 55, L"退出");
	outtextxy(205, 55, L"取色");
	outtextxy(285, 55, L"打开");
	outtextxy(690, 15, mode == LIGHTMOD ? L"切换为深色" : L"切换为浅色");
}

void update()
{
	fillrectangle(220, 15, 220 + 20, 15 + 20);
	mouseMsg = GetMouseMsg();	//更新鼠标消息
	if (mouseMsg.mkLButton)		//如果点击左键
	{
		//保存
		if (isIn(POINT2(30, 10), POINT2(90, 35)))
		{
			save();
		}

		//清除
		if (isIn(POINT2(30, 50), POINT2(90, 75)))
		{
			if (MessageBox(hwnd, L"确定要清除吗?", L"清除", 48 + MB_OKCANCEL) == 1)
			{
				clear();
			}
		}

		//帮助
		if (isIn(POINT2(110, 10), POINT2(170, 35)))
		{
			MessageBox(hwnd,
				L"作者: Include源来是小白.\n版本: V2.0\n1. 点击画布上方的颜色条, 即可更改画笔颜色\n鼠标右键可擦除一个像素点\n2. 点击切换主题按钮, 即可切换主题\n切换主题的bug: 切换后的窗口有时会跑到显示器边缘\n3. 点击\"取色\"按钮后, 点击画布上的色块即可取色\n4. 点击\"保存\"按钮后, 保存图像格式为 .tmg\n5. 保存的图像会覆盖同文件名的文件\n6. 点击\"打开\"按钮, 选择 .tmg格式文件打开图像\n按钮的bug: 多次连续点击会重复弹出窗口",
				L"关于",
				64 + MB_OK
			);
		}

		//退出
		if (isIn(POINT2(110, 50), POINT2(170, 75)))
		{
			if (MessageBox(hwnd, L"确定要退出吗?", L"关闭程序", 48 + MB_OKCANCEL) == 1)
			{
				closegraph();		//关闭窗口
				std::system("echo 请关闭此窗口");
			}
		}

		//绘制功能
		if (mouseMsg.y > 100)	//鼠标在绘图区域
		{
			if (enableGetColor)
			{
				COLORREF gotColor = getpixel(mouseMsg.x, mouseMsg.y);
				setfillcolor(gotColor);
				setlinecolor(gotColor);
				enableGetColor = false;
			}
			else
			{
				//计算绘制像素的位置
				int x = mouseMsg.x / cellSize;
				int y = mouseMsg.y / cellSize;
				solidrectangle(x * cellSize + 1, y * cellSize + 1, x * cellSize + cellSize - 1, y * cellSize + cellSize - 1);

				savemap.push_back(PXDATA(x, y, getfillcolor()));	//添加到保存列表
				//std::cout << x << ", " << y << ": " << getfillcolor() << std::endl; //调试文本
			}
		}

		//切换颜色
		if (isIn(POINT2(250, 15), POINT2(670, 35)))
		{
			COLORREF pen = getpixel(mouseMsg.x, mouseMsg.y);
			setfillcolor(pen);
			setlinecolor(pen);
		}

		//切换主题
		if (isIn(POINT2(680, 10), POINT2(780, 35)))
		{
			if (MessageBox(hwnd, L"切换后当前内容会丢失, 是否切换?", L"切换", 48 + MB_OKCANCEL) == 1)
			{
				mode = (mode == LIGHTMOD) ? DARKMOD : LIGHTMOD;
				closegraph();
				init();
			}
		}

		//取色
		if (isIn(POINT2(190, 50), POINT2(250, 75)))
		{
			enableGetColor = true;
		}

		//打开
		if (isIn(POINT2(270, 50), POINT2(330, 75)))
		{
			
			if (MessageBox(hwnd, L"当前文件未保存, 是否保存?", L"打开", 48 + MB_OKCANCEL) == 1)
			{
				save();
			}
			else
			{
				clear();
				open();
			}
		}
	}

	if (mouseMsg.mkRButton)		//点击右键
	{
		COLORREF tf = getfillcolor();
		COLORREF tl = getlinecolor();

		setfillcolor(mode == LIGHTMOD ? WHITE : BACKCOLOR);
		setlinecolor(mode == LIGHTMOD ? WHITE : BACKCOLOR);

		int x = mouseMsg.x / cellSize;	//计算像素位置
		int y = mouseMsg.y / cellSize;
		solidrectangle(x* cellSize + 1, y* cellSize + 1, x* cellSize + cellSize - 1, y* cellSize + cellSize - 1);

		setfillcolor(tf);
		setlinecolor(tl);

		savemap.push_back(PXDATA(x, y, mode == LIGHTMOD ? WHITE : BACKCOLOR));
		//std::cout << x << ", " << y << ": " << ((mode == LIGHTMOD) ? WHITE : BACKCOLOR) << std::endl; //调试文本
	}
}

void clear()
{
	savemap.clear();
	for (int x = 0; x < winX * cellSize; x += cellSize)
	{
		for (int y = 100; y < winY * cellSize; y += cellSize)
		{
			COLORREF tf = getfillcolor();
			COLORREF tl = getlinecolor();

			setfillcolor(mode == LIGHTMOD ? WHITE : BACKCOLOR);
			setlinecolor(RGB(200, 200, 200));
			solidrectangle(x + 1, y + 1, x + cellSize - 1, y + cellSize - 1);
			
			setfillcolor(tf);
			setlinecolor(tl);
		}
	}
}

void drawRect(POINT2 p, int w, int h, COLOR c)
{
	setfillcolor(RGB(c.r, c.g, c.b));
	setlinecolor(RGB(c.r, c.g, c.b));
	fillrectangle(p.x, p.y, p.x + w, p.y + h);
}

bool isIn(POINT2 lt, POINT2 rb)
{
	if (lt.x < mouseMsg.x && mouseMsg.x < rb.x
		&& lt.y < mouseMsg.y && mouseMsg.y < rb.y)
	{
		return true;
	}
	return false;
}

void save()
{
	OPENFILENAMEW ofn;
	WCHAR szFile[MAX_PATH] = { 0 };
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"TMG Files\0*.tmg;\0All Files\0*.*\0";
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;

	if (GetSaveFileNameW(&ofn))
	{
		std::wstring filePath = ofn.lpstrFile;
		std::ofstream img;
		img.open(filePath, std::ios::out);
		if (!img.is_open())
		{
			MessageBox(hwnd, L"保存失败，保存文件无法创建。", L"错误", 16 + MB_OK);
		}
		else
		{
			for (int i = 0; i < savemap.size(); i++)
			{
				std::string temp = std::to_string(savemap[i].x) + " " + std::to_string(savemap[i].y) + " " + std::to_string(savemap[i].c) + "\n";
				img << temp;
			}
			img.close();

			MessageBox(hwnd, L"保存成功", L"保存到本地", 64 + MB_OK);
		}
	}
}

void open()
{
	std::wstring filePath;
	if (openImageDialog(hwnd, filePath))
	{
		// 打开选择的图片文件
		savemap.clear();
		openmap.clear();
		std::ifstream txtimg;
		txtimg.open(filePath, std::ios::in);
		while (txtimg.good())
		{
			std::string data;
			getline(txtimg, data);
			if (!data.empty())
			{
				openmap.push_back(PXDATA(data));
				savemap.push_back(PXDATA(data));
			}
		}

		for (int i = 0; i < openmap.size(); i++)
		{
			int x = openmap[i].x * cellSize + 1; // 计算绘制的像素在窗口中的实际位置
			int y = openmap[i].y * cellSize + 1;
			int w = cellSize - 2;				 // 计算绘制的像素大小
			int h = cellSize - 2;
			setfillcolor(openmap[i].c);
			setlinecolor(openmap[i].c);
			solidrectangle(x, y, x + w, y + h);
		}
	}
}