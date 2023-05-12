#include <Windows.h>
#include <tchar.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


#pragma once
#define OnAboutApp		2
#define OnExitSoftware		4
const int MAX_ENTER_LENGTH = 10;                //максимальное число вводимых цифр
const int MAX_LENGTH = MAX_ENTER_LENGTH * 2;     //максимальное число в дробях

HWND hEdit;
HWND hButtons[22];
HWND hTextBox;
HWND hErrorEdit;
HWND hWnd;
HWND hSqr, hRev, hDiv, hMult, hSub, hAdd, hEqual;
HWND hC;

WCHAR curOpNomText[MAX_LENGTH + 1], curOpDenomText[MAX_LENGTH + 1];
WCHAR editText[MAX_LENGTH * 2];

static const int ButtonWidth = 50;
static const int ButtonHeight = 50;
bool isError;
bool showDenom;
int state;    // 1 - операнд1, 2 - оператор, 3 - операнд2, 4 - =
bool curOpState;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void MainWndAddMenus(HWND hWnd);
void MainWndAddWidgets(HWND hWnd);







struct Op {         
    int nom;        
    int denom;      
    int sign;
};

Op op1;
Op curOp;
int curOperator; 

void SetStr(WCHAR* str, const WCHAR* s) {
    wsprintf(str, TEXT("%s"), s);
}

void SetStr(WCHAR* str, int i) {
    wsprintf(str, TEXT("%d"), i);
}

void AddStr(WCHAR* str, WCHAR* c) {
    wsprintf(str, TEXT("%s%s"), str, c);
}

void ChangeErrorStatus() {
    if (isError) {
        isError = false;
        SetWindowText(hErrorEdit, L"");
    }
    else {
        isError = true;
        SetWindowText(hErrorEdit, L"E");
    }
    UpdateWindow(hWnd);
}

void AddDigitToStr(int digit) {
    if (!curOpState) {
        int i = wcslen(curOpNomText);
        if (i == 1 && curOpNomText[0] == '0')
            SetStr(curOpNomText, L"");
        wsprintf(curOpNomText, TEXT("%s%d"), curOpNomText, digit);
    }
    else
        wsprintf(curOpDenomText, TEXT("%s%d"), curOpDenomText, digit);
}

void SetEditText() {
    if (curOp.sign == 1) {
        SetStr(editText, L"");
    }
    else
        SetStr(editText, L"-");

    if (showDenom) {
        if (!curOpState && wcslen(curOpDenomText) == 0) {
            AddStr(editText, curOpNomText);
        }
        else {
            wsprintf(editText, TEXT("%s%s/%s"), editText, curOpNomText, curOpDenomText);
        }
    }
    else {
        if (((state == 1 || state == 3) && (curOpState)) || ((state == 2 || state == 4) && (wcslen(curOpDenomText) > 0) && (curOpDenomText[0] != '1')))
            wsprintf(editText, TEXT("%s%s/%s"), editText, curOpNomText, curOpDenomText);
        else
            AddStr(editText, curOpNomText);
    }
    SetWindowText(hEdit, editText);
    UpdateWindow(hWnd);
}

bool canAddDigit(int digit) {
    if (!curOpState) {
        int i = wcslen(curOpNomText);
        if (i < MAX_ENTER_LENGTH)
            return true;
        return false;
    }
    else {
        int i = wcslen(curOpDenomText);
        if (i < MAX_ENTER_LENGTH && (i > 0 || digit != 0))
            return true;
        return false;
    }
}

void ClearOp(Op& op) {
    op.nom = 0;
    op.denom = 1;
    op.sign = 1;
}

void ClearCurOp() {
    ClearOp(curOp);
    SetStr(curOpNomText, L"0");
    SetStr(curOpDenomText, L"");
    curOpState = false;
    SetEditText();
    if (isError) ChangeErrorStatus();
}

void Clear() {
    ClearCurOp();
    ClearOp(op1);
    state = 1;
}

void Copy(Op& source, Op& target) {
    target.nom = source.nom;
    target.denom = source.denom;
    target.sign = source.sign;
}

int GCD(int a, int b) {     // НОД
    while (b != 0) {
        int tmp = a % b;
        a = b;
        b = tmp;
    }
    return a;
}

void TryToCancel(Op& op) {
    int i = GCD(op.nom, op.denom);
    op.nom /= i;
    op.denom /= i;
}

int ICM(int a, int b) {               // НОК
    return (a * b) / GCD(a, b);
}

bool Div(Op& op1, Op& op2) {
    if (op2.nom == 0) return false;
    op1.nom *= op2.denom;
    op1.denom *= op2.nom;
    op1.sign *= op2.sign;
    return true;
}

bool Mult(Op& op1, Op& op2) {
    op1.nom *= op2.nom;
    op1.denom *= op2.denom;
    op1.sign *= op2.sign;
    return true;
}

bool Add(Op& op1, Op& op2) {
    int i = ICM(op1.denom, op2.denom);
    int buf1 = i / op1.denom;
    int buf2 = i / op2.denom;
    op1.nom = op1.nom * buf1 * op1.sign + op2.nom * buf2 * op2.sign;
    if (op1.nom >= 0)
        op1.sign = 1;
    else {
        op1.sign = -1;
        op1.nom = abs(op1.nom);
    }
    op1.denom = i;
    return true;
}

bool Sub(Op& op1, Op& op2) {
    op2.sign *= -1;
    Add(op1, op2);
    return true;
}

void AddDigit(int digit) {
    if (isError) ChangeErrorStatus();
    switch (state)
    {
    case 1:
        if (canAddDigit(digit))
            AddDigitToStr(digit);
        break;
    case 2:
        if (canAddDigit(digit))
            AddDigitToStr(digit);
        state = 3;
        break;
    case 3:
        if (canAddDigit(digit))
            AddDigitToStr(digit);
        break;
    case 4:
        state = 1;
        Clear();
        AddDigit(digit);
        break;
    default:
        break;
    }
    SetEditText();
}

void SetCurOp() {
    curOp.nom = _ttoi(curOpNomText);
    int i = _ttoi(curOpDenomText);
    if (i == 0)
        curOp.denom = 1;
    else curOp.denom = i;
}

void SetTextCurOp() {
    SetStr(curOpNomText, curOp.nom);
    SetStr(curOpDenomText, curOp.denom);
}

void Equal() {
    if (state != 1) {
        state = 4;
        SetCurOp();
        bool check;
        switch (curOperator)
        {
        case 1:
            check = Div(op1, curOp);
            break;
        case 2:
            check = Mult(op1, curOp);
            break;
        case 3:
            check = Sub(op1, curOp);
            break;
        case 4:
            check = Add(op1, curOp);
            break;
        default:
            break;
        }
        if (check) {
            TryToCancel(op1);
            Copy(op1, curOp);
            SetTextCurOp();
            SetEditText();
        }
        else {
            Clear();
            ChangeErrorStatus();
        }
    }
}

void SetOperator(int i) {
    switch (state)
    {
    case 1: //деление
        state = 2;
        SetCurOp();
        Copy(curOp, op1);
        ClearCurOp();
        curOperator = i;
        break;
    case 2: //умножение
        curOperator = i;
        break;
    case 3: //вычитание 
        Equal();
        state = 1;
        SetOperator(i);
        break;
    case 4: //сложение
        state = 1;
        SetOperator(i);
        break;
    default:
        break;
    }
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR CmdLine, int CmdShow) {
	WNDCLASS wcex;
	MSG msg;

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInst;
    wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"Calculator";
    
	if (!RegisterClassW(&wcex)) { return -1; }

    CreateWindow( L"Calculator", L"Калькулятор", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 310, 400, NULL, NULL, NULL, NULL);

    while (GetMessage(&msg, NULL, NULL, NULL)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}



BOOL CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message)
    {
    case WM_CLOSE:
        EndDialog(hWnd, 0);
        return FALSE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hWnd, 0);
            return FALSE;
        }
    default:
        break;
    }

    return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    WORD code;
    switch (message) {
    case WM_KEYDOWN: {
        code = LOWORD(wParam);
        if (code == 0x30 || code == 0x60)
            AddDigit(0);
        if (code == 0x31 || code == 0x61)
            AddDigit(1);
        if (code == 0x32 || code == 0x62)
            AddDigit(2);
        if (code == 0x33 || code == 0x63)
            AddDigit(3);
        if (code == 0x34 || code == 0x64)
            AddDigit(4);
        if (code == 0x35 || code == 0x65)
            AddDigit(5);
        if (code == 0x36 || code == 0x66)
            AddDigit(6);
        if (code == 0x37 || code == 0x67)
            AddDigit(7);
        if (code == 0x38 || code == 0x68)
            AddDigit(8);
        if (code == 0x39 || code == 0x69)
            AddDigit(9);
        if (code == VK_DIVIDE) {
            if ((state == 1 || state == 3) && (!curOpState)) {
                curOpState = true;
                SetEditText();
            }
            else SetOperator(1);
        }
        if (code == VK_MULTIPLY)
            SetOperator(2);
        if (code == VK_SUBTRACT)
            SetOperator(3);
        if (code == VK_ADD)
            SetOperator(4);
        if (code == VK_RETURN)
            Equal();
        if (code == VK_BACK)
        {
            if (!curOpState) {
                int i = wcslen(curOpNomText);
                if (i > 0) curOpNomText[i - 1] = 0;
                if (i == 1) curOpNomText[0] = '0';
                SetEditText();
            }
            else {
                int i = wcslen(curOpDenomText);
                if (i > 0) curOpDenomText[i - 1] = 0;
                if (i == 1) curOpNomText[0] = '0';
                SetEditText();
            }
        }
        if (code == VK_DELETE)
            Clear();
    }
                   break;
    case WM_COMMAND: {
        
        switch (wParam) {
        case OnAboutApp:
            MessageBoxA(hWnd, "Это калькулятор", "О приложении", MB_OK);
            break;
        case OnExitSoftware:
            PostQuitMessage(0);
            break;
        }
        if (lParam == (LPARAM)hButtons[0]) {
            AddDigit(0);
        }
        if (lParam == (LPARAM)hButtons[1]) {
            AddDigit(1);
        }
        if (lParam == (LPARAM)hButtons[2]) {
            AddDigit(2);
        }
        if (lParam == (LPARAM)hButtons[3]) {
            AddDigit(3);
        }
        if (lParam == (LPARAM)hButtons[4]) {
            AddDigit(4);
        }
        if (lParam == (LPARAM)hButtons[5]) {
            AddDigit(5);
        }
        if (lParam == (LPARAM)hButtons[6]) {
            AddDigit(6);
        }
        if (lParam == (LPARAM)hButtons[7]) {
            AddDigit(7);
        }
        if (lParam == (LPARAM)hButtons[8]) {
            AddDigit(8);
        }
        if (lParam == (LPARAM)hButtons[9]) {
            AddDigit(9);
        }

        if (lParam == (LPARAM)hDiv) {
            if ((state == 1 || state == 3) && (!curOpState)) {
                curOpState = true;
                SetEditText();
            }
            else SetOperator(1);
        }

        if (lParam == (LPARAM)hMult) {
            SetOperator(2);
        }
        if (lParam == (LPARAM)hSub) {
            SetOperator(3);
        }
        if (lParam == (LPARAM)hAdd) {
            SetOperator(4);
        }
        if (lParam == (LPARAM)hEqual) {
            Equal();
        }


        if (lParam == (LPARAM)hC) {
            Clear();
        }
    }
        break;
    case WM_CREATE:
        MainWndAddMenus(hWnd);
        MainWndAddWidgets(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default: return DefWindowProc(hWnd, message, wParam, lParam);
    }
    SetFocus(hWnd);
    return 0;
}

void MainWndAddMenus(HWND hWnd) {
    HMENU RootMenu = CreateMenu();
    HMENU SubMenu = CreateMenu();

    AppendMenu(SubMenu, MF_STRING, OnAboutApp, L"О приложении");
    AppendMenu(SubMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(SubMenu, MF_STRING, OnExitSoftware, L"Выход");
    AppendMenu(RootMenu, MF_POPUP, (UINT_PTR)SubMenu, L"Меню");
    SetMenu(hWnd, RootMenu);
}

void MainWndAddWidgets(HWND hWnd) {
    hEdit = CreateWindowA("edit", "0", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_RIGHT, 10, 5, 270, 25, hWnd, NULL, NULL, NULL);


    hButtons[0] = CreateWindowA("BUTTON", "0", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        10, 260, ButtonWidth * 2 + 5, ButtonHeight, hWnd, NULL, NULL, NULL);

    hButtons[1] = CreateWindowA("BUTTON", "1", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        10, 205, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hButtons[2] = CreateWindowA("BUTTON", "2", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth + 15, 205, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hButtons[3] = CreateWindowA("BUTTON", "3", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth * 2 + 20, 205, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hButtons[4] = CreateWindowA("BUTTON", "4", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        10, 150, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hButtons[5] = CreateWindowA("BUTTON", "5", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth + 15, 150, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hButtons[6] = CreateWindowA("BUTTON", "6", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth * 2 + 20, 150, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hButtons[7] = CreateWindowA("BUTTON", "7", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        10, 95, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hButtons[8] = CreateWindowA("BUTTON", "8", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth + 15, 95, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hButtons[9] = CreateWindowA("BUTTON", "9", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth * 2 + 20, 95, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hButtons[10] = CreateWindowA("BUTTON", ".", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth * 2 + 20, 260, ButtonWidth * 2 + 5, ButtonHeight, hWnd, NULL, NULL, NULL);

    hAdd = hButtons[11] = CreateWindowA("BUTTON", "+", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth * 3 + 25, 205, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hSub = hButtons[12] = CreateWindowA("BUTTON", "-", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth * 3 + 25, 150, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hMult = hButtons[13] = CreateWindowA("BUTTON", "x", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth * 3 + 25, 95, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hDiv = hButtons[14] = CreateWindowA("BUTTON", "/", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth * 3 + 25, 40, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hButtons[15] = CreateWindowA("BUTTON", "!", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth * 4 + 30, 150, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hButtons[16] = CreateWindowA("BUTTON", "P", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth * 4 + 30, 40, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hButtons[17] = CreateWindowA("BUTTON", "C", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth * 4 + 30, 95, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hButtons[18] = CreateWindowA("BUTTON", "(", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth + 15, 40, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hButtons[19] = CreateWindowA("BUTTON", ")", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth * 2 + 20, 40, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);

    hC = hButtons[20] = CreateWindowA("BUTTON", "<-", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        10, 40, ButtonWidth, ButtonHeight, hWnd, NULL, NULL, NULL);
    hEqual = hButtons[21] = CreateWindowA("BUTTON", "=", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        ButtonWidth * 4 + 30, 205, ButtonWidth, ButtonHeight * 2 + 5, hWnd, NULL, NULL, NULL);

    // Устанавливаем шрифты для всех элементов
    HFONT hFont = CreateFont(-14, 0, 0, 0, 400, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");
    SendMessage(hTextBox, WM_SETFONT, (WPARAM)hFont, TRUE);
    for (int i = 0; i < 22; i++) {
        SendMessage(hButtons[i], WM_SETFONT, (WPARAM)hFont, TRUE);
    }
}
