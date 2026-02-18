# WeightedSet
Hi, my name is Luke Bridges, and the class I worked on for group 19 is called WeightedSet. As the name implies, it’s a data structure with some set-like operations--you can add elements, remove them, and check whether an element is in the set--but the elements also have double values associated with them, called “weights”, and you can randomly choose an element from the set with a probability proportional to its weight. So, say you add 3 elements, with weights 1, 2, and 3 respectively. Then the first has a 1/6 probability of being chosen, the second has 1/3, and the third has 1/2. We get those by dividing the weight of each element by the total weight of the elements in the set, which is 6. 

I have a little demo program, partly modified from one of my tests, which shows off some of this. We start out by adding 1, 2, and 3, with those weights I just mentioned. Then we draw a bunch of samples with the GetRandomElement function, see how many times each element comes up, and print the proportion of each vs. the probability we should get. Then we remove 3 and do the same thing. You can see in the output for the first case that we get 1/6, 1/3, 1/2, as expected. Then after removing 3, with weight 3, the total weight is 3. So now 1, with weight 1, should have a 1/3 probability of being chosen, and 2 should have 2/3, which is what we get.

# StateGridPosition
Hi, I’m Milo Baran and I worked on the StateGridPosition class. This class tracks position and orientation of a single agent in a grid, specifically the StateGrid class. StateGridPosition does a few important things: stores the agent's x and y coordinates, stores the direction the agent is facing using a Direction enum, and provides methods for agent movement, distance calculation and position comparisons. 

Now I’ll walk through some of the key methods. First I’ll go over the movement methods. MoveForward and MoveBackward change the agent's position. Using StateGrid’s inBounds method they check if a move is allowed. True or false is returned to indicate whether the move was done or not. Their implementation is in the corresponding cpp file.

The TurnLeft and TurnRight methods rotate the agents 90 degrees, while the TurnAround method rotate the agent 180 degrees. They use modular arithmetic on the Direction enum values to properly accomplish these rotations.

I’ll talk about EuclideanDistance. This simply computes the euclidean distance between one position and another.

I added getters for direction and positions. And then a setter for direction.
Finally, there's comparison functionality. SameCell checks if two StateGridPosition objects are in the same cell. Also I set up the spaceship operator to allow for any type of comparison for StateGridPosition objects. 

That’s it for the StateGridPosition class. 
