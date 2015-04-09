#ifndef VOLUME_H
#define VOLUME_H

#include "Common.h"
#include "File.h"

class Volume
{
public:
	GLubyte *m_memblock3D;
	GLuint m_tex;
	int m_numBytesInBufferFilled;
	int m_currTimeStep;
	int m_timeSteps;
	float m_timePerFrame;
	int m_numDims;
	int m_xRes, m_yRes, m_zRes;
	int m_bytesPerElement;
	bool m_littleEndian;
	std::string m_elementType;
	std::string m_rawFilePath;
	int m_textureSize;
public:
	Volume();
	void parseMHD(const std::string &folderPath, const std::string &filePath);
	void parseMHD(File &file);
	void parsePVM(File &file);
	void reverseEndianness();
	void printProperties();
	void generate();
	void advance();
	GLuint getTextureId() const;
private:
	void readRaw();
	void copyFileToBuffer(const std::string &fileName, int &numBytesInBufferFilled);
};


#endif