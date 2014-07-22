#include <GL\glew.h>
#include <Engine\Renderer\Shader\ShaderProgram.h>
#include <fstream>
#pragma warning(disable: 4127)
#pragma warning(push)
#include <Qt/qdebug.h>
#include <Qt/qcoreapplication.h>
#include <Qt/qimage.h>
#include <Qt/qfile.h>
#pragma warning(pop)
#include <cassert>


GLuint ShaderProgram::currentProgram;
int ShaderProgram::numOfTextures = 0;

struct CodeBlock { // used to store shader code
	GLuint id;
	std::string code;
};

 std::string ShaderProgram::file2str(const char * filePath) {
	std::ifstream file(filePath);
	return std::string(
		std::istreambuf_iterator<char>(file),
		std::istreambuf_iterator<char>());
}
bool ShaderProgram::validFile(const char * filePath) {
	std::ifstream file(filePath);
	bool valid = file.good();
	file.close();
	return valid;
}

void ShaderProgram::startup() {
	numOfPrams = 0;
	programID = glCreateProgram();
	qDebug() << "Creating Shader Program ID: " << programID;
}
void ShaderProgram::shutdown() {
	// ?
}
void ShaderProgram::buildBasicProgram(const char * vertexShaderFilePath, const char * fragmentShaderFilePath) {
	//startup();
	bool win = false;
	win = addProgram(vertexShaderFilePath,GL_VERTEX_SHADER);
	if(win) win = addProgram(fragmentShaderFilePath,GL_FRAGMENT_SHADER);
	if(win) linkAndRun();
}
bool ShaderProgram::addProgram(const char * filePath, unsigned short shaderType) {
	qDebug() << "\nAttempting to load file: " << filePath;
	
	bool isValid = validFile(filePath);
	if(isValid) {
		isValid = addProgram_srcCode(file2str(filePath),shaderType);
	} else {
		qDebug() << "File(" << filePath << ") was not found\n";
		assert(false);
	}
	return isValid;
}
bool ShaderProgram::addProgram_srcCode(const char * shaderCode, unsigned short shaderType) {
	return addProgram_srcCode(std::string(shaderCode),shaderType);
}
bool ShaderProgram::addProgram_srcCode(std::string shaderCode, unsigned short shaderType) {
	bool isValid;
	CodeBlock shaderInfo;
	shaderInfo.code = shaderCode;
	shaderInfo.id = glCreateShader(shaderType);
	qDebug() << "Shader Load Successful ID: " << shaderInfo.id;

	isValid = complileShader(shaderInfo.code.c_str(),shaderInfo.id,true);
	if(isValid) {
		glAttachShader(programID,shaderInfo.id);
		qDebug() << "File(" << shaderInfo.id << ") Complile Successful ProgramID: " << programID << "\n";
	} else {
		qDebug() << "File(" << shaderInfo.id << ") Failed to Complile - NOT ADDED TO PROGRAM\n";
		assert(false);
	}
	return isValid;
}

int ShaderProgram::getUniform(const char* title) {
	return glGetUniformLocation(getProgramID(),title);
}
void ShaderProgram::passUniform(const char* name, ParameterType parameterType, const void * value) {
	uint location = getUniform(name);
	passUniform(location,parameterType,value);
}
void ShaderProgram::passUniform(uint location, ParameterType parameterType, const void * value) {
	if(parameterType == ParameterType::PT_FLOAT) {
		glUniform1f(location, *(float*)value);
	} else if(parameterType == ParameterType::PT_VEC2) {
		glUniform2fv(location,1,(float*)value);
	} else if(parameterType == ParameterType::PT_VEC3) {
		glUniform3fv(location,1,(float*)value);
	} else if(parameterType == ParameterType::PT_VEC4) {
		glUniform4fv(location,1,(float*)value);
	} else if(parameterType == ParameterType::PT_MAT3) {
		glUniformMatrix3fv(location,1,false,(float*)value);
	} else if(parameterType == ParameterType::PT_MAT4) {
		glUniformMatrix4fv(location,1,false,(float*)value);
	} else if(parameterType == ParameterType::PT_TEXTURE || ParameterType::PT_BOOLEAN || ParameterType::PT_INT) {
		int decodedValue = *(int*)value;
		glUniform1i(location,decodedValue);
	} else {
		//wat?
		qDebug() << "Uniform " << location << " of type: " << parameterType << " was not passed down correctly";
		assert(false);
	}
}
void ShaderProgram::saveUniform(const char* name, ParameterType parameterType, const void * value) {
	prams[numOfPrams++].init(this,name,parameterType,value);
}

void ShaderProgram::passSavedUniforms_try() {
	if(validPush)
		passSavedUniforms_force();
}
void ShaderProgram::passSavedUniforms_force() {
	for (uint i = 0; i < numOfPrams; i++)
	{
		prams[i].sendData();
	}
	validPush = false;
}
bool ShaderProgram::getValidPush() {
	return validPush;
}
void ShaderProgram::resetValidPush() {
	validPush = true;
}

bool ShaderProgram::complileShader(const char * code, GLuint id, bool debug) {
	bool valid = true;
	const char * codeAdapt[1];
	codeAdapt[0] = code;
	glShaderSource(id,1,codeAdapt,0);
	
	//qDebug() << "Compiling Shader " << id;
	glCompileShader(id);
	

	if(debug) {
		GLint compileStatus;
		glGetShaderiv(id,GL_COMPILE_STATUS, &compileStatus);
		if(compileStatus != GL_TRUE) {
			GLint logLength;
			glGetShaderiv(id,GL_INFO_LOG_LENGTH,&logLength);
			char * buffer = new char[logLength];
			GLsizei someRandom;
			glGetShaderInfoLog(id,logLength,&someRandom,buffer);
			qDebug() << buffer;
			delete [] buffer;

			valid = false;
		}
	}
	return valid;
}

void ShaderProgram::link() {
	qDebug() << "Linking Program ID: " << programID;
	glLinkProgram(programID);
}

GLuint ShaderProgram::getProgramID() {
	return programID;
}
GLuint ShaderProgram::getCurrentlyUsedProgram() {
	return currentProgram;
}
bool ShaderProgram::isCurrentProgram() {
	return (currentProgram == programID);
}
void ShaderProgram::useProgram() {
	if(!isCurrentProgram()) {
		//qDebug() << "Regestering Shader Program  from " << currentProgram << " to " << programID << " into pipeline";
		currentProgram = programID;
		glUseProgram(programID);
	}
}

GLuint ShaderProgram::linkAndRun() {
	link();
	useProgram();
	return programID;
}

QString formatFileName(QString fileName) {
	QString formatedName = fileName.replace(QRegExp("[_]")," ");
	formatedName = formatedName.remove(".jpg",Qt::CaseInsensitive);
	formatedName = formatedName.remove(".png",Qt::CaseInsensitive);
	int lastSlash = formatedName.lastIndexOf('/');
	formatedName = formatedName.mid(lastSlash,formatedName.size()-lastSlash);
	lastSlash = formatedName.lastIndexOf('\\');
	formatedName = formatedName.mid(lastSlash,formatedName.size()-lastSlash);
	return formatedName;
}

QImage ShaderProgram::getImageFromFile(QString fileName, bool flipHorz, bool flipVert) {
	QImage myTexture = QGLWidget::convertToGLFormat(QImage(fileName).mirrored(flipHorz,flipVert));

	if(myTexture.isNull()) {
		qDebug() << "attempt to load " << fileName << " failed";
		assert(false);
	} else {
		QString formatedName = fileName.replace(QRegExp("[_]")," ");
		formatedName = formatedName.remove(".jpg",Qt::CaseInsensitive);
		formatedName = formatedName.remove(".png",Qt::CaseInsensitive);
		int lastSlash = formatedName.lastIndexOf('/');
		formatedName = formatedName.mid(lastSlash,formatedName.size()-lastSlash);
		lastSlash = formatedName.lastIndexOf('\\');
		formatedName = formatedName.mid(lastSlash,formatedName.size()-lastSlash);
		qDebug() << "Texture Loaded ( " << myTexture.width() << "x" << myTexture.width() << " ): " << formatFileName(fileName);
	}

	return myTexture;
}
//returns the bufferID
GLuint ShaderProgram::load2DTexture(QImage image, GLenum type) {
	return load2DTexture(image.bits(),image.width(),image.height(), type);
}
GLuint ShaderProgram::load2DTexture(QString fileName, bool flipHorz, bool flipVert) {
	QString filePath = /**/QCoreApplication::applicationDirPath() + /**/fileName;
	QFile tempFile(filePath);
	if(tempFile.exists()) {
		QImage data = getImageFromFile(filePath,flipHorz,flipVert);
		return load2DTexture(data);
	} else {
		qDebug() << "Invalid file path " << formatFileName(filePath) << " Texture not loaded";
		assert(false);
		return -1;
	}
}
GLuint ShaderProgram::load2DTexture(ubyte * data, uint width, uint height, GLenum fileType) {
	//uint ID = numOfTextures++;
	GLuint bufferID;

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1,&bufferID);
	glActiveTexture(GL_TEXTURE0+(bufferID-1));
	glBindTexture(GL_TEXTURE_2D, bufferID);
	
	glTexImage2D(GL_TEXTURE_2D,0, fileType, width, height, 0, fileType, GL_UNSIGNED_BYTE, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	return (bufferID-1);
	//return ID;
}