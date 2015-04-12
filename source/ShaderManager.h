#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#pragma once

#include "Shader.h"
#include "Common.h"
#include <set>



/**@brief A singleton class that manages the lifetime of ShaderProgam objects. 
@details Each pair(vertex shader, fragment shader) is considered a unique ShaderProgram.Loading of shaders should be done 
through this class. If a requested pair(vertex shader, fragment shader) has been loaded then the existing cached 
program is returned so duplication does not occur. A ShaderProgram is associated at Model level but different models
can use the same ShaderProgram.
*/
class ShaderManager
{
	
public:
	static ShaderManager& get();
	/**delete list of ShaderProgram */
	~ShaderManager();

	/**Load the shaders and return a pointer to the resulting program. May already be cached*/
	Shader* getShader(File &vertexShader, File &fragmentShader);

private:
	ShaderManager();
	GLuint addShader(const char* shaderSource, GLenum shaderType);
	GLuint createProgram(File &vertexShader, File &fragmentShader);


private:
	typedef std::set<Shader*> ShaderSet;
	ShaderSet m_loadedShaders;
	GLuint m_currProgramId; //returned from createProgram
	//it is good practice to delete the shader after they have been linked
	GLuint m_tempVertexShaderId;
	GLuint m_tempFragmentShaderId;
	
};
#endif // !SHADERMANAGER_H
