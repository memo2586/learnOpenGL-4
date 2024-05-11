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

#pragma comment (lib, "assimp-vc143-mt.lib")

// window config
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 55.0f), glm::vec2(SCR_WIDTH / 2.f, SCR_HEIGHT / 2.f));
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

	unsigned int amount = 50000;
	glm::mat4* modelMatrices;
	modelMatrices = new glm::mat4[amount];
	srand(glfwGetTime()); // 初始化随机种子    
	float radius = 50.0;
	float offset = 2.5f;
	for (unsigned int i = 0; i < amount; i++)
	{
		float test = rand();
		glm::mat4 model = glm::mat4(1.f);
		// 1. 位移：分布在半径为 'radius' 的圆形上，偏移的范围是 [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // 让行星带的高度比x和z的宽度要小
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. 缩放：在 0.05 和 0.25f 之间缩放
		float scale = (rand() % 3) / 100.0f + 0.005;
		model = glm::scale(model, glm::vec3(scale));

		// 3. 旋转：绕着一个（半）随机选择的旋转轴向量进行随机的旋转
		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. 添加到矩阵的数组中
		modelMatrices[i] = model;
	}

	Shader lightShader("shader/3.3.only_diff.vert", "shader/3.3.only_diff.frag");
	Shader rockShader("shader/3.3.rockShader.vert", "shader/3.3.only_diff.frag");

	Model* planet = new Model("model/star/planet.obj");
	Model* rock = new Model("model/star/rock.obj");

	UniformBufferManager* ubm = new UniformBufferManager();
	unsigned int mvp = ubm->createUniformBuffer(256, GL_DYNAMIC_DRAW);
	ubm->uniformBufferBinding(mvp, 0);
	unsigned int lighting = ubm->createUniformBuffer(64, GL_DYNAMIC_DRAW);
	ubm->uniformBufferBinding(lighting, 1);
	lightShader.uniformBlockBinding("mvp", 0);
	rockShader.uniformBlockBinding("mvp", 0);
	lightShader.uniformBlockBinding("lighting", 1);
	rockShader.uniformBlockBinding("lighting", 1);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
	for (int i = 0; i < rock->meshes.size(); i++) {
		unsigned int vao = rock->meshes[i].VAO;
		glBindVertexArray(vao);
		GLsizei v4size = sizeof(glm::vec4);
		// 顶点属性最大允许的数据大小为一个vec4
		// mat4可以作为4个vec4发送给着色器
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * v4size, (void*)0);
		glEnableVertexAttribArray(3);
		glVertexAttribDivisor(3, 1);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * v4size, (void*)(v4size));
		glEnableVertexAttribArray(4);
		glVertexAttribDivisor(4, 1);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * v4size, (void*)(2 * v4size));
		glEnableVertexAttribArray(5);
		glVertexAttribDivisor(5, 1);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * v4size, (void*)(3 * v4size));
		glEnableVertexAttribArray(6);
		glVertexAttribDivisor(6, 1);
		glBindVertexArray(0);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//camera.position = glm::vec3(0.f, 0.f, -50.f);
	//camera.front = glm::normalize(-camera.position);
	camera.lock = true;
	float dir = 0.f;

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

		// mvp
		glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 180.0f);
		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 model = glm::mat4(1.f);
		ubm->setUniformBufferMat4f(mvp, "model", model);
		ubm->setUniformBufferMat4f(mvp, "view", view);
		ubm->setUniformBufferMat4f(mvp, "projection", projection);

		// lighting
		ubm->setUniformBufferVec3f(lighting, "dircetion", glm::vec3(sin(dir), -.5f, cos(dir)));
		ubm->setUniformBufferVec3f(lighting, "ambient", glm::vec3(.05f));
		ubm->setUniformBufferVec3f(lighting, "diffuse", glm::vec3(5.f));

		// planet
		lightShader.use();
		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(0.f, -3.f, 0.f));
		model = glm::scale(model, glm::vec3(4.f));
		ubm->setUniformBufferMat4f(mvp, "model", model);
		planet->Draw(lightShader);
		// rock
		rockShader.use();
		for (unsigned int i = 0; i < rock->meshes.size(); i++) {
			glBindVertexArray(rock->meshes[i].VAO);
			glDrawElementsInstanced(GL_TRIANGLES, rock->meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount);
		}

		//Imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Menu");
		ImGui::Text("camera info: ");
		ImGui::Text("camera.position: %.2f %.2f %.2f", camera.position.x, camera.position.y, camera.position.z);
		ImGui::Text("camera.front: %.2f %.2f %.2f", camera.front.x, camera.front.y, camera.front.z);
		ImGui::Separator();
		ImGui::SliderFloat("LightDir", &dir, -glm::pi<float>(), glm::pi<float>());
		ImGui::End();
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteProgram(rockShader.ID);
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