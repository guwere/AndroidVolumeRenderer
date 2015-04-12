#include <algorithm>    // std::find_if
#include "ShaderManager.h"

using std::endl;
using std::cout;
using std::string;
using std::vector;
ShaderManager& ShaderManager::get()
{
	static ShaderManager singleton;
	return singleton;
}

ShaderManager::ShaderManager()
{

}

ShaderManager::~ShaderManager()
{
	ShaderSet::iterator it = m_loadedShaders.begin();
	for (; it != m_loadedShaders.end(); ++it)
		delete *it;
}


Shader* ShaderManager::getShader(File &vertexShader, File &fragmentShader)
{
	glGetString(GL_VERSION); // if opengl is not initialized this will throw a seg fault
	Shader *newShader = new Shader(vertexShader, fragmentShader);
	ShaderSet::const_iterator it =
		std::find_if(m_loadedShaders.begin(), m_loadedShaders.end(),
		[newShader](const Shader* other) -> bool { return *newShader == *other; });

	if (it == m_loadedShaders.end())
	{
		GLuint programId = createProgram(vertexShader, fragmentShader);
		newShader->m_Id = programId;
		m_loadedShaders.insert(newShader);
		return newShader;
	}
	else
	{
		delete newShader;
		//return existing program with those names
		return *it;
	}
	
}


GLuint ShaderManager::createProgram(File &vertexShader, File &fragmentShader)
{
	m_currProgramId = glCreateProgram();
	if (m_currProgramId == 0)
		LOGI("Error creating shader program id\n");
	// Create two shader objects, one for the vertex, and one for the fragment shader
	m_tempVertexShaderId = addShader(vertexShader.asString().c_str(), GL_VERTEX_SHADER);
	m_tempFragmentShaderId = addShader(fragmentShader.asString().c_str(), GL_FRAGMENT_SHADER);

	GLint success = 0;
	GLchar errorLog[1024] = { 0 };

	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(m_currProgramId);
	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(m_currProgramId);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(m_currProgramId, GL_VALIDATE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(m_currProgramId, sizeof(errorLog), NULL, errorLog);
		LOGI("Invalid shader program: [%s,%s]\n %s\n", vertexShader.m_fileName.c_str(), fragmentShader.m_fileName.c_str(),errorLog);
	}
	else
	{
		LOGI("Successfully created shader program: [%s,%s]\n", vertexShader.m_fileName.c_str(), fragmentShader.m_fileName.c_str());
	}

	//cleanup
	glDetachShader(m_currProgramId, m_tempVertexShaderId);
	glDetachShader(m_currProgramId, m_tempFragmentShaderId);
	glDeleteShader(m_tempVertexShaderId);
	glDeleteShader(m_tempFragmentShaderId);

	return m_currProgramId;
}

GLuint ShaderManager::addShader(const char* shaderSource, GLenum shaderType)
{
	// create a shader object
	GLuint shaderId = glCreateShader(shaderType);

	if (shaderId == 0) {
		LOGI("Error creating shader type %s\n", (shaderType == GL_VERTEX_SHADER) ? "GL_VERTEX_SHADER" : "GL_FRAGMENT_SHADER");
	}
	// Bind the source code to the shader, this happens before compilation
	glShaderSource(shaderId, 1, (const GLchar**)&shaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(shaderId);	
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar infoLog[1024];
		glGetShaderInfoLog(shaderId, 1024, NULL, infoLog);
		LOGI("Error compiling %s:%s\n%", (shaderType == GL_VERTEX_SHADER) ? "GL_VERTEX_SHADER" : "GL_FRAGMENT_SHADER", infoLog);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(m_currProgramId, shaderId);
	return shaderId;
}



