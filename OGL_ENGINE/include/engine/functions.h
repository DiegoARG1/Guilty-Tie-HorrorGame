#include <engine/utils.h>
#include <engine/variables.h>

//:::: CALLBACKS  Y FUNCIONES :::://
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void collidedObject_callback(string nameCollidedObject);
void collidedObject_callback(string nameCollidedObject, string nameCollidedObject2);

void joystick_callback(int jid, int event);
void processInput(GLFWwindow* window);
void actionKeys(GLFWwindow* window);

void setMultipleLight(Shader* shader, vector<glm::vec3> pointLightPositions);
void drawModels(Shader* shader, glm::mat4 view, glm::mat4 projection);
void loadEnviroment(Terrain* terrain, SkyBox* sky, glm::mat4 view, glm::mat4 projection);
void initScene(Shader ourShader);
void collisions();

// :::: CONTROL DE MANDO  ::::
void joystick_callback(int jid, int event) {}

// :::: TECLADO PRINCIPAL ::::
void processInput(GLFWwindow* window)
{
    // Cerrar Juego
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Movimiento WASD
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.MovementSpeed = 15.0f; // Velocidad de carrera
    else
        camera.MovementSpeed = 3.0f;  // Velocidad normal de caminata

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    float distanciaCabana = glm::distance(camera.Position, posicionEstructura);
    float distanciaCajuela = glm::distance(camera.Position, posicionAuto);
    float distToca = glm::distance(camera.Position, posicionTocadiscos);

    // Interaccion:(Tecla E)
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (!teclaEPulsada)
        {
            if (distanciaCabana < 5.0f)
            {
                abrirPuerta = !abrirPuerta;
                teclaEPulsada = true;
            }
            else if (distanciaCajuela < 5.0f)
            {
                abrirCajuela = !abrirCajuela;
                teclaEPulsada = true;
            }
            else if (distToca < 5.0f)
            {
                tocadiscosEncendido = !tocadiscosEncendido;
                teclaEPulsada = true;
            }
        }
    }
    else
    {
        teclaEPulsada = false;
    }

	//Linterna:(Tecla F)
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        if (!teclaFPulsada)
        {
            linternaEncendida = !linternaEncendida;
            teclaFPulsada = true;
        }
    }
    else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
    {
        teclaFPulsada = false;
    }

    // Salto
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        saltar = true;
}

// :::: EDITOR EN TIEMPO DE EJECUCION::::
void actionKeys(GLFWwindow* window) {}

// :::: RESOLUCIÓN DE PANTALLA ::::
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// :::: CÁMARA CON EL MOUSE ::::
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// :::: CLICS DEL MOUSE ::::
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {}

// :::: ZOOM ::::
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// :::: EVENTOS DE COLISIÓN ::::
void collidedObject_callback(string nameCollidedObject)
{

}

void collidedObject_callback(string nameCollidedObject, string nameCollidedObject2) {}