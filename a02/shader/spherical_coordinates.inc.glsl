
uniform bool cylindricMapping;
const float PI = 3.1415927;

vec2 SphericalCoordinates(vec3 r)
{
    // Note, coordinate system is (mathematical [x,y,z]) => (here: [x,-z,y])
    vec2 lookup_coord;
    // TODO: a) transform reflection vector into polar texture coordinates
    float theta = atan(r.z, r.x); // -PI, PI
    float u = (theta + PI) / (2*PI);
    float v;
    if (cylindricMapping){
        v = (r.y + 1.0) * 0.5;
    } else {
        float phi = acos(-r.y); // 0, PI
        v = (phi / PI);
    }
    lookup_coord = vec2(u, v);
    return lookup_coord;
}