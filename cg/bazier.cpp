#include<glad/glad.h>
#include<GLFW\glfw3.h>
#include<iostream>
#include<vector>
#include<algorithm>
#include<assert.h>
#include"shader.h"
#define sqr(x) (x)*(x)
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char* vertexShaderSource = "./shader.vs";
const char* fragmentShaderSource = "./shader.fs";


int mouseCount, CurveFixed, PointMoved;
float vertices[100010];

float transX(float x) {
	return (float)((float)(2 * x) - SCR_WIDTH) / (SCR_WIDTH * 1.0f);
}

float transY(float x) {
	return (float)(SCR_HEIGHT - (float)(2 * x)) / (SCR_HEIGHT * 1.0f);
}


struct point {
	float x, y;
	point(float _x = 0, float _y = 0) : x(_x), y(_y) {}
	void set(float _x, float _y) {
		x = _x, y = _y;
	}
	point operator -(point rhs) { return point(x - rhs.x, y - rhs.y); }
	point operator +(point rhs) { return point(x + rhs.x, y + rhs.y); }
	point operator *(float cof) { return point(x * cof, y * cof); }
	void push(std::vector<float>& vec) {
		vec.push_back(transX(x));
		vec.push_back(transY(y));
		vec.push_back(0.0);
	}
};

inline int dis(float xpos0, float ypos0, float xpos1, float ypos1) {
	return sqr(xpos0 - xpos1) + sqr(ypos0 - ypos1);
}

int judgeCursorPos(float xpos0, float ypos0, float xpos1, float ypos1) {
	return (sqr(xpos0 - xpos1) + sqr(ypos0 - ypos1) <= 25.0);
}
int judgeCursorPos(point a, point b) {
	float xpos0 = a.x, xpos1 = b.x, ypos0 = a.y, ypos1 = b.y;
	return (sqr(xpos0 - xpos1) + sqr(ypos0 - ypos1) <= 25.0);
}

point dividedPoint(point a, point b, double f) {
	return a + (b - a) * f;
}

class Bazier {
private:
	std::vector<point> org;
	std::vector<point> process[100];
	std::vector<point> res;
	std::vector<float> pointsOrg, pointsProcess, pointsRes;
	point* ModifyingPoint;

public:
	void generate(float t) {
		process[0].clear();
		process[0].assign(org.begin(), org.end());
		for (int i = 1; i < org.size(); ++i) {
			process[i].clear();
			for (int j = 1; j < process[i - 1].size(); ++j)
				process[i].push_back(dividedPoint(process[i - 1][j - 1], process[i - 1][j], t));
		}
		res.push_back(process[org.size() - 1].back());
	}


	void print() {
		std::cout << "org: " << org.size() << ", res: " << res.size() << std::endl;
		std::cout << "pointsOrg: " << pointsOrg.size() << ", pointsProcess: "
			<< pointsProcess.size() << ", pointsRes: " << pointsRes.size() << std::endl;
	}

	void render(Shader& ourShader, unsigned int& VAO, unsigned int& VBO, int renderProcess) {
		print();
		pointsOrg.clear();
		pointsProcess.clear();
		pointsRes.clear();
		for (int i = 1; i < org.size(); ++i) {
			org[i - 1].push(pointsOrg);
			org[i].push(pointsOrg);
		}
		int n = org.size() - 1;
		std::cout << n << std::endl;
		for (int i = 1; i < n; ++i) {
			//std::cout << "org.size() - 1: " << org.size() - 1 << ", i: " << i << std::endl;
			for (int j = 1; j < process[i].size(); ++j) {
				process[i][j - 1].push(pointsProcess);
				process[i][j].push(pointsProcess);
			}
		}

		for (int i = 1; i < res.size(); ++i) {
			res[i - 1].push(pointsRes);
			res[i].push(pointsRes);
		}
		print();
		ourShader.setVec4("ourColor", 1.0f, 1.0f, 1.0f, 1.0f);// Draw Org Points
		ourShader.use();
		GLfloat* temp = pointsOrg.data();
		int size = pointsOrg.size();
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(GLfloat), temp);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, size / 3);
		// render Process
		if (renderProcess) {
			ourShader.setVec4("ourColor", 1.0f, 0.4f, 0.3f, 1.0f);
			ourShader.use();
			temp = pointsProcess.data();
			size = pointsProcess.size();
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(GLfloat), temp);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(VAO);
			glDrawArrays(GL_LINES, 0, size / 3);
		}
		//render Result
		ourShader.setVec4("ourColor", 0.0f, 1.0f, 0.0f, 1.0f);
		ourShader.use();
		temp = pointsRes.data();
		size = pointsRes.size();
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(GLfloat), temp);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, size / 3);
	}

	void clear() {
		ModifyingPoint = nullptr;
		for (int i = 0; i < org.size(); ++i)
			process[i].clear();
		org.clear();
		res.clear();
		CurveFixed = 0;
	}

	void clearModifyingPoint() {
		ModifyingPoint = nullptr;
	}

	int findPoint(point P) {
		for (auto& i : org) {
			if (judgeCursorPos(P, i)) {
				ModifyingPoint = &i;
				return 1;
			}
		}
		return 0;
	}

	void pointInput(point P) {
		if (org.size() == 0) org.push_back(P);
		org.push_back(P);
		ModifyingPoint = &org[org.size() - 1];
	}

	void updateModifyingPoint(point P) {
		if (ModifyingPoint == nullptr) return;
		ModifyingPoint->set(P.x, P.y);
	}

	void inputEnded() {
		if (!CurveFixed) org.pop_back();
		CurveFixed = 1;
	}

	void backToOrg() {
		res.clear();
	}

	void reinsert() {
		pointInput(org.back());
	}
}BazierCurve;

float T;
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		T = 0.0;
		BazierCurve.backToOrg();
		BazierCurve.inputEnded();
		BazierCurve.clearModifyingPoint();
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		BazierCurve.clear();
	}
	else if (key == GLFW_KEY_I && action == GLFW_PRESS) {
		CurveFixed = 0;
		BazierCurve.backToOrg();
		BazierCurve.reinsert();
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		float pos[2];
		glfwGetCursorPos(window, &xpos, &ypos);
		pos[0] = (float)xpos, pos[1] = (float)ypos;
		if (!CurveFixed) BazierCurve.pointInput(point(pos[0], pos[1]));
		else {
			if (PointMoved == 0) {
				if (BazierCurve.findPoint(point(pos[0], pos[1]))) {
					BazierCurve.backToOrg();
					PointMoved = 1;
				}
			}
			else {
				BazierCurve.clearModifyingPoint();
				PointMoved = 0;
			}
		}
	}
}

void processInput(GLFWwindow* window) {
	float pos[2];
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	pos[0] = (float)xpos, pos[1] = (float)ypos;
	BazierCurve.updateModifyingPoint(point(pos[0], pos[1]));
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	int gladErr = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); //Starts up GLAD
	if (!gladErr) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //感知窗口回调函数
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	Shader ourShader(vertexShaderSource, fragmentShaderSource);

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	T = 1.0f;
	while (!glfwWindowShouldClose(window)) {

		glClearColor(0.05f, 0.0f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);
		if (T < 1.0 && CurveFixed) {
			T += 0.005;
			BazierCurve.generate(T);
		}
		BazierCurve.render(ourShader, VAO, VBO, T < 1);
		//Polygon.printSize();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}