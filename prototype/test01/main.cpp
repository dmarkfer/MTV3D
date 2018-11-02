#include "stdafx.h"
#include "test01.h"

#include <vector>
#include <utility>
#include <algorithm>
#include <math.h>


#define MAX_LOADSTRING 100

#define BUTTON_IDENTIFIER 1
#define DRAWBUTTON_IDENTIFIER 2

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600


#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")


struct CUSTOMVERTEX {
	FLOAT X, Y, Z;
	DWORD COLOR;
};

enum AxisType {
	XAxisType,
	YAxisType,
	ZAxisType
};

struct LegendColor {
	int red;
	int green;
	int blue;
};

struct PointWithValue {
	double x, y, z;
	long double value;
};


OPENFILENAME ofnObj;

static float scaleBase = 1.0f;
bool scaleNotSet = true;


int referencedAxisLength;
int otherAxisLength;

double refRealLen;
double othRealLen;

double greaterAxisLength;


long double minPointValue = -1.0L;
long double maxPointValue = 0.0L;

long double minPointValueRelError = -1.0L;
long double maxPointValueRelError = 0.0L;


std::vector<std::pair<long double, LegendColor> > legend;
std::vector<std::pair<long double, LegendColor> > legendRelError;


std::vector<PointWithValue> pointsToDraw;
std::vector<PointWithValue> pointsToDrawRelError;


int numberOfVerticesToDraw;

double minAxisX = 0.0;
double maxAxisX = 0.0;
double minAxisY = 0.0;
double maxAxisY = 0.0;
double minAxisZ = 0.0;
double maxAxisZ = 0.0;



HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

LPDIRECT3D9 d3d;
LPDIRECT3DDEVICE9 d3ddev;
LPDIRECT3DVERTEXBUFFER9 v_buffer = NULL;
LPDIRECT3DINDEXBUFFER9 i_buffer;


std::vector<PointWithValue> pointsWithValues;
std::vector<PointWithValue> pointsWithValuesRelError;


CUSTOMVERTEX *vertices;
int *indices;

AxisType selectedAxis;
WCHAR axisValueString[110];
DOUBLE axisValueDouble;
DOUBLE closestAxisValueDouble;

HWND hWnd;
HWND hButton;
HWND radioGroupHandle;
HWND radioButtonX;
HWND radioButtonY;
HWND radioButtonZ;
HWND hDrawButton;
HWND axisValueBox;



ATOM                MyRegisterClass(HINSTANCE);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    drawProcedure(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    legendProcedure(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
VOID				readData(LPCTSTR);
inline DWORD		utf8CharacterCounter(LPCH);

VOID initD3D(HWND hWnd);
VOID renderFrame(VOID);
VOID cleanD3D(VOID);
VOID initGraphics(VOID);

VOID drawVertices(HWND parentWindow);
VOID constructLegend(VOID);

VOID createLegendWindow(VOID);

inline bool sortFunction(PointWithValue, PointWithValue);

inline int myRound(long double);


#define CUSTOMFVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TEST01, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TEST01));

    MSG msg;

    
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEST01));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TEST01);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   hWnd = CreateWindowW(
	   szWindowClass,
	   L"FER Project App",
	   WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT, 0,
	   800, 500,
	   nullptr,
	   nullptr,
	   hInstance,
	   nullptr
   );

   CreateWindow(
	   L"STATIC",
	   L"Select file to load:",
	   WS_VISIBLE | WS_CHILD,
	   220, 50,
	   120, 25,
	   hWnd, NULL, NULL, NULL
   );

   hButton = CreateWindow(
	   L"BUTTON",
	   L"Open file",
	   WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
	   370, 50,
	   100, 30,
	   hWnd,
	   (HMENU) BUTTON_IDENTIFIER,
	   (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
	   nullptr
   );

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case DRAWBUTTON_IDENTIFIER:

				// RESET STORAGE
				cleanD3D();

				pointsToDraw.clear();



				EnableWindow(axisValueBox, FALSE);
				EnableWindow(radioGroupHandle, FALSE);
				EnableWindow(radioButtonX, FALSE);
				EnableWindow(radioButtonY, FALSE);
				EnableWindow(radioButtonZ, FALSE);
				EnableWindow(hDrawButton, FALSE);

				if (Button_GetCheck(radioButtonX) == BST_CHECKED) {
					selectedAxis = XAxisType;
				}
				else if (Button_GetCheck(radioButtonY) == BST_CHECKED) {
					selectedAxis = YAxisType;
				}
				else {
					selectedAxis = ZAxisType;
				}

				GetWindowText(axisValueBox, axisValueString, 100);
				swscanf_s(axisValueString, L"%lf", &axisValueDouble);
				/*WCHAR hg[100];
				swprintf_s(hg, L"%lf", axisValueDouble);
				OutputDebugString(hg);
				OutputDebugString(L"|debug done\n");
				if (axisValueDouble == -0.5) {
					OutputDebugString(L"jednako\n");
				}*/

				drawVertices(hWnd);

				/*WCHAR er[1000];
				swprintf_s(er, L"%d", selectedAxis);
				MessageBox(hWnd, er, L"Click Me", MB_OK);*/

				EnableWindow(radioGroupHandle, TRUE);
				EnableWindow(radioButtonX, TRUE);
				EnableWindow(radioButtonY, TRUE);
				EnableWindow(radioButtonZ, TRUE);
				EnableWindow(axisValueBox, TRUE);
				EnableWindow(hDrawButton, TRUE);

				break;
			case BUTTON_IDENTIFIER:
				if (HIWORD(wParam) == BN_CLICKED) {
					/*UINT nButton = (UINT) LOWORD(wParam);
					HWND hButtonWnd = (HWND) lParam;
					LPCWSTR szMessage = L" Hello sb";
					MessageBox(hWnd, szMessage, L"Click Me", MB_OK);*/

					
					ZeroMemory(&ofnObj, sizeof(ofnObj) );
					ofnObj.lStructSize = sizeof(ofnObj);
					//ofnObj.lpstrFilter = L"*.*";
					ofnObj.nFileOffset = 1;
					//ofnObj.hwndOwner = NULL;
					ofnObj.lpstrFile = new WCHAR[1001];
					ofnObj.lpstrFile[0] = '\0';
					ofnObj.nMaxFile = 1000;
					ofnObj.lpstrFileTitle = new WCHAR[101];
					ofnObj.lpstrFileTitle[0] = '\0';
					ofnObj.nMaxFileTitle = 100;
					ofnObj.lpstrTitle = L"Selecting data file";
					//ofnObj.nFilterIndex = 1;
					ofnObj.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

					if (GetOpenFileName(&ofnObj) ) {
						EnableWindow(hButton, FALSE);
						
						readData(ofnObj.lpstrFile);


						delete[] ofnObj.lpstrFile;
						delete[] ofnObj.lpstrFileTitle;


						/*WCHAR er[100];
						swprintf_s(er, L"M I N  %lf %lf %lf\n", minAxisX, minAxisY, minAxisZ);
						OutputDebugString(er);
						swprintf_s(er, L"M A X  %lf %lf %lf\n", maxAxisX, maxAxisY, maxAxisZ);
						OutputDebugString(er);
						swprintf_s(er, L"VALUE  %e  %e\n", minPointValue, maxPointValue);
						OutputDebugString(er);*/


						CreateWindow(
							L"STATIC",
							L"File loaded",
							WS_VISIBLE | WS_CHILD,
							300, 100,
							80, 25,
							hWnd, NULL, NULL, NULL
						);

						radioGroupHandle = CreateWindow(
							L"BUTTON",
							L"Select axis:",
							WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
							180, 180,
							100, 100,
							hWnd, NULL, NULL, NULL
						);

						radioButtonX = CreateWindow(
							L"BUTTON",
							L"X",
							WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
							210, 200,
							50, 20,
							hWnd, NULL, NULL, NULL
						);

						radioButtonY = CreateWindow(
							L"BUTTON",
							L"Y",
							WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
							210, 225,
							50, 20,
							hWnd, NULL, NULL, NULL
						);

						radioButtonZ = CreateWindow(
							L"BUTTON",
							L"Z",
							WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
							210, 250,
							50, 20,
							hWnd, NULL, NULL, NULL
						);

						SendMessage(radioButtonZ, BM_SETCHECK, BST_CHECKED, 0);




						WCHAR axisSegments[300];
						swprintf_s(axisSegments, L"Range:\n   x -> [ %1.2lf , %1.2lf ]\n   y -> [ %1.2lf , %1.2lf ]\n   z -> [ %1.2lf , %1.2lf ]",
							minAxisX, maxAxisX,
							minAxisY, maxAxisY,
							minAxisZ, maxAxisZ
						);

						CreateWindow(
							L"STATIC",
							axisSegments,
							WS_VISIBLE | WS_CHILD,
							290, 190,
							200, 80,
							hWnd, NULL, NULL, NULL
						);


						WCHAR valueBoundaries[300];
						swprintf_s(valueBoundaries, L"Result boundaries:\n   min:  %e\n   max:  %e", minPointValue, maxPointValue);

						CreateWindow(
							L"STATIC",
							valueBoundaries,
							WS_VISIBLE | WS_CHILD,
							500, 190,
							180, 50,
							hWnd, NULL, NULL, NULL
						);


						//createLegendWindow();


						CreateWindow(
							L"STATIC",
							L"Enter axis value: ",
							WS_VISIBLE | WS_CHILD,
							220, 300,
							120, 20,
							hWnd, NULL, NULL, NULL
						);

						axisValueBox = CreateWindow(
							L"EDIT",
							NULL,
							WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER | ES_LEFT,
							350, 300,
							80, 20,
							hWnd, NULL, NULL, NULL
						);

						//Edit_SetCueBannerText(axisValueBox, L"Ent");


						//constructLegend();

						
						/*WCHAR er[1000];
						for (std::vector<std::pair<long double, LegendColor> >::iterator it = legend.begin(); it < legend.end(); ++it) {
							int r = (*it).second.red;
							int g = (*it).second.green;
							int b = (*it).second.blue;
							long double lim = (*it).first;
							swprintf_s(er, L"%d %d %d %e\n", r, g, b, lim);
							OutputDebugString(er);
						}*/


						hDrawButton = CreateWindow(
							L"BUTTON",
							L"Create graphics",
							WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
							480, 300,
							150, 30,
							hWnd,
							(HMENU)DRAWBUTTON_IDENTIFIER,
							(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
							nullptr
						);

						//MessageBox(nullptr, L"Reading data from file ...", L"Reading data", MB_OK);
					}
					/*else {
						MessageBox(nullptr, L"Could not open the file!", L"Error", MB_ICONERROR);
					}*/
				}
				break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


LRESULT CALLBACK drawProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	WORD fwKeys;
	WORD zDelta;
	WORD xPos;
	WORD yPos;

	switch (message) {
	case WM_MOUSEWHEEL:

		fwKeys = GET_KEYSTATE_WPARAM(wParam);
		zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		xPos = GET_X_LPARAM(lParam);
		yPos = GET_Y_LPARAM(lParam);

		/*WCHAR er[1000];
		swprintf_s(er, L"%d %d %d %d\n", fwKeys, zDelta, xPos, yPos);
		OutputDebugString(er);*/

		if (zDelta == 120) {
			// zoom in
			scaleBase *= 1.05f;
		}
		else {
			// zoom out
			scaleBase *= 0.95f;
		}

		return 0;
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


VOID readData(LPCTSTR fileNameAndPath) {

	SetCursor(LoadCursor(NULL, IDC_WAIT));

	HANDLE hFile = CreateFile(
		fileNameAndPath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		MessageBox(nullptr, fileNameAndPath, L"Error", MB_ICONERROR);
		return;
	}

	DWORD fileSize = GetFileSize(hFile, NULL);

	if (fileSize != 0xFFFFFFFF) {
		
		LPCH fileBinaryContent = new char[fileSize + 1];
		//FillMemory(fileContentBinary, fileSize + 1, '\0');
		//OutputDebugString(L"PRVI");
		//OutputDebugString(fileContent);
		//OutputDebugString(L"DRUGI");

		if (fileBinaryContent) {
			DWORD dwRead;

			if (ReadFile(hFile, fileBinaryContent, fileSize, &dwRead, NULL) ) {

				fileBinaryContent[fileSize] = '\0';

				DWORD numberOfUTF8Characters = utf8CharacterCounter(fileBinaryContent);

				LPWSTR fileUTF8Content = new WCHAR[numberOfUTF8Characters + 1];

				MultiByteToWideChar(
					CP_UTF8,
					0,
					fileBinaryContent,
					fileSize,
					fileUTF8Content,
					numberOfUTF8Characters
				);

				delete[] fileBinaryContent;

				fileUTF8Content[numberOfUTF8Characters] = '\0';


				/*LPSTR qw = new CHAR[fileSize + 1];

				WideCharToMultiByte(
					CP_UTF8,
					0,
					fileContent,
					wideSize,
					qw,
					fileSize,
					NULL,
					NULL
				);


				HANDLE writeFile = CreateFile(
					L"C:\\projects\\test100.txt",
					GENERIC_WRITE,
					FILE_SHARE_READ,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL
				);

				DWORD r = 0;

				WriteFile(
					writeFile,
					qw,
					fileSize,
					&r,
					NULL
				);

				CloseHandle(writeFile);*/


				/*OutputDebugString(L"TRECI");
				OutputDebugString(fileContent);
				OutputDebugString(L"CETVRTI");*/
				/*WCHAR t[20];
				swprintf_s(t, 20, L"%d\0", wideSize);
				OutputDebugString(t);
				OutputDebugString(L"\n");
				swprintf_s(t, 20, L"%d\0", r);
				OutputDebugString(t);*/

				//MessageBox(nullptr, L"Please wait", L"Creating vertices", MB_OK);


				std::wistringstream fileContentAsWideSStream(fileUTF8Content);
				delete[] fileUTF8Content;
				std::wstring fileLineAsWideString;

				for (int temp_cnt = 0; temp_cnt < 15; ++temp_cnt) {
					std::getline(fileContentAsWideSStream, fileLineAsWideString);
				}


				LPCWSTR fileLineAsWideCStr = fileLineAsWideString.c_str();
				bool isPhoton = false;

				for (int i = 0; fileLineAsWideCStr[i] != '\0'; ++i) {
					if (fileLineAsWideCStr[i] != 'X') {
						if (fileLineAsWideCStr[i] != ' '  &&  fileLineAsWideCStr[i] != '\t') {
							isPhoton = true;

							break;
						}
					}
					else {
						break;
					}
				}


				//int counter = 0;
				while (std::getline(fileContentAsWideSStream, fileLineAsWideString) ) {
					//++counter;
					fileLineAsWideCStr = fileLineAsWideString.c_str();

					double x, y, z;
					long double value;
					long double relError;
					
					if (isPhoton) {
						swscanf_s(fileLineAsWideCStr, L"%*s %lf %lf %lf %lf %lf", &x, &y, &z, &value, &relError);
					}
					else {
						swscanf_s(fileLineAsWideCStr, L"%lf %lf %lf %lf %lf", &x, &y, &z, &value, &relError);
					}


					if (value < minPointValue  &&  value > 0.0L) {
						minPointValue = value;
					}
					else if (value > maxPointValue) {
						maxPointValue = value;

						if (minPointValue < 0.0L) {
							minPointValue = value;
						}
					}


					if (relError < minPointValueRelError  &&  relError > 0.0L) {
						minPointValueRelError = relError;
					}
					else if (relError > maxPointValueRelError) {
						maxPointValueRelError = relError;

						if (minPointValueRelError < 0.0L) {
							minPointValueRelError = relError;
						}
					}


					
					if (x < minAxisX) {
						minAxisX = x;
					}
					else if (x > maxAxisX) {
						maxAxisX = x;
					}

					if (y < minAxisY) {
						minAxisY = y;
					}
					else if (y > maxAxisY) {
						maxAxisY = y;
					}

					if (z < minAxisZ) {
						minAxisZ = z;
					}
					else if (z > maxAxisZ) {
						maxAxisZ = z;
					}


					PointWithValue point;
					point.x = x;
					point.y = y;
					point.z = z;
					point.value = value;

					pointsWithValues.push_back(point);


					PointWithValue pointRelError;
					pointRelError.x = x;
					pointRelError.y = y;
					pointRelError.z = z;
					pointRelError.value = relError;

					pointsWithValuesRelError.push_back(pointRelError);


					/*WCHAR er[1000];

					PointWithValue tempPoint = pointsWithValues.at(pointsWithValues.size()-1);

					swprintf_s(er, L"%d %lf %lf %lf %e\n", counter, tempPoint.x, tempPoint.y, tempPoint.z, tempPoint.value);

					OutputDebugString(er);*/
				}



				/*LPCWSTR fileLineAsWideCStr = fileLineAsWideString.c_str();

				double x, y, z;
				long double value;

				swscanf_s(fileLineAsWideCStr, L"%lf %lf %lf %lf %*s", &x, &y, &z, &value);

				WCHAR er[1000];

				swprintf_s(er, L"%lf %lf %lf %e", x, y, z, value);

				OutputDebugString(er);*/

				SetCursor(LoadCursor(NULL, IDC_ARROW));

				MessageBox(nullptr, L"DONE", L"Vertices created", MB_OK);

			} else MessageBox(nullptr, L"three", L"Error", MB_ICONERROR);

		} else MessageBox(nullptr, L"two", L"Error", MB_ICONERROR);

	} else MessageBox(nullptr, L"one", L"Error", MB_ICONERROR);

	CloseHandle(hFile);

}


inline DWORD utf8CharacterCounter(LPCH fileContentBinary) {
	DWORD counter = 0;

	for (int i = 0; fileContentBinary[i] != '\0'; ++i) {
		if ( (fileContentBinary[i] & 0xC0)  !=  0x80 ) {
			++counter;
		}
	}

	return counter;
}


VOID initD3D(HWND hWnd) {

	switch (selectedAxis) {
	case XAxisType:
		refRealLen = std::abs(maxAxisY - minAxisY);
		othRealLen = std::abs(maxAxisZ - minAxisZ);

		greaterAxisLength = (refRealLen > othRealLen) ? refRealLen : othRealLen;
		break;
	case YAxisType:
		refRealLen = std::abs(maxAxisZ - minAxisZ);
		othRealLen = std::abs(maxAxisX - minAxisX);

		greaterAxisLength = (refRealLen > othRealLen) ? refRealLen : othRealLen;
		break;
	default:
		refRealLen = std::abs(maxAxisX - minAxisX);
		othRealLen = std::abs(maxAxisY - minAxisY);

		greaterAxisLength = (refRealLen > othRealLen) ? refRealLen : othRealLen;
		break;
	}



	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	D3DPRESENT_PARAMETERS d3dpp;

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferWidth = SCREEN_WIDTH;
	d3dpp.BackBufferHeight = SCREEN_HEIGHT;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	// create a device class using this information and the info from the d3dpp struct
	d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddev
	);

	initGraphics();    // call the function to initialize the cube

	d3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);    // turn off the 3D lighting
	d3ddev->SetRenderState(D3DRS_ZENABLE, TRUE);    // turn on the z-buffer

	d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);    // turn on the color blending
	d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);    // set source factor
	d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);    // set dest factor
	d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);    // set the operation
}


// render a single frame
VOID renderFrame(VOID) {
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	d3ddev->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	d3ddev->BeginScene();

	// select which vertex format we are using
	d3ddev->SetFVF(CUSTOMFVF);

	// set an ever-increasing float value
	//static float index = 0.0f; index += 0.03f;
	
	// set the view transform
	D3DXMATRIX matView;    // the view transform matrix
	D3DXMatrixLookAtLH(
		&matView,
		//&D3DXVECTOR3((float)sin(index) * 360.0f, 0.0f, -((float)(greaterAxisLength / 2.) + 100.0f)),    // the camera position
		&D3DXVECTOR3(0.0f, (float)(greaterAxisLength * 0.6), -(float)(greaterAxisLength * 0.55f)),    // the camera position
		&D3DXVECTOR3(0.0f, 0.0f, 0.0f),    // the look-at position
		&D3DXVECTOR3(0.0f, 1.0f, 0.0f)
	);    // the up direction
	d3ddev->SetTransform(D3DTS_VIEW, &matView);    // set the view transform to matView 
	
												   // set the projection transform
	D3DXMATRIX matProjection;    // the projection transform matrix
	D3DXMatrixPerspectiveFovLH(
		&matProjection,
		D3DXToRadian(70),    // the horizontal field of view
		(FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, // aspect ratio
		1.0f,    // the near view-plane
		(float)(greaterAxisLength * 2.f * 1.1f)
	);    // the far view-plane
	d3ddev->SetTransform(D3DTS_PROJECTION, &matProjection);    // set the projection

															   // set the stream source
	d3ddev->SetStreamSource(0, v_buffer, 0, sizeof(CUSTOMVERTEX));

	// set the first world transform
	D3DXMATRIX matRotateX;
	D3DXMATRIX matRotateY;
	D3DXMATRIX matScale;
	D3DXMATRIX matTranslate;
	//D3DXMatrixScaling(&matScale, 0.1f, 0.1f, 0.1f);
	/*D3DXMatrixTranslation(&matTranslate, 0.0f, 0.0f, -10.0f);
	d3ddev->SetTransform(D3DTS_WORLD, &(matTranslate));    // set the world transform

														   // draw the first square
	d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);*/


	if (scaleNotSet) {
		scaleBase = 5. / std::sqrt(greaterAxisLength);
		scaleNotSet = false;
	}

	static float index = 0.0f; index += 0.001f;

	// set the second world transform
	D3DXMatrixRotationX(&matRotateX, D3DXToRadian(-90.0f));
	D3DXMatrixRotationY(&matRotateY, D3DXToRadian(360.0f * index));
	D3DXMatrixScaling(&matScale, scaleBase, scaleBase, scaleBase);

	// translate to center
	double translateX;
	double translateY;

	switch (selectedAxis) {
	case XAxisType:
		translateX = std::abs(std::abs(maxAxisY) - std::abs(minAxisY)) / 2.;
		if (std::abs(minAxisY) < std::abs(maxAxisY)) {
			translateX *= -1;
		}

		translateY = std::abs(std::abs(maxAxisZ) - std::abs(minAxisZ)) / 2.;
		if (std::abs(minAxisZ) < std::abs(maxAxisZ)) {
			translateY *= -1;
		}

		break;
	case YAxisType:
		translateX = std::abs(std::abs(maxAxisZ) - std::abs(minAxisZ)) / 2.;
		if (std::abs(minAxisZ) < std::abs(maxAxisZ)) {
			translateX *= -1;
		}

		translateY = std::abs(std::abs(maxAxisX) - std::abs(minAxisX)) / 2.;
		if (std::abs(minAxisX) < std::abs(maxAxisX)) {
			translateY *= -1;
		}

		break;
	default:
		translateX = std::abs(std::abs(maxAxisX) - std::abs(minAxisX)) / 2.;
		if (std::abs(minAxisX) < std::abs(maxAxisX)) {
			translateX *= -1;
		}

		translateY = std::abs(std::abs(maxAxisY) - std::abs(minAxisY)) / 2.;
		if (std::abs(minAxisY) < std::abs(maxAxisY)) {
			translateY *= -1;
		}

		break;
	}

	D3DXMatrixTranslation(&matTranslate, (float)translateX, (float)translateY, 0.0f);


	D3DXMATRIX matTranslateLeft;
	D3DXMatrixTranslation(&matTranslateLeft, - (float)(greaterAxisLength * 1.42 * 0.5 * 0.51), 0.0f, 0.0f);

	
	d3ddev->SetTransform(D3DTS_WORLD, &(matTranslate * matScale * matRotateX * matRotateY * matTranslateLeft));    // set the world transform

	d3ddev->SetIndices(i_buffer);


	d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numberOfVerticesToDraw, 0, (referencedAxisLength - 1) * (otherAxisLength - 1) * 2);






	d3ddev->SetStreamSource(0, v_buffer, numberOfVerticesToDraw * sizeof(CUSTOMVERTEX), sizeof(CUSTOMVERTEX));


	// set the second world transform
	D3DXMatrixRotationX(&matRotateX, D3DXToRadian(-90.0f));
	D3DXMatrixRotationY(&matRotateY, D3DXToRadian(360.0f * index));
	D3DXMatrixScaling(&matScale, scaleBase, scaleBase, scaleBase);


	D3DXMatrixTranslation(&matTranslate, (float)translateX, (float)translateY, 0.0f);


	D3DXMATRIX matTranslateRight;
	D3DXMatrixTranslation(&matTranslateRight, (float)(greaterAxisLength * 1.42 * 0.5 * 0.51), 0.0f, 0.0f);


	d3ddev->SetTransform(D3DTS_WORLD, &(matTranslate * matScale * matRotateX * matRotateY * matTranslateRight));    // set the world transform

	d3ddev->SetIndices(i_buffer);


	d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numberOfVerticesToDraw, 0, (referencedAxisLength - 1) * (otherAxisLength - 1) * 2);






	d3ddev->EndScene();

	d3ddev->Present(NULL, NULL, NULL, NULL);
}


// clean up Direct3D and COM
VOID cleanD3D(VOID) {
	if (v_buffer) {
		v_buffer->Release();    // close and release the vertex buffer
	}

	if (d3ddev) {
		d3ddev->Release();    // close and release the 3D device
	}
	
	if (d3d) {
		d3d->Release();    // close and release Direct3D
	}

	if (i_buffer) {
		i_buffer->Release();
	}

	delete[] vertices;
	delete[] indices;

}


// put the 3D models into video RAM
VOID initGraphics(VOID) {

	bool notSet = true;

	for (std::vector<PointWithValue>::iterator it = pointsWithValues.begin(); it != pointsWithValues.end(); ++it) {
		switch (selectedAxis) {
		case XAxisType:
			if (notSet) {
				closestAxisValueDouble = it->x;
				notSet = false;
			}
			else if (std::abs(axisValueDouble - it->x) < std::abs(axisValueDouble - closestAxisValueDouble)) {
				closestAxisValueDouble = it->x;
			}

			break;
		case YAxisType:
			if (notSet) {
				closestAxisValueDouble = it->y;
				notSet = false;
			}
			else if (std::abs(axisValueDouble - it->y) < std::abs(axisValueDouble - closestAxisValueDouble)) {
				closestAxisValueDouble = it->y;
			}

			break;
		default:
			if (notSet) {
				closestAxisValueDouble = it->z;
				notSet = false;
			}
			else if (std::abs(axisValueDouble - it->z) < std::abs(axisValueDouble - closestAxisValueDouble)) {
				closestAxisValueDouble = it->z;
			}

			break;
		}
	}


	maxPointValue = 0.0L;
	minPointValue = -1.0L;

	maxPointValueRelError = 0.0L;
	minPointValueRelError = -1.0L;


	for (std::vector<PointWithValue>::iterator it = pointsWithValues.begin(); it != pointsWithValues.end(); ++it) {
		switch (selectedAxis) {
		case XAxisType:
			if (closestAxisValueDouble == it->x) {
				PointWithValue tempPoint;
				tempPoint.x = it->y;
				tempPoint.y = it->z;
				tempPoint.z = 0.0;
				tempPoint.value = it->value;

				pointsToDraw.push_back( tempPoint );

				if (tempPoint.value > maxPointValue) {
					maxPointValue = tempPoint.value;

					if (minPointValue < 0.0L) {
						minPointValue = tempPoint.value;
					}
				}
				else if (tempPoint.value < minPointValue  &&  tempPoint.value > 0.0L) {
					minPointValue = tempPoint.value;
				}


				tempPoint.value = pointsWithValuesRelError.at(it - pointsWithValues.begin()).value;

				pointsToDrawRelError.push_back(tempPoint);

				if (tempPoint.value > maxPointValueRelError) {
					maxPointValueRelError = tempPoint.value;

					if (minPointValueRelError < 0.0L) {
						minPointValueRelError = tempPoint.value;
					}
				}
				else if (tempPoint.value < minPointValueRelError  &&  tempPoint.value > 0.0L) {
					minPointValueRelError = tempPoint.value;
				}
			}
			break;
		case YAxisType:
			if (closestAxisValueDouble == it->y) {
				PointWithValue tempPoint;
				tempPoint.x = it->z;
				tempPoint.y = it->x;
				tempPoint.z = 0.0;
				tempPoint.value = it->value;

				pointsToDraw.push_back( tempPoint );

				if (tempPoint.value > maxPointValue) {
					maxPointValue = tempPoint.value;

					if (minPointValue < 0.0L) {
						minPointValue = tempPoint.value;
					}
				}
				else if (tempPoint.value < minPointValue  &&  tempPoint.value > 0.0L) {
					minPointValue = tempPoint.value;
				}


				tempPoint.value = pointsWithValuesRelError.at(it - pointsWithValues.begin()).value;

				pointsToDrawRelError.push_back(tempPoint);

				if (tempPoint.value > maxPointValueRelError) {
					maxPointValueRelError = tempPoint.value;

					if (minPointValueRelError < 0.0L) {
						minPointValueRelError = tempPoint.value;
					}
				}
				else if (tempPoint.value < minPointValueRelError  &&  tempPoint.value > 0.0L) {
					minPointValueRelError = tempPoint.value;
				}
			}
			break;
		default:
			if (closestAxisValueDouble == it->z) {
				PointWithValue tempPoint;
				tempPoint.x = it->x;
				tempPoint.y = it->y;
				tempPoint.z = 0.0;
				tempPoint.value = it->value;

				pointsToDraw.push_back( tempPoint );

				if (tempPoint.value > maxPointValue) {
					maxPointValue = tempPoint.value;

					if (minPointValue < 0.0L) {
						minPointValue = tempPoint.value;
					}
				}
				else if (tempPoint.value < minPointValue  &&  tempPoint.value > 0.0L) {
					minPointValue = tempPoint.value;
				}


				tempPoint.value = pointsWithValuesRelError.at(it - pointsWithValues.begin()).value;

				pointsToDrawRelError.push_back(tempPoint);

				if (tempPoint.value > maxPointValueRelError) {
					maxPointValueRelError = tempPoint.value;

					if (minPointValueRelError < 0.0L) {
						minPointValueRelError = tempPoint.value;
					}
				}
				else if (tempPoint.value < minPointValueRelError  &&  tempPoint.value > 0.0L) {
					minPointValueRelError = tempPoint.value;
				}
			}
			break;
		}
	}


	constructLegend();


	// SORTING POINTS TO DRAW
	std::sort(pointsToDraw.begin(), pointsToDraw.end(), sortFunction);
	std::sort(pointsToDrawRelError.begin(), pointsToDrawRelError.end(), sortFunction);


	numberOfVerticesToDraw = pointsToDraw.size();

	vertices = new CUSTOMVERTEX[2 * numberOfVerticesToDraw];

	ZeroMemory(vertices, 2 * numberOfVerticesToDraw * sizeof(CUSTOMVERTEX) );


	int columnCounter = 0;
	bool otherAxisSet = false;
	otherAxisLength = 0;

	for (std::vector<PointWithValue>::iterator it = pointsToDraw.begin(); it != pointsToDraw.end(); ++it) {
		if (it == pointsToDraw.begin()) {
			++otherAxisLength;
		}
		else if (it->x == (it - 1)->x) {
			if (! otherAxisSet) {
				++otherAxisLength;
			}
		}
		else {
			if (! otherAxisSet) {
				otherAxisSet = true;
			}

			++columnCounter;
		}


		CUSTOMVERTEX tempVertex;
		tempVertex.X = it->x;
		tempVertex.Y = it->y;
		tempVertex.Z = (it->value == 0.0L) ? 0.0f : (greaterAxisLength / 2.) * std::abs(it->value - minPointValue) / std::abs(maxPointValue - minPointValue);

		if (it->value == 0.0L) {
			tempVertex.COLOR = D3DCOLOR_ARGB(
				255,
				255,
				255,
				255
			);
		}
		else if (std::log10(it->value) <= std::log10(legend.at(1).first)) {
			int fade = 127 - myRound(127. * std::abs(std::log10(it->value) - std::log10(legend.at(0).first)) / std::abs(std::log10(legend.at(1).first) - std::log10(legend.at(0).first)));

			tempVertex.COLOR = D3DCOLOR_ARGB(
				255,
				fade,
				legend.at(1).second.green,
				255
			);
		}
		else if (std::log10(it->value) <= std::log10(legend.at(2).first)) {
			int green = myRound(255. * std::abs(std::log10(it->value) - std::log10(legend.at(1).first)) / std::abs(std::log10(legend.at(2).first) - std::log10(legend.at(1).first)));

			tempVertex.COLOR = D3DCOLOR_ARGB(
				255,
				legend.at(2).second.red,
				green,
				legend.at(2).second.blue
			);
		}
		else if (std::log10(it->value) <= std::log10(legend.at(3).first)) {
			int blue = 255 - myRound(255. * std::abs(std::log10(it->value) - std::log10(legend.at(2).first)) / std::abs(std::log10(legend.at(3).first) - std::log10(legend.at(2).first)));

			tempVertex.COLOR = D3DCOLOR_ARGB(
				255,
				legend.at(3).second.red,
				legend.at(3).second.green,
				blue
			);
		}
		else if (std::log10(it->value) <= std::log10(legend.at(4).first)) {
			int red = myRound(255. * std::abs(std::log10(it->value) - std::log10(legend.at(3).first)) / std::abs(std::log10(legend.at(4).first) - std::log10(legend.at(3).first)));

			tempVertex.COLOR = D3DCOLOR_ARGB(
				255,
				red,
				legend.at(4).second.green,
				legend.at(4).second.blue
			);
		}
		else if (std::log10(it->value) <= std::log10(legend.at(5).first)) {
			int green = 255 - myRound(255. * std::abs(std::log10(it->value) - std::log10(legend.at(4).first)) / std::abs(std::log10(legend.at(5).first) - std::log10(legend.at(4).first)));

			tempVertex.COLOR = D3DCOLOR_ARGB(
				255,
				legend.at(5).second.red,
				green,
				legend.at(5).second.blue
			);
		}

		vertices[it - pointsToDraw.begin()] = tempVertex;
	}

	referencedAxisLength = columnCounter + 1;




	for (std::vector<PointWithValue>::iterator it = pointsToDrawRelError.begin(); it != pointsToDrawRelError.end(); ++it) {

		CUSTOMVERTEX tempVertex;
		tempVertex.X = it->x;
		tempVertex.Y = it->y;
		tempVertex.Z = (it->value == 0.0L) ? 0.0f : (greaterAxisLength / 2.0L) * (1.0L - std::abs(it->value - minPointValueRelError) / std::abs(maxPointValueRelError - minPointValueRelError));

		if (it->value == 0.0L) {
			tempVertex.COLOR = D3DCOLOR_ARGB(
				255,
				255,
				255,
				255
			);
		}
		else if (std::log10(it->value) <= std::log10(legendRelError.at(1).first)) {
			int red = myRound(255. * std::abs(std::log10(it->value) - std::log10(legendRelError.at(0).first)) / std::abs(std::log10(legendRelError.at(1).first) - std::log10(legendRelError.at(0).first)));

			tempVertex.COLOR = D3DCOLOR_ARGB(
				255,
				255,
				0,
				red
			);
		}
		else if (std::log10(it->value) <= std::log10(legendRelError.at(2).first)) {
			int blueFade = 255. - myRound(255. * std::abs(std::log10(it->value) - std::log10(legendRelError.at(1).first)) / std::abs(std::log10(legendRelError.at(2).first) - std::log10(legendRelError.at(1).first)));

			tempVertex.COLOR = D3DCOLOR_ARGB(
				255,
				blueFade,
				0,
				255
			);
		}


		vertices[numberOfVerticesToDraw + it - pointsToDrawRelError.begin()] = tempVertex;
	}




	/*WCHAR er[1000];
	swprintf_s(er, L"%d\n", pointsToDraw.size() );
	OutputDebugString(er);*/
	/*PointWithValue rt;
	rt.x = -10;
	rt.y = 5;
	rt.z = 0;
	pointsToDraw.push_back(rt);

	rt.x = 10;
	rt.y = 5;
	rt.z = 0;
	pointsToDraw.push_back(rt);

	rt.x = -10;
	rt.y = 0;
	rt.z = 0;
	pointsToDraw.push_back(rt);

	rt.x = 10;
	rt.y = 0;
	rt.z = 0;
	pointsToDraw.push_back(rt);

	rt.x = -10;
	rt.y = -5;
	rt.z = 0;
	pointsToDraw.push_back(rt);

	rt.x = -10;
	rt.y = -5;
	rt.z = 0;
	pointsToDraw.push_back(rt);*/

	// create the vertices using the CUSTOMVERTEX struct
	/*double referencedAxisMin;
	double referencedAxisMax;

	switch (selectedAxis) {
	case XAxisType:
		referencedAxisMin = minAxisY;
		referencedAxisMax = maxAxisY;

		break;
	case YAxisType:
		referencedAxisMin = minAxisZ;
		referencedAxisMax = maxAxisZ;

		break;
	default:
		referencedAxisMin = minAxisX;
		referencedAxisMax = maxAxisX;

		break;
	}*/

	
	/*WCHAR er[100];
	swprintf_s(er, L"%d\n", pointsToDraw.size() );
	OutputDebugString(er);*/


	/*for (std::vector<PointWithValue>::iterator firstColumn = pointsToDraw.begin(); firstColumn != pointsToDraw.end(); ++firstColumn) {

		CUSTOMVERTEX tempVertex;
		tempVertex.X = it->x;
		tempVertex.Y = it->y;
		tempVertex.Z = it->z;
		tempVertex.COLOR = D3DCOLOR_ARGB(255, 0, 0, 255);

		vertices[it - pointsToDraw.begin()] = tempVertex;
	}*/

	//OutputDebugString(L"D O N E\n");

	/*for (int i = 0; i < pointsToDraw.size(); ++i) {
		WCHAR er[100];
		swprintf_s(er, L"%lf %lf %lf\n", vertices[i].X, vertices[i].Y, vertices[i].Z );
		OutputDebugString(er);
	}*/

	/*WCHAR er[100];
	swprintf_s(er, L"%d\n", sizeof(vertices) );
	OutputDebugString(er);*/

	// OLD VERTICES
	//CUSTMVERTEX vertices[] = {
		// square 1
		/*{ -99.5f, -19.5f, -49.5f, D3DCOLOR_ARGB(255, 0, 0, 255), },
		{ -99.5f, -20.5f, 3.5f, D3DCOLOR_ARGB(255, 0, 255, 0), },
		{ -99.5f, 3.5f, 100.5f, D3DCOLOR_ARGB(255, 255, 0, 0), },
		{ -99.5f, -3.5f, 3.5f, D3DCOLOR_ARGB(255, 0, 255, 255), },*/
		/*{ -9.5f, -1.5f, 9.5f, D3DCOLOR_ARGB(255, 0, 0, 255), },
		{ -9.5f, -1.5f, 8.5f, D3DCOLOR_ARGB(255, 0, 255, 0), },
		{ -9.5f, -1.5f, 7.5f, D3DCOLOR_ARGB(255, 255, 0, 0), },
		{ -9.5f, -1.5f, 6.5f, D3DCOLOR_ARGB(255, 0, 255, 255), },*/

		// square 2
		/*{ -5.0f, 5.0f, 0.0f, D3DCOLOR_ARGB(192, 0, 0, 255), },
		{ 5.0f, 5.0f, 0.0f, D3DCOLOR_ARGB(192, 0, 255, 0), },
		{ -5.0f, -5.0f, 0.0f, D3DCOLOR_ARGB(192, 255, 0, 0), },
		{ 5.0f, -5.0f, 0.0f, D3DCOLOR_ARGB(192, 0, 255, 255), },
		{ -5.0f, -15.0f, 0.0f, D3DCOLOR_ARGB(192, 0, 50, 0), },
		{ 5.0f, -15.0f, 0.0f, D3DCOLOR_ARGB(192, 0, 200, 0), },
	};*/

	/*WCHAR er[100];
	swprintf_s(er, L"%d %d\n", sizeof(vertices), sizeof(CUSTOMVERTEX) );
	OutputDebugString(er);*/

	// create a vertex buffer interface called v_buffer
	d3ddev->CreateVertexBuffer(
		2 * numberOfVerticesToDraw * sizeof(CUSTOMVERTEX),
		0,
		CUSTOMFVF,
		D3DPOOL_MANAGED,
		&v_buffer,
		NULL
	);

	VOID* pVoid;    // a void pointer

					// lock v_buffer and load the vertices into it
	v_buffer->Lock(0, 0, (void**)&pVoid, 0);
	memcpy(pVoid, vertices, 2 * numberOfVerticesToDraw * sizeof(CUSTOMVERTEX) );
	v_buffer->Unlock();


	/*WCHAR er[100];
	swprintf_s(er, L"%d %d\n", referencedAxisLength, otherAxisLength);
	OutputDebugString(er);*/
	indices = new int[6 * referencedAxisLength * otherAxisLength];

	ZeroMemory(indices, 6 * referencedAxisLength * otherAxisLength * sizeof(int) );

	int indicesCounter = 0;

	for (int horizontal = 1; horizontal < referencedAxisLength; ++horizontal) {
		for (int vertical = 0; vertical < otherAxisLength - 1; ++vertical) {
			indices[indicesCounter] = (horizontal - 1) * otherAxisLength + vertical + 1;
			indices[indicesCounter + 1] = horizontal * otherAxisLength + vertical;
			indices[indicesCounter + 2] = (horizontal - 1) * otherAxisLength + vertical;

			indices[indicesCounter + 3] = horizontal * otherAxisLength + vertical + 1;
			indices[indicesCounter + 4] = horizontal * otherAxisLength + vertical;
			indices[indicesCounter + 5] = (horizontal - 1) * otherAxisLength + vertical + 1;

			indicesCounter += 6;
		}
	}

	/*WCHAR ww[500];
	swprintf_s(ww, L"vert: %d\nind: %d\nrefLen: %d\nothLen: %d\nnOfPoints: %d\n", numberOfVerticesToDraw, indicesCounter, referencedAxisLength, otherAxisLength, numberOfVerticesToDraw);
	OutputDebugString(ww);*/

	d3ddev->CreateIndexBuffer(
		indicesCounter * sizeof(int),
		0,
		D3DFMT_INDEX32,
		D3DPOOL_MANAGED,
		&i_buffer,
		NULL
	);

	i_buffer->Lock(0, 0, (void**)&pVoid, 0);
	memcpy(pVoid, indices, indicesCounter * sizeof(int) );
	i_buffer->Unlock();

}


VOID drawVertices(HWND parentWindow) {

	HWND hDrawWindow;
	WNDCLASSEX drawClass;

	ZeroMemory(&drawClass, sizeof(WNDCLASSEX) );

	drawClass.cbSize = sizeof(WNDCLASSEX);
	drawClass.style = CS_HREDRAW | CS_VREDRAW;
	drawClass.lpfnWndProc = drawProcedure;
	drawClass.hInstance = hInst;
	drawClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	drawClass.lpszClassName = L"DrawClass";

	RegisterClassEx(&drawClass);

	hDrawWindow = CreateWindowEx(
		NULL,
		L"DrawClass",
		L"Plot",
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		NULL,
		NULL,
		hInst,
		NULL
	);

	ShowWindow(hDrawWindow, SW_SHOWDEFAULT);

	// set up and initialize Direct3D
	initD3D(hDrawWindow);

	// enter the main loop:
	MSG msg;

	while (TRUE) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT) {
			break;
		}

		renderFrame();
	}

	cleanD3D();
}


VOID constructLegend(VOID) {
	legend.clear();

	/*WCHAR er[100];
	swprintf_s(er, L"%e  %e\n", minPointValue, maxPointValue);
	OutputDebugString(er);*/

	long double fifth = (std::log10(maxPointValue) - std::log10(minPointValue)) / 5.L;
	if (fifth < 0.) {
		fifth *= -1;
	}


	LegendColor colorPurple;
	colorPurple.red = 255;
	colorPurple.green = 0;
	colorPurple.blue = 255;
	legend.push_back(std::make_pair(minPointValue, colorPurple) );

	LegendColor colorDarkBlue;
	colorDarkBlue.red = 0;
	colorDarkBlue.green = 0;
	colorDarkBlue.blue = 255;
	legend.push_back(std::make_pair(std::pow(10, std::log10(minPointValue) + fifth), colorDarkBlue) );

	LegendColor colorLightBlue;
	colorLightBlue.red = 0;
	colorLightBlue.green = 255;
	colorLightBlue.blue = 255;
	legend.push_back(std::make_pair(std::pow(10, std::log10(minPointValue) + 2 * fifth), colorLightBlue) );

	LegendColor colorGreen;
	colorGreen.red = 0;
	colorGreen.green = 255;
	colorGreen.blue = 0;
	legend.push_back(std::make_pair(std::pow(10, std::log10(minPointValue) + 3 * fifth), colorGreen) );

	LegendColor colorYellow;
	colorYellow.red = 255;
	colorYellow.green = 255;
	colorYellow.blue = 0;
	legend.push_back(std::make_pair(std::pow(10, std::log10(minPointValue) + 4 * fifth), colorYellow) );

	LegendColor colorRed;
	colorRed.red = 255;
	colorRed.green = 0;
	colorRed.blue = 0;
	legend.push_back(std::make_pair(maxPointValue, colorRed) );

	/*for (std::vector<std::pair<long double, LegendColor> >::iterator it = legend.begin(); it != legend.end(); ++it) {
		WCHAR er[100];
		swprintf_s(er, L"%e\n", it->first);
		OutputDebugString(er);
	}*/
	/*WCHAR qw[100];
	swprintf_s(qw, L"%e\n", minPointValue);
	OutputDebugString(qw);*/




	legendRelError.clear();

	/*WCHAR er[100];
	swprintf_s(er, L"%e  %e\n", minPointValue, maxPointValue);
	OutputDebugString(er);*/

	long double half = (std::log10(maxPointValueRelError) - std::log10(minPointValueRelError)) / 2.L;
	if (fifth < 0.) {
		fifth *= -1;
	}


	colorPurple;
	colorPurple.red = 0;
	colorPurple.green = 0;
	colorPurple.blue = 255;
	legendRelError.push_back(std::make_pair(minPointValueRelError, colorPurple));

	colorDarkBlue;
	colorDarkBlue.red = 255;
	colorDarkBlue.green = 0;
	colorDarkBlue.blue = 255;
	legendRelError.push_back(std::make_pair(std::pow(10, std::log10(minPointValueRelError) + half), colorDarkBlue));

	colorLightBlue;
	colorLightBlue.red = 255;
	colorLightBlue.green = 0;
	colorLightBlue.blue = 0;
	legendRelError.push_back(std::make_pair(std::pow(10, std::log10(maxPointValueRelError)), colorLightBlue));

}


inline bool sortFunction(PointWithValue a, PointWithValue b) {
	if (a.x < b.x) {
		return true;
	}
	else if (a.x > b.x) {
		return false;
	}
	else {
		if (a.y > b.y) {
			return true;
		}
		else {
			return false;
		}
	}
}





/*VOID createLegendWindow(VOID) {

	HWND hLegendWindow;
	WNDCLASSEX legendClass;

	ZeroMemory(&legendClass, sizeof(WNDCLASSEX));

	legendClass.cbSize = sizeof(WNDCLASSEX);
	legendClass.style = CS_HREDRAW | CS_VREDRAW;
	legendClass.lpfnWndProc = legendProcedure;
	legendClass.hInstance = hInst;
	legendClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	legendClass.lpszClassName = L"LegendClass";

	RegisterClassEx(&legendClass);

	hLegendWindow = CreateWindowEx(
		NULL,
		L"LegendClass",
		L"Legend",
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		100,
		200,
		NULL,
		NULL,
		hInst,
		NULL
	);

	ShowWindow(hLegendWindow, SW_SHOWDEFAULT);

	// enter the main loop:
	MSG msg;

	while (TRUE) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT) {
			break;
		}
	}
}*/


/*LRESULT CALLBACK legendProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	switch (message) {
	case WM_COMMAND:
		break;
	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		SetBkColor(
			hdc,
			RGB(0, 255, 0)
		);

		SetTextColor(
			hdc,
			RGB(255, 0, 0)
		);

		Rectangle(
			hdc,
			10, 10,
			50, 50
		);

		EndPaint(hWnd, &ps);

		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}*/


inline int myRound(long double number) {
	return std::floor(number + 0.5L);
}
