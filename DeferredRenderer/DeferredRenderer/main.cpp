#include "DirectX11Manager.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	DirectX11Manager manager;
	if (FAILED(manager.Init(hInstance, nCmdShow)))
		return -1;

	//Shader���쐬
	VertexShader vs;
	PixelShader ps;
	InputLayout il;
	vs.Attach(manager.CreateVertexShader("Assets/Shaders/2DPipeLine.hlsl", "vsMain"));
	ps.Attach(manager.CreatePixelShader("Assets/Shaders/2DPipeLine.hlsl", "psMain"));

	//InputLayout�̍쐬
	D3D11_INPUT_ELEMENT_DESC elem[] = {
		{ "POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	0,	D3D11_INPUT_PER_VERTEX_DATA,	0},
		{ "COLOR"	,	0,	DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	12,	D3D11_INPUT_PER_VERTEX_DATA,	0},
		{ "TEXCOORD",	0,	DXGI_FORMAT_R32G32_FLOAT,		0,	28,	D3D11_INPUT_PER_VERTEX_DATA,	0},
	};
	il.Attach(manager.CreateInputLayout(elem, 3, "Assets/Shaders/2DPipeLine.hlsl", "vsMain"));

	//���_����ݒ�
	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT4 col;
		XMFLOAT2 uv;
	};
	vector<Vertex> vertexs =
	{
		{ XMFLOAT3(-0.5f,-0.5f,0), XMFLOAT4(1,0,0,1),XMFLOAT2(0,1)},
		{ XMFLOAT3(0.5f,-0.5f,0), XMFLOAT4(0,1,0,1),XMFLOAT2(1,1)},
		{ XMFLOAT3(0.5f, 0.5f,0), XMFLOAT4(0,0,1,1),XMFLOAT2(1,0)},
		{ XMFLOAT3(-0.5f, 0.5f,0), XMFLOAT4(0,0,0,1),XMFLOAT2(0,0)}
	};
	VertexBuffer vb;
	vb.Attach(manager.CreateVertexBuffer(vertexs.data(), static_cast<UINT>(vertexs.size())));

	//�C���f�b�N�X���̐ݒ�
	vector<UINT> idxs = { 0,1,2,0,2,3 };
	IndexBuffer ib;
	ib.Attach(manager.CreateIndexBuffer(idxs.data(), static_cast<UINT>(idxs.size())));

	//�e�N�X�`���̍쐬
	ShaderTexture texture;
	texture.Attach(manager.CreateTextureFromFile("Assets/Textures/testMask.png"));

	MSG msg = { 0 };
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (WM_QUIT == msg.message) return 0;

		//MainLoop
		manager.DrawBegin();

		//�|���S�����������߂̊e��p�����[�^�Z�b�g
		manager.SetVertexShader(vs.Get());
		manager.SetPixelShader(ps.Get());
		manager.SetInputLayout(il.Get());
		manager.SetVertexBuffer(vb.Get(), sizeof(Vertex));
		manager.SetIndexBuffer(ib.Get());
		manager.SetTexture2D(0, texture.Get());

		//DrawCall
		manager.DrawIndexed(static_cast<UINT>(idxs.size()));

		manager.DrawEnd();
	}

	return 0;
}