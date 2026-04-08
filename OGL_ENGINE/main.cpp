
#include <GLFW/glfw3.h>
#include <engine/Billboard.h>
#include <engine/CollisionBox.h>
#include <engine/Objectives.h>
#include <engine/Particles.h>
#include <engine/Plane.h>
#include <engine/QuadTexture.h>
#include <engine/RigidModel.h>
#include <engine/Terrain.h>
#include <engine/functions.h>
#include <engine/shader_m.h>
#include <engine/skybox.h>
#include <engine/textrenderer.h>
#include <engine/Audio.h>
#include <glad/glad.h>
#include <iostream>
#include <thread>

int main()
{
    //:::: INICIALIZAMOS GLFW CON LA VERSIÓN 3.3 :::://
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Cambiamos el nombre de la ventana a tu juego
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Guilty Tie", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetJoystickCallback(joystick_callback);

    // Oculta el mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader ourShader("shaders/multiple_lighting.vs", "shaders/multiple_lighting.fs");
    initScene(ourShader);

    // AQUÍ CARGAS TU MAPA DE ALTURAS DEL BOSQUE
    Terrain terrain("textures//terrenus_guilty2.png", texturePaths);
    SkyBox sky(1.0f, "6");

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = (currentFrame - lastFrame);
        lastFrame = currentFrame;

        processInput(window);

        // FONDO NEGRO PURO PARA EL TERROR
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        drawModels(&ourShader, view, projection);
        loadEnviroment(&terrain, &sky, view, projection);

        // Físicas básicas (Gravedad y salto)
        if (saltar) {
            if (posicion_y < (maxima_altura + posicion_suelo)) posicion_y += 0.1f;
            else saltar = false;
        }
        else {
            if (posicion_y > posicion_suelo) posicion_y -= 0.1f;
        }
        // :::: 1. LÍMITES DEL MAPA (Muros invisibles para no caer al vacío) ::::
        // El mapa mide 100x100 (de -50 a 50). Te encerramos en 49 para estar seguros.
        if (camera.PosPersonaje.x > 49.0f) camera.PosPersonaje.x = 49.0f;
        if (camera.PosPersonaje.x < -49.0f) camera.PosPersonaje.x = -49.0f;
        if (camera.PosPersonaje.z > 49.0f) camera.PosPersonaje.z = 49.0f;
        if (camera.PosPersonaje.z < -49.0f) camera.PosPersonaje.z = -49.0f;
        // Cámara estricta de 1ra persona
        camera.Position.x = camera.PosPersonaje.x;
        camera.Position.z = camera.PosPersonaje.z;

        // 1. Calculamos dónde DEBERÍA estar el suelo
        float altura_matematica = terrain.Superficie(camera.Position.x, camera.Position.z);
        float altura_objetivo = (altura_matematica * 300.0f); // Aplica tu escala aquí

        // 2. LAS RODILLAS VIRTUALES (Lerp)
        // En lugar de igualar la altura de golpe, nos acercamos un porcentaje en cada frame.
        // Si subes el 10.0f, la cámara se ajusta más rápido (más duro). Si lo bajas, es más flotante.
        float velocidad_suavizado = 5.0f * deltaTime;
        camera.PosPersonaje.y = camera.PosPersonaje.y + (altura_objetivo - camera.PosPersonaje.y) * velocidad_suavizado;

        // 3. Colocamos los ojos 1.8 metros por encima del cuerpo ya suavizado
        camera.Position.y = camera.PosPersonaje.y + 1.8f;

        collisions();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete[] texturePaths;
    sky.Release();
    terrain.Release();
    glfwTerminate();

    return 0;
}

void initScene(Shader ourShader)
{
    // 1. POSICIÓN INICIAL DEL JUGADOR
    camera.Position = glm::vec3(23.0f, 1.8f, 29.0f);
	camera.PosPersonaje = camera.Position;

    // 2. TEXTURAS DEL SUELO (Cambia estas por tu tierra o lodo)
    texturePaths = new const char* [4];
    texturePaths[0] = "textures/multitexture_colors.jpg";
    texturePaths[1] = "textures/TexturaP1.jpg";
    texturePaths[2] = "textures/TexturaP2.png";
    texturePaths[3] = "textures/TexturaP3.jpg";

    // 3. LUCES DEL MAPA (Ahorita las dejamos así para que veas, luego las apagamos)
    pointLightPositions.push_back(glm::vec3(20.3f, 5.2f, 20.0f));
    pointLightPositions.push_back(glm::vec3(20.3f, 2.0f, 30.0f));
    pointLightPositions.push_back(glm::vec3(1.0f, 9.3f, -7.0f));
    pointLightPositions.push_back(glm::vec3(0.0f, 10.0f, -3.0f));

    models.push_back(Model("Linterna", "models/Linterna/Flashlight.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 0.05f));

    models.push_back(Model("Cabana", "models/Cabana/Cabana.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));
    models.push_back(Model("Puerta", "models/Cabana/Puerta.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));
    models.push_back(Model("Pino1", "models/Pinos/Pino1.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));
    models.push_back(Model("Pino2", "models/Pinos/Pino2.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));
    models.push_back(Model("Pino3", "models/Pinos/Pino3.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));

    glEnable(GL_DEPTH_TEST);
    camera.setCollBox();
    ourShader.use();
}

void loadEnviroment(Terrain* terrain, SkyBox* sky, glm::mat4 view, glm::mat4 projection)
{
    // 1. PRIMERO aplicamos luces y configuraciones al terreno
    terrain->getShader()->use();
    setMultipleLight(terrain->getShader(), pointLightPositions);
    terrain->getShader()->setFloat("shininess", 10.0f);

    // 2. LUEGO lo dibujamos
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0, -2.5f, 0.0f));
    model = glm::scale(model, glm::vec3(100.5f, 300.0f, 100.5f));
    terrain->draw(model, view, projection);

    // 3. PRIMERO aplicamos luces al cielo
    sky->getShader()->use();
    setMultipleLight(sky->getShader(), pointLightPositions);
    sky->getShader()->setFloat("shininess", 10.0f);

    // 4. LUEGO lo dibujamos
    glm::mat4 skyModel = glm::mat4(1.0f);
    skyModel = glm::translate(skyModel, glm::vec3(0.0f, 0.0f, 0.0f));
    skyModel = glm::scale(skyModel, glm::vec3(200.0f, 200.0f, 200.0f));
    sky->draw(skyModel, view, projection, skyPos);
}

void drawModels(Shader* shader, glm::mat4 view, glm::mat4 projection)
{
    shader->setFloat("material.shininess", 10.0f);
    setMultipleLight(shader, pointLightPositions);

    // :::: DIBUJAR LINTERNA EN LA MANO ::::
    glm::mat4 modelLinterna = glm::mat4(1.0f);

    // 1. Pegada a la cámara
    modelLinterna = glm::translate(modelLinterna, camera.Position);

    // 2. Rotación sincronizada
    modelLinterna = glm::rotate(modelLinterna, glm::radians(-camera.Yaw - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLinterna = glm::rotate(modelLinterna, glm::radians(camera.Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    modelLinterna = glm::rotate(modelLinterna, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    // 3. POSICIÓN DE PRUEBA: Justo en el centro de tu pantalla para que no se escape
    modelLinterna = glm::translate(modelLinterna, glm::vec3(0.20f, -0.2f, 0.5f));

    // 4. Escala media (0.05f) para asegurarnos de verla
    modelLinterna = glm::scale(modelLinterna, glm::vec3(0.07f));

    if (!models.empty()) {
        // TRUCO DE LUZ SUTIL: Apenas lo suficiente para que el metal se vea entre las sombras
        shader->setVec3("dirLights[0].ambient", 0.1f, 0.1f, 0.15f);
        shader->setVec3("dirLights[0].diffuse", 0.0f, 0.0f, 0.0f);

        models[0].Draw(*shader, modelLinterna);

        // APAGAMOS EL TRUCO: Devolvemos los valores exactos de la noche para el terreno
        shader->setVec3("dirLights[0].ambient", 0.03f, 0.03f, 0.05f);
        shader->setVec3("dirLights[0].diffuse", 0.02f, 0.02f, 0.03f);
    }
    // :::: GENERADOR DEL BOSQUE TÉTRICO ::::
    for (int i = 0; i < posicionesBosque.size(); i++)
    {
        glm::mat4 modelPino = glm::mat4(1.0f);
        modelPino = glm::translate(modelPino, posicionesBosque[i]);

        // TRUCO 1: Variedad de modelos. 
        // Usamos la operación módulo (%) para alternar entre el índice 3, 4 y 5.
        // Así el árbol 0 será Pino1, el árbol 1 será Pino2, el árbol 2 será Pino3, y se repite.
        int tipoPino = 3 + (i % 3);

        // TRUCO 2: Escala pseudo-aleatoria. 
        // Hace que algunos pinos sean gigantes y otros pequeños para que no se vean clonados.
        float escalaPino = 2.0f + ((i % 5) * 0.4f);
        modelPino = glm::scale(modelPino, glm::vec3(escalaPino));

        // TRUCO 3: Rotación pseudo-aleatoria.
        // Gira cada pino un ángulo distinto para que sus ramas apunten a lugares diferentes.
        modelPino = glm::rotate(modelPino, glm::radians(i * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        if (models.size() > tipoPino) {
            models[tipoPino].Draw(*shader, modelPino);
        }
    }

    // :::: ATERRIZAJE DE LA ESTRUCTURA ::::

    float gradosRotacion = 180.0f;

    // DIBUJAR LA CABAÑA
    glm::mat4 modelCabana = glm::mat4(1.0f);
    modelCabana = glm::translate(modelCabana, posicionEstructura);

    modelCabana = glm::rotate(modelCabana, glm::radians(gradosRotacion), glm::vec3(0.0f, 1.0f, 0.0f));

    // Subimos la escala a 3.0f por si 1.0f era muy pequeña
    modelCabana = glm::scale(modelCabana, glm::vec3(1.0f));

    if (models.size() > 1) {
        models[1].Draw(*shader, modelCabana);
    }

    // :::: ANIMACIÓN DE LA PUERTA ::::
    if (abrirPuerta && anguloPuerta < 90.0f) {
        anguloPuerta += 45.0f * deltaTime; // 45.0f es la velocidad a la que se abre
    }

    // :::: DIBUJAR LA PUERTA (models[2]) ::::
    glm::mat4 modelPuerta = glm::mat4(1.0f);
    modelPuerta = glm::translate(modelPuerta, posicionEstructura);

    // ATENCIÓN AQUÍ: Sumamos los grados base (gradosRotacion) + la animación (anguloPuerta)
    modelPuerta = glm::rotate(modelPuerta, glm::radians(gradosRotacion + anguloPuerta), glm::vec3(0.0f, 1.0f, 0.0f));

    modelPuerta = glm::scale(modelPuerta, glm::vec3(1.0f)); // Recuerda usar la misma escala que le pusiste a la cabaña

    if (models.size() > 2) {
        models[2].Draw(*shader, modelPuerta);
    }
}

void setMultipleLight(Shader* shader, vector<glm::vec3> pointLightPositions)
{
    shader->setVec3("viewPos", camera.Position);

    // 1. LUZ DE LUNA (Regresamos al terror de tu foto original)
    shader->setVec3("dirLights[0].direction", glm::vec3(-0.2f, -1.0f, -0.3f));
    shader->setVec3("dirLights[0].ambient", 0.03f, 0.03f, 0.05f); // Azul muy, muy oscuro
    shader->setVec3("dirLights[0].diffuse", 0.02f, 0.02f, 0.03f); // Luz directa casi nula
    shader->setVec3("dirLights[0].specular", 0.0f, 0.0f, 0.0f);
    // :::: APAGADO ABSOLUTO DE LUCES EXTRAS (Evita la luz fantasma) ::::
    for (int i = 1; i < 4; i++) {
        string num = std::to_string(i);

        // Limpiar DirLights
        shader->setVec3("dirLights[" + num + "].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        shader->setVec3("dirLights[" + num + "].ambient", 0.0f, 0.0f, 0.0f); // <-- CULPABLE REPARADO
        shader->setVec3("dirLights[" + num + "].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("dirLights[" + num + "].specular", 0.0f, 0.0f, 0.0f);

        // Limpiar PointLights
        shader->setVec3("pointLights[" + num + "].position", glm::vec3(0.0f, -10.0f, 0.0f));
        shader->setVec3("pointLights[" + num + "].ambient", 0.0f, 0.0f, 0.0f); // <-- CULPABLE REPARADO
        shader->setVec3("pointLights[" + num + "].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("pointLights[" + num + "].specular", 0.0f, 0.0f, 0.0f);
        shader->setFloat("pointLights[" + num + "].constant", 1.0f);

        // Limpiar SpotLights
        shader->setVec3("spotLights[" + num + "].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        shader->setVec3("spotLights[" + num + "].position", glm::vec3(0.0f, -10.0f, 0.0f));
        shader->setVec3("spotLights[" + num + "].ambient", 0.0f, 0.0f, 0.0f); // <-- CULPABLE REPARADO
        shader->setVec3("spotLights[" + num + "].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLights[" + num + "].specular", 0.0f, 0.0f, 0.0f);
        shader->setFloat("spotLights[" + num + "].constant", 1.0f);
    }

    // 2. TU LINTERNA (Control On/Off seguro)
    if (linternaEncendida)
    {
        // TRUCO: Un puntito de luz justo en la punta de la linterna para que parezca encendida
        // Calculamos la punta (un poco más adelante que el origen del spotLight)
        glm::vec3 puntaLinterna = camera.Position + (camera.Right * 0.45f) + (camera.Up * -0.35f) + (camera.Front * 0.2f);

        shader->setVec3("pointLights[0].position", puntaLinterna);
        shader->setVec3("pointLights[0].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("pointLights[0].diffuse", 1.0f, 1.0f, 0.8f); // Tono cálido
        shader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);

        // Estos valores súper altos hacen que la luz "muera" rapidísimo (no iluminará el piso, solo la linterna)
        shader->setFloat("pointLights[0].constant", 1.0f);
        shader->setFloat("pointLights[0].linear", 2.0f);
        shader->setFloat("pointLights[0].quadratic", 5.0f);

        // Tu linterna (SpotLight) se queda igualita...
        shader->setVec3("spotLights[0].position", camera.Position + (camera.Right * 0.45f) + (camera.Up * -0.35f));
        shader->setVec3("spotLights[0].direction", camera.Front);
        shader->setVec3("spotLights[0].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLights[0].diffuse", 3.0f, 3.0f, 2.5f); // Tono cálido realista
        shader->setVec3("spotLights[0].specular", 1.0f, 1.0f, 1.0f);
        shader->setFloat("spotLights[0].constant", 1.0f);
        shader->setFloat("spotLights[0].linear", 0.02f);
        shader->setFloat("spotLights[0].quadratic", 0.001f);
        shader->setFloat("spotLights[0].cutOff", glm::cos(glm::radians(10.0f)));
        shader->setFloat("spotLights[0].outerCutOff", glm::cos(glm::radians(15.0f)));
    }
    else
    {
        // TODO APAGADO (Aseguramos que nada brille en la oscuridad)
        shader->setVec3("pointLights[0].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("pointLights[0].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("pointLights[0].specular", 0.0f, 0.0f, 0.0f);
        shader->setFloat("pointLights[0].constant", 1.0f);

        shader->setVec3("spotLights[0].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        shader->setVec3("spotLights[0].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLights[0].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLights[0].specular", 0.0f, 0.0f, 0.0f);
        shader->setFloat("spotLights[0].constant", 1.0f);
    }

    shader->setInt("lightType", 1);
    shader->setInt("maxRenderLights", 4);
}

void collisions()
{
    //Detecta las colisiones de las cajas individuales
    //TODO LO DE LAS COLISIONES VA AQUÍ
    detectColls(collboxes, &camera, renderCollBox, collidedObject_callback);
}