world traffic;

let square_walker : car {
    init : {
        let a : int = 0;
        __spawn__ = make_point(3,7);
    };
    turn : {
        if ((a % 4) == 0) {
            move(down);
        } else if ((a%4) == 1) {
            move(right);
        } else if ((a%4) == 2) {
            move(up);
        } else {
            move(left);
        }
        a = a + 1;
    };
};

let right_walker : car {
    init : {
        __spawn__ = make_point(1, 1);
	__destination__ = make_point(10, 10);
    };
    turn : {
    	let x : int = get_x(__position__);
	let y : int = get_y(__position__);
	let d_x : int = get_x(__destination__);
	let d_y : int = get_y(__destination__);

	if (x < d_x) move(right);
	else if (x > d_y) move(left);
	else if (y < d_y) move(up);
	else if (y > d_y) move(down);
    };
};