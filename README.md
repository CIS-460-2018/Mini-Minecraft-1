# Mini Minecraft: Mario Siblings

Features:
- A "player" controlled by first-person camera controls
- Usage of mouse cursor to determine player orientation
- WASD keys to move left/right/forward/backward and Space bar to jump
- 'F' key for fly mode, player is not affected by gravity in this mode
- Collision handle with the terrain: At the start, the player falls from the sky, then stops once he/she reaches the ground -> the free fall is implemented by using the gravitational acceleration and the collision is checked every time the timer is called

Strategies:
- In order to handle collision check, I decided to surround my player with a bounding box and called getBlockAt() method for each corner while doing volume casting. In my method, I use an incrementer that moves the player along the desired direction slowly and checks in each incrementation whether there is a collision.

Difficulties/Issues:
- My collision method handles the collision with the terrain, but does not handle any other collision (e.g. collision with a grass block). I will need to take more time on this to debug
- The translation along the right vector affects the position very little compared to the translation along the forward vector. They are much more balanced when the movements are not affected by velocity, acceleration


