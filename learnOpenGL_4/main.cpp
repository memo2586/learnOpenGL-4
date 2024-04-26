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
	Shader shader("shader/3.3.shader.vert", "shader/3.3.shader.frag");
	Shader colorShader("shader/3.3.shader.vert", "shader/3.3.only_color.frag");

	Model* floor = new Model("model/room/floor.obj");
	Model* couch = new Model("model/room/couch.obj");
	Model* pointlight = new Model("model/pointlight/pointlight.obj");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glm::mat4 model;
	glm::mat4 normal;

	// 控制变量
	camera.position = glm::vec3(2.5f, 1.5f, -1.5f);
	camera.front = glm::vec3(-.83f, -.34f, .45f);
	float couch_height = 0.f;

	while (!glfwWindowShouldClose(window)) {
		// timing
		float curTime = static_cast<float>(glfwGetTime());
		deltaTime = curTime - lastTime;
		lastTime = curTime;

		// input
		keyboard.processInput(window, deltaTime);

		// render init
		glStencilMask(0xff);	// 先让buffer可写以便清空buffer
		glClearColor(0.f, 0.f, 0.f, 1.f);
		// 模板测试 & 深度测试都通过 -> 设置模板值为1
		// 模板测试通过 & 深度测试不通过 -> 设置模板值为1（绘制物体被遮挡时依旧绘制边框，有种透视的效果）
		glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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

		// model: floor
		shader.use();
		model = glm::mat4(1.f);
		normal = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.f, 0.f, 0.f));
		model = glm::scale(model, glm::vec3(1.f));
		model = glm::rotate(model, glm::radians(0.f), glm::vec3(1.f, 0.f, 0.f));
		normal = glm::transpose(glm::inverse(model));
		shader.setMat4f("model", 1, glm::value_ptr(model));
		shader.setMat4f("nrmMat", 1, glm::value_ptr(normal));
		floor->Draw(shader);
		// 在绘制完其他物体以后

		// model: couch
		// 先绘制一遍模型，模板设置为总是通过，会让绘制模型的片段的模板值全部设置为1
		glStencilFunc(GL_ALWAYS, 1, 0xff);
		// 启用buffer写入
		glStencilMask(0xff);
		model = glm::mat4(1.0f);
		normal = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.f, couch_height, 0.f));
		model = glm::scale(model, glm::vec3(.5f));
		normal = glm::transpose(glm::inverse(model));
		shader.setMat4f("model", 1, glm::value_ptr(model));
		shader.setMat4f("nrmMat", 1, glm::value_ptr(normal));
		couch->Draw(shader);
		// couch broad
		colorShader.use();
		// 设置模板值不为1的片段通过测试
		glStencilFunc(GL_NOTEQUAL, 1, 0xff);
		// 禁用写入buffer
		glStencilMask(0x00);
		// 关闭深度测试，可以让边框总是显示不会被遮挡
		glDisable(GL_DEPTH_TEST);
		model = glm::mat4(1.0f);
		normal = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.f, couch_height, 0.f));
		model = glm::scale(model, glm::vec3(.51f));	// 稍微放大一下模型，渲染时比原来稍大的模型片段会被渲染
		normal = glm::transpose(glm::inverse(model));
		colorShader.setMat4f("projection", 1, glm::value_ptr(projection));
		colorShader.setMat4f("view", 1, glm::value_ptr(view));
		colorShader.setMat4f("model", 1, glm::value_ptr(model));
		colorShader.setMat4f("nrmMat", 1, glm::value_ptr(normal));
		// 用渲染为纯色的shader
		couch->Draw(colorShader);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);
		// 重新启用深度测试
		glEnable(GL_DEPTH_TEST);
		
		// model: light
		lightShader.use();
		lightShader.setMat4f("projection", 1, glm::value_ptr(projection));
		lightShader.setMat4f("view", 1, glm::value_ptr(view));
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(1.f, 1.f, 0.f));
		model = glm::scale(model, glm::vec3(.01f));
		lightShader.setMat4f("model", 1, glm::value_ptr(model));
		pointlight->Draw(lightShader);

		//Imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Menu");
		ImGui::Text("camera info: ");
		ImGui::Text("camera.position: %.2f %.2f %.2f", camera.position.x, camera.position.y, camera.position.z);
		ImGui::Text("camera.front: %.2f %.2f %.2f", camera.front.x, camera.front.y, camera.front.z);
		ImGui::Separator();
		ImGui::SliderFloat("couch.height", &couch_height, -5.f, 5.f);
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

	delete couch;
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