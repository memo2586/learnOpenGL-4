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
	Shader colorShader("shader/3.3.shader.vert", "shader/3.3.only_color.frag");
	Shader shader("shader/3.3.shader.vert", "shader/3.3.shader.frag");

	Model* floor = new Model("model/room/floor.obj");
	Model* nanosuit = new Model("model/nanosuit/nanosuit.obj");
	Model* bignanosuit = new Model("model/nanosuit/1.03nanosuit.obj");
	Model* cube = new Model("model/room/0.5cube.obj");
	Model* pointlight = new Model("model/pointlight/pointlight.obj");

	glEnable(GL_DEPTH_TEST);
	glm::mat4 model;
	glm::mat4 normal;

	// 控制变量
	camera.position = glm::vec3(2.5f, 1.5f, -1.5f);
	//camera.front = glm::vec3(-.83f, -.34f, .45f);

	glEnable(GL_CULL_FACE);	// 开启面剔除

	//glCullFace(GL_FRONT);	// 设置剔除的面的类型，试试剔除前面
	//glFrontFace(GL_CW);	// 尝试定义顺时针是前面(因为剔除的是前面所以将顺时针的剔除，可以得到正常的效果)

	//glCullFace(GL_BACK);	// 剔除背面
	//glFrontFace(GL_CW);		// 定义顺时针是前面(会剔除逆时针的面，所以结果是背面被渲染出来)

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

		// render
		shader.use();
		glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();
		shader.setMat4f("projection", 1, glm::value_ptr(projection));
		shader.setMat4f("view", 1, glm::value_ptr(view));
		shader.setVec3f("viewPos", camera.position);
		shader.setFloat("material.shininess", 32.f);

		// lighting
		shader.setBool("pointLight.enable", 1);
		shader.setVec3f("pointLight.position", glm::vec3(1.f, 1.f, 0.f));
		shader.setVec3f("pointLight.ambient", .1f, .1f, .1f);
		shader.setVec3f("pointLight.diffuse", .5f, .5f, .5f);
		shader.setVec3f("pointLight.specular", 1.f, 1.f, 1.f);
		shader.setFloat("pointLight.constant", 1.f);
		shader.setFloat("pointLight.linear", .09f);
		shader.setFloat("pointLight.quadratic", .032f);

		// model: cube
		//model = glm::mat4(1.0f);
		//normal = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(0., .25f, 0.f));
		//model = glm::scale(model, glm::vec3(.5f));
		//normal = glm::transpose(glm::inverse(model));
		//shader.setMat4f("model", 1, glm::value_ptr(model));
		//shader.setMat4f("nrmMat", 1, glm::value_ptr(normal));
		//cube->Draw(shader);
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
		
		// model: light
		lightShader.use();
		lightShader.setMat4f("projection", 1, glm::value_ptr(projection));
		lightShader.setMat4f("view", 1, glm::value_ptr(view));
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(1.f, 1.f, 0.f));
		model = glm::scale(model, glm::vec3(.01f));
		lightShader.setMat4f("model", 1, glm::value_ptr(model));
		pointlight->Draw(lightShader);

		// 先设置为剔除正面
		glCullFace(GL_FRONT);
		colorShader.use();
		colorShader.setMat4f("projection", 1, glm::value_ptr(projection));
		colorShader.setMat4f("view", 1, glm::value_ptr(view));
		model = glm::mat4(1.0f);
		normal = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.f, 0.f, 0.f));
		model = glm::scale(model, glm::vec3(.1f));
		normal = glm::transpose(glm::inverse(model));
		colorShader.setMat4f("model", 1, glm::value_ptr(model));
		shader.setMat4f("nrmMat", 1, glm::value_ptr(normal));
		bignanosuit->Draw(colorShader);

		// 恢复为剔除背面后正常绘制原来的模型，就可以得到一个纯色的描边
		glCullFace(GL_BACK);
		shader.use();
		model = glm::mat4(1.0f);
		normal = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.f, 0.f, 0.f));
		model = glm::scale(model, glm::vec3(.1f));
		normal = glm::transpose(glm::inverse(model));
		shader.setMat4f("model", 1, glm::value_ptr(model));
		shader.setMat4f("nrmMat", 1, glm::value_ptr(normal));
		nanosuit->Draw(shader);

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

	delete cube;
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