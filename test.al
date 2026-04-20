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
        __spawn__ = make_point(4,5);
    };
    turn : {
       move(right);
    };
};