
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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader ourShader("shaders/multiple_lighting.vs", "shaders/multiple_lighting.fs");
    initScene(ourShader);

    Terrain terrain("textures//terrenus_guilty2.png", texturePaths);
    SkyBox sky(1.0f, "6");

    // :::: NUEVO: INICIALIZAR EL HUD DE TEXTO ::::
    // Se le pasan las medidas de tu pantalla que tienes en tu variables.h
    TextRenderer Text(SCR_WIDTH, SCR_HEIGHT);
    Text.Load("fonts/fuente.ttf", 24); // Asegúrate de que el nombre coincida con tu archivo

    // :::: NUEVO: PEGAR OBJETOS ALEATORIOS AL SUELO ::::
    for (int i = 0; i < listaBaterias.size(); i++) {
        glm::vec3 posActual = listaBaterias[i].getPosicion();

        // Calculamos la matemática y le RESTAMOS los 2.5m del desfase visual del mapa
        float alturaReal = (terrain.Superficie(posActual.x, posActual.z) * 300.0f) - 2.5f;

        // Ahora sí, lo acomodamos para que descanse sobre el lodo
        posActual.y = alturaReal + 0.2f;

        listaBaterias[i].setPosicion(posActual);
    }

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = (currentFrame - lastFrame);
        lastFrame = currentFrame;

        processInput(window);

        //std::cout << "X: " << camera.Position.x << " Y: " << camera.Position.y << " Z: " << camera.Position.z << std::endl;

        // FONDO NEGRO
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        drawModels(&ourShader, view, projection);
        loadEnviroment(&terrain, &sky, view, projection);

        // Fisicas basicas (Gravedad y salto)
        if (saltar) {
            if (posicion_y < (maxima_altura + posicion_suelo)) posicion_y += 0.1f;
            else saltar = false;
        }
        else {
            if (posicion_y > posicion_suelo) posicion_y -= 0.1f;
        }
        // ::::LIMITES DEL MAPA::::
        if (camera.PosPersonaje.x > 49.0f) camera.PosPersonaje.x = 49.0f;
        if (camera.PosPersonaje.x < -49.0f) camera.PosPersonaje.x = -49.0f;
        if (camera.PosPersonaje.z > 49.0f) camera.PosPersonaje.z = 49.0f;
        if (camera.PosPersonaje.z < -49.0f) camera.PosPersonaje.z = -49.0f;
        // Camara estricta de 1ra persona
        camera.Position.x = camera.PosPersonaje.x;
        camera.Position.z = camera.PosPersonaje.z;

        //Calculamos donde esta el suelo
        float altura_matematica = terrain.Superficie(camera.Position.x, camera.Position.z);
        float altura_objetivo = (altura_matematica * 300.0f);

        // Ajuste lento
        // En lugar de igualar la altura de golpe, nos acercamos un porcentaje en cada frame
        float velocidad_suavizado = 5.0f * deltaTime;
        camera.PosPersonaje.y = camera.PosPersonaje.y + (altura_objetivo - camera.PosPersonaje.y) * velocidad_suavizado;

        //Colocamos los ojos 1.8m
        camera.Position.y = camera.PosPersonaje.y + 1.8f;

        // :::: MECÁNICA DE SUPERVIVENCIA: DRENAJE DE BATERÍA ::::
        if (linternaEncendida) {
            // Se gasta 2% por cada segundo real (gracias al deltaTime)
            bateriaLinterna -= 0.5f * deltaTime;

            // Si se acaba, la apagamos a la fuerza
            if (bateriaLinterna <= 0.0f) {
                bateriaLinterna = 0.0f;
                linternaEncendida = false;
            }
        }

        collisions();

        // :::: DIBUJAR HUD EN PANTALLA (BARRA RETRO) ::::
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        int numRayitas = (int)((bateriaLinterna / 100.0f) * 20.0f);
        std::string barraVisual = "[";
        for (int i = 0; i < 20; i++) {
            if (i < numRayitas) barraVisual += "|";
            else barraVisual += " ";
        }
        barraVisual += "]";

        std::string textoBateria = "Bateria " + barraVisual + " " + std::to_string((int)bateriaLinterna) + "%";

        // Lo dibujamos en la esquina superior izquierda
        Text.RenderText(textoBateria, 25.0f, 25.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

        glEnable(GL_DEPTH_TEST);

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
    //POSICION INICIAL DEL JUGADOR
    camera.Position = glm::vec3(23.0f, 1.8f, 29.0f);
	camera.PosPersonaje = camera.Position;

    //TEXTURAS DEL SUELO
    texturePaths = new const char* [4];
    texturePaths[0] = "textures/multitexturaGT.jpg";
    texturePaths[1] = "textures/Lodo2.jpg";
    texturePaths[2] = "textures/Bosque.jpg";
    texturePaths[3] = "textures/Grava.jpg";

    //LUCES DEL MAPA
    pointLightPositions.push_back(glm::vec3(20.3f, 5.2f, 20.0f));
    pointLightPositions.push_back(glm::vec3(20.3f, 2.0f, 30.0f));
    pointLightPositions.push_back(glm::vec3(1.0f, 9.3f, -7.0f));
    pointLightPositions.push_back(glm::vec3(0.0f, 10.0f, -3.0f));

    models.push_back(Model("Linterna", "models/Linterna/Flashlight.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 0.05f));//1
    models.push_back(Model("Cabana", "models/Cabana/Cabana.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//2
    models.push_back(Model("Puerta", "models/Cabana/Puerta.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//3
    models.push_back(Model("Pino1", "models/Pinos/Pino1.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//4
    models.push_back(Model("Pino2", "models/Pinos/Pino2.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//5
    models.push_back(Model("Pino3", "models/Pinos/Pino3.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//6
    models.push_back(Model("Auto", "models/Carro/Carro.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//7
	models.push_back(Model("Cajuela", "models/Carro/Cajuela.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//8
    models.push_back(Model("Toca_Base", "models/TocaDiscos/Toca_Base.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//9
    models.push_back(Model("Toca_Disco", "models/TocaDiscos/Toca_Disco.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//10
    models.push_back(Model("Cartel", "models/Cartel/Missing_Poster.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//11
    models.push_back(Model("Bateria", "models/Bateria/Bateria.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//12
    models.push_back(Model("Mesa", "models/Mesa/Mesa.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//13
    models.push_back(Model("Banca", "models/Banca/Banca.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//14
    models.push_back(Model("Saco", "models/SacoDormir/SacoDormir.obj", glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 1.0f));//15

    // :::: GENERADOR DE BATERÍAS ALEATORIAS ::::
    // Le damos una "semilla" basada en el reloj de tu PC para que siempre sea distinto
    srand(static_cast<unsigned int>(time(0)));

    for (int i = 0; i < 5; i++) {
        // Genera coordenadas X y Z aleatorias entre -40 y 40
        float randX = -40.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 80.0f));
        float randZ = -40.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 80.0f));
        glm::vec3 posAleatoria = glm::vec3(randX, 18.0f, randZ);

        // Usamos la clase BateriaRecargable que creamos
        listaBaterias.push_back(BateriaRecargable("Bateria_" + std::to_string(i), posAleatoria, 25.0f));
    }

    glEnable(GL_DEPTH_TEST);
    camera.setCollBox();
    ourShader.use();
}

void loadEnviroment(Terrain* terrain, SkyBox* sky, glm::mat4 view, glm::mat4 projection)
{
    //luces y configuraciones de terreno
    terrain->getShader()->use();
    setMultipleLight(terrain->getShader(), pointLightPositions);
    terrain->getShader()->setFloat("shininess", 10.0f);

    //dibujar terreno
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0, -2.5f, 0.0f));
    model = glm::scale(model, glm::vec3(100.5f, 300.0f, 100.5f));
    terrain->draw(model, view, projection);

    //luces cielo
    sky->getShader()->use();
    setMultipleLight(sky->getShader(), pointLightPositions);
    sky->getShader()->setFloat("shininess", 10.0f);

    //dibujar cielo
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

    modelLinterna = glm::translate(modelLinterna, camera.Position);
    modelLinterna = glm::rotate(modelLinterna, glm::radians(-camera.Yaw - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLinterna = glm::rotate(modelLinterna, glm::radians(camera.Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    modelLinterna = glm::rotate(modelLinterna, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLinterna = glm::translate(modelLinterna, glm::vec3(0.20f, -0.2f, 0.5f));
    modelLinterna = glm::scale(modelLinterna, glm::vec3(0.07f));

    if (!models.empty()) {
        shader->setVec3("dirLights[0].ambient", 0.1f, 0.1f, 0.15f);
        shader->setVec3("dirLights[0].diffuse", 0.0f, 0.0f, 0.0f);
        models[0].Draw(*shader, modelLinterna);
        shader->setVec3("dirLights[0].ambient", 0.03f, 0.03f, 0.05f);
        shader->setVec3("dirLights[0].diffuse", 0.02f, 0.02f, 0.03f);
    }
    // :::: GENERADOR DEL BOSQUE ::::
    for (int i = 0; i < posicionesBosque.size(); i++)
    {
        glm::mat4 modelPino = glm::mat4(1.0f);
        modelPino = glm::translate(modelPino, posicionesBosque[i]);

        //Variedad de modelos
        int tipoPino = 3 + (i % 3);

        //Escala pseudo-aleatoria
        // Hace que algunos pinos sean gigantes y otros pequeños para que no se vean clonados
        float escalaPino = 2.0f + ((i % 5) * 0.4f);
        modelPino = glm::scale(modelPino, glm::vec3(escalaPino));

        //Rotacion pseudo-aleatoria.
        //Gira cada pino un angulo distinto para que sus ramas apunten a lugares diferentes
        modelPino = glm::rotate(modelPino, glm::radians(i * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        if (models.size() > tipoPino) {
            models[tipoPino].Draw(*shader, modelPino);
        }
    }

    // :::: ATERRIZAJE DE LA ESTRUCTURA ::::

    float gradosRotacion = 180.0f;

    //CABAÑA
    glm::mat4 modelCabana = glm::mat4(1.0f);
    modelCabana = glm::translate(modelCabana, posicionEstructura);
    modelCabana = glm::rotate(modelCabana, glm::radians(gradosRotacion), glm::vec3(0.0f, 1.0f, 0.0f));
    modelCabana = glm::scale(modelCabana, glm::vec3(1.0f));

    if (models.size() > 1) {
        models[1].Draw(*shader, modelCabana);
    }

    // :::: ANIMACION DE LA PUERTA ::::
    if (abrirPuerta && anguloPuerta < 90.0f) {
        anguloPuerta += 45.0f * deltaTime;
    }

    // :::: PUERTA ::::
    glm::mat4 modelPuerta = glm::mat4(1.0f);
    modelPuerta = glm::translate(modelPuerta, posicionEstructura);

    modelPuerta = glm::rotate(modelPuerta, glm::radians(gradosRotacion + anguloPuerta), glm::vec3(0.0f, 1.0f, 0.0f));

    modelPuerta = glm::scale(modelPuerta, glm::vec3(1.0f));

    if (models.size() > 2) {
        models[2].Draw(*shader, modelPuerta);
    }

    float orientacionAuto = 280.0f;
    //Auto
    glm::mat4 modelAuto = glm::mat4(1.0f);
    modelAuto = glm::translate(modelAuto, posicionAuto);
    modelAuto = glm::rotate(modelAuto, glm::radians(orientacionAuto), glm::vec3(0.0f, 1.0f, 0.0f));
    modelAuto = glm::scale(modelAuto, glm::vec3(3.1f));

    if (models.size() > 6) {
        models[6].Draw(*shader, modelAuto);
    }

    //ANIMACION DE LA CAJUELA
    if (abrirCajuela && anguloCajuela < 60.0f) {
        anguloCajuela += 45.0f * deltaTime;
    }
    else if (!abrirCajuela && anguloCajuela > 0.0f) {
        anguloCajuela -= 45.0f * deltaTime;
    }

    if (anguloCajuela > 60.0f) anguloCajuela = 60.0f;
    if (anguloCajuela < 0.0f) anguloCajuela = 0.0f;

    //CAJUELA
    glm::mat4 modelCajuela = glm::mat4(1.0f);
    modelCajuela = glm::translate(modelCajuela, posicionAuto);
    modelCajuela = glm::rotate(modelCajuela, glm::radians(orientacionAuto), glm::vec3(0.0f, 1.0f, 0.0f));

    modelCajuela = glm::rotate(modelCajuela, glm::radians(-anguloCajuela), glm::vec3(0.0f, 0.0f, 1.0f));
    modelCajuela = glm::scale(modelCajuela, glm::vec3(3.1f));

    if (models.size() > 7) {
        models[7].Draw(*shader, modelCajuela);
    }

    // :::: LOGICA DEL DISCO ::::
    if (tocadiscosEncendido) {
        if (velocidadDisco < 200.0f) velocidadDisco += 50.0f * deltaTime; //Acelera
    }
    else {
        if (velocidadDisco > 0.0f) velocidadDisco -= 30.0f * deltaTime; //Desacelera por friccion
        if (velocidadDisco < 0.0f) velocidadDisco = 0.0f;
    }
    anguloDisco += velocidadDisco * deltaTime;

	//Base tocadiscos
    glm::mat4 modelBase = glm::mat4(1.0f);
    modelBase = glm::translate(modelBase, posicionTocadiscos);
    modelBase = glm::scale(modelBase, glm::vec3(0.5f));
    if (models.size() > 8) models[8].Draw(*shader, modelBase);

    //Disco
    glm::mat4 modelDisco = glm::mat4(1.0f);
    modelDisco = glm::translate(modelDisco, posicionTocadiscos);
    modelDisco = glm::rotate(modelDisco, glm::radians(anguloDisco), glm::vec3(0.0f, 1.0f, 0.0f));
    modelDisco = glm::scale(modelDisco, glm::vec3(0.5f));
    if (models.size() > 9) models[9].Draw(*shader, modelDisco);

    // :::: MUEBLES :::
    //MESA
    glm::mat4 modelMesa = glm::mat4(1.0f);
    modelMesa = glm::translate(modelMesa, posicionMesa);
    modelMesa = glm::scale(modelMesa, glm::vec3(0.5f)); // Ajusta si es muy grande/pequeña
    if (models.size() > 12) models[12].Draw(*shader, modelMesa);

    //BANCA
    glm::mat4 modelBanca = glm::mat4(1.0f);
    modelBanca = glm::translate(modelBanca, posicionBanca);
    modelBanca = glm::rotate(modelBanca, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelBanca = glm::scale(modelBanca, glm::vec3(3.0f));
    if (models.size() > 13) models[13].Draw(*shader, modelBanca);

    //SACO DE DORMIR
    glm::mat4 modelSaco = glm::mat4(1.0f);
    modelSaco = glm::translate(modelSaco, posicionSaco);
    modelSaco = glm::rotate(modelSaco, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelSaco = glm::scale(modelSaco, glm::vec3(0.2f));
    if (models.size() > 14) models[14].Draw(*shader, modelSaco);

    // :::: OBJETOS DENTRO DE LA CAJUELA ::::

    //CARTEL "MISSING"
    glm::mat4 modelCartel = glm::mat4(1.0f);
    modelCartel = glm::translate(modelCartel, posicionCartel);
    modelCartel = glm::rotate(modelCartel, glm::radians(280.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelCartel = glm::rotate(modelCartel, glm::radians(5.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    modelCartel = glm::scale(modelCartel, glm::vec3(0.5f));

    if (models.size() > 10) models[10].Draw(*shader, modelCartel);

    //BATERÍA
    glm::mat4 modelBateria = glm::mat4(1.0f);
    modelBateria = glm::translate(modelBateria, posicionBateria);
    modelBateria = glm::rotate(modelBateria, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    modelBateria = glm::scale(modelBateria, glm::vec3(7.0f));

    if (models.size() > 11) models[11].Draw(*shader, modelBateria);

    // :::: DIBUJAR BATERÍAS ALEATORIAS DEL BOSQUE ::::
    for (int i = 0; i < listaBaterias.size(); i++) {
        // El método isActivo() viene del Encapsulamiento de nuestra clase
        if (listaBaterias[i].isActivo()) {
            glm::mat4 modelBat = glm::mat4(1.0f);
            modelBat = glm::translate(modelBat, listaBaterias[i].getPosicion());
            // Las hacemos un poco más grandes para que el jugador las vea entre los árboles
            modelBat = glm::scale(modelBat, glm::vec3(4.0f));

            // Dibujamos el modelo 11 (que es tu Bateria.obj)
            if (models.size() > 11) models[11].Draw(*shader, modelBat);
        }
    }
}

void setMultipleLight(Shader* shader, vector<glm::vec3> pointLightPositions)
{
    shader->setVec3("viewPos", camera.Position);

    //LUZ DE LUNA
    shader->setVec3("dirLights[0].direction", glm::vec3(-0.2f, -1.0f, -0.3f));
    shader->setVec3("dirLights[0].ambient", 0.03f, 0.03f, 0.05f); // Azul muy, muy oscuro
    shader->setVec3("dirLights[0].diffuse", 0.02f, 0.02f, 0.03f); // Luz directa casi nula
    shader->setVec3("dirLights[0].specular", 0.0f, 0.0f, 0.0f);
    // :::: APAGADO ABSOLUTO DE LUCES EXTRAS ::::
    for (int i = 1; i < 4; i++) {
        string num = std::to_string(i);

        // Limpiar DirLights
        shader->setVec3("dirLights[" + num + "].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        shader->setVec3("dirLights[" + num + "].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("dirLights[" + num + "].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("dirLights[" + num + "].specular", 0.0f, 0.0f, 0.0f);

        // Limpiar PointLights
        shader->setVec3("pointLights[" + num + "].position", glm::vec3(0.0f, -10.0f, 0.0f));
        shader->setVec3("pointLights[" + num + "].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("pointLights[" + num + "].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("pointLights[" + num + "].specular", 0.0f, 0.0f, 0.0f);
        shader->setFloat("pointLights[" + num + "].constant", 1.0f);

        // Limpiar SpotLights
        shader->setVec3("spotLights[" + num + "].direction", glm::vec3(0.0f, -1.0f, 0.0f));
        shader->setVec3("spotLights[" + num + "].position", glm::vec3(0.0f, -10.0f, 0.0f));
        shader->setVec3("spotLights[" + num + "].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLights[" + num + "].diffuse", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLights[" + num + "].specular", 0.0f, 0.0f, 0.0f);
        shader->setFloat("spotLights[" + num + "].constant", 1.0f);
    }

    if (linternaEncendida)
    {
        glm::vec3 puntaLinterna = camera.Position + (camera.Right * 0.45f) + (camera.Up * -0.35f) + (camera.Front * 0.2f);

        shader->setVec3("pointLights[0].position", puntaLinterna);
        shader->setVec3("pointLights[0].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("pointLights[0].diffuse", 1.0f, 1.0f, 0.8f);
        shader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        shader->setFloat("pointLights[0].constant", 1.0f);
        shader->setFloat("pointLights[0].linear", 2.0f);
        shader->setFloat("pointLights[0].quadratic", 5.0f);
        shader->setVec3("spotLights[0].position", camera.Position + (camera.Right * 0.45f) + (camera.Up * -0.35f));
        shader->setVec3("spotLights[0].direction", camera.Front);
        shader->setVec3("spotLights[0].ambient", 0.0f, 0.0f, 0.0f);
        shader->setVec3("spotLights[0].diffuse", 3.0f, 3.0f, 2.5f);
        shader->setVec3("spotLights[0].specular", 1.0f, 1.0f, 1.0f);
        shader->setFloat("spotLights[0].constant", 1.0f);
        shader->setFloat("spotLights[0].linear", 0.02f);
        shader->setFloat("spotLights[0].quadratic", 0.001f);
        shader->setFloat("spotLights[0].cutOff", glm::cos(glm::radians(10.0f)));
        shader->setFloat("spotLights[0].outerCutOff", glm::cos(glm::radians(15.0f)));
    }
    else
    {
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
    detectColls(collboxes, &camera, renderCollBox, collidedObject_callback);
}