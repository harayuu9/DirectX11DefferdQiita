#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>	//dinput8.lib dxguid.lib�̃����N���K�v�ł��B

#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "dinput8.lib")
//�L�[�{�[�h
class CInputKeyboard
{
	friend class CInput;

	IDirectInputDevice8*	m_pDIDevKB;			//�f�o�C�X
	bool					m_Keystate[256];	//�X�V���ɉ����Ă���ON(�������ςȂ����E��)
	bool					m_KeyAction[256];	//���������ゾ��ON(�������ςȂ���OFF�ɂȂ�)
												//�z��C���f�b�N�X��DIK_SPACE�Ȃ�

	CInputKeyboard();//private�ɂ��Ă�B�N���X�������CInput���炾���B
	bool Create(IDirectInput8* pDInput, HWND hWnd);
	void Release();
	void Update();

public:
	bool ChkKeyDown(int key) { return m_Keystate[key]; }
	bool ChkKeyAction(int key) { return m_KeyAction[key]; }
};

//�}�E�X
class CInputMouse
{
	friend class CInput;

	IDirectInputDevice8*	m_pDIMouse;		//�f�o�C�X
	bool CreateMouse();
	void UpdateInput_Mouse();
	void ReleaseMouse();

	HWND					m_hWnd;
	int						m_wndWid;
	int						m_wndHgt;
public:
	bool					m_windowMode;
private:
	long					m_posX;	//�}�E�X���W
	long					m_posY;

	long					m_moveX;//�}�E�X�̈ړ���
	long					m_moveY;//�}�E�X�̈ړ���

	long					m_moveWheel;

	bool					m_LDown;	//Update���_�̃{�^�����
	bool					m_RDown;
	bool					m_MDown;

	bool					m_LAction;	//�{�^�����������ゾ��ON�ɂȂ�(�N���b�N���E���Ƃ���)
	bool					m_RAction;
	bool					m_MAction;

	bool					m_LClicked; //�������p
	bool					m_RClicked;
	bool					m_MClicked;

	CInputMouse();
	bool Create(IDirectInput8* pDInput, HWND hWnd);
	void Release();
	void Update();

public:

	long PosX() { return m_posX; }
	long PosY() { return m_posY; }

	long MoveX() { return m_moveX; }
	long MoveY() { return m_moveY; }

	bool IsLAction() { return m_LAction; }
	bool IsRAction() { return m_RAction; }
	bool IsMAction() { return m_MAction; }

	bool IsLDown() { return m_LClicked; }
	bool IsRDown() { return m_RClicked; }
	bool IsMDown() { return m_MClicked; }

	//�E�B���h�E���[�h�̎w��
	//�J�[�\���ʒu��API�����邩�ADirectInput�����邩�̈Ⴂ
	void SetWindowMode(bool isWindowmode) { m_windowMode = isWindowmode; }//true:�E�B���h�E���[�h false:�t���X�N���[��

	void SetCursorPos(int x, int y)
	{
		m_posX = x;
		m_posY = y;
	}

	void SetCursorMove(int x, int y)
	{
		m_moveX = x; m_moveY = y;
	}
	long GetWheel() { return m_moveWheel; }
	void SetWheel(long n) { m_moveWheel = n; }
};

//�Q�[���p�b�h---------------------------------
//(�\���L�[�ƂS�̃{�^��������)
class CInputGamepad
{
	friend class CInput;

	IDirectInputDevice8*	m_pDIDevJS;			//�f�o�C�X
	bool					gamepadState[8];	//�X�V���ɉ����Ă���ON
	bool					gamepadAction[8];	//�����Ď���X�V����܂�ON
#define GamePadBtn_UP		0
#define GamePadBtn_DOWN		1
#define GamePadBtn_LEFT		2
#define GamePadBtn_RIGHT	3
#define GamePadBtn_Btn0		4
#define GamePadBtn_Btn1		5
#define GamePadBtn_Btn2		6
#define GamePadBtn_Btn3		7

	CInputGamepad();
	bool Create(IDirectInput8* pDInput, HWND hWnd);
	void Release();
	void Update();

public:

	bool ChkGamePadDown(int btn) { return gamepadState[btn]; }
	bool ChkGamePadAction(int btn) { return gamepadAction[btn]; }
};

//�L�[�{�[�h�A�}�E�X�A�Q�[���p�b�h���͂��Ǘ�����N���X
class CInput
{
	HWND					m_hWnd;

	//DirectInput�I�u�W�F�N�g
	IDirectInput8*			m_pDInput;

	//CInput�Ŏg���f�o�C�X�i�������Ώۂɂ���j
	int						m_useDevice;
#define UseInputDevice_KEYBOARD	0x01
#define UseInputDevice_MOUSE	0x02
#define UseInputDevice_GAMEPAD	0x04
#define UseInputDevice_ALL		(UseInputDevice_KEYBOARD | UseInputDevice_MOUSE | UseInputDevice_GAMEPAD)

	//�e�f�o�C�X
	CInputKeyboard		m_keyboard;
	CInputMouse			m_mouse;
	CInputGamepad		m_gamepad;

public:

	CInput();
	~CInput();

	bool Create(HWND hWnd, int useDevice = UseInputDevice_ALL);
	void Release();

public:
	//���͏�Ԃ��X�V����
	void InputUpdate() {
		m_keyboard.Update();
		m_mouse.Update();
		m_gamepad.Update();
	}

	//�e�f�o�C�X���Q�Ƃ���(����������͏�Ԃ��擾����)
	CInputKeyboard* Keyboard() { return &m_keyboard; }
	CInputMouse* Mouse() { return &m_mouse; }
	CInputGamepad* Gamepad() { return &m_gamepad; }
};
