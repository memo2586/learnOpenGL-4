#include <iostream>
#include <vector>
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
Camera camera(glm::vec3(0.f), glm::vec2(SCR_WIDTH / 2.f, SCR_HEIGHT / 2.f));
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

	vector<glm::vec3> vegetation;
	vegetation.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
	vegetation.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
	vegetation.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
	vegetation.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
	vegetation.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

	Shader lightShader("shader/3.3.only_diff.vert", "shader/3.3.only_diff.frag");
	Shader shader("shader/3.3.shader.vert", "shader/3.3.shader.frag");

	Model* floor = new Model("model/room/floor.obj");
	//Model* kusa = new Model("model/kusacube/kusacube.obj");
	Model* blend_window = new Model("model/window/blend_window.obj");
	Model* pointlight = new Model("model/pointlight/pointlight.obj");

	glEnable(GL_DEPTH_TEST);
	glm::mat4 model;
	glm::mat4 normal;

	// 控制变量
	camera.position = glm::vec3(-4.5f, .2f, 0.f);
	camera.front = glm::vec3(1.f, 0.f, 0.f);
	glm::vec3 pointlight_pos;

	// 启用混合
	glEnable(GL_BLEND);
	// 设置混合函数
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(window)) {
		// timing
		float curTime = static_cast<float>(glfwGetTime());
		deltaTime = curTime - lastTime;
		lastTime = curTime;

		// position
		pointlight_pos = glm::vec3(sin(curTime), sin(curTime) / 4.f + .3f, cos(curTime));

		// input
		keyboard.processInput(window, deltaTime);

		// render init
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render
		glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();
		// model: light
		lightShader.use();
		lightShader.setMat4f("projection", 1, glm::value_ptr(projection));
		lightShader.setMat4f("view", 1, glm::value_ptr(view));
		model = glm::mat4(1.0f);
		model = glm::translate(model, pointlight_pos);
		model = glm::scale(model, glm::vec3(.01f));
		lightShader.setMat4f("model", 1, glm::value_ptr(model));
		pointlight->Draw(lightShader);

		shader.use();
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
		shader.setFloat("pointLight.linear", .045f);
		shader.setFloat("pointLight.quadratic", .0075f);

		// model: kusa
		//for (int i = 0; i < vegetation.size(); i++) {
		//	model = glm::mat4(1.0f);
		//	normal = glm::mat4(1.0f);
		//	model = glm::translate(model, vegetation[i]);
		//	model = glm::scale(model, glm::vec3(.3f));
		//	normal = glm::transpose(glm::inverse(model));
		//	shader.setMat4f("model", 1, glm::value_ptr(model));
		//	shader.setMat4f("nrmMat", 1, glm::value_ptr(normal));
		//	kusa->Draw(shader);
		//}

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

		// blend_window
		model = glm::mat4(1.f);
		normal = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.f, .25f, 0.f));
		model = glm::scale(model, glm::vec3(.5f));
		model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
		normal = glm::transpose(glm::inverse(model));
		shader.setMat4f("model", 1, glm::value_ptr(model));
		shader.setMat4f("nrmMat", 1, glm::value_ptr(normal));
		blend_window->Draw(shader);

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

	//delete kusa;
	delete blend_window;
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