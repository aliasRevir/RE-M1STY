#include <stdio.h>
#include <time.h>
#include <windows.h>

/*
VK_LBUTTON				0x01		鼠标左键
VK_RBUTTON				0x02		鼠标右键
VK_MBUTTON				0x04		鼠标中键
VK_BACK					0x08		BACKSPACE 键
VK_TAB					0x09		Tab 键
VK_RETURN				0x0D		Enter 键
VK_SHIFT				0x10		SHIFT 键
VK_CONTROL				0x11		CTRL 键
VK_MENU					0x12		Alt 键
VK_PAUSE				0x13		PAUSE 键
VK_CAPITAL				0x14		CAPS LOCK 键
VK_SPACE				0x20		空格键
VK_PRIOR				0x21		PAGE UP 键
VK_NEXT					0x22		PAGE DOWN 键
VK_END					0x23		END 键
VK_HOME					0x24		HOME 键
VK_LEFT					0x25		LEFT ARROW 键
VK_UP					0x26		UP ARROW 键
VK_RIGHT				0x27		RIGHT ARROW 键
VK_DOWN					0x28		DOWN ARROW 键
VK_SNAPSHOT				0x2C		PRINT SCREEN 键
VK_INSERT				0x2D		INS 键
VK_DELETE				0x2E		DEL 键
/						0x30-0x39	0-9 键
/						0x41-0x5A	A-Z 键
VK_LWIN					0x5B		左 Windows
VK_RWIN					0x5C		右 Windows
VK_NUMPAD0-VK_NUMPAD9	0x60-0x69	数字键盘 0-9
VK_MULTIPLY				0x6A		乘号键
VK_ADD					0x6B		小键盘加号键
VK_SEPARATOR			0x6C		小键盘分隔符
VK_SUBTRACT				0x6D		小键盘减号
VK_DECIMAL				0x6E		小键盘句点
VK_DIVIDE				0x6F		小键盘除号
VK_F1-VK_F12			0x70-0x7B	F1-F12
VK_NUMLOCK				0x90		NUM LOCK
VK_SCROLL				0x91		SCROLL LOCK
VK_VOLUME_MUTE			0xAD		静音
VK_VOLUME_DOWN			0xAE		音量减
VK_VOLUME_UP			0xAF		音量加
VK_OEM_1				0xBA		键;:
VK_OEM_PLUS				0xBB		键+
VK_OEM_COMMA			0xBC		键,
VK_OEM_MINUS			0xBD		键-
VK_OEM_PERIOD			0xBE		键.
VK_OEM_2				0xBF		键/?
VK_OEM_3				0xC0		键`~
VK_OEM_4				0xDB		键[{
VK_OEM_5				0xDC		键\|
VK_OEM_6				0xDD		键]}
VK_OEM_7				0xDE		键'"
*/

typedef unsigned char uchar;

uchar getKey(char p) {
	if(isdigit(p)) return 0x30 + (p - '0');
	if(isalpha(p)) return 0x41 + (tolower(p) - 'a');
	switch(p) {
		case ';' : case ':' : return VK_OEM_1;		// 0xBA
		case '+' : case '=' : return VK_OEM_PLUS;	// 0xBB
		case ',' : case '<' : return VK_OEM_COMMA;	// 0xBC
		case '-' : case '_' : return VK_OEM_MINUS;	// 0xBD
		case '.' : case '>' : return VK_OEM_PERIOD;	// 0xBE
		case '/' : case '?' : return VK_OEM_2;		// 0xBF
		case '`' : case '~' : return VK_OEM_3;		// 0xC0
		case '[' : case '{' : return VK_OEM_4;		// 0xDB
		case '\\': case '|' : return VK_OEM_5;		// 0xDC
		case ']' : case '}' : return VK_OEM_6;		// 0xDD
		case '\'': case '\"': return VK_OEM_7;		// 0xDE
	}
	return 0;
}

int ifKeyDownMul(uchar p[], int cnt) { // All Down
	for(int idx = 0; idx < cnt; idx ++)
		if(GetKeyState(p[idx]) >= 0) return 0;
	return 1;
}

void waitKeyDownMul(uchar p[], int cnt) {
	while(!ifKeyDownMul(p, cnt)) Sleep(1);
}

int ifKeyUpMul(uchar p[], int cnt) { // Any Up
	for(int idx = 0; idx < cnt; idx ++)
		if(GetKeyState(p[idx]) >= 0) return 1;
	return 0;
}

void waitKeyUpMul(uchar p[], int cnt) {
	while(!ifKeyUpMul(p, cnt)) Sleep(1);
}

const double limitTime = 0.3; // Press {limitTime} seconds to trigger
const double gapTimeAlpha = 0.004;
const double gapTimeHwnd = 0.004;

double getDeltaTime(clock_t st, clock_t ed) { return (ed - st) / (double) CLOCKS_PER_SEC; }
uchar umax(int a, int b) { return a > b ? a : b; }
uchar umin(int a, int b) { return a < b ? a : b; }
int imax(int a, int b) { return a > b ? a : b; }
int imin(int a, int b) { return a < b ? a : b; }

void setHwndAlpha(HWND hwnd, uchar alpha) {
	SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
}
void setHwndPos(HWND hwnd, RECT rect) {
	SetWindowPos(hwnd, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
}

void updateHwnd(HWND *phwnd, RECT *prect, uchar *palpha) {
	*phwnd = GetForegroundWindow();
	GetWindowRect(*phwnd, prect);
	DWORD dwExStyle = GetWindowLong(*phwnd, GWL_EXSTYLE);
	if(!(dwExStyle & WS_EX_LAYERED)) {
		SetWindowLong(*phwnd, GWL_EXSTYLE, dwExStyle|0x80000);
		*palpha = 255;
		setHwndAlpha(*phwnd, *palpha);
	} else {
		long unsigned temp_LWA_ALPHA = 0x00000002, *temp = &temp_LWA_ALPHA;
		GetLayeredWindowAttributes(*phwnd, NULL, palpha, temp);
	}
}

void doMainLoop() {
	uchar __incOpacity[3]  = {VK_MENU, getKey('N'), getKey('+')};
	uchar __decOpacity[3]  = {VK_MENU, getKey('N'), getKey('-')};
	uchar __setOpacity0[3] = {VK_MENU, getKey('N'), getKey('0')};
	uchar __setOpacity1[3] = {VK_MENU, getKey('N'), getKey('1')};
	uchar __setOpacity8[3] = {VK_MENU, getKey('N'), getKey('8')};
	uchar __setOpacity9[3] = {VK_MENU, getKey('N'), getKey('9')};
	uchar __setWindowR[3]  = {VK_MENU, getKey('N'), VK_RIGHT};
	uchar __setWindowL[3]  = {VK_MENU, getKey('N'), VK_LEFT};
	uchar __setWindowD[3]  = {VK_MENU, getKey('N'), VK_DOWN};
	uchar __setWindowU[3]  = {VK_MENU, getKey('N'), VK_UP};
	uchar __hideWindow[3]  = {VK_MENU, getKey('N'), getKey('\\')};
	uchar __toggleClick[3] = {VK_MENU, getKey('N'), getKey('[')};
	uchar __toggleTop[3]   = {VK_MENU, getKey('N'), getKey(']')};
	uchar __exitLoop[3]    = {VK_MENU, getKey('N'), VK_BACK};

	HWND hwnd;
	RECT rect;
	uchar alpha;
	for(; 1; Sleep(1)) {
		if(ifKeyDownMul(__incOpacity, sizeof(__incOpacity))) {
			updateHwnd(&hwnd, &rect, &alpha);
			alpha = umin(255, alpha + 1);
			setHwndAlpha(hwnd, alpha);
			clock_t st = clock();
			uchar talpha = alpha;
			while(ifKeyDownMul(__incOpacity, sizeof(__incOpacity))) {
				clock_t ed = clock();
				if(getDeltaTime(st, ed) >= limitTime) {
					int cnt = (getDeltaTime(st, ed) - limitTime) / gapTimeAlpha;
					alpha = umin(255, talpha + cnt);
					setHwndAlpha(hwnd, alpha);
				}
			}
		}
		if(ifKeyDownMul(__decOpacity, sizeof(__decOpacity))) {
			updateHwnd(&hwnd, &rect, &alpha);
			alpha = umax(1, alpha - 1);
			setHwndAlpha(hwnd, alpha);
			clock_t st = clock();
			uchar talpha = alpha;
			while(ifKeyDownMul(__decOpacity, sizeof(__decOpacity))) {
				clock_t ed = clock();
				if(getDeltaTime(st, ed) >= limitTime) {
					int cnt = (getDeltaTime(st, ed) - limitTime) / gapTimeAlpha;
					alpha = umax(1, talpha - cnt);
					setHwndAlpha(hwnd, alpha);
				}
			}
		}
		if(ifKeyDownMul(__setOpacity0, sizeof(__setOpacity0))) {
			updateHwnd(&hwnd, &rect, &alpha);
			alpha = 1;
			setHwndAlpha(hwnd, alpha);
		}
		if(ifKeyDownMul(__setOpacity1, sizeof(__setOpacity1))) {
			updateHwnd(&hwnd, &rect, &alpha);
			alpha = 30;
			setHwndAlpha(hwnd, alpha);
		}
		if(ifKeyDownMul(__setOpacity8, sizeof(__setOpacity8))) {
			updateHwnd(&hwnd, &rect, &alpha);
			alpha = 225;
			setHwndAlpha(hwnd, alpha);
		}
		if(ifKeyDownMul(__setOpacity9, sizeof(__setOpacity9))) {
			updateHwnd(&hwnd, &rect, &alpha);
			alpha = 255;
			setHwndAlpha(hwnd, alpha);
		}
		if(ifKeyDownMul(__setWindowR, sizeof(__setWindowR))) {
			updateHwnd(&hwnd, &rect, &alpha);
			rect.right = imax(rect.left + 10, rect.right + 1);
			setHwndPos(hwnd, rect);
			clock_t st = clock();
			int tr = rect.right;
			while(ifKeyDownMul(__setWindowR, sizeof(__setWindowR))) {
				clock_t ed = clock();
				if(getDeltaTime(st, ed) >= limitTime) {
					int cnt = (getDeltaTime(st, ed) - limitTime) / gapTimeHwnd;
					rect.right = tr + cnt;
					setHwndPos(hwnd, rect);
				}
			}
		}
		if(ifKeyDownMul(__setWindowL, sizeof(__setWindowL))) {
			updateHwnd(&hwnd, &rect, &alpha);
			rect.right = imax(rect.left + 10, rect.right - 1);
			setHwndPos(hwnd, rect);
			clock_t st = clock();
			int tr = rect.right;
			while(ifKeyDownMul(__setWindowL, sizeof(__setWindowL))) {
				clock_t ed = clock();
				if(getDeltaTime(st, ed) >= limitTime) {
					int cnt = (getDeltaTime(st, ed) - limitTime) / gapTimeHwnd;
					rect.right = imax(rect.left + 10, tr - cnt);
					setHwndPos(hwnd, rect);
				}
			}
		}
		if(ifKeyDownMul(__setWindowD, sizeof(__setWindowD))) {
			updateHwnd(&hwnd, &rect, &alpha);
			rect.bottom = imax(rect.top + 10, rect.bottom + 1);
			setHwndPos(hwnd, rect);
			clock_t st = clock();
			int tr = rect.bottom;
			while(ifKeyDownMul(__setWindowD, sizeof(__setWindowD))) {
				clock_t ed = clock();
				if(getDeltaTime(st, ed) >= limitTime) {
					int cnt = (getDeltaTime(st, ed) - limitTime) / gapTimeHwnd;
					rect.bottom = tr + cnt;
					setHwndPos(hwnd, rect);
				}
			}
		}
		if(ifKeyDownMul(__setWindowU, sizeof(__setWindowU))) {
			updateHwnd(&hwnd, &rect, &alpha);
			rect.bottom = imax(rect.top + 10, rect.bottom - 1);
			setHwndPos(hwnd, rect);
			clock_t st = clock();
			int tr = rect.bottom;
			while(ifKeyDownMul(__setWindowU, sizeof(__setWindowU))) {
				clock_t ed = clock();
				if(getDeltaTime(st, ed) >= limitTime) {
					int cnt = (getDeltaTime(st, ed) - limitTime) / gapTimeHwnd;
					rect.bottom = imax(rect.top + 10, tr - cnt);
					setHwndPos(hwnd, rect);
				}
			}
		}
		if(ifKeyDownMul(__hideWindow, sizeof(__hideWindow))) {
			updateHwnd(&hwnd, &rect, &alpha);
			ShowWindow(hwnd, 0);
			DWORD dwExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			SetWindowLong(hwnd, GWL_EXSTYLE, (dwExStyle | WS_EX_TOOLWINDOW) & WS_EX_APPWINDOW);
			waitKeyUpMul(__hideWindow, sizeof(__hideWindow));
		}
		if(ifKeyDownMul(__toggleClick, sizeof(__toggleClick))) {
			updateHwnd(&hwnd, &rect, &alpha);
			DWORD dwExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			SetWindowLong(hwnd, GWL_EXSTYLE, dwExStyle ^ 0x20L);
			waitKeyUpMul(__toggleClick, sizeof(__toggleClick));
		}
		if(ifKeyDownMul(__toggleTop, sizeof(__toggleTop))) {
			updateHwnd(&hwnd, &rect, &alpha);
			if(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) {
				SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}else{
				SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}
			waitKeyUpMul(__toggleTop, sizeof(__toggleTop));
		}
		if(ifKeyDownMul(__exitLoop, sizeof(__exitLoop))) {
			return;
		}
	}
}

void printHelp() {
	puts("Press Alt + { N } + Num { 0, 1, 8, 9 } to Set Opacity.\n");
	puts("Press Alt + { N } + { + } / { - } to Increase / Decrease Opacity.\n");
	puts("Press Alt + { N } + { Arrow Keys } to Resize.\n");
	puts("Press Alt + { N } + { [ } to Toggle Clicking Through the Window.\n");
	puts("Press Alt + { N } + { ] } to Toggle Always-on-Top.\n");
	puts("Press Alt + { N } + { \\ } to Hide a Window.");
	puts(" >> Once Hidden, It Can't be Shown Again.\n");
	puts("Press Alt + { N } + Backspace to Exit.\n");
}


int main() {
	printHelp();
	doMainLoop();
}