#include "TransferFunction.h"
#include "tinyxml2.h"

TransferFunction::TransferFunction()
{
	// Want the transfer functions to have 256 possible values
	m_colorTable.resize(256);
}

void TransferFunction::parseVoreenXML(File &file)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError r = doc.Parse(file.asString().c_str());

	if (r != tinyxml2::XML_NO_ERROR)
		std::cout << "failed to open file" << std::endl;

	tinyxml2::XMLElement* transFuncIntensity = doc.FirstChildElement("VoreenData")->FirstChildElement("TransFuncIntensity");

	tinyxml2::XMLElement* key = doc.FirstChildElement("VoreenData")->FirstChildElement("TransFuncIntensity")->FirstChildElement("Keys")->FirstChildElement("key");

	while (key)
	{
		float intensity = atof(key->FirstChildElement("intensity")->Attribute("value"));
		m_intensities.push_back(intensity);

		int r = atoi(key->FirstChildElement("colorL")->Attribute("r"));
		int g = atoi(key->FirstChildElement("colorL")->Attribute("g"));
		int b = atoi(key->FirstChildElement("colorL")->Attribute("b"));
		int a = atoi(key->FirstChildElement("colorL")->Attribute("a"));

		m_colors.push_back(glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f));

		key = key->NextSiblingElement();
	}

	m_numIntensities = m_intensities.size();

}

void TransferFunction::generate1DTexture()
{
	m_colors.resize(m_numIntensities);
	//Generate the 1D texture
	glGenTextures(1, &m_tfTexture);
	glBindTexture(GL_TEXTURE_2D, m_tfTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_FLOAT, 0); //1d textures not supported in opengl es
	glBindTexture(GL_TEXTURE_2D, 0);

	loadLookup();

	glBindTexture(GL_TEXTURE_2D, m_tfTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGBA, GL_FLOAT, &m_colorTable[0]);
	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint TransferFunction::getTextureId() const
{
	return m_tfTexture;
}

void TransferFunction::loadLookup()
{
	glm::vec4 previousColor(0.0f);
	float previousIntensity = 0.0f;
	int next = 0;

	for (int i=0; i<256; i++)
	{
		float currentIntensity = (float)i / (float)255;

		while (next < m_numIntensities && currentIntensity > m_intensities[next])
		{
			previousIntensity = m_intensities[next];
			previousColor = m_colors[next];
			next++;
		}

		if (next < m_numIntensities)
			m_colorTable[i] = LERPColor(previousColor, m_colors[next], previousIntensity, m_intensities[next], currentIntensity);
		else
			m_colorTable[i] = LERPColor(previousColor, glm::vec4(0.0f), previousIntensity, 1.0f, currentIntensity);
	}

}

glm::vec4 TransferFunction::LERPColor(glm::vec4 firstColor, glm::vec4 secondColor, float firstIntensity, float secondIntensity, float currentIntensity)
{
	float difference = secondIntensity - firstIntensity;

	if (difference > 0.0f)
	{
		float fraction = (currentIntensity - firstIntensity) / difference;

		return firstColor + ((secondColor - firstColor) * fraction);
	}
	else
		return firstColor;
}
