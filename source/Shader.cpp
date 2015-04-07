#include "Shader.h"


Shader::Shader(const File &vertexShader, const File &fragmentShader)
	//:vertexShader(vertexShader), fragmentShader(fragmentShader)
{
	this->vertexShader = vertexShader;
	this->fragmentShader = fragmentShader;
}


GLuint Shader::getId() const
{
	return m_Id;
}

bool Shader::operator==(const Shader &other) const
{
	return (vertexShader == other.vertexShader) && (fragmentShader == other.fragmentShader);
}



