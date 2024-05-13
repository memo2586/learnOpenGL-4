#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

struct uniformStruct{
	unsigned int base;	// 基准对齐量
	unsigned int offset;	// 偏移量

	uniformStruct() { base = 0; offset = 0; }
	uniformStruct(unsigned int _base, unsigned int _offset) :
		base(_base), offset(_offset) {};
};

class Shader {
public:
	// 程序ID
	unsigned int ID;

	Shader(const char* vertexPath, const char* fragmentPath);
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath);
	void use();
	// uniform工具函数
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value);
	void setVec2f(const std::string& name, float xValue, float yValue);
	void setVec2f(const std::string& name, glm::vec2 value);
	void setVec3f(const std::string& name, float xValue, float yValue, float zValue);
	void setVec3f(const std::string& name, glm::vec3 value);
	void setMat4f(const std::string& name, int count, GLfloat* value);
	void setMat4f(const std::string& name, GLfloat* value, int count);
	void setMat4f(const std::string& name, glm::mat4 value, int count);
	void uniformBlockBinding(const std::string& name, unsigned int index);
	
};

class UniformBufferManager {
private:
	std::map<std::string, uniformStruct>* uniformBufferStruct;
	unsigned int index;

	unsigned int createUniformBuffer(GLsizeiptr size, GLenum usage);
public:
	unsigned int ID;

	UniformBufferManager(GLsizeiptr size, GLenum usage = GL_STATIC_DRAW) {
		uniformBufferStruct = new std::map<std::string, uniformStruct>;
		ID = createUniformBuffer(size, usage);
	}
	~UniformBufferManager() {
		delete uniformBufferStruct;
		glDeleteBuffers(GL_UNIFORM_BUFFER, &ID);
	}

	void uniformBufferBinding(unsigned index);
	void setUniformBufferBool(const std::string& name, int value);
	void setUniformBufferInt(const std::string name, int value);
	void setUniformBufferIntArray(const std::string name, unsigned int size, int value[]);
	void setUniformBufferFloat(const std::string name, float value);
	void setUniformBufferFloatArray(const std::string name, unsigned int size, float value[]);
	void setUniformBufferVec2f(const std::string name, glm::vec2 value);
	void setUniformBufferVec2fArray(const std::string name, unsigned int size, glm::vec2 value[]);
	void setUniformBufferVec3f(const std::string name, glm::vec3 value);
	void setUniformBufferVec3fArray(const std::string name, unsigned int size, glm::vec3 value[]);
	void setUniformBufferVec4f(const std::string name, glm::vec4 value);
	void setUniformBufferVec4fArray(const std::string name, unsigned int size, glm::vec4 value[]);
	void setUniformBufferMat3f(const std::string name, glm::mat3 value);
	void setUniformBufferMat4f(const std::string name, glm::mat4 value);
};

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;

	// 异常处理
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream;
		std::stringstream fShaderStream;
		// 读取缓存内容到数据流
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		vShaderFile.close();
		fShaderFile.close();

		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure error) {
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	unsigned int vertex, fragment;
	int success;
	char infoLog[512];

	// vertex
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATTON_FAILED\n" << infoLog << std::endl;
	}

	// fragment
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_FRAGMENT_SHADER, &success);
	if (!success) {
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "ERROE::SHADER::FRAGMENT::COMPILATTON_FAILED\n" << infoLog << std::endl;
	}

	// shader program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
{
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;

	// 异常处理
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		gShaderFile.open(geometryPath);
		std::stringstream vShaderStream;
		std::stringstream fShaderStream;
		std::stringstream gShaderStream;
		// 读取缓存内容到数据流
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		gShaderStream << gShaderFile.rdbuf();
		vShaderFile.close();
		fShaderFile.close();
		gShaderFile.close();

		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		geometryCode = gShaderStream.str();
	}
	catch (std::ifstream::failure error) {
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	const char* gShaderCode = geometryCode.c_str();

	unsigned int vertex, fragment, geometry;
	int success;
	char infoLog[512];

	// vertex
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATTON_FAILED\n" << infoLog << std::endl;
	}

	// fragment
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_FRAGMENT_SHADER, &success);
	if (!success) {
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "ERROE::SHADER::FRAGMENT::COMPILATTON_FAILED\n" << infoLog << std::endl;
	}

	// geometry
	geometry = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(geometry, 1, &gShaderCode, NULL);
	glCompileShader(geometry);
	glGetShaderiv(geometry, GL_GEOMETRY_SHADER, &success);
	if (!success) {
		glGetShaderInfoLog(geometry, 512, NULL, infoLog);
		std::cout << "ERROE::SHADER::GEOMETRY::COMPILATTON_FAILED\n" << infoLog << std::endl;
	}

	// shader program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glAttachShader(ID, geometry);
	glLinkProgram(ID);
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
	glDeleteShader(geometry);
}

void Shader::use()
{
	glUseProgram(ID);
}

// uniform 工具函数
void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value)
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2f(const std::string& name, float xValue, float yValue) {
	glUniform2f(glGetUniformLocation(ID, name.c_str()), xValue, yValue);
}

void Shader::setVec2f(const std::string& name, glm::vec2 value) {
	glUniform2f(glGetUniformLocation(ID, name.c_str()), value.x, value.y);
}

void Shader::setVec3f(const std::string& name, float xValue, float yValue, float zValue) {
	glUniform3f(glGetUniformLocation(ID, name.c_str()), xValue, yValue, zValue);
}

void Shader::setVec3f(const std::string& name, glm::vec3 value) {
	glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
}

void Shader::setMat4f(const std::string& name, int count, GLfloat* value) {
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), count, GL_FALSE, value);
}

void Shader::setMat4f(const std::string& name, GLfloat* value, int count = 1) {
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), count, GL_FALSE, value);
}

void Shader::setMat4f(const std::string& name, glm::mat4 value, int count = 1) {
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), count, GL_FALSE, glm::value_ptr(value));
}

// unifromBuffer 工具函数
void Shader::uniformBlockBinding(const std::string& name, unsigned int index) {
	unsigned int block_index = glGetUniformBlockIndex(ID, name.c_str());
	glUniformBlockBinding(ID, block_index, index);
}

unsigned int UniformBufferManager::createUniformBuffer(GLsizeiptr size, GLenum usage) {
	unsigned int uboID;
	glGenBuffers(1, &uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, uboID);
	glBufferData(GL_UNIFORM_BUFFER, size, NULL, usage);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	index = 0;
	return uboID;
}

void UniformBufferManager::uniformBufferBinding(unsigned index) {
	glBindBufferBase(GL_UNIFORM_BUFFER, index, ID);
}

void UniformBufferManager::setUniformBufferBool(const std::string& name, int value) {
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
	if (!uniformBufferStruct->count(name)) {
		glBufferSubData(GL_UNIFORM_BUFFER, index, 4, &value);
		(*uniformBufferStruct)[name] = uniformStruct(4, index);
		index += 4;
	}
	else {
		unsigned int base = (*uniformBufferStruct)[name].base;
		unsigned int offset = (*uniformBufferStruct)[name].offset;
		glBufferSubData(GL_UNIFORM_BUFFER, offset, base, &value);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBufferManager::setUniformBufferInt(const std::string name, int value) {
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
	if (!uniformBufferStruct->count(name)) {
		glBufferSubData(GL_UNIFORM_BUFFER, index, 4, &value);
		(*uniformBufferStruct)[name] = uniformStruct(4, index);
		index += 4;
	}
	else {
		unsigned int base = (*uniformBufferStruct)[name].base;
		unsigned int offset = (*uniformBufferStruct)[name].offset;
		glBufferSubData(GL_UNIFORM_BUFFER, offset, base, &value);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBufferManager::setUniformBufferIntArray(const std::string name, unsigned int size, int value[]) {
	for (int i = 0; i < size; i++) {
		setUniformBufferInt(name + "[" + std::to_string(i) + "]", value[i]);
		if (index % 16 != 0)
			index = index + 16 - (index % 16);
	}
}

void UniformBufferManager::setUniformBufferFloat(const std::string name, float value) {
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
	if (!uniformBufferStruct->count(name)) {
		glBufferSubData(GL_UNIFORM_BUFFER, index, 4, &value);
		(*uniformBufferStruct)[name] = uniformStruct(4, index);
		index += 4;
	}
	else {
		unsigned int base = (*uniformBufferStruct)[name].base;
		unsigned int offset = (*uniformBufferStruct)[name].offset;
		glBufferSubData(GL_UNIFORM_BUFFER, offset, base, &value);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBufferManager::setUniformBufferFloatArray(const std::string name, unsigned int size, float value[]) {
	for (int i = 0; i < size; i++) {
		setUniformBufferFloat(name + "[" + std::to_string(i) + "]", value[i]);
		if (index % 16 != 0)
			index = index + 16 - (index % 16);
	}
}

void UniformBufferManager::setUniformBufferVec2f(const std::string name, glm::vec2 value) {
	float data[] = { value.x, value.y };
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
	if (!uniformBufferStruct->count(name)) {
		// 必须是16的倍数，对(*curUniformBufferIndex)取16的整数倍
		unsigned int cur;
		if (index % 16 == 0) cur = index;
		else cur = index + 16 - (index % 16);

		glBufferSubData(GL_UNIFORM_BUFFER, cur, 8, &data);
		(*uniformBufferStruct)[name] = uniformStruct(8, cur);
		index = cur + 8;
	}
	else {
		unsigned int base = (*uniformBufferStruct)[name].base;
		unsigned int offset = (*uniformBufferStruct)[name].offset;
		glBufferSubData(GL_UNIFORM_BUFFER, offset, base, &data);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBufferManager::setUniformBufferVec2fArray(const std::string name, unsigned int size, glm::vec2 value[]) {
	for (int i = 0; i < size; i++) {
		setUniformBufferVec2f(name + "[" + std::to_string(i) + "]", value[i]);
		if (index % 16 != 0)
			index = index + 16 - (index % 16);
	}
}

void UniformBufferManager::setUniformBufferVec3f(const std::string name, glm::vec3 value) {
	float data[] = { value.x, value.y, value.z };
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
	if (!uniformBufferStruct->count(name)) {
		// 必须是16的倍数，对(*curUniformBufferIndex)取16的整数倍
		unsigned int cur;
		if (index % 16 == 0) cur = index;
		else cur = index + 16 - (index % 16);

		glBufferSubData(GL_UNIFORM_BUFFER, cur, 12, &data);
		(*uniformBufferStruct)[name] = uniformStruct(12, cur);
		index = cur + 12;
	}
	else {
		unsigned int base = (*uniformBufferStruct)[name].base;
		unsigned int offset = (*uniformBufferStruct)[name].offset;
		glBufferSubData(GL_UNIFORM_BUFFER, offset, base, &data);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBufferManager::setUniformBufferVec3fArray(const std::string name, unsigned int size, glm::vec3 value[]) {
	for (int i = 0; i < size; i++) {
		setUniformBufferVec3f(name + "[" + std::to_string(i) + "]", value[i]);
		if (index % 16 != 0)
			index = index + 16 - (index % 16);
	}
}

void UniformBufferManager::setUniformBufferVec4f(const std::string name, glm::vec4 value) {
	float data[] = { value.x, value.y, value.z, value.w };
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
	if (!uniformBufferStruct->count(name)) {
		// 必须是16的倍数，对(*curUniformBufferIndex)取16的整数倍
		unsigned int cur;
		if (index % 16 == 0) cur = index;
		else cur = index + 16 - (index % 16);

		glBufferSubData(GL_UNIFORM_BUFFER, cur, 16, &data);
		(*uniformBufferStruct)[name] = uniformStruct(16, cur);
		index = cur + 16;
	}
	else {
		unsigned int base = (*uniformBufferStruct)[name].base;
		unsigned int offset = (*uniformBufferStruct)[name].offset;
		glBufferSubData(GL_UNIFORM_BUFFER, offset, base, &data);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBufferManager::setUniformBufferVec4fArray(const std::string name, unsigned int size, glm::vec4 value[]) {
	for (int i = 0; i < size; i++) {
		setUniformBufferVec4f(name + "[" + std::to_string(i) + "]", value[i]);
		if (index % 16 != 0)
			index = index + 16 - (index % 16);
	}
}

void UniformBufferManager::setUniformBufferMat3f(const std::string name, glm::mat3 value) {
	// 矩阵算数组，在他之后的第一个变量的偏移地址应该是下一个16的倍数
	for (int i = 0; i < 3; i++) {
		glm::vec3 row = value[i];
		setUniformBufferVec3f(name + "[" + std::to_string(i) + "]", row);
	}
	if (index % 16 != 0)
		index = index + 16 - (index % 16);
}

void UniformBufferManager::setUniformBufferMat4f(const std::string name, glm::mat4 value) {
	// 矩阵算数组，在他之后的第一个变量的偏移地址应该是下一个16的倍数
	for (int i = 0; i < 4; i++) {
		glm::vec4 row = value[i];
		setUniformBufferVec4f(name + "[" + std::to_string(i) + "]", row);
	}
	if (index % 16 != 0)
		index = index + 16 - (index % 16);
}

#endif
