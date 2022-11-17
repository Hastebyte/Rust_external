#pragma once


struct ImVec2
{
	float x, y;
	ImVec2() { x = y = 0.0f; }
	ImVec2(float _x, float _y) { x = _x; y = _y; }
};

struct ImVec4
{
	float x, y, z, w;
	ImVec4() { x = y = z = w = 0.0f; }
	ImVec4(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
};


struct ImColor
{
	float x, y, z, w;
	ImColor() { x = y = z = w = 0.0f; }
	ImColor(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
};


class Renderer
{
public:
	void Initialize();

	void SetSize(float width, float height);
	ImVec2 GetSize();

	void BeginScene();
	void DrawScene();
	void EndScene();

	static Renderer* GetInstance();

	void RenderText(const ImVec2& pos, float size, ImColor color, bool center, bool outline, const char* format, ...);
	void DrawLine(const ImVec2& from, const ImVec2& to, ImColor color, float thickness = 1.0f);
	void DrawBox(const ImVec2& Pos, const ImVec2& Size, ImColor color);
	void DrawBoxOutline(const ImVec2& Pos, const ImVec2& Size, ImColor linecolor, ImColor color);
	void CornerBox(const ImVec2& Pos, const ImVec2& Size, ImColor color);
	void CornerBoxOutline(const ImVec2& Pos, const ImVec2& Size, ImColor linecolor, ImColor color);
	void DrawFillRect(const ImVec2& position, const ImVec2&  size, ImColor color);
	void DrawRect(const ImVec2& position, const ImVec2&  size, ImColor color);
	void DrawCircle(const ImVec2& position, float radius, ImColor color, float thickness = 1.0f);
	void DrawCircleFilled(const ImVec2& position, float radius, ImColor color);

private:
	Renderer();
	~Renderer();

	float m_Width, m_Height;

	static Renderer* m_pInstance;
};
