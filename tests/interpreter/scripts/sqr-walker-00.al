// Square walker for infection world
world infection;

// Define square walker
let square_walker : student {
    init : {
    	 let self.step_idx : int = 0;
    };
    turn : {
    	 // Declare but don't define the next direction
    	 let next_dir : direction;

	 // Select the next direction dependent on step_idx % 4
    	 if (self.step_idx % 4 == 0)
	    next_dir = right;
	 else if (self.step_idx % 4 == 1)
	    next_dir = up;
	 else if (self.step_idx % 4 == 2)
	    next_dir = left;
	 else if (self.step_idx % 4 == 3)
	    next_dir = right;

	 // Make the move decided on
	 move(next_dir);
    };
};
