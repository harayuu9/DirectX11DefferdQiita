#include "myInput8.h"
#pragma warning(disable : 4644)
CInput::CInput()
{
	m_pDInput = NULL;
	m_hWnd = NULL;
	m_useDevice = 0;
}

CInput::~CInput()
{
	Release();
}

//�f�o�C�X�̍쐬
bool CInput::Create(HWND hWnd, int useDevice)
{
	if (m_pDInput) {
		Release();
	}
	m_hWnd = hWnd;

	//DirectInput�I�u�W�F�N�g�̐���
	HRESULT hr = DirectInput8Create(::GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDInput, NULL);
	if (FAILED(hr)) {
		return false;
	}

	m_useDevice = useDevice;
	if (useDevice & UseInputDevice_KEYBOARD) { m_keyboard.Create(m_pDInput, hWnd); }
	if (useDevice & UseInputDevice_MOUSE) { m_mouse.Create(m_pDInput, hWnd); }
	if (useDevice & UseInputDevice_GAMEPAD) { m_gamepad.Create(m_pDInput, hWnd); }

	return true;
}

//�f�o�C�X�̉��
void CInput::Release()
{
	//�e�f�o�C�X���
	m_keyboard.Release();
	m_mouse.Release();
	m_gamepad.Release();

	//DirectInput�I�u�W�F�N�g�̉��
	if (m_pDInput != NULL) {
		m_pDInput->Release();
		m_pDInput = NULL;
	}

	m_hWnd = NULL;
	m_useDevice = 0;
}

CInputGamepad::CInputGamepad()
{
	m_pDIDevJS = NULL;
	ZeroMemory(gamepadState, sizeof(gamepadState));
	ZeroMemory(gamepadAction, sizeof(gamepadAction));
}

//�Q�[���p�b�h�f�o�C�X�̍쐬-�f�o�C�X�񋓂̌��ʂ��󂯎��\����
struct DIDeviceEnumPrm
{
	bool isFind;
	GUID guid;
};

//�Q�[���p�b�h�f�o�C�X�̍쐬-�f�o�C�X��񋓂��ăQ�[���p�b�h��T��
static BOOL CALLBACK DIEnumDeviceCallback(LPCDIDEVICEINSTANCE ipddi, LPVOID pvRef)
{
	DIDeviceEnumPrm* prm = (DIDeviceEnumPrm*)pvRef;
	prm->guid = ipddi->guidInstance;
	prm->isFind = true;

	return DIENUM_STOP;	//�񋓂𑱂���Ȃ�DIENUM_CONTINUE
}

//�Q�[���p�b�h�f�o�C�X�̍쐬
bool CInputGamepad::Create(IDirectInput8* pDInput, HWND hWnd)
{
	HRESULT hr;

	if (!pDInput) {
		return false;
	}

	try {
		DIDeviceEnumPrm prm;
		prm.isFind = false;

		//�Q�[���p�b�h�f�o�C�X��񋓂��Č���������GUID���擾����
		pDInput->EnumDevices(DI8DEVTYPE_JOYSTICK, DIEnumDeviceCallback, (LPVOID)&prm, DIEDFL_ATTACHEDONLY);

		if (prm.isFind != true) {
			throw "�Q�[���p�b�h�͌�����܂���ł���";
		}

		hr = pDInput->CreateDevice(prm.guid, &m_pDIDevJS, NULL);

		if (FAILED(hr)) {
			throw "err";
		}

		m_pDIDevJS->SetDataFormat(&c_dfDIJoystick);

		//�����[�h���Βl���[�h��
		DIPROPDWORD diprop;
		ZeroMemory(&diprop, sizeof(diprop));
		diprop.diph.dwSize = sizeof(diprop);
		diprop.diph.dwHeaderSize = sizeof(diprop.diph);
		diprop.diph.dwObj = 0;
		diprop.diph.dwHow = DIPH_DEVICE;
		diprop.dwData = DIPROPAXISMODE_ABS;
		m_pDIDevJS->SetProperty(DIPROP_AXISMODE, &diprop.diph);

		//���̒l�͈̔͐ݒ�
		//�\���L�[�������Ă��Ȃ��Ƃ���0�ɂȂ�悤��
		DIPROPRANGE diprg;
		ZeroMemory(&diprg, sizeof(diprg));
		diprg.diph.dwSize = sizeof(diprg);
		diprg.diph.dwHeaderSize = sizeof(diprg.diph);
		diprg.diph.dwObj = DIJOFS_X;
		diprg.diph.dwHow = DIPH_BYOFFSET;
		diprg.lMin = -1000;
		diprg.lMax = 1000;
		m_pDIDevJS->SetProperty(DIPROP_RANGE, &diprg.diph);

		diprg.diph.dwObj = DIJOFS_Y;
		m_pDIDevJS->SetProperty(DIPROP_RANGE, &diprg.diph);

		//�o�b�t�@�T�C�Y�̐ݒ�
		ZeroMemory(&diprop, sizeof(diprop));
		diprop.diph.dwSize = sizeof(diprop);
		diprop.diph.dwHeaderSize = sizeof(diprop.diph);
		diprop.diph.dwObj = 0;
		diprop.diph.dwHow = DIPH_DEVICE;
		diprop.dwData = 1000;
		hr = m_pDIDevJS->SetProperty(DIPROP_BUFFERSIZE, &diprop.diph);
		if (FAILED(hr)) {
			return false;
		}

		//�������[�h�̐ݒ�
		hr = m_pDIDevJS->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		if (FAILED(hr))
		{
			throw "err";
		}

		//���͂�������
		m_pDIDevJS->Acquire();
	}
	catch (...) {
		return false;
	}

	return true;
}

//�Q�[���p�b�h�̓��͏�Ԃ��X�V�i�\���L�[�ƂS�̃{�^�������`�F�b�N���Ă���j
void CInputGamepad::Update()
{
	if (!m_pDIDevJS) { return; }

	ZeroMemory(gamepadAction, sizeof(gamepadAction));

	DIDEVICEOBJECTDATA od;
	DWORD dwItems = 1;
	HRESULT hr;
	while (true) {
		hr = m_pDIDevJS->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &od, &dwItems, 0);

		if (hr == DIERR_INPUTLOST) {
			m_pDIDevJS->Acquire();
		}
		else
			if (dwItems == 0 || FAILED(hr)) {
				if (hr == DIERR_NOTACQUIRED) {
					m_pDIDevJS->Acquire();
				}

				break;
			}
			else {
				switch (od.dwOfs) {
				case DIJOFS_X:
					gamepadState[GamePadBtn_LEFT] = false;
					gamepadState[GamePadBtn_RIGHT] = false;
					if ((int)od.dwData < 0) {
						gamepadState[GamePadBtn_LEFT] = true;
						gamepadAction[GamePadBtn_LEFT] = true;
					}
					else
						if ((int)od.dwData > 0) {
							gamepadState[GamePadBtn_RIGHT] = true;
							gamepadAction[GamePadBtn_RIGHT] = true;
						}
					break;
				case DIJOFS_Y:
					gamepadState[GamePadBtn_UP] = false;
					gamepadState[GamePadBtn_DOWN] = false;
					if ((int)od.dwData < 0) {
						gamepadState[GamePadBtn_UP] = true;
						gamepadAction[GamePadBtn_UP] = true;
					}
					else
						if ((int)od.dwData > 0) {
							gamepadState[GamePadBtn_DOWN] = true;
							gamepadAction[GamePadBtn_DOWN] = true;
						}
					break;
				case DIJOFS_BUTTON0:
					gamepadState[GamePadBtn_Btn0] = (od.dwData & 0x80) ? true : false;

					if (gamepadState[GamePadBtn_Btn0]) {
						gamepadAction[GamePadBtn_Btn0] = true;
					}
					break;
				case DIJOFS_BUTTON1:
					gamepadState[GamePadBtn_Btn1] = (od.dwData & 0x80) ? true : false;

					if (gamepadState[GamePadBtn_Btn1]) {
						gamepadAction[GamePadBtn_Btn1] = true;
					}
					break;
				case DIJOFS_BUTTON2:
					gamepadState[GamePadBtn_Btn2] = (od.dwData & 0x80) ? true : false;

					if (gamepadState[GamePadBtn_Btn2]) {
						gamepadAction[GamePadBtn_Btn2] = true;
					}
					break;
				case DIJOFS_BUTTON3:
					gamepadState[GamePadBtn_Btn3] = (od.dwData & 0x80) ? true : false;

					if (gamepadState[GamePadBtn_Btn3]) {
						gamepadAction[GamePadBtn_Btn3] = true;
					}
					break;
				}
			}
	}
}
//�Q�[���p�b�h�f�o�C�X�̉��
void CInputGamepad::Release()
{
	if (m_pDIDevJS != NULL) {
		m_pDIDevJS->Unacquire();
		m_pDIDevJS->Release();
		m_pDIDevJS = NULL;
	}
	ZeroMemory(gamepadState, sizeof(gamepadState));
	ZeroMemory(gamepadAction, sizeof(gamepadAction));
}
CInputKeyboard::CInputKeyboard()
{
	m_pDIDevKB = NULL;
	ZeroMemory(m_Keystate, sizeof(m_Keystate));
	ZeroMemory(m_KeyAction, sizeof(m_KeyAction));
}

//�L�[�{�[�h�f�o�C�X���쐬
bool CInputKeyboard::Create(IDirectInput8* pDInput, HWND hWnd)
{
	HRESULT hr;

	if (!pDInput) {
		return false;
	}

	// �L�[�{�[�h�f�o�C�X���쐬
	hr = pDInput->CreateDevice(GUID_SysKeyboard, &m_pDIDevKB, NULL);
	if (FAILED(hr))
	{
		return false;
	}

	//�f�[�^�t�H�[�}�b�g�̐ݒ�
	hr = m_pDIDevKB->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr))
	{
		return false;
	}

	//�o�b�t�@�T�C�Y�̐ݒ�
	DIPROPDWORD diprop;
	diprop.diph.dwSize = sizeof(diprop);
	diprop.diph.dwHeaderSize = sizeof(diprop.diph);
	diprop.diph.dwObj = 0;
	diprop.diph.dwHow = DIPH_DEVICE;
	diprop.dwData = 1000;
	hr = m_pDIDevKB->SetProperty(DIPROP_BUFFERSIZE, &diprop.diph);
	if (FAILED(hr)) {
		return false;
	}

	//�������[�h�̐ݒ�
	hr = m_pDIDevKB->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (FAILED(hr))
	{
		return false;
	}

	//���͂�������
	m_pDIDevKB->Acquire();
	return true;
}

//�L�[�{�[�h�̓��͏�Ԃ��X�V
void CInputKeyboard::Update()
{
	ZeroMemory(m_KeyAction, sizeof(m_KeyAction));

	DIDEVICEOBJECTDATA od;
	DWORD dwItems = 1;
	HRESULT hr;
	while (true) {
		hr = m_pDIDevKB->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &od, &dwItems, 0);

		if (hr == DIERR_INPUTLOST) {
			m_pDIDevKB->Acquire();
		}
		else
			if (dwItems == 0 || FAILED(hr)) {
				if (hr == DIERR_NOTACQUIRED) {
					m_pDIDevKB->Acquire();
				}

				break;
			}
			else {
				m_Keystate[od.dwOfs] = (od.dwData & 0x80) ? true : false;

				if (m_Keystate[od.dwOfs]) {
					m_KeyAction[od.dwOfs] = true;
				}
			}
	}
}

//�L�[�{�[�h�f�o�C�X�̉��
void CInputKeyboard::Release()
{
	if (m_pDIDevKB) {
		m_pDIDevKB->Unacquire();
		m_pDIDevKB->Release();
		m_pDIDevKB = NULL;
	}
	ZeroMemory(m_Keystate, sizeof(m_Keystate));
	ZeroMemory(m_KeyAction, sizeof(m_KeyAction));
}
CInputMouse::CInputMouse()
{
	m_hWnd = NULL;
	m_pDIMouse = NULL;
	m_posX = 0;
	m_posY = 0;
	m_moveX = 0;
	m_moveY = 0;
	m_LDown = false;
	m_RDown = false;
	m_MDown = false;
	m_LAction = false;
	m_RAction = false;
	m_MAction = false;
	m_LClicked = m_RClicked = m_MClicked = false;

	m_windowMode = true;
}

//�}�E�X�f�o�C�X�̍쐬
bool CInputMouse::Create(IDirectInput8* pDInput, HWND hWnd)
{
	HRESULT hr;

	if (!pDInput) {
		return false;
	}

	m_hWnd = hWnd;
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	m_wndWid = rect.right - rect.left;
	m_wndHgt = rect.bottom - rect.top;
	m_posX = m_wndWid / 2;
	m_posY = m_wndHgt / 2;

	//�}�E�X�f�o�C�X���쐬
	hr = pDInput->CreateDevice(GUID_SysMouse, &m_pDIMouse, NULL);
	if (FAILED(hr)) {
		return false;
	}

	//�f�[�^�t�H�[�}�b�g�̐ݒ�
	hr = m_pDIMouse->SetDataFormat(&c_dfDIMouse2);
	if (FAILED(hr)) {
		return false;
	}

	//�o�b�t�@�T�C�Y�̐ݒ�
	DIPROPDWORD diprop;
	diprop.diph.dwSize = sizeof(diprop);
	diprop.diph.dwHeaderSize = sizeof(diprop.diph);
	diprop.diph.dwObj = 0;
	diprop.diph.dwHow = DIPH_DEVICE;
	diprop.dwData = 1000;
	hr = m_pDIMouse->SetProperty(DIPROP_BUFFERSIZE, &diprop.diph);
	if (FAILED(hr)) {
		return false;
	}

	//�������[�h�̐ݒ�
	hr = m_pDIMouse->SetCooperativeLevel(m_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (FAILED(hr)) {
		return false;
	}

	//���͂�������
	m_pDIMouse->Acquire();

	return true;
}

//�}�E�X�̓��͏�Ԃ��X�V
void CInputMouse::Update()
{
	//���Z�b�g����B���̂��߃N���b�N��UpdateMouse()������ĂԑO�ɏ������Ȃ���΂Ȃ�Ȃ��B
	m_LAction = false;
	m_RAction = false;
	m_MAction = false;
	m_moveX = 0;
	m_moveY = 0;

	DIDEVICEOBJECTDATA od;
	DWORD dwItems = 1;
	HRESULT hr;
	while (true) {
		hr = m_pDIMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &od, &dwItems, 0);

		if (hr == DIERR_INPUTLOST) {
			m_pDIMouse->Acquire();
		}
		else
			if (dwItems == 0 || FAILED(hr)) {
				if (hr == DIERR_NOTACQUIRED) {
					m_pDIMouse->Acquire();
				}

				break;
			}
			else {
				switch (od.dwOfs) {
				case DIMOFS_X:
					if (m_windowMode) {
						m_posX += (int)od.dwData;
						m_moveX = (int)od.dwData;
						m_posX = max(0, min(m_posX, m_wndWid));//�E�B���h�E����͂ݏo�Ȃ��悤����
					}
					break;
				case DIMOFS_Y:
					if (m_windowMode) {
						m_posY += (int)od.dwData;
						m_moveY = (int)od.dwData;
						m_posY = max(0, min(m_posY, m_wndHgt));
					}
					break;
				case DIMOFS_BUTTON0://���{�^��
					m_LDown = (od.dwData & 0x80) ? true : false;
					if (m_LDown) { m_LAction = true; }
					m_LClicked = !m_LClicked;
					break;
				case DIMOFS_BUTTON1://�E�{�^��
					m_RDown = (od.dwData & 0x80) ? true : false;
					if (m_RDown) { m_RAction = true; }
					m_RClicked = !m_RClicked;
					break;
				case DIMOFS_BUTTON2://���{�^��
					m_MDown = (od.dwData & 0x80) ? true : false;
					if (m_MDown) { m_MAction = true; }
					m_MClicked = !m_MClicked;
					break;
				}
			}
	}
}

//�}�E�X�f�o�C�X�̉��
void CInputMouse::Release()
{
	if (m_pDIMouse != NULL) {
		m_pDIMouse->Unacquire();
		m_pDIMouse->Release();
		m_pDIMouse = NULL;
	}

	m_hWnd = NULL;
	m_posX = 0;
	m_posY = 0;
	m_LDown = false;
	m_RDown = false;
	m_MDown = false;
	m_LAction = false;
	m_RAction = false;
	m_MAction = false;
}