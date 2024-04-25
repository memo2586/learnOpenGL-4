#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
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

#pragma comment (lib, "assimp-vc143-mt.lib")

// window config
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// camera
Camera camera(glm::vec3(0.f), glm::vec2(SCR_WIDTH / 2.f, SCR_HEIGHT / 2.f));
Camera frameCamera(glm::vec3(0.f), glm::vec2(SCR_WIDTH / 2.f, SCR_HEIGHT / 2.f));
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
unsigned int loadTexture(char const* path);

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
	Shader colorShader("shader/3.3.only_color.vert", "shader/3.3.only_color.frag");
	Shader shader("shader/3.3.shader.vert", "shader/3.3.shader.frag");

	Model* couch = new Model("model/room/couch.obj");
	Model* floor = new Model("model/room/floor.obj");
	Model* pointlight = new Model("model/pointlight/pointlight.obj");

	float Vertices[] = {
		-1.0f,  1.0f, 0.f,  0.0f, 1.0f,
		-1.0f, -1.0f, 0.f,  0.0f, 0.0f,
		 1.0f, -1.0f, 0.f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.f,  0.0f, 1.0f,
		 1.0f, -1.0f, 0.f,  1.0f, 0.0f,
		 1.0f,  1.0f, 0.f,  1.0f, 1.0f
	};
	unsigned int frameVAO, frameVBO;
	glGenVertexArrays(1, &frameVAO);
	glBindVertexArray(frameVAO);
	glGenBuffers(1, &frameVBO);
	glBindBuffer(GL_ARRAY_BUFFER, frameVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);

	// 创建和绑定一个新的帧缓冲对象
	unsigned int FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	// 创建和绑定一个纹理后，为其分配内存与设置纹理参数,并将它绑定到帧缓冲对象
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);	// 将其作为颜色附件绑定到帧缓冲对象
	// 因为要用到深度测试所以还要给帧缓冲对象绑定一个深度（和模板）附件
	// 由于不用对这个深度附件采样，所以创建一个渲染缓存对象作为附件
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);	// 这个渲染缓存对象内部格式为24bit深度和8bit模板
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);	// 绑定附件并告知帧缓冲对象这是一个深度和模板附件
	// 检查帧缓冲是否完整
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER::Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glm::mat4 model;
	glm::mat4 normal;

	// 控制变量
	camera.position = glm::vec3(3.76f, 1.24f, 0.f);
	camera.front = glm::vec3(-1.f, -0.2f, 0.f);

	while (!glfwWindowShouldClose(window)) {
		// timing
		float curTime = static_cast<float>(glfwGetTime());
		deltaTime = curTime - lastTime;
		lastTime = curTime;
		glm::vec3 pointlight_pos = glm::vec3(sin(curTime) * 2.5f, 1.f, cos(curTime) * 2.5f);

		// input
		keyboard.processInput(window, deltaTime);

		glEnable(GL_CULL_FACE);
		// 离屏渲染，渲染结果会被保存到FBO的颜色附件texture上
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glEnable(GL_DEPTH_TEST);
		glClearColor(.0f, .0f, .0f, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		// 进行离屏渲染
		{
			frameCamera.position = pointlight_pos;
			frameCamera.front = -(pointlight_pos - glm::vec3(0.f));
			shader.use();
			glm::mat4 projection = glm::perspective(glm::radians(frameCamera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			glm::mat4 view = frameCamera.getViewMatrix();
			shader.setMat4f("projection", 1, glm::value_ptr(projection));
			shader.setMat4f("view", 1, glm::value_ptr(view));
			shader.setVec3f("viewPos", frameCamera.position);
			shader.setFloat("material.shininess", 32.f);

			// lighting
			shader.setBool("pointLight.enable", 1);
			shader.setVec3f("pointLight.position", pointlight_pos);
			shader.setVec3f("pointLight.ambient", .1f, .1f, .1f);
			shader.setVec3f("pointLight.diffuse", .5f, .5f, .5f);
			shader.setVec3f("pointLight.specular", 1.f, 1.f, 1.f);
			shader.setFloat("pointLight.constant", 1.f);
			shader.setFloat("pointLight.linear", .09f);
			shader.setFloat("pointLight.quadratic", .032f);

			// model: floor
			model = glm::mat4(1.f);
			normal = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.f, 0.f, 0.f));
			model = glm::scale(model, glm::vec3(1.f));
			model = glm::rotate(model, glm::radians(0.f), glm::vec3(1.f, 0.f, 0.f));
			normal = glm::transpose(glm::inverse(model));
			shader.setMat4f("model", 1, glm::value_ptr(model));
			shader.setMat4f("nrmMat", 1, glm::value_ptr(normal));
			floor->Draw(shader);

			// model: couch
			model = glm::mat4(1.f);
			normal = glm::mat4(1.f);
			model = glm::translate(model, glm::vec3(0.f, 0.0001f, 0.f));
			model = glm::scale(model, glm::vec3(.5f));
			normal = glm::transpose(glm::inverse(model));
			shader.setMat4f("model", model);
			shader.setMat4f("nrmMat", normal);
			couch->Draw(shader);
		}
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// render
		// render init
		// 正常渲染场景
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();
		{
			shader.setMat4f("projection", 1, glm::value_ptr(projection));
			shader.setMat4f("view", 1, glm::value_ptr(view));
			shader.setVec3f("viewPos", camera.position);
			shader.setFloat("material.shininess", 32.f);

			// lighting
			shader.setBool("pointLight.enable", 1);
			shader.setVec3f("pointLight.position", pointlight_pos);
			shader.setVec3f("pointLight.ambient", .1f, .1f, .1f);
			shader.setVec3f("pointLight.diffuse", .5f, .5f, .5f);
			shader.setVec3f("pointLight.specular", 1.f, 1.f, 1.f);
			shader.setFloat("pointLight.constant", 1.f);
			shader.setFloat("pointLight.linear", .09f);
			shader.setFloat("pointLight.quadratic", .032f);

			// model: floor
			model = glm::mat4(1.f);
			normal = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.f, 0.f, 0.f));
			model = glm::scale(model, glm::vec3(1.f));
			model = glm::rotate(model, glm::radians(0.f), glm::vec3(1.f, 0.f, 0.f));
			normal = glm::transpose(glm::inverse(model));
			shader.setMat4f("model", 1, glm::value_ptr(model));
			shader.setMat4f("nrmMat", 1, glm::value_ptr(normal));
			floor->Draw(shader);

			// model: couch
			model = glm::mat4(1.f);
			normal = glm::mat4(1.f);
			model = glm::translate(model, glm::vec3(0.f, 0.0001f, 0.f));
			model = glm::scale(model, glm::vec3(.5f));
			normal = glm::transpose(glm::inverse(model));
			shader.setMat4f("model", model);
			shader.setMat4f("nrmMat", normal);
			couch->Draw(shader);

			// model: light
			lightShader.use();
			lightShader.setMat4f("projection", 1, glm::value_ptr(projection));
			lightShader.setMat4f("view", 1, glm::value_ptr(view));
			model = glm::mat4(1.0f);
			model = glm::translate(model, pointlight_pos);
			model = glm::scale(model, glm::vec3(.01f));
			lightShader.setMat4f("model", 1, glm::value_ptr(model));
			pointlight->Draw(lightShader);
		}

		glDisable(GL_CULL_FACE);
		// draw frame
		colorShader.use();
		glBindVertexArray(frameVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);	// 把颜色附件作为纹理贴图渲染到平面上
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(0.f, 1.4f, 0.f));
		model = glm::scale(model, glm::vec3(.5f));
		model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
		colorShader.setMat4f("view", view);
		colorShader.setMat4f("projection", projection);
		colorShader.setMat4f("model", model);
		colorShader.setInt("frame_texture", 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

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

	delete floor;
	delete pointlight;
	glDeleteVertexArrays(1, &frameVAO);
	glDeleteBuffers(1, &frameVBO);
	glDeleteTextures(1, &texture);
	glDeleteProgram(colorShader.ID);
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

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}