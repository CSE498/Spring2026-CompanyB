world traffic;

let square_walker : student {
    init : {
        let a : int = 0;
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

let right_walker : student {
    init : {
    };
    turn : {
       move(right);
    };
};