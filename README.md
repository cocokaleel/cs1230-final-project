# Final Project
* CSCI 1230 (Intro to Computer Graphics) 
* Team Alumninum Foil

## Idea
Our original idea is to have a shiny sphereical ball (such as a marble) manuver through a complex procedural terrain with a physics simulation. However, due to the complexities of the object detection/collision algorithm, we decided to implement a fixed camera path around the trimesh.

There are three components to our project: 1) a height map based on drawn and input images, 2) a realtime raytracer for reflections and shadows, and 3) a fixed camera path using Bezier curves.

### Height Map (Procedural Texturing)
We are able to implement an 100 x 100 resolution height map from either input images or input drawn pictures. The GUI allows users to upload their own image and also clear both the canvas and height map. The height and terrain is determined by the value of the pixel when covnerted to grayscale.

### Camera Path
Additionally, we are also able to move the camera along a defined path by Bezier curves. Once the camera position reaches a certain height, it will change direction to another Bezier curve path from the function. Eventually, the path can be defined with specific control points that are used to approximate the tangents of the other end points.

### Realtime (GPU) Raytracer
Furthermore, the GPU raytracer uses a fullscreen quad (2 triangles that cover the full screen) that pass as a VAO into a vertex shader, which only passes the info immediately into a fragment shader for each uv coordinate pair representing a pixel in the screen. For each uv coordinate, we "shoot" a ray from the digital camera position through this coordinate point and trace it (as of right now) through the list of object primitives (currently only spheres) defined by their multivariable representations (not polygonal meshse) to find intersections. We then use Phong lighting and shadows to determine the color of that pixel, and trace the reflected ray to iterate this process 3 times (GLSL doesn't support recursion). Thus, we are tracing all pixels. Also, lighting doesn't include an exception for a point-light existing in the middle of the scene.

After this geometry rendering process, we have implemented 3 filtering functions: luma grayscale (per pixel), inverse (per pixel), and a 5-wide box blur. Instead of the geometry and Phong rendering going directly to the screen, they are rendered to a framebuffer that stores depth and stencil information in a renderbuffer and color in a texture. After the phong lighting, we pass the texture through the filter shaders onto a fullscreen quad, which then is rendered to the screen (default framebuffer).

### Object Detection/Collision
For future development, we can maneuver spherical objects around our complex mesh. By implementing the GJK Algorithm, we can abstract detecting collisions from any convex shapes by generating “simplexes.” By calculating the Minkowski difference between vertices, the objects have collided if the resulting simplex contains the origin. The simplex will be built based on different dimensions whether our current object hits a vertex, edge, or plane of the other object. Then, we can apply the EPA Algorithm for collision responses between different objects forces.
