#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>

#include <iostream>
#include <string>
#include <cmath>
#include <memory>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

enum class AnimationState
{
	Idle,
	Walk,
	Run,
	Action,
	Taunt
};

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// third-person camera
float orbitYaw = 180.0f;
float orbitPitch = 30.0f;
float cameraDistance = 4.5f;
const float minCameraDistance = 2.0f;
const float maxCameraDistance = 9.0f;
const float cameraTargetHeight = 1.8f;

// character controller
glm::vec3 characterPosition(0.0f, -0.4f, 0.0f);
float characterYaw = 0.0f;
const float walkSpeed = 1.8f;
const float runSpeed = 3.3f;
const float groundY = -0.4f;
const float planeHalfSize = 20.0f;
const float characterScale = 0.01f;
AnimationState currentState = AnimationState::Idle;
bool triggerJump = false;
bool triggerTaunt = false;
float actionTimer = 0.0f;
float actionDuration = 0.9f;
float tauntTimer = 0.0f;
float tauntDuration = 1.0f;
float animationSpeed = 0.0f;
glm::vec3 currentGroundVelocity(0.0f, 0.0f, 0.0f);
glm::vec3 jumpMomentumVelocity(0.0f, 0.0f, 0.0f);
const float jumpMomentumCarryMultiplier = 1.45f;
const float jumpMomentumDecay = 1.2f;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetWindowTitle(window, "Third-Person Skeletal Controller  |  WASD Move  Shift Run  Space Jump  F Taunt  Mouse Orbit  Wheel Zoom");

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("anim_model.vs", "anim_model.fs");
	Shader walkAreaShader("walk_area.vs", "walk_area.fs");
	Shader skyboxShader("skybox.vs", "skybox.fs");

	float walkAreaVertices[] = {
		-planeHalfSize, groundY, -planeHalfSize,
		-planeHalfSize, groundY, planeHalfSize,
		planeHalfSize, groundY, planeHalfSize,
		planeHalfSize, groundY, -planeHalfSize};

	unsigned int walkAreaIndices[] = {
		0, 1, 2,
		0, 2, 3};

	unsigned int walkAreaVAO = 0, walkAreaVBO = 0, walkAreaEBO = 0;
	glGenVertexArrays(1, &walkAreaVAO);
	glGenBuffers(1, &walkAreaVBO);
	glGenBuffers(1, &walkAreaEBO);

	glBindVertexArray(walkAreaVAO);
	glBindBuffer(GL_ARRAY_BUFFER, walkAreaVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(walkAreaVertices), walkAreaVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, walkAreaEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(walkAreaIndices), walkAreaIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	// Create a full-screen quad for the HDR background.
	float skyboxVertices[] = {
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f,
		-1.0f, -1.0f,
		 1.0f,  1.0f,
		-1.0f,  1.0f
	};

	unsigned int skyboxVAO = 0, skyboxVBO = 0;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	// Load HDR skybox texture
	unsigned int hdrTexture = 0;
	int width, height, nrComponents;
	float *data = stbi_loadf(FileSystem::getPath("resources/hdr/citrus_orchard_road_puresky_1k.hdr").c_str(), &width, &height, &nrComponents, 0);
	
	if (data)
	{
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
		std::cout << "Successfully loaded HDR skybox" << std::endl;
	}
	else
	{
		std::cout << "Failed to load HDR skybox. Using default background." << std::endl;
	}

	// load models
	// -----------
	const std::string walkAsset = "resources/test/Walking.fbx";
	const std::string runAsset = "resources/test/Running.fbx";
	const std::string jumpAsset = "resources/test/Jump.fbx";
	const std::string tauntAsset = "resources/test/Taunt.fbx";

	std::unique_ptr<Model> ourModel = std::make_unique<Model>(FileSystem::getPath(walkAsset));
	std::unique_ptr<Animation> walkAnimation = std::make_unique<Animation>(FileSystem::getPath(walkAsset), ourModel.get());
	std::unique_ptr<Animation> runAnimation = std::make_unique<Animation>(FileSystem::getPath(runAsset), ourModel.get());
	std::unique_ptr<Animation> jumpAnimation = std::make_unique<Animation>(FileSystem::getPath(jumpAsset), ourModel.get());
	std::unique_ptr<Animation> tauntAnimation = std::make_unique<Animation>(FileSystem::getPath(tauntAsset), ourModel.get());

	if (ourModel->meshes.empty() || walkAnimation->GetDuration() <= 0.0f || runAnimation->GetDuration() <= 0.0f || jumpAnimation->GetDuration() <= 0.0f || tauntAnimation->GetDuration() <= 0.0f)
	{
		std::cout << "ERROR::ASSET::Failed to load required assets: " << walkAsset << ", " << runAsset << ", " << jumpAsset << " and/or " << tauntAsset << std::endl;
		std::cout << "ERROR::ASSET::Please provide valid animated FBX files for this demo." << std::endl;
		glDeleteVertexArrays(1, &walkAreaVAO);
		glDeleteBuffers(1, &walkAreaVBO);
		glDeleteBuffers(1, &walkAreaEBO);
		glfwTerminate();
		return -1;
	}

	if (jumpAnimation->GetTicksPerSecond() > 0.0f)
		actionDuration = jumpAnimation->GetDuration() / jumpAnimation->GetTicksPerSecond();
	if (tauntAnimation->GetTicksPerSecond() > 0.0f)
		tauntDuration = tauntAnimation->GetDuration() / tauntAnimation->GetTicksPerSecond();

	Animator animator(walkAnimation.get());
	animator.UpdateAnimation(0.0f);
	AnimationState previousState = currentState;

	// draw in wireframe
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		if (triggerTaunt && currentState != AnimationState::Action && currentState != AnimationState::Taunt)
		{
			currentState = AnimationState::Taunt;
			tauntTimer = tauntDuration;
			jumpMomentumVelocity = glm::vec3(0.0f);
			triggerTaunt = false;
		}
		else if (triggerJump && currentState != AnimationState::Action && currentState != AnimationState::Taunt)
		{
			currentState = AnimationState::Action;
			actionTimer = actionDuration;
			jumpMomentumVelocity = currentGroundVelocity * jumpMomentumCarryMultiplier;
			triggerJump = false;
		}
		else
		{
			triggerTaunt = false;
			triggerJump = false;
		}

		if (currentState == AnimationState::Action)
		{
			characterPosition += jumpMomentumVelocity * deltaTime;
			jumpMomentumVelocity = glm::mix(jumpMomentumVelocity, glm::vec3(0.0f), glm::clamp(jumpMomentumDecay * deltaTime, 0.0f, 1.0f));

			actionTimer -= deltaTime;
			if (actionTimer <= 0.0f)
			{
				actionTimer = 0.0f;
				jumpMomentumVelocity = glm::vec3(0.0f);
				currentState = AnimationState::Idle;
			}
		}
		else if (currentState == AnimationState::Taunt)
		{
			tauntTimer -= deltaTime;
			if (tauntTimer <= 0.0f)
			{
				tauntTimer = 0.0f;
				currentState = AnimationState::Idle;
			}
		}

		if (currentState == AnimationState::Idle)
			animationSpeed = 0.0f;
		else if (currentState == AnimationState::Walk)
			animationSpeed = 0.85f;
		else if (currentState == AnimationState::Run)
			animationSpeed = 1.0f;
		else
			animationSpeed = 1.0f;

		if (currentState != previousState)
		{
			if (currentState == AnimationState::Run)
				animator.PlayAnimation(runAnimation.get());
			else if (currentState == AnimationState::Action)
				animator.PlayAnimation(jumpAnimation.get());
			else if (currentState == AnimationState::Taunt)
				animator.PlayAnimation(tauntAnimation.get());
			else
				animator.PlayAnimation(walkAnimation.get());

			animator.UpdateAnimation(0.0f);
		}

		if (animationSpeed > 0.0f)
			animator.UpdateAnimation(deltaTime * animationSpeed);

		previousState = currentState;

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 cameraTarget = characterPosition + glm::vec3(0.0f, cameraTargetHeight, 0.0f);
		float yawRad = glm::radians(orbitYaw);
		float pitchRad = glm::radians(orbitPitch);
		glm::vec3 cameraOffset;
		cameraOffset.x = cameraDistance * cosf(pitchRad) * sinf(yawRad);
		cameraOffset.y = cameraDistance * sinf(pitchRad);
		cameraOffset.z = cameraDistance * cosf(pitchRad) * cosf(yawRad);

		camera.Position = cameraTarget + cameraOffset;
		camera.Front = glm::normalize(cameraTarget - camera.Position);
		camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
		camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		// Render the HDR background as a screen-space pass.
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		skyboxShader.use();
		skyboxShader.setMat4("projection", projection);
		skyboxShader.setMat4("inverseProjection", glm::inverse(projection));
		skyboxShader.setMat4("inverseView", glm::inverse(view));
		if (hdrTexture != 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, hdrTexture);
			skyboxShader.setInt("equirectangularMap", 0);
		}
		glBindVertexArray(skyboxVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		// don't forget to enable shader before setting uniforms
		ourShader.use();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		// draw walkable player area (filled quad + outlined border)
		walkAreaShader.use();
		walkAreaShader.setMat4("projection", projection);
		walkAreaShader.setMat4("view", view);
		glm::mat4 walkAreaModel = glm::mat4(1.0f);
		walkAreaShader.setMat4("model", walkAreaModel);
		walkAreaShader.setVec3("color", glm::vec3(0.12f, 0.18f, 0.16f));

		// Draw the walk area as a one-sided floor so its underside is never visible.
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		glBindVertexArray(walkAreaVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glm::mat4 walkAreaBorderModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.01f, 0.0f));
		walkAreaShader.setMat4("model", walkAreaBorderModel);
		walkAreaShader.setVec3("color", glm::vec3(0.25f, 0.95f, 0.65f));
		glLineWidth(2.0f);
		glDrawArrays(GL_LINE_LOOP, 0, 4);
		glBindVertexArray(0);
		glDisable(GL_CULL_FACE);

		// Switch back to the animated model shader after drawing the floor.
		ourShader.use();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);
		ourShader.setVec3("fallbackColor", glm::vec3(0.88f, 0.38f, 0.38f));

		auto transforms = animator.GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i)
			ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, characterPosition);
		model = glm::rotate(model, glm::radians(characterYaw), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(characterScale, characterScale, characterScale));
		ourShader.setMat4("model", model);
		ourModel->Draw(ourShader);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glDeleteVertexArrays(1, &walkAreaVAO);
	glDeleteBuffers(1, &walkAreaVBO);
	glDeleteBuffers(1, &walkAreaEBO);

	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteTextures(1, &hdrTexture);

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	static bool fWasDown = false;
	static bool spaceWasDown = false;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fWasDown)
		triggerTaunt = true;
	fWasDown = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceWasDown)
		triggerJump = true;
	spaceWasDown = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

	glm::vec3 planarForward = glm::vec3(camera.Front.x, 0.0f, camera.Front.z);
	if (glm::length(planarForward) < 0.0001f)
		planarForward = glm::vec3(0.0f, 0.0f, -1.0f);
	else
		planarForward = glm::normalize(planarForward);
	glm::vec3 planarRight = glm::normalize(glm::cross(planarForward, glm::vec3(0.0f, 1.0f, 0.0f)));

	glm::vec3 moveInput(0.0f, 0.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveInput += planarForward;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveInput -= planarForward;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		moveInput -= planarRight;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		moveInput += planarRight;

	if (currentState != AnimationState::Action && currentState != AnimationState::Taunt)
	{
		if (glm::length(moveInput) > 0.0f)
		{
			glm::vec3 moveDir = glm::normalize(moveInput);
			bool running = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
			float speed = running ? runSpeed : walkSpeed;
			currentGroundVelocity = moveDir * speed;

			characterPosition += moveDir * speed * deltaTime;
			characterYaw = glm::degrees(atan2f(moveDir.x, moveDir.z));
			currentState = running ? AnimationState::Run : AnimationState::Walk;
		}
		else
		{
			currentGroundVelocity = glm::vec3(0.0f);
			currentState = AnimationState::Idle;
		}
	}

	// Keep the controller strictly on a flat XZ plane.
	characterPosition.y = groundY;
	characterPosition.x = glm::clamp(characterPosition.x, -planeHalfSize, planeHalfSize);
	characterPosition.z = glm::clamp(characterPosition.z, -planeHalfSize, planeHalfSize);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = ypos - lastY;

	lastX = xpos;
	lastY = ypos;

	const float orbitSensitivity = 0.18f;
	orbitYaw += xoffset * orbitSensitivity;
	orbitPitch += yoffset * orbitSensitivity;

	if (orbitPitch > 70.0f)
		orbitPitch = 70.0f;
	if (orbitPitch < -15.0f)
		orbitPitch = -15.0f;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	cameraDistance -= static_cast<float>(yoffset) * 0.4f;
	if (cameraDistance < minCameraDistance)
		cameraDistance = minCameraDistance;
	if (cameraDistance > maxCameraDistance)
		cameraDistance = maxCameraDistance;
}
