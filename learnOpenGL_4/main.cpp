#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include <LearnOpenGL/shader_s.h>
#include <LearnOpenGL/camera.h>
#include <LearnOpenGL/keyboard.h>
#include <LearnOpenGL/mesh.h>
#include <LearnOpenGL/model.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"
#include "sphere.h"

#pragma comment (lib, "assimp-vc143-mt.lib")

// window config
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// camera
Camera camera(glm::vec3(0.f));
bool firstMouse = true;
float lastXpos = SCR_WIDTH / 2.f;
float lastYpos = SCR_HEIGHT / 2.f;

// keyboard
Keyboard keyboard(&camera);

// timing
float deltaTime = 0.f;
float lastTime = 0.f;

/* --------------------------------------------------- */

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double XposIn, double YposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int loadCubemap(std::vector<std::string> filePath);

int main() {
	// init glfwwindow config
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// create glfwwindow
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL4", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// init glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}
	// glfwwindow setting
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// callback fun
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	// init dear imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");
	ImGui::StyleColorsDark();
	// create assimp importer
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		"bun_zipper.ply",
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType
	);

	Shader lightShader("shader/3.3.only_diff.vert", "shader/3.3.only_diff.frag");
	Shader skyboxShader("shader/3.3.skyboxShader.vert", "shader/3.3.skyboxShader.frag");
	Shader envShader("shader/3.3.envShader.vert", "shader/3.3.envShader.frag");
	Shader shader("shader/3.3.shader.vert", "shader/3.3.shader.frag");

	Model* floor = new Model("model/room/floor.obj");
	Model* nanosuit = new Model("model/nanosuit/nanosuit.obj");
	Model* pointlight = new Model("model/pointlight/pointlight.obj");

	// skybox file path vector
	std::vector<std::string> skyboxFilePath = {
		"skybox/right.jpg",
		"skybox/left.jpg",
		"skybox/top.jpg",
		"skybox/bottom.jpg",
		"skybox/front.jpg",	// 因为摄像机朝向-z，这里前后要反过来
		"skybox/back.jpg",
	};
	
	unsigned int skybox = loadCubemap(skyboxFilePath);
	unsigned int skyboxVAO, vbo;
	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER ,0);

	glEnable(GL_DEPTH_TEST);
	glm::mat4 model;
	glm::mat4 normal;

	// 控制变量
	camera.position = glm::vec3(0.f);
	camera.front = glm::vec3(-.83f, -.34f, .45f);

	while (!glfwWindowShouldClose(window)) {
		// timing
		float curTime = static_cast<float>(glfwGetTime());
		deltaTime = curTime - lastTime;
		lastTime = curTime;

		// input
		keyboard.processInput(window, deltaTime);

		// render init
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();
		// render

		//// 第一个渲染天空盒
		//// 被物体遮挡的地方也会绘制天空盒，浪费处理器性能
		//glDepthMask(GL_FALSE);
		//skyboxShader.use();
		//skyboxShader.setMat4f("view", glm::mat4(glm::mat3(view)));	// 移除位移
		//skyboxShader.setMat4f("projection", projection);
		//glBindVertexArray(skyboxVAO);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
		//glDrawArrays(GL_TRIANGLES, 0, 36);
		//glDepthMask(GL_TRUE);

		shader.use();
		shader.setMat4f("projection", 1, glm::value_ptr(projection));
		shader.setMat4f("view", 1, glm::value_ptr(view));
		shader.setVec3f("viewPos", camera.position);
		shader.setFloat("material.shininess", 32.f);

		// lighting
		//shader.setBool("pointLight.enable", 0);
		//shader.setVec3f("pointLight.position", glm::vec3(1.f, 1.f, 0.f));
		//shader.setVec3f("pointLight.ambient", .1f, .1f, .1f);
		//shader.setVec3f("pointLight.diffuse", .5f, .5f, .5f);
		//shader.setVec3f("pointLight.specular", 1.f, 1.f, 1.f);
		//shader.setFloat("pointLight.constant", 1.f);
		//shader.setFloat("pointLight.linear", .09f);
		//shader.setFloat("pointLight.quadratic", .032f);
		shader.setBool("dirLight.enable", true);
		shader.setVec3f("dirLight.direction", glm::vec3(.5f, -.5f, .7f));
		shader.setVec3f("dirLight.ambient", glm::vec3(.5f));
		shader.setVec3f("dirLight.diffuse", glm::vec3(.8f));
		shader.setVec3f("dirLight.specular", glm::vec3(1.f));

		// model: floor
		model = glm::mat4(1.f);
		normal = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.f, 0.f, 0.f));
		model = glm::scale(model, glm::vec3(1.f));
		model = glm::rotate(model, glm::radians(0.f), glm::vec3(1.f, 0.f, 0.f));
		normal = glm::transpose(glm::inverse(model));
		shader.setMat4f("model", 1, glm::value_ptr(model));
		shader.setMat4f("nrmMat", 1, glm::value_ptr(normal));
		//floor->Draw(shader);
		
		//// model: light
		//lightShader.use();
		//lightShader.setMat4f("projection", 1, glm::value_ptr(projection));
		//lightShader.setMat4f("view", 1, glm::value_ptr(view));
		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(1.f, 1.f, 0.f));
		//model = glm::scale(model, glm::vec3(.01f));
		//lightShader.setMat4f("model", 1, glm::value_ptr(model));
		//pointlight->Draw(lightShader);

		// model: nanosuit
		envShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
		envShader.setInt("skybox", 0);
		envShader.setVec3f("viewPos", camera.position);
		envShader.setMat4f("view", view);
		envShader.setMat4f("projection", projection);
		model = glm::mat4(1.0f);
		normal = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.f, 0.001f, 0.f));
		model = glm::scale(model, glm::vec3(.1f));
		//model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
		normal = glm::transpose(glm::inverse(model));
		envShader.setMat4f("model", 1, glm::value_ptr(model));
		envShader.setMat4f("nrmMat", 1, glm::value_ptr(normal));
		nanosuit->Draw(envShader);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		// 优化：提前深度缓冲，只在通过测试的片段绘制天空盒
		// 另外需要修改skyboxShader欺骗opengl我们的天空盒深度恒为1
		glDepthFunc(GL_LEQUAL);	// 更改深度函数，以便当值等于深度缓冲区的内容时深度测试通过
		skyboxShader.use();
		skyboxShader.setMat4f("view", glm::mat4(glm::mat3(view)));	// 移除位移
		skyboxShader.setMat4f("projection", projection);
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthFunc(GL_LESS);

		//Imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Menu");
		ImGui::Text("camera info: ");
		ImGui::Text("camera.position: %.2f %.2f %.2f", camera.position.x, camera.position.y, camera.position.z);
		ImGui::Text("camera.front: %.2f %.2f %.2f", camera.front.x, camera.front.y, camera.front.z);
		ImGui::Separator();
		ImGui::End();
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	delete nanosuit;
	delete floor;
	delete pointlight;
	glDeleteProgram(shader.ID);
	glDeleteProgram(lightShader.ID);
	glfwTerminate();

	return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double XposIn, double YposIn)
{
	camera.mouse_callback(window, XposIn, YposIn);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.scroll_callback(window, xoffset, yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	keyboard.cameraLock_key_callback(window, key, scancode, action, mods);
	keyboard.mouse_display_key_callback(window, key, scancode, action, mods);
}

unsigned int loadCubemap(std::vector<std::string> filePath) {
	// 创建立方体贴图
	unsigned int skybox;
	glGenTextures(1, &skybox);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
	// 坏消息：我们要调用glTexImage2D函数6次；好消息：枚举值的int值是连续递增的
	int width, height, nrChannels;
	unsigned char* data;
	for (unsigned int i = 0; i < filePath.size(); i++) {
		data = stbi_load(filePath[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else {
			std::cout << "Cubemap texture failed to load at path: " << filePath[i] << std::endl;
			stbi_image_free(data);
		}
	}
	// 设定环绕以及过滤方式
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	return skybox;
}