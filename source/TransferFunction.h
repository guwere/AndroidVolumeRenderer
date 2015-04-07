#ifndef TRANSFERFUNCTION_H
#define TRANSFERFUNCTION_H
#pragma once

#include "Common.h"
#include "File.h"
#include <glm/glm.hpp>

class TransferFunction
{
public:
	std::vector<glm::vec4> m_colors; //!< The colors extracted from the file
	std::vector<float> m_intensities;//!< an intensity value is between 0.0 and 1.0 is assigned a specific color
	int m_numIntensities; //!< The number of color intensities extracted. Should be the same amount as @param m_colors
	GLuint m_tfTexture; //!< the buffer id of the 1D texture in the gpu
	std::vector<glm::vec4> m_colorTable; //!< contains the colours in the buffer. Also contains the interpolated inbetween colours

	TransferFunction();
	void parseVoreenXML(File &file);
	void generate();
	GLuint getTextureId() const;
private:
	void loadLookup();
	glm::vec4 LERPColor(glm::vec4 firstColor, glm::vec4 secondColor, float firstIntensity, float secondIntensity, float currentIntensity);
};
#endif


