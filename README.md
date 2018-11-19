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
