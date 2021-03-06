// Include GLFW
#include </home/nclarke/Desktop/deps/glfw/include/GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include </home/nclarke/Desktop/deps/glm/include/glm/glm.hpp>
#include </home/nclarke/Desktop/deps/glm/include/glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;


glm::mat4 getProjectionMatrix(){
        return ProjectionMatrix;
}


// Initial position : on +Z
glm::vec3 position = glm::vec3( 15, 0, 0 );
// Initial horizontal angle : toward -Z
float horizontalAngle = 8.14f;
// Initial vertical angle : none
float verticalAngle = 3.0f;
// Initial Field of View
float initialFoV = 45.0f;
glm::vec3 direction = glm::vec3(0.0,0.0,0.0);

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.0005f;

glm::mat4 getViewMatrix(){
//   printf("position: (%lf, %lf, %lf)\n", position.x, position.y, position.z);
//   printf("direction: (%lf, %lf, %lf)\n", direction.x, direction.y, direction.z);
//   printf("horizontalAngle: (%lf)\n", horizontalAngle);
//   printf("verticalAngle: (%lf)\n", verticalAngle);
        return ViewMatrix;
}

void computeMatricesFromInputs(){

        // glfwGetTime is called only once, the first time this function is called
        static double lastTime = glfwGetTime();

        // Compute time difference between current and last frame
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);

        // Get mouse position
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Reset mouse position
        glfwSetCursorPos(window, 1024/2, 768/2);

        // new Orientation
        horizontalAngle += mouseSpeed * float(1024/2 - xpos );
        verticalAngle   += mouseSpeed * float( 768/2 - ypos );

        //Spherical coordinates to Cartesian coordinates conversion
        direction = vec3(cos(verticalAngle) * sin(horizontalAngle), sin(verticalAngle),cos(verticalAngle) * cos(horizontalAngle));
       
        // Right vector
        glm::vec3 right = 
         glm::vec3(sin(horizontalAngle - 3.14f/2.0f),0,cos(horizontalAngle - 3.14f/2.0f));
       
        // Up vector
        glm::vec3 up = glm::cross( right, direction );

        // forward really really fast
        if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
                position += direction * deltaTime * speed;
        }
        // backward really really fast
        if (glfwGetKey( window, GLFW_KEY_8 ) == GLFW_PRESS){
                position -= direction * deltaTime * 10.0f * speed;
        }
        // forward really really fast
        if (glfwGetKey( window, GLFW_KEY_9 ) == GLFW_PRESS){
                position += direction * deltaTime * 10.0f * speed;
        }
        // backward
        if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
                position -= direction * deltaTime * speed;
        }
        // right
        if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
                position += right * deltaTime * speed;
        }
        // left
        if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
                position -= right * deltaTime * speed;
        }

        float FoV = initialFoV;// - 5 * glfwGetMouseWheel();

        ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
        // Camera matrix
        ViewMatrix = glm::lookAt(position, position+direction,up);
        lastTime = currentTime;
}






