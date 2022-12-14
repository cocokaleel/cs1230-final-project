#version 330 core
//Adapted from lab 10

//RAYTRACING FUNCTIONS AND STRUCTS
struct Ray {
    vec4 startPosition;
    vec4 direction;
};
Ray getOriginalRayWorldSpace();
struct IntersectionResult {
    int triangleIndex;
    vec4 intersectionPositionWorld;
    vec4 intersectionNormalWorld;
};
IntersectionResult traceRay(Ray ray);
vec4 phongColor (IntersectionResult intersection);
Ray getReflectedRay (IntersectionResult intersection, Ray originalRay);
bool shadowRayClear(IntersectionResult shapeIntersection, int lightIndex);

//GLOBAL SCENE DATA: SHAPES, LIGHTS, AND CONSTANTS
// Camera info
uniform mat4 viewMatrix;
uniform mat4 viewMatrixInverse;

// Declare "in" variables for the world-space position and normal,
//         received post-interpolation from the vertex shader

// UV coordinate in variable representing a pixel in the full-screen quad
in vec2 uvCoords;
out vec4 fragColor;// output color, which goes into the openGL clipping situation

uniform vec4 cameraPositionWorld;
uniform float heightAngle;
uniform float widthAngle;

// Global lighting uniforms
uniform float ka;
uniform float kd;
uniform float ks;


// holds the number of lights that exist in the scene (for looping purposes)
uniform int numLights;
//create a struct for holding light information
struct LightColorPos {
    vec4 color;
    vec4 dir; //only for directional light
    vec4 pos; //only for point light and spot light
    int lightType; //0 is directional, 1 is point, 2 is for spot
    vec3 attenuation;
    float angle; //only for spot lights
    float penumbra; //only for spot lights
};
//create a uniform input for the light data
uniform LightColorPos lights[8];

struct TriangleData {
    vec3[3] points;
    vec3[3] normals;
    vec4 cAmbient;
    vec4 cDiffuse;
    vec4 cSpecular;
    float shininess;
    vec4 cReflective;
};
uniform TriangleData triangles[50];
uniform int numTriangles;
uniform sampler1D triangle_sampler;


//This main function raytraces a ray through the UV coordinate within a triangle on a full-screen quad. It raytraces only a single ray.
void main() {
    //Initialize fragColor
    fragColor = vec4(vec3(0.f),1.f);
//    fragColor = texture(triangle_sampler, vec2(2000,0));
//    return;
    if (texelFetch(triangle_sampler, 2000, 0).xyz.x == 0.f) {
        fragColor = vec4(1.f);
        return;
    }/*
    if (texture(triangle_sampler, vec2(2000,0)).r==0.f) {
        fragColor = vec4(1.f);
        return;
    }*/

    //Find ray direction with UVK calculation in camera space and then put it in worldspace
    Ray ray1 = getOriginalRayWorldSpace();
    IntersectionResult intersection = traceRay(ray1);

    //There is no recursion in GLSL,
    //Thus I had to do this iteratively :(
    if (intersection.triangleIndex != -1) {

        //If the ray hits something, get its Phong color
        fragColor = phongColor(intersection);
        Ray reflectedRay;

        for (int i = 0; i < 3; i++) {
            //bounce ray and trace it
            reflectedRay = getReflectedRay(intersection, ray1);
            IntersectionResult intersection2 = traceRay(reflectedRay);
            if (intersection.triangleIndex != -1) {
                //if bounced ray hits, add its color contribution to the shape
                vec4 shape2Color = phongColor(intersection2);
                fragColor += ks*triangles[intersection.triangleIndex].cReflective*shape2Color;

                intersection = intersection2;
                ray1 = reflectedRay;
            } else {
                //if bounced ray doesn't hit, stop reflecting it
                break;
            }
        }
    }
}

//generate the original ray, shot from the camera through the uvCoordinate on the full-screen quad
//this ray is in camera space
Ray getOriginalRayWorldSpace() {
    float k = 1.f;

    //Shift X and Y by -0.5 to be in the range [-0.5, 0.5]^2
    float x = uvCoords[0]-0.5;
    float y = uvCoords[1]-0.5;

    //use trigonometry to find the 3d space coordinate of where the ray goes into
    float U = 2*k*tan(widthAngle/2.f);
    float V = 2*k*tan(heightAngle/2.f);

    vec4 uvk = vec4(U*x, V*y, -1.f*k, 1.f); //use 1 for k value, this is arbitrary

    vec4 eye = vec4(0.f,0.f,0.f, 1.f);

    vec4 direction;
    direction = uvk - eye;
    //NOTE: QTCreator might show this instantiation patter as an error. It should work. Freaking QTCreator...
    Ray rayCameraSpace = Ray(eye,direction); //is it a problem that direction is a vec4 instead of a vec3
    Ray rayWorldSpace = Ray(viewMatrixInverse*rayCameraSpace.startPosition, normalize(viewMatrixInverse*rayCameraSpace.direction));
    return rayWorldSpace;
}

bool shadowRayClear(IntersectionResult shapeIntersection, int lightIndex) {
    vec4 rayDirection;
    vec4 rayPosition;
    //0 is directional, 1 is point, 2 is for spot
    if (lights[lightIndex].lightType == 0) {
        //if the light is directional, the ray direction is just the opposite direction
        //of the light direction
        rayDirection = vec4(-1*lights[lightIndex].dir);
    } else {
        //if the light is spot or point, the ray direction is the direction from the intersection
        //point to the position of the light
        rayDirection = vec4(lights[lightIndex].pos - shapeIntersection.intersectionPositionWorld);
    }

    rayPosition = vec4(0.05*rayDirection+shapeIntersection.intersectionPositionWorld);
    Ray shadowRay = Ray(rayPosition, rayDirection);

    IntersectionResult shadowRayIntersection = traceRay(shadowRay);

    //if the ray doesn't intersect any shapes, the triangleIndex of the intersection result will be -1
    return shadowRayIntersection.triangleIndex == -1;
}

//find the FIRST intersected shape (primitives as of right now) that the ray passes through and return the
//position of intersection, normal, and which shape (in the form of the shape's index) was intersected
IntersectionResult traceRay(Ray rayWorldSpace) {
    float t = 1. / 0.; //infinity
    int triangleIndex = -1;
    vec4 intersectionWorld;
    vec4 normalWorld;

    //loop through triangles
    for (int i = 0; i < numTriangles; i++) {
        //Formula inspiration from https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution
        //intersect
        vec3 edge01 = vec3(triangles[i].points[1]-triangles[i].points[0]);
        vec3 edge02 = vec3(triangles[i].points[2]-triangles[i].points[0]);
        vec3 planeNormal = cross(edge01, edge02);
        //check if ray and triangle are parallel so no impossible shenanigans happens
        float angleNormalAndDirection = dot(planeNormal, vec3(rayWorldSpace.direction));

        //if the angle between the two vectors is approximately zero, there is
        //no intersection and this triangle should be skipped
        if (abs(angleNormalAndDirection)<0.01) continue;


        //find the hit of the ray on the plane defined by the vertices of the triangle

        float d = -dot(planeNormal,triangles[i].points[0]); //d is a parameter of the parametric definition of a plane
        float t_i = -(dot(planeNormal, vec3(rayWorldSpace.startPosition))+d)/angleNormalAndDirection;

        if (t_i < 0 || t_i > t) continue; //if the intersection is behind

        //check if point of intersection is between vertices

        vec3 intersectionPoint = vec3(rayWorldSpace.startPosition + t_i * rayWorldSpace.direction);
        //check edge 0-1
        vec3 v0ToIntersectionPoint = vec3(intersectionPoint - triangles[i].points[0]);
        //check that the direction to p is in between the edge by making sure it's cross product is opposite the normal
        vec3 cross0 = cross(edge01,v0ToIntersectionPoint);
//        if (dot(planeNormal, cross0)<0) continue;
        //check edge 1-2
        vec3 edge12 = vec3(triangles[i].points[2]-triangles[i].points[1]);
        vec3 v1ToIntersectionPoint = vec3(intersectionPoint - triangles[i].points[1]);
        //check that the direction to p is in between the edge by making sure it's cross product is opposite the normal
        vec3 cross1 = cross(edge12,v1ToIntersectionPoint);
//        if (dot(planeNormal, cross1)<0) continue;
        //check edge 2-0
        vec3 edge20 = vec3(triangles[i].points[0]-triangles[i].points[2]);
        vec3 v2ToIntersectionPoint = vec3(intersectionPoint - triangles[i].points[2]);
        //check that the direction to p is in between the edge by making sure it's cross product is opposite the normal
        vec3 cross2 = cross(edge20,v2ToIntersectionPoint);
//        if (dot(planeNormal, cross2)<0) continue;

        //now that the point has passed all the tests, it should in theory be in the triangle
        if (dot(planeNormal, cross0)>=0 && dot(planeNormal, cross1)>=0 && dot(planeNormal, cross2)>=0) {
            t = t_i;
            triangleIndex = i;
            intersectionWorld = vec4(intersectionPoint, 1.f);
            normalWorld = vec4(planeNormal, 0.f);
        }

    }

    //If no shape intersection was found
    if (triangleIndex == -1) {
        //return a bad IntersectionResult
        return IntersectionResult(-1, vec4(1.f), vec4(1.f));
    }

    return IntersectionResult(triangleIndex, intersectionWorld, normalWorld);
}

//Get the color of the shape at the intersected point
vec4 phongColor (IntersectionResult intersection) {
    if (intersection.triangleIndex == -1) {
        return vec4(0.f, 0.f, 0.f, 1.f);
    }
    vec4 color = vec4(0.f, 0.f, 0.f, 1.f);
    vec3 normal = vec3(normalize(intersection.intersectionNormalWorld));
    color += ka * triangles[intersection.triangleIndex].cAmbient;

    for (int i = 0; i < numLights; i++) {
        if (!shadowRayClear(intersection, i)) {
            continue;//don't add any contribution from lights because the light source is blocked
        }
        if (lights[i].lightType == 0) {
                //DIRECTIONAL LIGHTS
                // Add diffuse component to output color
               float Id = kd * dot(normalize(normal), vec3(normalize(-lights[i].dir))); //only for directional lights

               //ADD FINAL DIFFUSE COMPONENT
               color += vec4(vec3(max(Id,0)), 0.f) * triangles[intersection.triangleIndex].cDiffuse * lights[i].color;

               // Add specular component to output color
//               vec4 pos4 = vec4(positionWorld, 1.f);

               vec4 reflected = reflect(-lights[i].dir, vec4(normal, 0.f));
               vec4 directionToCamera = cameraPositionWorld - intersection.intersectionPositionWorld;

               float dotRE = dot(normalize(reflected), normalize(directionToCamera));
               float spec = dotRE < 0 ? ks*pow(dotRE, triangles[intersection.triangleIndex].shininess) : 0;

               //ADD FINAL SPECULAR COMPONENT
               color += vec4(vec3(spec), 0.f)*lights[i].color * triangles[intersection.triangleIndex].cSpecular;

        } else if (lights[i].lightType == 1) {
               //POINT LIGHTS
               vec3 lightDirection = vec3(intersection.intersectionPositionWorld) - vec3(lights[i].pos);
               float lightDistance = distance(vec3(intersection.intersectionPositionWorld), vec3(lights[i].pos));
               float lightComponent = dot(vec3(normal), -normalize(lightDirection));

               //compute attenuation function
               float attenuationFactor = 1.f/(lights[i].attenuation[0] + lights[i].attenuation[1]*lightDistance + lights[i].attenuation[2]*lightDistance*lightDistance);

               //compute and add diffuse component
               float diffuseComponent = attenuationFactor*lightComponent*kd;
               color += vec4(vec3(max(diffuseComponent,0)), 0.f) * triangles[intersection.triangleIndex].cDiffuse * lights[i].color;

               //compute and add specular component
               vec3 reflectedLight = reflect(-normalize(lightDirection), vec3(normal));
               vec3 directionToCamera = normalize(vec3(cameraPositionWorld)-vec3(intersection.intersectionPositionWorld));
               float specularComponent = attenuationFactor * ks * pow(dot(reflectedLight, directionToCamera), triangles[intersection.triangleIndex].shininess);

               //if the light and the normal are in the same direction
               fragColor += dot(-lightDirection, vec3(normal)) > 0 ?vec4(vec3(max(specularComponent,0)), 0.f) * triangles[intersection.triangleIndex].cSpecular * lights[i].color : vec4(0);

        } else {
                //SPOT LIGHTS

                //calculate the falloff of the light intensity
                vec4 lightColor;
                vec3 lightDirection = vec3(intersection.intersectionPositionWorld) - vec3(lights[i].pos);
                float angleOfLight = acos(dot(normalize(lightDirection), vec3(normalize(lights[i].dir))));
                float outerTheta = lights[i].angle;
                float innerTheta = lights[i].angle-lights[i].penumbra;
                if (angleOfLight <= innerTheta) {
                    lightColor = lights[i].color;
                } else if (angleOfLight <=outerTheta) {
                    float division = (angleOfLight-innerTheta)/(outerTheta-innerTheta);
                    float falloff = -2.f*pow(division, 3.f) + 3.f*pow(division, 2.f);
                    lightColor = lights[i].color*(1.f-falloff);
                } else {
                    lightColor = vec4(0,0,0,1);
                }



                float lightDistance = distance(vec3(intersection.intersectionPositionWorld), vec3(lights[i].pos));
                float lightComponent = dot(vec3(normal), -normalize(lightDirection));

                //compute attenuation function
                float attenuationFactor = 1.f/(lights[i].attenuation[0] + lights[i].attenuation[1]*lightDistance + lights[i].attenuation[2]*lightDistance*lightDistance);

                //compute and add diffuse component
                float diffuseComponent = attenuationFactor*lightComponent*kd;
                color += vec4(vec3(max(diffuseComponent,0)), 0.f) * triangles[intersection.triangleIndex].cDiffuse * lightColor;

                //compute and add specular component
                vec3 reflectedLight = reflect(-normalize(lightDirection), vec3(normal));
                vec3 directionToCamera = normalize(vec3(cameraPositionWorld)-vec3(intersection.intersectionPositionWorld));
                float specularComponent = attenuationFactor * ks * pow(dot(reflectedLight, directionToCamera), triangles[intersection.triangleIndex].shininess);

                //if the light and the normal are in the same direction
                color += dot(-lightDirection, vec3(normal)) > 0 ?vec4(vec3(max(specularComponent,0)), 0.f) * triangles[intersection.triangleIndex].cSpecular * lightColor : vec4(0);

            }
        }

    return color;
}

//Get the reflected ray at the point of intersection so that traceRay can be run again iteratively
Ray getReflectedRay (IntersectionResult intersection, Ray originalRay) {
    //TODO replace
    vec4 normal = vec4(normalize(intersection.intersectionNormalWorld));
    vec4 origRayDirection = vec4(normalize(originalRay.direction));
    vec4 reflectedDirection = vec4(normalize(reflect(origRayDirection, normal)));

    Ray outgoingRay = Ray(vec4(intersection.intersectionPositionWorld+0.01f*reflectedDirection), reflectedDirection);
    //NOTE: QTCreator might show this instantiation patter as an error. It should work. Freaking QTCreator...
    return outgoingRay;
}
