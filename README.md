# Mini Minecraft: Mario Siblings

Milestone 1:
1. Efficient Terrain Storage - Prateek Agarwal
Creating the Chunk Object
- Creating the Chunk object was fairly straightforward. I changed the shader program code to accept only one VBO including the position, normal and color for all vertices.
VBO data for hull of Chunk
- I decided to move the createPosNorCol() function from Chunk to Terrain, since I needed access to the QHash map for checking blocks across the intersection of two chunks. The updateScene() function is to be called every time the block structure is updated (block added/removed, new terrain generated). This way the VBOs are only getting populated when the terrain changes rather than at every update().
Storing Chunk in QHash
- Using the example code provided on Piazza for bit manipulations, it was simple to convert an x and z coordinate to a int64_t for the QHash.
2. Procedural Terrain Generation - Yamir Tainwala
- I used the provided randomization function to generate a fractal brownian motion function that served as y co-ordinate (height) noise parameter.
- For creating and destroying boxes I casted a ray and traced points along the ray in the direction of the look vector until I encountered a box that was not empty. From here, for removal, I simply removed the first non-empty box i encountered within a certain distance t. For the box addition, once I found the above box (the first box not empty within t and in the direction of the look vector), I simply calculated the distance of all boxes adjacent to its faces from the position vector, and created a box in the position that was closest to the position vector, provided the box is empty.
- I didn't see the purpose of checking all the boxes around a person when one could simply trace the ray until a box was found in the correct direction. So I chose to implement ray tracing in this much simpler way.
- I faced difficulty when trying to understand how to generate new terrain once an edge was reached - whether I should be generating a new terrain instance or simply extending the dimensions of my current terrain, and further, how to ensure that it was added in the correct direction based on my position. I was also unable to change the m_blocks array to accommodate more blocks and wasn't sure how to proceed.

3. Game Engine and Player Physics (Gizem Dal):

- A "player" controlled by first-person camera controls
- Usage of mouse cursor to determine player orientation
- WASD keys to move left/right/forward/backward and Space bar to jump
- 'F' key for fly mode, player is not affected by gravity in this mode
- Collision handle with the terrain: At the start, the player falls from the sky, then stops once he/she reaches the ground -> the free fall is implemented by using the gravitational acceleration and the collision is checked every time the timer is called

Strategies:
In order to handle collision check, I decided to surround my player with a bounding box and called getBlockAt() method for each corner while doing volume casting. In my method, I use an incrementer that moves the player along the desired direction slowly and checks in each incrementation whether there is a collision.

Difficulties/Issues:
My collision method handles the collision with the terrain, but does not handle any other collision (e.g. collision with a grass block). I will need to take more time on this to debug
The translation along the right vector affects the position very little compared to the translation along the forward vector. They are much more balanced when the movements are not affected by velocity, acceleration

Note: There were some merging issues so we submitted the branch link for the player physics part.

Update:
- All the bugs from the previous part are fixed and merged with master.

<br />
<br />
<br />
Milestone 2:

1. L-Systems - Yamir Tainwala
- I used the l-system header files provided in class to model my l-system
- Once I rendered the terrain, I calculated the river l-systems and cut into the terrain based on the path traced by the turtle
- I used depth as a parameter to determine the width for that arm of the river, and manually generated a slope from twice the width down to the edge of the river (at its width)
- I used a probability condition based on a random function to decide whether to branch or not, if yes, then the turtle state is saved and if not then the grammar pertaining to that branch is removed from the axiom using string matching
- I used a second QStack to save turtle state for each branch encountered, and once a close bracket is encountered, the last saved state is popped off and used as the starting point for the next section of the axiom's grammar
- I used orientation to represent positive and negative x and z directions and case matched orientation during a rotation of the turtle to get the new orientation
- Each change in state of the turtle is saved to the turtleHistory QStack, but only drawLine() (forward movement of the turtle) increments the depth of the turtle. By checking that I call a draw function only if the next turtle state is at an incremented depth, I ensure that arbitrary lines are not drawn from some future state to a previously saved state, and further that nothing is drawn except during a forward motion of the turtle.

Difficulties:
- I was unable to understand the design of function pointers and instead used if-else matching to check the character and execute the corresponding rule implemented as member functions of the l-system class
- It took me a significant amount of time to figure out how to implement branching logic. I finally decided to use a second QStack to save turtle state at each branch and to pop it off and set the this turtle to be the popped off turtle once the branch was completed.

2. Multithreading and Swimming - Prateek Agarwal
- I added a new glsl file to handle creating a blue or red filter over the screen
- Created a new Quadrangle class that makes a quadrangle with the correct color that spans the entire screen in NDC space
- Created a VBOThread class to handle creating VBOs for new chunks on different threads
- Adjusted updateScene to use VBOThread to generate VBOs for every chunk on a separate thread, decreasing the time needed to generate new terrain
- Created new functionality to only render chunks that are changed by placeBlock or removeBlock as well as only generating the VBOs for new chunks rather than the entire terrain.

Difficulties:
- Debugging glsl to realize that I didn't set my count variable in my Drawable Quadrangle
- I spent some time figuring out what exactly needed to be on a separate thread, and figured out the hard way how passing variables to the GPU (chunk->create()) had to be done on the main thread.

3. Texturing and Texture Animation by Gizem Dal:

Implemented all of the requirements as listed on the course page:
- A way to load images as textures into OpenGL for your world.
- An addition to lambert.frag.glsl that makes use of a sampler2D to apply texture colors to a surface.
- An alteration to lambert.frag.glsl to apply a Blinn-Phong specular reflection model with a cosine power that varies depending on block type.
- An alteration to lambert.frag.glsl so that it includes a time variable as in Homework 5's OpenGL Fun, and uses this variable to animate the UVs on a LAVA block and WATER block. You may use a branching statement (i.e. IF statement) in your shader so that only LAVA and WATER are animated.
- Enabling alpha blending in MyGL::initializeGL so that transparency can be applied to WATER blocks.
- An additional interleaved VBO in the Chunk class in which you will store all non-opaque blocks (this is to handle how the OpenGL pipeline deals with transparency sorting).
- An alteration to the VBO for the Chunk class, so that it additionally supports UV coordinates (vec2) and cosine power (float). You may also consider adding some sort of "animateable" flag to your VBO so that only triangles with the LAVA and WATER textures are animated in your shader.
- Additional variables in the ShaderProgram class to support these texture features.

Strategies:
- I decided to have a helper method named getTexture in terrain.cpp, which allowed me to know what texture sample I should be using depending on the block type and face (since we need different texture samples for the top of a grass block versus the sides of a grass block, etc.). I used this information to get the top left corner of the texture sample in the createVertexPosNorUV method in terrain.cpp (which fills up the interleaved VBO’s for opaque and transparent blocks for chunks) and then got the other corner UV’s by simply adding/subtracting 1.0f / 16.0f from the x and y UV coordinates.
- As said in the instructions, I created two separate VBO’s for chunks: one for opaque and one for transparent blocks. To handle their drawing separately, I created separate generate, bind and draw methods as well as separate index counters for opaque and transparent blocks in the shaderProgram and Drawable.
- In order to animate the LAVA and WATER blocks, I used a uniform u_Time variable in the fragment shader. The way I implemented the animation is I keep incrementing the UV coordinates of the current pixel by one pixel (1.0f / 256.0f) along fs_UV.x to give the feeling of flow between the WATER and LAVA textures, and handle moving to the textures in the other row by bound check (whether the current fs_UV.x has exceeded 1)

Troubles:
- I’m not 100% sure if I implemented the Blinn Phong cosine power effect properly in my fragment shader since I’m not sure if I’m getting the desired shading effect. I’m using the formula from the lecture slides where I find the vector H (the mid vector between the view and light vectors) and dot it with the normal vector, but I might ask this to a TA at office hours just to make sure
- I’m not sure if the transparency outcome is the desired one: I’m able to see the opaque blocks behind the transparent ones but I also think that some transparent blocks are not only containing the faces of neighboring opaque blocks, but also the faces of opaque blocks that are way under the transparent block I’m looking at. I will probably need to do some debugging on this
- The animation of textures is slightly off at the very last transition. The way I increment fs_UV.x might be exceeding the bounds at the very end for a brief second, I will debug this after the presentation

Milestone 3:
1. Greyscale Image as a Height Map - Prateek Agarwal
- Loads a QFileDialog when the 'G' key is pressed
- Resizes the image to a size that doesn't expand the terrain too much
- using the greyscale value to set the height of all blocks in a certain area
Challenges:
- Sampling pixels in increments so that every pixel need not be visited (because images come in varying sizes, some more than 1000 pixels wide, which would be too many blocks). 
- when adding an image at the edge of the map, I had to make sure that new chunks were completely formed and not only filled where the height of the map was defined by the input picture. To do this, I first created fbm based chunks and then set the height based on the picture, so there would be no part of new chunks left as "holes".
2. Color Image as a Height Map - Prateek Agarwal
- sets the height of an area of the terrain as greyscale height mapping does by extracting the greyscale value of the sampled pixel.
- Used an eye dropper tool to sample the average color of the colored blocks given in the original texture file. 
- Added blocktypes for the 11 color blocks provided
- Using the color of each sampled pixel, I found the colored block closest in 3D distance to the sampled pixel color and assigned that block as the top block of the cell. 
Challenges:
- At first, I thought I would define RGB ranges in which to assign each of the 11 given color blocks. However, I thought it would be simpler to think about the problem as a 3D voroni diagram. Switching to this approach made the code simpler to write and execute.
3. Biomes - Prateek Agarwal
- Split the terrain into 128 x 128 areas where a random point is sampled and a random biome is assigned. 
- Based on distance to adjacent points, I create a voroni diagram that splits the entire terrain into different biomes and assigns the top block accordingly.
- I use the distance to neighboring biomes to weight the chance that the top block is of the nieghboring biome rather than the current biome. The chance is 50% when at a point equally close to the original biome and a neighboring biome (i.e: on the "edge"). 
Challenges:
- Smoothing between biomes proved challenging. I tried using the difference between the distance to the assigned biome and an adjacent biome, tried using different functions to weight the chance of assigining an adjacent biome, such as a simple cosine function and the glm::smoothstep() function. 
- I tried changing the heights based on the biome. When adding new chunks, you can see the attempt to change the height so that MOUNTAIN biomes are steeper than DESERT biomes. I still need to work on making the height transitions smooth though, because it is proving a bit difficult. 
I also added arrow keys as a way of panning the camera, in case that better suits your preferences.
