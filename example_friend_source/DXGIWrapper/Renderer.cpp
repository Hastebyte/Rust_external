#include "Headers.h"
#include "Renderer.h"

Renderer* Renderer::m_pInstance;

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::Initialize()
{
}

void Renderer::SetSize(float width, float height)
{
	this->m_Width = width;
	this->m_Height = height;
}

ImVec2 Renderer::GetSize()
{
	return ImVec2(this->m_Width, this->m_Height);
}

void Renderer::BeginScene()
{
}

void Renderer::DrawScene()
{
}

void Renderer::EndScene()
{
}

void Renderer::RenderText(const ImVec2& pos, float size, ImColor color, bool center, bool outline, const char* format, ...)
{
}

void Renderer::DrawLine(const ImVec2& from, const ImVec2& to, ImColor color, float thickness)
{

}

void Renderer::DrawBox(const ImVec2& Pos, const ImVec2& Size, ImColor color)
{

}

void Renderer::DrawBoxOutline(const ImVec2& Pos, const ImVec2& Size, ImColor linecolor, ImColor color)
{

}

void Renderer::CornerBox(const ImVec2& Pos, const ImVec2& Size, ImColor color)
{

}

void Renderer::CornerBoxOutline(const ImVec2& Pos, const ImVec2& Size, ImColor linecolor, ImColor color)
{

}


void Renderer::DrawCircle(const ImVec2& position, float radius, ImColor color, float thickness)
{

}

void Renderer::DrawCircleFilled(const ImVec2& position, float radius, ImColor color)
{

}

void Renderer::DrawFillRect(const ImVec2& position, const ImVec2&  size, ImColor color)
{

}

void Renderer::DrawRect(const ImVec2& position, const ImVec2&  size, ImColor color)
{

}

Renderer* Renderer::GetInstance()
{
	if (!m_pInstance)
		m_pInstance = new Renderer();

	return m_pInstance;
}