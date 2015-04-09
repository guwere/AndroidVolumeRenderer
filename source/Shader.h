#ifndef SHADER_H
#define SHADER_H

#pragma once
#include "Common.h"
#include <glm/glm.hpp>
#include "File.h"


struct Shader
{
public:
	
	Shader(const File &vertexShader, const File &fragmentShader);
	bool operator==(const Shader &other) const;
	GLuint getId() const;

public:
	File vertexShader, fragmentShader;
	GLuint m_Id;

};

#endif // SHADER_H
