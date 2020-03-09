# raytracer

Currently looking at photon mapping, then progressive photon mapping, then stochastic progressive photon mapping
To know photon mapping, going through literature from scratch

Need to learn now:
monte carlo ray tracing
the rendering equation - kajiya
backwards ray tracing - arvo

Monte carlo:
theory - http://old.cescg.org/CESCG97/csebfalvi/
implementation - https://github.com/kbladin/Monte_Carlo_Ray_Tracer
a review of monte carlo, and other ray tracing methods: https://eriksvjansson.net/papers/mcrt.pdf

notes in onenote or on paper

resources:
http://www.gabrielgambetta.com/computer-graphics-from-scratch/introduction.html

https://www.scratchapixel.com/

some side paper?
http://gamma.cs.unc.edu/RS/paper_rt07.pdf


use as a base
https://github.com/ssloy/tinyrenderer/wiki/Lesson-6:-Shaders-for-the-software-renderer

useful for denoising
file:///C:/Users/damckinn/Downloads/A_Survey_of_Blue-Noise_Sampling_and_Its_Applicatio.pdf

Illumination maps from backwards ray tracing: James Arvo, https://pdfs.semanticscholar.org/c088/399ad6dc648647e0e19e01b5485925957681.pdf
The illumination map is a 1x1 square that maps to the surface of each diffusely-reflective object via some defined function (eg. polar coordinates for a sphere). When a ray hits a diffuse object, the energy contribution is bilinearly partitioned between the four surrounding data points in the illumination map array and adds to their values (each ray is allocated some amount of energy). Specularly reflective surfaces only reflect, and I guess those in between do a bit of both. Light rays have to be distributed at a really high density, and even more so towards shapes with caustic behaviour. 
After the illumination stage, the energy value at each point in the array is divided by the area it encompasses in shape coordinates (formula in paper). This gives the point intensity. This essentially is photon density. Then when the camera rays hit diffuse objects, the value they take away is bilinearly interpolated from this array. 
