#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include <string>
#include <format>
#define _USE_MATH_DEFINES
#include <math.h>
#include <sstream> // std::stringstream
#include <istream> // std::getline


INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	HWND hDlg;
	hDlg = CreateDialog(
		hInstance,
		MAKEINTRESOURCE(IDD_DIALOG1),
		NULL, // 親ウィンドウはなし
		DialogProc // ダイアログボックスプロシージャ
	);
	ShowWindow(hDlg, nCmdShow);
	InvalidateRect(hDlg, NULL, true);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		// ダイアログボックス上でのメッセージだけを仕分け
		if (!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}

void ChangeFontSize(HFONT* hFont, HFONT* hAmPmFont, int currentFontSize) {
	*hFont = CreateFont(
		currentFontSize, // フォントの高さ(大きさ)。
		0,
		0, 0,
		FW_DONTCARE,
		FALSE,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DRAFT_QUALITY,
		DEFAULT_PITCH,
		L"ＭＳ Ｐゴシック"
	);
	*hAmPmFont = CreateFont(
		currentFontSize / 2, // フォントの高さ(大きさ)。
		0,
		0, 0,
		FW_DONTCARE,
		FALSE,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DRAFT_QUALITY,
		DEFAULT_PITCH,
		L"ＭＳ Ｐゴシック"
	);
}

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static HFONT hFont;
	static wchar_t strTime[128];

	static HBRUSH bkColorBrush = CreateSolidBrush(RGB(240, 255, 240)); //背景色のブラシを用意

	int minFontSize = 60;
	int maxFontSize = 120;
	int fontSizeDelta = 10;

	static bool is24h = true;
	static HFONT hAmPmFont;
	static int currentFontSize;

	static bool isDigital = true;

	static HPEN hPen;
	static HPEN hSecondPen;

	static OPENFILENAME ofn = { 0 };
	static wchar_t strFilePath[MAX_PATH] = { 0 };
	static DWORD dwSize;

	//ダイアログプロシージャ
	switch (uMsg)
	{
	case WM_CTLCOLORDLG:
		// WM_CTLCOLORDLG はシステムがダイアログ ボックスを描画する前に、ダイアログ ボックスに送信
		// ダイアログの背景色を設定
		return (LRESULT)bkColorBrush;
	case WM_CTLCOLORSTATIC: // STATICの背景色
		SetBkMode(((HDC)wParam), TRANSPARENT);
		SetTextColor(((HDC)wParam), RGB(200, 0, 0));//文字の色
		return (LRESULT)bkColorBrush;
	case WM_INITDIALOG: {

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hDlg;
		// 表示文字列\0フィルタ文字列\0
		ofn.lpstrFilter = L"Text files {*.txt}\0*.txt\0";
		ofn.lpstrCustomFilter = NULL;
		ofn.nMaxCustFilter = 0;
		ofn.nFilterIndex = 0;
		ofn.lpstrFile = strFilePath;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT; // ofnは保存ダイアログと共用。保存ダイアログ用のオプションも含めておく。


		// 1秒ごとにタイマー呼び出し
		SetTimer(hDlg, 1, 1000, NULL);

		// 指定のダイアログボックス内のコントロールへメッセージを送る
		for (int size = minFontSize; size <= maxFontSize; size += fontSizeDelta) {
			SendDlgItemMessage(hDlg,
				IDC_COMBO1,
				CB_ADDSTRING,
				0,
				(LPARAM)std::format(L"{:d}", size).c_str());
		}

		// maxFontSizeを選択
		SendDlgItemMessage(hDlg,
			IDC_COMBO1,
			CB_SETCURSEL,
			(maxFontSize - minFontSize) / fontSizeDelta,
			0);


		SendDlgItemMessage(hDlg,
			IDC_COMBO2,
			CB_ADDSTRING,
			0,
			(LPARAM)L"12H表示");

		SendDlgItemMessage(hDlg,
			IDC_COMBO2,
			CB_ADDSTRING,
			0,
			(LPARAM)L"24H表示");

		// 24Hを選択
		SendDlgItemMessage(hDlg,
			IDC_COMBO2,
			CB_SETCURSEL,
			1,
			0);

		// ラジオボタンにチェックを入れる
		SendDlgItemMessage(hDlg, IDC_RADIO_DIGITAL, BM_SETCHECK, BST_CHECKED, 0);


		currentFontSize = maxFontSize;

		// フォントを作成する
		hFont = CreateFont(
			currentFontSize,          // フォントの高さ(大きさ)。
			0,                    // フォントの幅。0でデフォルト。
			0, 0,                 // 角度
			FW_DONTCARE,          // 文字の太さ
			FALSE,                // フォントがイタリックならTRUE
			FALSE,                // 下線を引くならTRUE
			FALSE,                // 取り消し線を引くならTRUE
			DEFAULT_CHARSET,      // フォントの文字セット
			OUT_DEFAULT_PRECIS,   // 出力精度の設定
			CLIP_DEFAULT_PRECIS,  // クリッピング精度
			DRAFT_QUALITY,        // フォントの出力品質
			DEFAULT_PITCH,        // フォントのピッチとファミリを指定
			L"ＭＳ Ｐゴシック" // フォント名
		);

		hAmPmFont = CreateFont(
			currentFontSize / 2,          // フォントの高さ(大きさ)。
			0,                    // フォントの幅。0でデフォルト。
			0, 0,                 // 角度
			FW_DONTCARE,          // 文字の太さ
			FALSE,                // フォントがイタリックならTRUE
			FALSE,                // 下線を引くならTRUE
			FALSE,                // 取り消し線を引くならTRUE
			DEFAULT_CHARSET,      // フォントの文字セット
			OUT_DEFAULT_PRECIS,   // 出力精度の設定
			CLIP_DEFAULT_PRECIS,  // クリッピング精度
			DRAFT_QUALITY,        // フォントの出力品質
			DEFAULT_PITCH,        // フォントのピッチとファミリを指定
			L"ＭＳ Ｐゴシック" // フォント名
		);

		hPen = CreatePen(
			PS_SOLID,
			4,
			RGB(0, 0, 0)
		);

		hSecondPen = CreatePen(
			PS_SOLID,
			1,
			RGB(255, 0, 0)
		);
		return TRUE;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_SAVE: {
			if (!GetSaveFileName(&ofn)) return TRUE;
			HANDLE hFile = CreateFile(strFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			if (hFile == INVALID_HANDLE_VALUE) {
				MessageBox(NULL, L"ファイルが開けません", NULL, MB_OK);
				return TRUE;
			}
			std::string text = std::format("{},{},{}", currentFontSize, is24h ? 24 : 12, isDigital ? 'd' : 'a');
			WriteFile(hFile, text.c_str(), text.size(), &dwSize, NULL);

			CloseHandle(hFile);
				return TRUE;
		}
		case ID_LOAD: {
			if (!GetOpenFileName(&ofn)) return TRUE;
			HANDLE hFile = CreateFile(strFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			if (hFile == INVALID_HANDLE_VALUE) {
				MessageBox(NULL, L"ファイルが開けません", NULL, MB_OK);
				return TRUE;
			}

			std::string text;
			text.resize(GetFileSize(hFile, NULL));
			// &text[0] は内部文字列バッファーの先頭アドレス
			if (TRUE == ReadFile(hFile, &text[0], GetFileSize(hFile, NULL), &dwSize, NULL)) {
				OutputDebugStringA(text.c_str());
				std::stringstream stream{ text };
				std::string buf;
				// 設定ファイルのデータ形式
				// フォントサイズ(60-120),12 or 24
				// 例）70,12

				// 最初の,まで読込
				std::getline(stream, buf, ',');
				// フォントサイズ変更
				currentFontSize = atoi(buf.c_str());
				SendDlgItemMessage(hDlg,
					IDC_COMBO1,
					CB_SETCURSEL,
					(currentFontSize - minFontSize) / fontSizeDelta,
					0);
				ChangeFontSize(&hFont, &hAmPmFont, currentFontSize);

				// 12H/24H
				std::getline(stream, buf, ',');
				int style = atoi(buf.c_str());
				int index = 0;
				if (style == 12) is24h = false;
				else {
					is24h = true;
					index = 1;
				}
				SendDlgItemMessage(hDlg,
					IDC_COMBO2,
					CB_SETCURSEL,
					index,
					0);

				// Digital/Analog
				std::getline(stream, buf, ',');
				if (buf == "a") {
					isDigital = false;
					SendDlgItemMessage(hDlg,
						IDC_RADIO_DIGITAL,
						BM_SETCHECK,
						BST_UNCHECKED,
						0);
					SendDlgItemMessage(hDlg,
						IDC_RADIO_ANALOG,
						BM_SETCHECK,
						BST_CHECKED,
						0);
				}
				else {
					isDigital = true;
					SendDlgItemMessage(hDlg,
						IDC_RADIO_DIGITAL,
						BM_SETCHECK,
						BST_CHECKED,
						0);
					SendDlgItemMessage(hDlg,
						IDC_RADIO_ANALOG,
						BM_SETCHECK,
						BST_UNCHECKED,
						0);
				}


			}
			CloseHandle(hFile);
			return TRUE;
		}
		case IDC_RADIO_DIGITAL:
			if (HIWORD(wParam) == BN_CLICKED) {
				isDigital = true;
			}
			return TRUE;
		case IDC_RADIO_ANALOG:
			if (HIWORD(wParam) == BN_CLICKED) {
				isDigital = false;
			}
			return TRUE;
		case IDC_COMBO1:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int index = (int)SendDlgItemMessage(hDlg, IDC_COMBO1, CB_GETCURSEL, 0, 0);
				DeleteObject(hFont);
				currentFontSize = minFontSize + fontSizeDelta * index;
				ChangeFontSize(&hFont, &hAmPmFont, currentFontSize);
			}
			return TRUE;
		case IDC_COMBO2:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int index = (int)SendDlgItemMessage(hDlg, IDC_COMBO2, CB_GETCURSEL, 0, 0);
				if (index == 0) is24h = false;
				else is24h = true;
			}
		}
		return TRUE;
	case WM_TIMER:
		InvalidateRect(hDlg, NULL, true);
		return TRUE;
	case WM_PAINT: {
		SYSTEMTIME stTime;
		GetLocalTime(&stTime);

		if (isDigital) {
			int hour;
			if (is24h) hour = stTime.wHour;
			else hour = stTime.wHour % 12;
			std::wstring strClock = std::format(L"{:02}:{:02}:{:02}", hour, stTime.wMinute, stTime.wSecond);

			hdc = BeginPaint(hDlg, &ps);
			SelectObject(hdc, hFont);
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, RGB(180, 220, 180));
			RECT rc = { 10, 30, 450, 300 };
			DrawText(hdc, strClock.c_str(), -1, &rc, DT_CENTER | DT_VCENTER);

			if (!is24h) {
				SelectObject(hdc, hAmPmFont);
				RECT rcAmPm = { 10, 30 + currentFontSize, 450, 300 };
				if (stTime.wHour > 12) {
					DrawText(hdc, L"PM", -1, &rcAmPm, DT_CENTER | DT_VCENTER);
				}
				else {
					DrawText(hdc, L"AM", -1, &rcAmPm, DT_CENTER | DT_VCENTER);
				}
			}

			EndPaint(hDlg, &ps);

		}
		else {
			hdc = BeginPaint(hDlg, &ps);
			int centerX = 235; // 時計の中心のX座標
			int centerY = 100;  // 時計の中心のY座標
			int radius = 90;  // 時計の半径
			int scaleLen = 7; // 目盛りの長さ
			// 目盛りを描画
			for (int i = 0; i < 12; i++) {
				double fromX = (radius - scaleLen) * sin(2 * M_PI / 12.0 * i);
				double toX = radius * sin(2 * M_PI / 12.0 * i);
				double fromY = (radius - scaleLen) * cos(2 * M_PI / 12.0 * i);
				double toY = radius * cos(2 * M_PI / 12.0 * i);
				MoveToEx(hdc, centerX + fromX, centerY + fromY, NULL);
				LineTo(hdc, centerX + toX, centerY + toY);
			}

			SelectObject(hdc, hPen);
			// 短針を描画
			int shortLen = 40; // 短針の長さ
			int hour = stTime.wHour % 12;
			double toX = shortLen * sin(2 * M_PI / 12.0 * (hour + stTime.wMinute / 60.0));
			double toY = shortLen * cos(2 * M_PI / 12.0 * (hour + stTime.wMinute / 60.0));
			MoveToEx(hdc, centerX, centerY, NULL);
			LineTo(hdc, centerX + toX, centerY - toY);

			// 長針を描画
			int longLen = 75; // 短針の長さ
			toX = longLen * sin(2 * M_PI / 60.0 * stTime.wMinute);
			toY = longLen * cos(2 * M_PI / 60.0 * stTime.wMinute);
			MoveToEx(hdc, centerX, centerY, NULL);
			LineTo(hdc, centerX + toX, centerY - toY);

			// 秒針を描画
			SelectObject(hdc, hSecondPen);
			int secondLen = 88; // 秒針の長さ
			toX = secondLen * sin(2 * M_PI / 60.0 * stTime.wSecond);
			toY = secondLen * cos(2 * M_PI / 60.0 * stTime.wSecond);
			MoveToEx(hdc, centerX, centerY, NULL);
			LineTo(hdc, centerX + toX, centerY - toY);


			EndPaint(hDlg, &ps);
		}
		return TRUE;
	}
	case WM_CLOSE:
		DestroyWindow(hDlg);
		return TRUE;
	case WM_DESTROY:
		DeleteObject(hFont);
		DeleteObject(hAmPmFont);
		DeleteObject(hPen);
		DeleteObject(hSecondPen);
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}