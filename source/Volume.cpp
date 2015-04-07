#include "Volume.h"
#include "HelperFunctions.h"
#include <tinydir.h>
#include <algorithm>

using namespace std;

Volume::Volume()
	:m_timeSteps(1), m_timePerFrame(0.1f)
{

}
void Volume::parseMHD(const std::string &folderPath, const std::string &filePath)
{
	File file(folderPath, filePath);
	parseMHD(file);

}
void Volume::parseMHD(File &file)
{

	string line;
	while (getline(file.m_file, line))
	{
		std::istringstream iss(line);
		//line format
		//<Parameter> = <value>
		string param, equalsSign, value; // equals sign is irrelevant
		iss >> param;
		iss >> equalsSign;
		if (param == "Timesteps")
		{
			iss >> m_timeSteps;
		}
		else if (param == "NDims")
		{
			iss >> m_numDims;
		}
		else if (param == "DimSize")
		{
			iss >> m_xRes;
			iss >> m_yRes;
			iss >> m_zRes;
		}
		else if (param == "ElementType")
		{
			iss >> m_elementType;

			if (m_elementType == "MET_UCHAR")
				m_bytesPerElement = 1;
			else if (m_elementType == "SHORT")
				m_bytesPerElement = 2;
			else if (m_elementType == "FLOAT")
				m_bytesPerElement = 4;
		}
		else if (param == "ElementDataFile")
		{
			string filePath;
			iss >> filePath;
			m_rawFilePath = file.m_folderPath + filePath;
		}
		else if (param == "ElementByteOrderMSB")
		{
			string byteOrder;
			iss >> byteOrder;

			if (byteOrder == "TRUE" || byteOrder == "True")
				m_littleEndian = true;
			else
				m_littleEndian = false;
		}
		iss.clear();
	}
	readRaw();
	LOGI("Volume size: %d B",m_numBytesInBufferFilled);

}

// Reads in the raw binary data using properties copied in from header
void Volume::readRaw()
{
	// Allocate CPU memory block
	int bufferSize = m_xRes * m_yRes * m_zRes * m_bytesPerElement * m_timeSteps;
	m_memblock3D = new GLubyte [bufferSize];

	m_numBytesInBufferFilled = 0;

	std::string directory = m_rawFilePath;
	std::vector<std::string> files;
#ifdef WIN32
	directory.append("/*");
	WIN32_FIND_DATAA findFileData;
	HANDLE hFind;

	hFind = FindFirstFileA(directory.c_str(), &findFileData);

	// If multiple files in specified path then sort the contents and read them in sequentially
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindNextFileA(hFind, &findFileData);

		while (FindNextFileA(hFind, &findFileData) != 0)
			files.push_back(findFileData.cFileName);

	
#elif
	struct stat status;
	//LOGI("%s",directory.c_str());
	stat(directory.c_str(), &status);

	// If multiple files in specified path then sort the contents and read them in sequentially
	if (status.st_mode & S_IFDIR)
	{
		tinydir_dir dir;
		tinydir_open(&dir, directory.c_str());
		LOGI("TinyDir opened");

		while(dir.has_next)
		{
			tinydir_file file;
			tinydir_readfile(&dir, &file);
			files.push_back(std::string(file.name));
			tinydir_next(&dir);
		}
	
#endif
		std::sort(files.begin(), files.end(), HelperFunctions::numericalSort);

		for (int i=0; i<files.size(); i++)
			files[i] = std::string(m_rawFilePath + "/" + files[i]);

		for (int i=0; i<files.size(); i++)
		{
			copyFileToBuffer(files[i], m_numBytesInBufferFilled);
		}
	}
	else
	{
		LOGI("Only one volume file -- Copying to gpu buffer");
		copyFileToBuffer(m_rawFilePath, m_numBytesInBufferFilled);
	}
}


// Current file to be copied to buffer. Offset is for multiple files being read into single buffer, set as file offset
void Volume::copyFileToBuffer(const std::string &fileName, int &numBytesInBufferFilled)
{
	std::streampos size;

	std::ifstream myFile (fileName.c_str(), std::ios::in|std::ios::binary|std::ios::ate);

	if (myFile.is_open())
	{
		size = myFile.tellg();

		myFile.seekg (0, std::ios::beg);
		myFile.read ((char*)m_memblock3D + numBytesInBufferFilled, size);
		myFile.close();

		numBytesInBufferFilled += size;
	}
	else
	{
		LOGI("Unable to open volume file: %s\n", fileName.c_str());
	}

}

// Only needed if you want to analyse the volume on the CPU and the endianness is in the wrong order
void Volume::reverseEndianness()
{
	std::vector<GLubyte> bytes;
	bytes.resize(m_xRes * m_yRes * m_zRes * m_bytesPerElement);

	for (int i=0; i<bytes.size(); i+=m_bytesPerElement)
	{
		for (int j=0; j<m_bytesPerElement; j++)
		{
			bytes[i+j] = m_memblock3D[i+(m_bytesPerElement-(1+j))];
		}
	}

	memcpy(m_memblock3D, &bytes[0], m_xRes * m_yRes * m_zRes * m_bytesPerElement);
}

void Volume::printProperties()
{
	LOGI("--Volume properties--\n");
	LOGI("rawFilePath = %s\n", m_rawFilePath.c_str());
	LOGI("timesteps = %d\n", m_timeSteps);
	LOGI("timePerFrame = %d\n", m_timePerFrame);
	LOGI("numDims = %d\n", m_numDims);
	LOGI("resolution[x,y,z] = [%d,%d,%d]\n", m_xRes, m_yRes, m_zRes);
	LOGI("bytesPerElement = %d\n", m_bytesPerElement);
	if(m_littleEndian)
		LOGI("littleEndian = true\n");
	else
		LOGI("littleEndian = false\n");
	LOGI("-- --");
}

void Volume::generate()
{
	m_textureSize = m_xRes * m_yRes * m_zRes * m_bytesPerElement;

	glEnable(GL_TEXTURE_3D);
	glGenTextures(1, &m_tex);
	glBindTexture(GL_TEXTURE_3D, m_tex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


	// Reverses endianness in copy
	if (!m_littleEndian)
		glPixelStoref(GL_UNPACK_SWAP_BYTES, true);

	if (m_elementType == "MET_UCHAR")
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, m_xRes, m_yRes, m_zRes, 0,  GL_RED, GL_UNSIGNED_BYTE, m_memblock3D);

	else if (m_elementType == "SHORT")
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, m_xRes, m_yRes, m_zRes, 0, GL_RED, GL_UNSIGNED_SHORT, m_memblock3D);

	else if (m_elementType == "FLOAT")
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, m_xRes, m_yRes, m_zRes, 0, GL_RED, GL_FLOAT, m_memblock3D);

	glPixelStoref(GL_UNPACK_SWAP_BYTES, false);

	glBindTexture(GL_TEXTURE_3D, 0);


}

void Volume::advance()
{
	if(m_timeSteps > 1)
	{
		glBindTexture(GL_TEXTURE_3D, m_tex);
		if (!m_littleEndian)
			glPixelStoref(GL_UNPACK_SWAP_BYTES, true);

		if (m_elementType == "MET_UCHAR")
			glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, m_xRes, m_yRes, m_zRes, 0,  GL_RED, GL_UNSIGNED_BYTE, m_memblock3D + (m_textureSize * m_currTimeStep));

		else if (m_elementType == "SHORT")
			glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, m_xRes, m_yRes, m_zRes, 0, GL_RED, GL_UNSIGNED_SHORT, m_memblock3D + (m_textureSize * m_currTimeStep));

		else if (m_elementType == "FLOAT")
			glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, m_xRes, m_yRes, m_zRes, 0, GL_RED, GL_FLOAT, m_memblock3D + (m_textureSize * m_currTimeStep));

		glPixelStoref(GL_UNPACK_SWAP_BYTES, false);

		glBindTexture(GL_TEXTURE_3D, 0);

		if(m_currTimeStep >= m_timeSteps)
			m_currTimeStep = 1;
	}
}

GLuint Volume::getTextureId() const
{
	return m_tex;
}
